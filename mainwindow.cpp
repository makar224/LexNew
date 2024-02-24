#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

#include <fstream>
using namespace std;

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	QCoreApplication::setOrganizationName("MyCompany");
	QCoreApplication::setApplicationName("LexNew");

	/*trItemsL.append(new TranslationItem(tr("to interfere with"), tr("мешать кому-л, чему-л")));
	trItemsL.append(new TranslationItem(tr("to rely on(upon)"), tr("полагаться на")));
	trItemsL.append(new TranslationItem(tr("to insist on"), tr("настаивать на")));
	trItemsL.append(new TranslationItem(tr("to arrive at"), tr("прибывать в"), true, true));
	trItemsL.append(new TranslationItem(tr("to open with"), tr("открывать чем-н."), true, false));*/

	ui->setupUi(this);

	moveTranslationsDialog = new MoveTranslationsDialog;
	connect(ui->moveTranslationsButton, &QPushButton::clicked,
			moveTranslationsDialog, &QDialog::open);

	dictEditDialog = new DictionaryEditDialog;
	dictEditDialog->installEventFilter(this);
	connect(dictEditDialog, SIGNAL(addTranslationSig(const TranslationItem *)),
			this, SLOT(addTranslation(const TranslationItem *)));
	connect(dictEditDialog, SIGNAL(removeTranslationSig(const TranslationItem *)),
			this, SLOT(removeTranslation(const TranslationItem *)));
	connect(dictEditDialog, SIGNAL(editTranslationSig(const TranslationItem *)),
			this, SLOT(editTranslation(const TranslationItem *)));
	connect(ui->dictionaryNewAction, &QAction::triggered,
			this, &MainWindow::newDictionary);
	connect(ui->dictionaryOpenAction, &QAction::triggered,
			this, &MainWindow::openFile);
	connect(ui->editDictionaryAction, &QAction::triggered,
			dictEditDialog, &QDialog::open);
	connect(ui->dictionarySaveAction, &QAction::triggered,
			this, &MainWindow::saveFile);
	connect(ui->dictionarySaveAsAction, &QAction::triggered,
			this, &MainWindow::saveFileAs);
	connect(ui->exitAction, &QAction::triggered,
			this, &MainWindow::applicationQuit);
	//connect(ui->exitAction, &QAction::triggered,
	//		qApp, &QCoreApplication::quit);
	connect(this, &MainWindow::quitApplication,
			qApp, &QCoreApplication::quit);

	sessionDialog = new TranslationDialog(nullptr, &trItemsL);
	sessionDialog->installEventFilter(this);
	connect(sessionDialog, &TranslationDialog::excludeTranslation,
			moveTranslationsDialog, &MoveTranslationsDialog::excludeTranslation);

	// Загружаем настройки приложения
	// Загружаем позицию окна из settings
	QSettings settings;
	settings.beginGroup("MainWindow");
	const auto geometry = settings.value("geometry", QByteArray()).toByteArray();
	if (geometry.isEmpty()) {
		QRect screenRect = QGuiApplication::primaryScreen()->geometry();
		setGeometry( (screenRect.width()-width())/2, (screenRect.height()-height())/2, width(), height());
	}
	else
		restoreGeometry(geometry);
	settings.endGroup();

	mDictionaryFilePath = settings.value("dictionaryFilePath", "").toString();
	// Устанавливаем в диалоге переводов исходные значения
	restoreDefaultTranslationSettings(); // значения по умолчанию - в контролы (для первой загрузки приложения)
	settings.beginGroup("Translations");
	ui->sessionIntervalSpinBox->setValue(settings.value("sessionInverval", ui->sessionIntervalSpinBox->value()).toInt());
	ui->successTriesSpinBox->setValue(settings.value("successesForExclusion", ui->successTriesSpinBox->value()).toInt());
	ui->alternativesSpinBox->setValue(settings.value("alternativesNumber", ui->alternativesSpinBox->value()).toInt());
	ui->triesSpinBox->setValue(settings.value("triesNumber", ui->triesSpinBox->value()).toInt());
	settings.endGroup();
	applyTranslationSettings();

	// Загружаем данные из файлов
	/*// загружаем временный файл заучивания (если он есть)
	if (! loadTempFile()) {
		ifstream is("default.txt", ios::in);
		if (is) // файл открыт для чтения, значит он сущетсвует
			QMessageBox::warning(nullptr, tr("Загрузка приложения"),
								 tr("Не удалось загрузить данные о заучивании."));
		if (!mDictionaryFilePath.isEmpty()) {
			loadFile(mDictionaryFilePath);
		}
	}*/
	if (!mDictionaryFilePath.isEmpty()) {
		if (! loadDictionary(mDictionaryFilePath)) {
			QMessageBox::critical(nullptr, tr("Application loading"),
								 tr("Failed to load dictionary."));
		}
		if (! loadMemoData()) {
			ifstream is("default.txt", ios::in);
			if (is) // файл открыт для чтения, значит он сущетсвует
				QMessageBox::warning(nullptr, tr("Application loading"),
									 tr("Failed to load memorizing data."));
		}
	}

	connect(ui->applyButton, &QPushButton::clicked,
			this, &MainWindow::applyTranslationSettings);
	connect(ui->restoreDefaultsButton, &QPushButton::clicked,
			this, &MainWindow::restoreDefaultTranslationSettings);

	connect(ui->startSessionButton, &QPushButton::clicked,
				this, &MainWindow::startTranslationDialog);
	sessionStartTimer = new QTimer();
	//connect(sessionStartTimer, &QTimer::timeout,
	//			sessionDialog, &QWidget::showNormal);
	connect(sessionStartTimer, &QTimer::timeout,
			this, &MainWindow::startTranslationDialog);

	connect(ui->menu, &QMenu::aboutToShow,
			this, &MainWindow::menuAboutToShow);

	connect(ui->closeButton, &QPushButton::clicked,
			this, &QMainWindow::close);


	moveTranslationsDialog->setupTables(trItemsL);
	moveTranslationsDialog->setWindowModified(false);
	dictEditDialog->setupTable(trItemsL);
	dictEditDialog->setWindowModified(false);

	createActions();
	createTrayIcon();
	connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);
	trayIcon->show();
}
void MainWindow::applicationQuit() {
	QSettings settings;
	settings.setValue("MainWindow/geometry", saveGeometry());

	settings.setValue("dictionaryFilePath", mDictionaryFilePath);

	settings.beginGroup("Translations");
	settings.setValue("sessionInverval", ui->sessionIntervalSpinBox->value());
	settings.setValue("successesForExclusion", ui->successTriesSpinBox->value());
	settings.setValue("alternativesNumber", ui->alternativesSpinBox->value());
	settings.setValue("triesNumber", ui->triesSpinBox->value());
	settings.endGroup();

	//if (! saveTempFile()) {
	//	QMessageBox::warning(nullptr, tr("Выход из приложения"),
	//						 tr("Не удалось сохранить данные о заучивании. Вся информация будет потеряна."));
	//}

	if (! saveMemoData()) {
		QMessageBox::warning(nullptr, tr("Application quit"),
							 tr("Failed to save memorizing data. All the memorizing information will be lost."));
	}
	if (processUnsavedChanges()) {
		emit quitApplication();
	}
}
bool MainWindow::event(QEvent *e) {
	if (e->type() == QEvent::Close) {
		if (!sessionDialog->isVisible()) {
			if (sessionDialog->prepareTranslationRequest())
				sessionStartTimer->start(ui->sessionIntervalSpinBox->value() * 60 * 1000);
		}

		if (trayIcon->isVisible()) {
			e->ignore();
			hide();
			return true;
		}
	}
	else if (e->type() == QEvent::Show)
	{
		restoreTranslationSettings();
		sessionStartTimer->stop();
	}


	return QMainWindow::event(e);
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == sessionDialog) {
		if (event->type()==QEvent::Close /*|| event->type()==QEvent::Hide*/) {
			if(!isVisible()) {
				if (sessionDialog->prepareTranslationRequest())
					sessionStartTimer->start(ui->sessionIntervalSpinBox->value() * 60 * 1000);
			}
		}
		else if (event->type() == QEvent::Show)
		{
			sessionStartTimer->stop(); // -не обязательно: диалог может быть открыт по кнопке или по таймеру. Если по кнопке то таймер дб уже остановлен,
			// если по таймеру, то тоже остановлен - время истекло
			hide();
		}
		else if(event->type() == QEvent::MouseButtonPress) {
		// предполагаем, что выбран ответ и счетчик какого-то TranslationItem изменился
			moveTranslationsDialog->setWindowModified(true);
		}
	}
	if (obj == dictEditDialog) {
		if (event->type() == QEvent::UpdateRequest) {
			QString shownName = "Untitled";
			if (!mDictionaryFilePath.isEmpty())
				shownName = QFileInfo(mDictionaryFilePath).fileName();
			dictEditDialog->setWindowTitle(tr("%1%2 - %3").arg(shownName)
										   .arg(dictEditDialog->isWindowModified()?"[*]":"")
										   .arg("Edit"));
		}
	}

	// pass the event on to the parent class
	return QMainWindow::eventFilter(obj, event);
}
// вызов диалога переводов
void MainWindow::startTranslationDialog()
{
	if (qobject_cast<QTimer*>(QObject::sender()) == sessionStartTimer) {
		sessionDialog->showNormal();
		return;
	}
	else if (qobject_cast<QPushButton*>(QObject::sender()) == ui->startSessionButton)
	{
		// вызов по кнопке
		if (sessionDialog->prepareTranslationRequest())
			sessionDialog->showNormal();
	}

}
void MainWindow::menuAboutToShow() {
	ui->dictionarySaveAction->setEnabled(dictEditDialog->isWindowModified());
}
void MainWindow::createActions()
{
	minimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

	maximizeAction = new QAction(tr("Ma&ximize"), this);
	connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, &QAction::triggered, this, &MainWindow::applicationQuit);
	//connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}
void MainWindow::createTrayIcon() {
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(minimizeAction);
	trayIconMenu->addAction(maximizeAction);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);

	trayIcon->setIcon(QIcon(":/images/heart.png"));
	setWindowIcon(QIcon(":/images/heart.png"));
}
void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
	switch (reason) {
	case QSystemTrayIcon::Trigger:
		if (!sessionDialog->isVisible())
			showNormal();
		break;
	default:
		;
	}
}
void MainWindow::setVisible(bool visible)
{
	minimizeAction->setEnabled(visible);
	maximizeAction->setEnabled(!isMaximized());
	restoreAction->setEnabled(isMaximized() || !visible);
	QMainWindow::setVisible(visible);
}
void MainWindow::restoreDefaultTranslationSettings() {
	ui->sessionIntervalSpinBox->setValue(1);
	ui->successTriesSpinBox->setValue(3);
	ui->alternativesSpinBox->setValue(5);
	ui->triesSpinBox->setValue(4);

	applyTranslationSettings();
}
void MainWindow::applyTranslationSettings() {
	sessionDialog->setSessionInterval(ui->sessionIntervalSpinBox->value());
	sessionDialog->setSuccessesForExclusion(ui->successTriesSpinBox->value());
	sessionDialog->setAlternativesNumber(ui->alternativesSpinBox->value());
	sessionDialog->setTriesNumber(ui->triesSpinBox->value());
}
void MainWindow::restoreTranslationSettings() {
	ui->sessionIntervalSpinBox->setValue(sessionDialog->sessionInterval());
	ui->successTriesSpinBox->setValue(sessionDialog->successesForExclusion());
	ui->alternativesSpinBox->setValue(sessionDialog->alternativesNumber());
	ui->triesSpinBox->setValue(sessionDialog->triesNumber());
}
bool MainWindow::processUnsavedChanges() {
	if (dictEditDialog->isWindowModified()) {
		int r = QMessageBox::warning(this,
									 tr("Edit"), tr("The dictionary has been modified.\n"
														   "Do you want to save your changes?"),
									 QMessageBox::Yes | QMessageBox::Default,
									 QMessageBox::No,
									 QMessageBox::Cancel | QMessageBox::Escape);
		if (r == QMessageBox::Yes) {
			return saveFile();
		} else if (r == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
}
void MainWindow::newDictionary() {
	dictEditDialog->clearTable();
	moveTranslationsDialog->clearTables();
	mDictionaryFilePath.clear();
	clearTrItems();
	QFile("default.txt").remove();
	dictEditDialog->open();
}
void MainWindow::openFile() {
	if (processUnsavedChanges()) {

		QString filePath = QFileDialog::getOpenFileName(this, tr("Открытие файла словаря"), ".");
		if (!filePath.isEmpty()) {
			if (loadDictionary(filePath)) {
				QFile("default.txt").remove();
				dictEditDialog->setupTable(trItemsL);
				moveTranslationsDialog->setupTables(trItemsL);
				dictEditDialog->open();
			}
		}
	}
}
bool MainWindow::saveFile() {
	if (!mDictionaryFilePath.isEmpty())
	{
		if (!saveDictionary(mDictionaryFilePath))
			return false;
//		if (! saveTempFile()) {
//			QMessageBox::warning(nullptr, tr("Saving data"),
//								 tr("Не удалось сохранить данные о заучивании. Вся информация будет потеряна."));
//			//return false;
//		}
		return true;
	}
	else
		return saveFileAs();
}
bool MainWindow::saveFileAs() {
	mDictionaryFilePath = QFileDialog::getSaveFileName(this, tr("Сохранение файла словаря"), ".");
	if (!mDictionaryFilePath.isEmpty()) {
		if (! saveDictionary(mDictionaryFilePath))
			return false;
//		if (! saveTempFile()) {
//			QMessageBox::warning(nullptr, tr("Saving data"),
//								 tr("Не удалось сохранить данные о заучивании. Вся информация будет потеряна."));
//			//return false;
//		}
		return true;
	}
	return false;
}
bool MainWindow::loadDictionary(const QString& path) {
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream in(&file);
	clearTrItems();
	TranslationItem *tip = nullptr;
	QString line;
	while (true) {
		line = in.readLine();
		if (in.status() != QTextStream::Ok)
			return false;
		if (line.isNull())
			break;
		if (nullptr==(tip=processLine(line)))
			continue;
		trItemsL.append(tip);
	}

	setDictFilePath(path);
	return true;
}
TranslationItem* MainWindow::processLine(const QString& line) const {
	int ind = line.indexOf('/');
	if (-1 == ind)
		return nullptr;
	QString expr1 = line.left(ind).trimmed();
	if (expr1.isEmpty())
		return nullptr;
	QString expr2 = line.right(line.length()-ind-1).trimmed();
	if (expr2.isEmpty())
		return nullptr;
	return new TranslationItem(expr1, expr2);
}
bool MainWindow::saveDictionary(const QString& path) {
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		return false;
	QTextStream out(&file);
	foreach(const TranslationItem* tip, trItemsL) {
		out << tip->firstExpr() + "/" + tip->secondExpr() + "\n";
		if (out.status() != QTextStream::Ok) {
			qDebug() << "Ошибка записи фпайла" + path;
			return false;
		}
	}
	/*file.close();
	if (out.status() != QTextStream::Ok) {
		qDebug() << "Ошибка записи файла " + path;
		return false;
	}*/

	setDictFilePath(path);
	return true;
}
void MainWindow::setDictFilePath(const QString& path) {
	mDictionaryFilePath = path;
	dictEditDialog->setWindowModified(false);
	moveTranslationsDialog->setWindowModified(false);
}
bool MainWindow::loadMemoData() {
	if (trItemsL.isEmpty())
		return true;
	ifstream ifs("default.txt", ios::in); // вх поток на файл default.txt в тек директории
	if (!ifs)
		return false;
	bool b = false;
	int n = 0;
	foreach(TranslationItem *tip, trItemsL) {
		if (ifs.eof())
			return false;
		ifs >> std::boolalpha >> b;
		if (ifs.fail()  || ifs.eof())
			return false;
		tip->setExcluded(b);
		ifs >> std::boolalpha >> b;
		if (ifs.fail() || ifs.eof())
			return false;
		tip->setInvert(b);
		ifs >> n;
		if (ifs.fail())
			return false;
		tip->setSuccessCounter(n);
	}
	return true;
}
bool MainWindow::saveMemoData() const {
	if (trItemsL.isEmpty())
		return true;
	ofstream ofs("default.txt", ios::out); // вых поток на файл default.txt в тек директории
	if (!ofs)
		return false;
	foreach(const TranslationItem *tip, trItemsL) {
		ofs << std::boolalpha << tip->isExcluded();
		if (! ofs)
			return false;
		ofs << std::boolalpha << tip->isInvert();
		if (! ofs)
			return false;
		ofs << tip->successCounter();
		if (! ofs)
			return false;
	}
	return true;
}
/*bool MainWindow::loadTempFile() {
	ifstream ifs("default.txt", ios::in); // вх поток на файл default.txt в тек директории
	if (!ifs)
		return false;
	clearTrItems();
	while (!ifs.eof()) {
		//qDebug("ifs.tellg=%d", ifs.tellg());
		TranslationItem *tip = new TranslationItem;
		ifs >> *tip;
		trItemsL.append(tip);
		if (ifs.fail()) {
			clearTrItems();
			return false;
		}
	}
	return true;
}
bool MainWindow::saveTempFile() const {
	ofstream ofs("default.txt", ios::out); // вых поток на файл default.txt в тек директории
	if (!ofs)
		return false;
	foreach(const TranslationItem *tip, trItemsL) {
		ofs << *tip;
		if (! ofs) {
			return false;
		}
	}
	return true;
}*/
void MainWindow::addTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	trItemsL.append(const_cast<TranslationItem *>(tip));
	moveTranslationsDialog->addTranslation(tip);
}
void MainWindow::removeTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	moveTranslationsDialog->removeTranslation(tip);
	trItemsL.removeOne(const_cast<TranslationItem*>(tip));
	delete tip;
}
void MainWindow::editTranslation(const TranslationItem *tip) {
	// у tip должен быть уже изменен текст
	Q_ASSERT(nullptr != tip);
	moveTranslationsDialog->editTranslation(tip);
}
void MainWindow::clearTrItems() {
	for (TranslationItem *tip: trItemsL)
		if (nullptr != tip)
			delete tip;
	trItemsL.clear();
}
MainWindow::~MainWindow()
{
    delete ui;
	delete moveTranslationsDialog;
	delete dictEditDialog;
	delete sessionDialog;
	clearTrItems();
}


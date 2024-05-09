#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>
#include <QMouseEvent>
#include <QDebug>

#include <fstream>
using namespace std;

#include "mainwindow.h"
#include "ui_mainwindow.h"

//#if defined (Q_OS_LINUX)
//#define TEMP_DIRECTORY_PATH "/tmp"
//#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	QCoreApplication::setOrganizationName("MyCompany");
	QCoreApplication::setApplicationName("LexNew");

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
	connect(dictEditDialog->saveButton, &QPushButton::clicked,
			this, &MainWindow::saveFile);

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
	connect(this, &MainWindow::quitApplication,
			qApp, &QCoreApplication::quit);

	sessionDialog = new TranslationDialog(nullptr, &trItemsL);
	sessionDialog->installEventFilter(this);
	connect(sessionDialog, &TranslationDialog::excludeTranslation,
			moveTranslationsDialog, &MoveTranslationsDialog::excludeTranslation);
	connect(sessionDialog->alternativesBox(), SIGNAL(activated(int)),
			this, SLOT(sessionDialogAlternativesBoxActivated(int)));

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
	setFixedSize(width(), height());

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
	if (!mDictionaryFilePath.isEmpty()) {
		if (! loadDictionary(mDictionaryFilePath)) {
			QMessageBox::critical(nullptr, tr("Application loading"),
								 tr("Failed to load the dictionary."));
		}
		if (! loadMemoData()) {
			// Было: если файл открыт для чтения, значит он сущетсвует и это ошибка конвертации, только тогда выводится сообщение
			// А что, если он не открыт для чтения? Его нет или он поврежден. Сообщение не выводится?
			// Сообщение выводится, если ошибка конвертации файла default.txt
			// (или не удалось открыть файл, в том числе, если он не был найден или поврежден).
			// Раз путь к словарю не пустой, значит словарь открывался, и данные
			// при последнем выходе должны были сохраниться в файл default.txt, и он должен был существовать.
			// Раз его нет, значит выводится сообщение
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
	else if (e->type() == QEvent::UpdateRequest) {
		int nExcluded=0;
		foreach(TranslationItem *tip, trItemsL) {
			if (tip->isExcluded())
				++nExcluded;
		}
        QString shownName = "";
		if (!mDictionaryFilePath.isEmpty())
			shownName = QFileInfo(mDictionaryFilePath).fileName();
		ui->statusbar->showMessage(tr("%1%2 переводов, %3 активно, %4 иключено.").
								   arg(shownName+(shownName.isEmpty()?"":": ")).
								   arg(trItemsL.count()).
								   arg(trItemsL.count()-nExcluded).
								   arg(nExcluded));
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
			//hide();
		}
	}
	else if (obj == dictEditDialog) {
		if (event->type() == QEvent::UpdateRequest) {
			QString shownName = "Untitled";
			if (!mDictionaryFilePath.isEmpty())
				shownName = QFileInfo(mDictionaryFilePath).fileName();
			dictEditDialog->setWindowTitle(tr("%1%2 - %3").arg(shownName).
										   arg(dictEditDialog->isWindowModified()?"[*]":"").
										   arg("Edit"));
			dictEditDialog->saveButton->setEnabled(dictEditDialog->isWindowModified());
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
        sessionDialog->setFocus();
		return;
	}
	else if (qobject_cast<QPushButton*>(QObject::sender()) == ui->startSessionButton)
	{
		// вызов по кнопке
		sessionDialog->setWindowModality(Qt::WindowModal);
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

	trayIcon->setIcon(QIcon(":/images/Lex.png"));
	setWindowIcon(QIcon(":/images/Lex.png"));
}
void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
	switch (reason) {
	case QSystemTrayIcon::Trigger:
		//if (!sessionDialog->isVisible())
		showNormal();
		break;
	default:
		;
	}
}
void MainWindow::sessionDialogAlternativesBoxActivated(int) {
	moveTranslationsDialog->setWindowModified(true);
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
	if (sessionDialog->isVisible())
		sessionDialog->prepareTranslationRequest();
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
	removeMemoDataFile();
	dictEditDialog->open();
}
void MainWindow::openFile() {
	if (processUnsavedChanges()) {

		QString filePath = QFileDialog::getOpenFileName(this, tr("Открытие файла словаря"), ".");
		if (!filePath.isEmpty()) {
			if (loadDictionary(filePath)) {
				removeMemoDataFile();
				dictEditDialog->setupTable(trItemsL);
				moveTranslationsDialog->setupTables(trItemsL);
				dictEditDialog->open();
			}
		}
	}
}
bool MainWindow::removeMemoDataFile() {
#if defined (Q_OS_WIN32)
	return QFile("default.txt").remove());
#elif defined (Q_OS_LINUX)
    QString baseDirPath;
    char *home = getenv("HOME");
    if (NULL != home)
        baseDirPath = QString::fromLocal8Bit(home) + "/";
    //QString baseDirPath = TEMP_DIRECTORY_PATH;
    if (!QFile(baseDirPath+".lexnew/default.txt").remove())
		return false;
#else
	return QFile("default.txt").remove();
#endif
	return true;
}
bool MainWindow::saveFile() {
	if (!mDictionaryFilePath.isEmpty())	{
		return saveDictionary(mDictionaryFilePath);
	}
	else
		return saveFileAs();
}
bool MainWindow::saveFileAs() {
	mDictionaryFilePath = QFileDialog::getSaveFileName(this, tr("Сохранение файла словаря"), ".");
	if (!mDictionaryFilePath.isEmpty()) {
		return saveDictionary(mDictionaryFilePath);
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

#if defined (Q_OS_WIN32)
	ifstream ifs("default.txt", ios::in); // вх поток на файл default.txt в тек директории
#elif defined (Q_OS_LINUX)
    QString baseDirPath;
    char *home = getenv("HOME");
    if (NULL != home)
        baseDirPath = QString::fromLocal8Bit(home) + "/";
    //QString baseDirPath = TEMP_DIRECTORY_PATH;
	//QDir dir(baseDirPath);
    if (!QDir(baseDirPath).exists(".lexnew"))
		return false;
    ifstream ifs(baseDirPath.toStdString()+".lexnew/default.txt", ios::in);
#else
	ifstream ifs("default.txt", ios::in); // вх поток на файл default.txt в тек директории
#endif
	if (!ifs)
		return false;

	bool b = false;
	int n = 0;
	foreach(TranslationItem *tip, trItemsL) {
		if (ifs.eof())
			return false;
		ifs >> std::boolalpha >> b;
		if (ifs.fail() || ifs.eof())
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
#if defined (Q_OS_WIN32)
	ofstream ofs("default.txt", ios::out); // вых поток на файл default.txt в тек директории
#elif defined (Q_OS_LINUX)
    char *home = getenv("HOME");
    QString baseDirPath;
    if (NULL != home)
        baseDirPath = QString::fromLocal8Bit(home) + "/";
    //QString baseDirPath = TEMP_DIRECTORY_PATH;
	QDir dir(baseDirPath);
    if (!dir.exists(".lexnew")) {
        if (!dir.mkdir(".lexnew"//, 0x6066 //since Qt 6.3
					   ))
			return false;
	}
    ofstream ofs(baseDirPath.toStdString() + ".lexnew/default.txt", ios::out);
#else
	ofstream ofs("default.txt", ios::out); // вых поток на файл default.txt в тек директории
#endif
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


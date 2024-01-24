#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "ui_movetranslations.h"
//#include "translationmodel.h"
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	//, ui1(new Ui::MoveTranslationsWidget)
	//, moveTranslationsWidget(parent)
{
	trItemsv.append(new TranslationItem(tr("to interfere with"), tr("мешать кому-л, чему-л")));
	trItemsv.append(new TranslationItem(tr("to rely on(upon)"), tr("полагаться на")));
	trItemsv.append(new TranslationItem(tr("to insist on"), tr("настаивать на")));
	trItemsv.append(new TranslationItem(tr("to arrive at"), tr("прибывать в"), true, true));
	trItemsv.append(new TranslationItem(tr("to open with"), tr("открывать чем-н."), true, false));

	ui->setupUi(this);

	// Загрузка сохраненных настроек приложения и положения окна из settings-файла


	/*ui1->setupUi(&moveTranslationsWidget);

	connect(ui->moveTranslationsButton, &QPushButton::clicked,
			&moveTranslationsWidget, &QWidget::show);

	TranslationModel *model = new TranslationModel();
	ui1->currentTableView->setModel(model);
	for (int column = 0; column < model->columnCount(); ++column)
		ui1->currentTableView->resizeColumnToContents(column);
	//ui1->exclusionTableView->setModel(model);*/

	dialog1 = new MoveTranslationsDialog;
	connect(ui->moveTranslationsButton, &QPushButton::clicked,
			dialog1, &QDialog::open);

	/*connect(this, &MainWindow::dictionaryAddTranslation,
			dialog1, &MoveTranslationsDialog::addTranslation);
	connect(this, &MainWindow::dictionaryRemoveTranslation,
			dialog1, &MoveTranslationsDialog::removeTranslation);
	connect(this, &MainWindow::dictionaryEditTranslation,
			dialog1, &MoveTranslationsDialog::editTranslation);*/
	dialog1->setupTables(trItemsv);

	dialog2 = new DictionaryEditDialog;
	//connect(ui->dictionaryEditButton, &QPushButton::clicked,
	//		dialog2, &QDialog::open);
	connect(dialog2, SIGNAL(addTranslationSig(const TranslationItem *)),
			this, SLOT(addTranslation(const TranslationItem *)));
	connect(dialog2, SIGNAL(removeTranslationSig(const TranslationItem *)),
			this, SLOT(removeTranslation(const TranslationItem *)));
	connect(dialog2, SIGNAL(editTranslationSig(const TranslationItem *)),
			this, SLOT(editTranslation(const TranslationItem *)));
	dialog2->setupTable(trItemsv);
	connect(ui->dictionaryEditAction, &QAction::triggered,
			dialog2, &QDialog::open);

	sessionDialog = new TranslationDialog(nullptr, &trItemsv);
	connect(sessionDialog, &TranslationDialog::excludeTranslation,
			dialog1, &MoveTranslationsDialog::excludeTranslation);
	//connect(dialog3, SIGNAL(finished(int)),
	//		this, SLOT(translationDialogFinished(int)));

	// Устанавливаем диалоге переводов исходные значения с контролов
	// ... загрузить установки из QSettings ...
	restoreDefaultTranslationSettings(); // временно так
	// ...

	connect(ui->applyButton, &QPushButton::clicked,
			this, &MainWindow::applyTranslationSettings);
	connect(ui->restoreDefaultsButton, &QPushButton::clicked,
			this, &MainWindow::restoreDefaultTranslationSettings);

	connect(ui->closeButton, &QPushButton::clicked,
			this, &QMainWindow::close);

	//dialog3->setModal(false);
	//dialog3->setWindowModality(Qt::ApplicationModal);
	sessionDialog->installEventFilter(this);
	connect(ui->startSessionButton, &QPushButton::clicked,
				this, &MainWindow::startTranslationDialog);
	sessionStartTimer = new QTimer();
	//connect(sessionStartTimer, &QTimer::timeout,
	//			dialog3, &QWidget::showNormal);
	connect(sessionStartTimer, &QTimer::timeout,
			this, &MainWindow::startTranslationDialog);

	createActions();
	createTrayIcon();
	connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);
	trayIcon->show();
}
bool MainWindow::event(QEvent *e) {
	if (e->type() == QEvent::Close) {
		if (!sessionDialog->isVisible()) {
			if (sessionDialog->prepareTranslationRequest())
				//sessionStartTimer->start(ui->sessionIntervalSpinBox->value() * 60 * 1000);
				sessionStartTimer->start(ui->sessionIntervalSpinBox->value() * 5 * 1000);
		}
		if (trayIcon->isVisible()) {
			e->ignore();
			hide();
			return true;
		}
	}
	else if (e->type() == QEvent::Show) {
		//if (sessionStartTimer->isActive())
		sessionStartTimer->stop();
	}

	return QMainWindow::event(e);
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == sessionDialog) {
		if (event->type()==QEvent::Close /*|| event->type()==QEvent::Hide*/) {
			if(!isVisible()) {
				//Q_ASSERT(!sessionStartTimer->isActive());
				if (sessionDialog->prepareTranslationRequest())
					//sessionStartTimer->start(ui->sessionIntervalSpinBox->value() * 60 * 1000);
					sessionStartTimer->start(ui->sessionIntervalSpinBox->value() * 5 * 1000);
			}
		}
		else if (event->type() == QEvent::Show)
		{ // диалог может быть открыт по кнопке или по таймеру. Если по кнопке то таймер дб уже остановлен,
			// если по таймеру, то тоже остановлен - время истекло
			sessionStartTimer->stop();
			hide();
		}
	}

	// pass the event on to the parent class
	return QMainWindow::eventFilter(obj, event);
}
// вызов диалога переводов
void MainWindow::startTranslationDialog()
{
	//QTimer *timer= qobject_cast<QTimer*>(QObject::sender());
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
void MainWindow::createActions()
{
	minimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

	maximizeAction = new QAction(tr("Ma&ximize"), this);
	connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
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
	/*case QSystemTrayIcon::DoubleClick:
		iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1) % iconComboBox->count());
		break;
	case QSystemTrayIcon::MiddleClick:
		showMessage();
		break;*/
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
	sessionDialog->setSessionMinutes(ui->sessionIntervalSpinBox->value());
	sessionDialog->setSuccessesForExclusion(ui->successTriesSpinBox->value());
	sessionDialog->setAlternativesNumber(ui->alternativesSpinBox->value());
	sessionDialog->setTriesNumber(ui->triesSpinBox->value());
}

void MainWindow::addTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	trItemsv.append(const_cast<TranslationItem *>(tip));
	//emit dictionaryAddTranslation(tip);
	dialog1->addTranslation(tip);
}
void MainWindow::removeTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	//emit dictionaryRemoveTranslation(tip);
	dialog1->removeTranslation(tip);
	trItemsv.removeOne(const_cast<TranslationItem*>(tip));
	delete tip;
}
void MainWindow::editTranslation(const TranslationItem *tip) {
	// у tip должен быть уже изменен текст
	Q_ASSERT(nullptr != tip);
	//emit dictionaryEditTranslation(tip);
	dialog1->editTranslation(tip);
}
MainWindow::~MainWindow()
{
    delete ui;
	//delete ui1;
	delete dialog1;
	delete dialog2;
	delete sessionDialog;
	for (TranslationItem *tip: trItemsv)
		if (nullptr != tip)
			delete tip;
}


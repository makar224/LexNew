#include <QGuiApplication>
#include <QScreen>
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	QCoreApplication::setOrganizationName("MyCompany");
	QCoreApplication::setApplicationName("LexNew");

	trItemsL.append(new TranslationItem(tr("to interfere with"), tr("мешать кому-л, чему-л")));
	trItemsL.append(new TranslationItem(tr("to rely on(upon)"), tr("полагаться на")));
	trItemsL.append(new TranslationItem(tr("to insist on"), tr("настаивать на")));
	trItemsL.append(new TranslationItem(tr("to arrive at"), tr("прибывать в"), true, true));
	trItemsL.append(new TranslationItem(tr("to open with"), tr("открывать чем-н."), true, false));

	ui->setupUi(this);

	dialog1 = new MoveTranslationsDialog;
	connect(ui->moveTranslationsButton, &QPushButton::clicked,
			dialog1, &QDialog::open);

	dialog1->setupTables(trItemsL);

	dialog2 = new DictionaryEditDialog;
	connect(dialog2, SIGNAL(addTranslationSig(const TranslationItem *)),
			this, SLOT(addTranslation(const TranslationItem *)));
	connect(dialog2, SIGNAL(removeTranslationSig(const TranslationItem *)),
			this, SLOT(removeTranslation(const TranslationItem *)));
	connect(dialog2, SIGNAL(editTranslationSig(const TranslationItem *)),
			this, SLOT(editTranslation(const TranslationItem *)));
	dialog2->setupTable(trItemsL);
	connect(ui->dictionaryEditAction, &QAction::triggered,
			dialog2, &QDialog::open);

	sessionDialog = new TranslationDialog(nullptr, &trItemsL);
	connect(sessionDialog, &TranslationDialog::excludeTranslation,
			dialog1, &MoveTranslationsDialog::excludeTranslation);

	// Устанавливаем в диалоге переводов исходные значения
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
	//QString dictionaryPath = settings.value("dictionaryPath", "").toString();

	restoreDefaultTranslationSettings(); // значения по умолчанию - в контролы (для первой загрузки приложения)
	settings.beginGroup("Translations");
	ui->sessionIntervalSpinBox->setValue(settings.value("sessionInverval", ui->sessionIntervalSpinBox->value()).toInt());
	ui->successTriesSpinBox->setValue(settings.value("successesForExclusion", ui->successTriesSpinBox->value()).toInt());
	ui->alternativesSpinBox->setValue(settings.value("alternativesNumber", ui->alternativesSpinBox->value()).toInt());
	ui->triesSpinBox->setValue(settings.value("triesNumber", ui->triesSpinBox->value()).toInt());
	settings.endGroup();
	applyTranslationSettings();
	// ...

	connect(ui->applyButton, &QPushButton::clicked,
			this, &MainWindow::applyTranslationSettings);
	connect(ui->restoreDefaultsButton, &QPushButton::clicked,
			this, &MainWindow::restoreDefaultTranslationSettings);

	connect(ui->closeButton, &QPushButton::clicked,
			this, &QMainWindow::close);

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
void MainWindow::applicationQuit() {
	QSettings settings;
	settings.setValue("MainWindow/geometry", saveGeometry());

	//settings.setValue("dictionaryPath", );

	settings.beginGroup("Translations");
	settings.setValue("sessionInverval", ui->sessionIntervalSpinBox->value());
	settings.setValue("successesForExclusion", ui->successTriesSpinBox->value());
	settings.setValue("alternativesNumber", ui->alternativesSpinBox->value());
	settings.setValue("triesNumber", ui->triesSpinBox->value());
	settings.endGroup();
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
void MainWindow::addTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	trItemsL.append(const_cast<TranslationItem *>(tip));
	dialog1->addTranslation(tip);
}
void MainWindow::removeTranslation(const TranslationItem *tip) {
	Q_ASSERT(nullptr != tip);
	dialog1->removeTranslation(tip);
	trItemsL.removeOne(const_cast<TranslationItem*>(tip));
	delete tip;
}
void MainWindow::editTranslation(const TranslationItem *tip) {
	// у tip должен быть уже изменен текст
	Q_ASSERT(nullptr != tip);
	dialog1->editTranslation(tip);
}
MainWindow::~MainWindow()
{
    delete ui;
	delete dialog1;
	delete dialog2;
	delete sessionDialog;
	for (TranslationItem *tip: trItemsL)
		if (nullptr != tip)
			delete tip;
}


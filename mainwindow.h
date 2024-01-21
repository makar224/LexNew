#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include "movetranslationsdialog.h"
#include "dictionaryeditdialog.h"
//#include "ui_movetranslations.h"
#include "translationmodel.h"
#include "translationdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;
			 //class MoveTranslationsWidget;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
	
	void setVisible(bool visible) override;
	
protected slots:
	void applyTranslationSettings();
	void restoreDefaultTranslationSettings();
	void startTranslationDialog();

	void addTranslation(const TranslationItem *tip);
	void removeTranslation(const TranslationItem *tip);
	void editTranslation(const TranslationItem *tip);

signals:
	void dictionaryAddTranslation(const TranslationItem*);
	void dictionaryRemoveTranslation(const TranslationItem*);
	void dictionaryEditTranslation(const TranslationItem*);
protected:
	//void closeEvent(QCloseEvent *) override;
	bool event(QEvent *event) override;
	bool eventFilter(QObject *, QEvent *) override;
private slots:
	void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
private:
	void createActions();
	void createTrayIcon();
    Ui::MainWindow *ui;
	//Ui::MoveTranslationsWidget *ui1;
	//QWidget moveTranslationsWidget;
	MoveTranslationsDialog *dialog1;
	DictionaryEditDialog *dialog2;
	TranslationDialog *sessionDialog;
	QVector<TranslationItem *> trItemsv;
	QTimer *sessionStartTimer;
	
	QAction *minimizeAction;
	QAction *maximizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
};
#endif // MAINWINDOW_H

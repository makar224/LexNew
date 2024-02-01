#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QSettings>
#include "movetranslationsdialog.h"
#include "dictionaryeditdialog.h"
#include "translationmodel.h"
#include "translationdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;
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
	void restoreTranslationSettings();

	void startTranslationDialog();

	void addTranslation(const TranslationItem *tip);
	void removeTranslation(const TranslationItem *tip);
	void editTranslation(const TranslationItem *tip);

signals:
	void dictionaryAddTranslation(const TranslationItem*);
	void dictionaryRemoveTranslation(const TranslationItem*);
	void dictionaryEditTranslation(const TranslationItem*);
protected:
	bool event(QEvent *event) override;
	bool eventFilter(QObject *, QEvent *) override;
private slots:
	void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void applicationQuit();
private:
	void createActions();
	void createTrayIcon();
    Ui::MainWindow *ui;
	MoveTranslationsDialog *dialog1;
	DictionaryEditDialog *dialog2;
	TranslationDialog *sessionDialog;
	QList<TranslationItem *> trItemsL;
	QTimer *sessionStartTimer;

	QSettings *settings;
	
	QAction *minimizeAction;
	QAction *maximizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
};
#endif // MAINWINDOW_H

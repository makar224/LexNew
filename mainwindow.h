#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
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

	void addTranslation(const TranslationItem *tip);
	void removeTranslation(const TranslationItem *tip);
	void editTranslation(const TranslationItem *tip);

signals:
	void quitApplication();
protected:
	bool event(QEvent *event) override;
	bool eventFilter(QObject *, QEvent *) override;
private slots:
	void startTranslationDialog();
	void menuAboutToShow();
	void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void sessionDialogAlternativesBoxActivated(int);
	void applicationQuit();
	void newDictionary();
	void openFile();
	bool saveFile();
	bool saveFileAs();
private:
	void restoreTranslationSettings();
	void createActions();
	void createTrayIcon();
	bool processUnsavedChanges();
	bool loadDictionary(const QString& path);
	TranslationItem *processLine(const QString& line) const;
	bool saveDictionary(const QString& path);
	bool loadMemoData();
	bool saveMemoData() const;
	void setDictFilePath(const QString& path);
	void clearTrItems();
    Ui::MainWindow *ui;
	MoveTranslationsDialog *moveTranslationsDialog;
	DictionaryEditDialog *dictEditDialog;
	TranslationDialog *sessionDialog;
	QList<TranslationItem *> trItemsL;
	QTimer *sessionStartTimer;

	QString mDictionaryFilePath;
	
	QAction *minimizeAction;
	QAction *maximizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
};
#endif // MAINWINDOW_H

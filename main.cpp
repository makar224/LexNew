#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(LexNew);

	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(nullptr, QObject::tr("Systray"),
							  QObject::tr("I couldn't detect any system tray "
										  "on this system."));
		return 1;
	}
	QApplication::setQuitOnLastWindowClosed(false);

    MainWindow w;
    w.show();
    return a.exec();
}

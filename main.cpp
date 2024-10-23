#include "mainwindow.h"
#include <QApplication>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(LexNew);

	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QApplication a(argc, argv);

	// Определяем не запущен ли уже другой экземпляр приложения
	//---------------------------------------------------------
	QSystemSemaphore semaphore("LexUniqSemaphoreId", 1);  // создаём семафор для исключения гонок
	semaphore.acquire(); // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью

#ifndef Q_OS_WIN32
	// в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
	// поэтому необходимо избавиться от данного мусора
	QSharedMemory nix_fix_shared_memory("LexUniqSharedMemId");
	if (nix_fix_shared_memory.attach())
	{
		nix_fix_shared_memory.detach();
	}
#endif

	QSharedMemory sharedMemory("LexUniqSharedMemId");  // Создаём экземпляр разделяемой памяти
	bool is_running;            // переменную для проверки уже запущенного приложения
	if (sharedMemory.attach())
	{                           // пытаемся присоединить экземпляр разделяемой памяти
		// к уже существующему сегменту
		is_running = true;      // Если успешно, то определяем, что уже есть запущенный экземпляр
	}
	else
	{
		sharedMemory.create(1); // В противном случае выделяем 1 байт памяти
		is_running = false;     // И определяем, что других экземпляров не запущено
	}
	semaphore.release();        // Опускаем семафор

	// Если уже запущен один экземпляр приложения, то сообщаем об этом пользователю
	// и завершаем работу текущего экземпляра приложения
	if (is_running)
	{
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setText(QObject::tr("Приложение уже запущено.\n"
									   "Можно запустить только один экземпляр приложения."));
		msgBox.exec();
		return 1;
	}
	//---------------------------------------------------------

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

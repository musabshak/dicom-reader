
// Qt header files
#include <QApplication>
#include <QCoreApplication>
#include <QFile>

// Our header files
#include "ui.h"

void main()
{
	// dpi scaling
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);


	// Create the Qt application
	int argc = 0;
	QApplication app(argc, 0);

	// Create the user interface
	ui myui;

	QFile file("../src/stylesheet.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());

	app.setStyleSheet(styleSheet);

	// Start the Qt application event loop
	app.exec();
}

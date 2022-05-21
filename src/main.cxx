
// Qt header files
#include <QApplication>
#include <QCoreApplication>

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

	// Start the Qt application event loop
	app.exec();
}


// Prevent this header file from being included multiple times
#pragma once

// VTK header files
#include <vtkSmartPointer.h>
#include <vtkPNGReader.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageThreshold.h>

// Qt header files
#include <QMainWindow.h>
#include <QLayout.h>
#include <QPushButton.h>
#include <QLabel.h>
#include <QSlider.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QString.h>
#include <qsignalmapper.h>
#include <QFileDialog.h>
#include <QDir.h>
#include <QTextStream.h>
#include <QMenuBar.h>
#include <QMenu.h>

// Class that represents the main window for our application
class ui : public QMainWindow
{
	Q_OBJECT
public:
	// Declare three QVTKOpenGLNativeWidget here
	QVTKOpenGLNativeWidget* viewport1, * viewport2, * viewport3;

	// Delcare three vtkGenericOpenGLRenderWindow here
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> window1, window2, window3;



	

	// Constructor (happens when created)
	ui() {

		// S==================== METADATA ==================== //
		
		// Resize the window
		this->resize(1200, 400);

		this->setWindowTitle("DICOM Reader");

		// S======================= WIDGETS ======================= //
		
		// Create the "central" (primary) widget for the window
		QWidget* widget = new QWidget();
		this->setCentralWidget(widget);

		QPushButton* button_choose_dir = new QPushButton("Choose Directory");

		/* Add a menu item with one action to QMainWindow's in-built menu bar*/
		QAction* load_action = new QAction("Load DICOM data");
		auto fileMenu = menuBar()->addMenu("&File");
		fileMenu->addAction(load_action);



		// S================ CREATE LAYOUTs ================ //
		QVBoxLayout* layout_vertical_main = new QVBoxLayout();
		QHBoxLayout* layout_row0 = new QHBoxLayout();
		QHBoxLayout* layout_row1 = new QHBoxLayout();

		// S========== ADD WIDGETS/LAYOUTS TO LAYOUTs ========= //
		widget->setLayout(layout_vertical_main);

		layout_vertical_main->addLayout(layout_row0);
		layout_vertical_main->addLayout(layout_row1);

		//layout_row0->addWidget(button_choose_dir);
		//layout_row0->addStretch();
		

		// S========== CONNECT SIGNALS TO SLOTS ========= //
		connect(button_choose_dir, SIGNAL(released()), this, SLOT(choose_directory()));
		connect(load_action, SIGNAL(triggered()), this, SLOT(load_DICOM()));

		// Display the window
		this->show();
	}

public slots:

	
	/*
	Pop up a dialog window that allows user to choose a directory. The dialog opens to 
	the root folder of the current project.
	*/
	void choose_directory() {

		// tr() is a Qt function for translating strings to multiple languages
		QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			"../" + QDir::currentPath(),
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		QTextStream qt_cout(stdout);
		qt_cout << dir;

	}

	void load_DICOM() {
		choose_directory();
	}

};
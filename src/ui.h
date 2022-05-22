
// Prevent this header file from being included multiple times
#pragma once

// VTK header files
#include <vtkDICOMImageReader.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>



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

	QSlider* slider_x, *slider_y, *slider_z;

	vtkSmartPointer<vtkGenericOpenGLRenderWindow> window_volume, window_x, window_y, window_z;

	vtkSmartPointer<vtkMatrix4x4> reslice_axes;
	vtkSmartPointer<vtkImageReslice> reslice;
	vtkSmartPointer<vtkImageActor> iactor;
	vtkSmartPointer<vtkRenderer> renderer;

	bool is_data_loaded = false;


	// Constructor (happens when created)
	ui() {

		// S==================== METADATA ==================== //
		
		// Resize the window
		this->resize(1200, 400);

		this->setWindowTitle("DICOM Reader");

		// S======================= WIDGETS ======================= //
		
		// Create the "central" (primary) widget for the window
		QWidget* widget = new QWidget();

		slider_x = new QSlider();
		slider_x->setOrientation(Qt::Vertical);

		slider_y = new QSlider();
		slider_y->setOrientation(Qt::Vertical);

		slider_z = new QSlider();
		slider_z->setOrientation(Qt::Vertical);

		// QT's VTK viewports
		QVTKOpenGLNativeWidget* viewport_volume = new QVTKOpenGLNativeWidget();
		QVTKOpenGLNativeWidget* viewport_x = new QVTKOpenGLNativeWidget();
		QVTKOpenGLNativeWidget* viewport_y = new QVTKOpenGLNativeWidget();
		QVTKOpenGLNativeWidget* viewport_z = new QVTKOpenGLNativeWidget();
		
		// VTK's special render windows
		window_volume = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		window_x = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		window_y = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		window_z = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

		// support for high DPI screens
		viewport_volume->enableHiDPI();
		viewport_x->enableHiDPI();
		viewport_y->enableHiDPI();
		viewport_z->enableHiDPI();

		// set QT viewports' render windows to the VTK render windows
		viewport_volume->SetRenderWindow(window_volume);
		viewport_x->SetRenderWindow(window_x);
		viewport_y->SetRenderWindow(window_y);
		viewport_z->SetRenderWindow(window_z);

		// set a minimum size for the viewports
		viewport_volume->setMinimumSize(400, 400);
		viewport_x->setMinimumSize(400, 400);
		viewport_y->setMinimumSize(400, 400);
		viewport_z->setMinimumSize(400, 400);
		
		/* Add a menu item with one action to QMainWindow's in-built menu bar */
		QAction* load_action = new QAction("Load DICOM data");
		auto fileMenu = menuBar()->addMenu("&File");
		fileMenu->addAction(load_action);

		/* Slice labels */

		QLabel* label01 = new QLabel("#");
		label01->setFixedSize(60, 20);


		// S================ CREATE LAYOUTs ================ //
		QVBoxLayout* layout_vertical_main = new QVBoxLayout();
		QHBoxLayout* layout_row0 = new QHBoxLayout();
		QHBoxLayout* layout_row1 = new QHBoxLayout();

		QVBoxLayout* layout_slicenum_01 = new QVBoxLayout();


		// S========== ADD WIDGETS/LAYOUTS TO LAYOUTs ========= //
		this->setCentralWidget(widget);
		widget->setLayout(layout_vertical_main);

		layout_vertical_main->addLayout(layout_row0);
		layout_vertical_main->addLayout(layout_row1);

		layout_row0->addSpacing(22); // no slider for volume view
		layout_row0->addWidget(viewport_volume);
		layout_row0->addWidget(slider_x);
		layout_row0->addWidget(viewport_x);

		viewport_x->setLayout(layout_slicenum_01);
		layout_slicenum_01->addWidget(label01);
		layout_slicenum_01->addStretch();

		layout_row1->addWidget(slider_y);
		layout_row1->addWidget(viewport_y);
		layout_row1->addWidget(slider_z);
		layout_row1->addWidget(viewport_z);
		
		

		// S========== CONNECT SIGNALS TO SLOTS ========= //
		connect(load_action, SIGNAL(triggered()), this, SLOT(load_DICOM()));
		connect(slider_x, SIGNAL(valueChanged(int)), this, SLOT(sliderx_changed(int)));
		//connect(slider_y, SIGNAL(valueChanged(int)), this, SLOT(sliderx_changed(int)));
		//connect(slider_z, SIGNAL(valueChanged(int)), this, SLOT(sliderx_changed(int)));

		// Display the window
		this->show();
	}

	/*
	Pop up a dialog window that allows user to choose a directory. Returns a QDir object containing 
	the absolute path to the user-specified data directory.
	*/
	QDir choose_directory() {

		// tr() is a Qt function for translating strings to multiple languages
		QDir dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			QDir::currentPath(),
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		QTextStream qt_cout(stdout);
		
		// QDir current_dir = QDir(QDir::currentPath());
		//qt_cout << current_dir.relativeFilePath(dir.absolutePath());

		return dir;

	}

	void load_DICOM_image_x() {

		is_data_loaded = false;
		cout << "loading data\n";

		//QDir dicom_dir = choose_directory();

		// Read all the DICOM files in the specified directory.
		vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();

		// TODO: figure out a better way to convert QString to a char * to pass to SetDirectoryName
		//reader->SetDirectoryName(dicom_dir.absolutePath().toStdString().c_str());
		reader->SetDirectoryName("../data");

		// Force update, since we need to get information about the data dimensions
		reader->Update();

		int* dims = reader->GetOutput()->GetDimensions(); // Get the data dimensions
		double* range = reader->GetOutput()->GetScalarRange(); // Get the range of intensity values

		// Create a matrix which defines a plane. This plane will be used to "slice" the data
		static double axial[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

		reslice_axes = vtkSmartPointer<vtkMatrix4x4>::New();
		reslice_axes->DeepCopy(axial);

		// vtkImageReslice is the filter that does the slicing (slices a 3D dataset to become 2D)
		reslice = vtkSmartPointer<vtkImageReslice>::New();
		reslice->SetInputConnection(reader->GetOutputPort()); // connect the reader to this filter
		reslice->SetOutputDimensionality(2);
		reslice->SetResliceAxes(reslice_axes); // tell it what plane to slice with
		reslice->SetInterpolationModeToLinear();
		reslice->Update();

		// Create an actor for the image. You'll notice we skipped the mapper step. This is because
		// image actors have a default mapper we can use as is if we don't want to change 
		// the colormap etc.
		iactor = vtkSmartPointer<vtkImageActor>::New();

		// Connect the reslice filter to the mapper
		iactor->GetMapper()->SetInputConnection(reslice->GetOutputPort());

		// Renderer
		renderer = vtkSmartPointer<vtkRenderer>::New();
		renderer->AddActor(iactor); // Add the actor to the renderer

		// Add the renderer to the render window
		window_x->AddRenderer(renderer);

		// Render (display the image)
		window_x->Render();

		// Ensure we aren't clipping any of the image (cameras have a front and back plane that 
		// clips for performance)
		renderer->ResetCameraClippingRange();

		// Tell our slider widget what the min/max slice numbers are
		slider_x->setRange(0, dims[2] - 1);


		cout << "finished loading data\n";
		is_data_loaded = true;

	}

	/*
	Render DICOM as volume
	*/
	void load_DICOM_volume() {

		is_data_loaded = false;
		cout << "loading data\n";

		//QDir dicom_dir = choose_directory();

		// Read all the DICOM files in the specified directory.
		vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();

		// TODO: figure out a better way to convert QString to a char * to pass to SetDirectoryName
		//reader->SetDirectoryName(dicom_dir.absolutePath().toStdString().c_str());
		reader->SetDirectoryName("../data");

		// Force update, since we need to get information about the data dimensions
		reader->Update();

		/* Code taken from in-class example */
		// volume mapper
		vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
		volumeMapper->SetBlendModeToComposite(); // composite
		volumeMapper->SetInputConnection(reader->GetOutputPort());
		volumeMapper->SetRequestedRenderModeToRayCast();

		// volume properties
		vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
		volumeProperty->ShadeOff();
		volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

		// opacity
		vtkSmartPointer<vtkPiecewiseFunction> opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
		opacity->AddPoint(0.0, 0.0);
		opacity->AddPoint(90.0, 0.05);
		opacity->AddPoint(555.0, 0.1);
		volumeProperty->SetScalarOpacity(opacity);

		// colormap
		vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
		color->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
		color->AddRGBPoint(300.0, 1.0, 0.0, 0.0);
		color->AddRGBPoint(555.0, 1.0, 1.0, 1.0);
		volumeProperty->SetColor(color);

		// create volume using the volume mapper and volume properties
		vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
		volume->SetMapper(volumeMapper);
		volume->SetProperty(volumeProperty);

		// renderer
		vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
		renderer->AddViewProp(volume);
		renderer->ResetCamera();

		// render window
		window_volume->AddRenderer(renderer);
		window_volume->Render();

		// interactor
		/*vtkSmartPointer<vtkRenderWindowInteractor> iren =
			vtkSmartPointer<vtkRenderWindowInteractor>::New();
		iren->SetRenderWindow(window_volume);*/

		// start interactor
		//iren->Start();

		cout << "finished loading data\n";
		is_data_loaded = true;
	}

public slots:

	/*void load_DICOM() {
		QDir dicom_dir = choose_directory();
	}*/

	void load_DICOM() {
		load_DICOM_image_x();
		load_DICOM_volume();
	}


	void sliderx_changed(int value) {

		if (!is_data_loaded) {
			cout << "data not loaded yet!\n";
			return;
		}

		// Set the slice
		reslice_axes->SetElement(2, 3, value);
		reslice->Modified();

		// Re-render the image data
		window_x->Render();

		// Convert int value to QString type, expected by setText
		//label->setText(QString::number(value));

	}

};
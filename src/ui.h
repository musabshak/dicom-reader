
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
class ui : public QMainWindow {

	Q_OBJECT
public:
	// constants
	int VOLUME = 0;
	int AXIAL = 1;    // xy- (setElement is 2,3)
	int CORONAL = 2;  // x-z (setElement is 1,3)
	int SAGITTAL = 3; // -yz (setElement is 0,1)

	static const int NUM_VIEWPORTS = 4;

	char slice_label_texts[4][50] = {
		"",
		"Axial (xy). Slice: ",
		"Coronal (xz). Slice: ",
		"Sagittal (yz). Slice: ",
	};

	// dataset column headings
	QLabel* col0_heading, * col1_heading;

	// opacity sliders
	QSlider* opacity_slider0, * opacity_slider1;

	// slice opacity slider labels
	QLabel* opacity_label0, * opacity_label1;

	// 3 sliders to control slice # for each of the 3 planes 
	// (and accompanying labels) (one extra for index purposes)
	QSlider* slider_arr[NUM_VIEWPORTS]; // 0th element is blank
	QLabel* slider_label_arr[NUM_VIEWPORTS];

	// 4 Qt viewports with a vtk render window in each
	QVTKOpenGLNativeWidget* viewport_arr[NUM_VIEWPORTS];
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> window_arr[NUM_VIEWPORTS];

	// vtk actors, filters, renderers for dataset 1
	vtkSmartPointer<vtkMatrix4x4> reslice_axes_arr[NUM_VIEWPORTS];
	vtkSmartPointer<vtkImageReslice> reslice_arr[NUM_VIEWPORTS];
	vtkSmartPointer<vtkImageActor> iactor_arr[NUM_VIEWPORTS];
	vtkSmartPointer<vtkRenderer> renderer_arr[NUM_VIEWPORTS];

	bool is_data1_loaded = false;
	bool is_data2_loaded = false;

	// matrices that defines planes for slicing
	// https://public.kitware.com/pipermail/vtkusers/2016-January/093908.html
	double axial_plane[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	double coronal_plane[16] = {
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, -1, 0, 0,
		0, 0, 0, 1 };

	double sagittal_plane[16] = {
		0, 0, -1, 0,
		1, 0, 0, 0,
		0, -1, 0, 0,
		0, 0, 0, 1 };

	// summarize above in an array of int arrays
	double* plane_arr[4] = { NULL, axial_plane, coronal_plane, sagittal_plane };


	// Constructor (happens when created)
	ui() {

		// S==================== METADATA ==================== //

		// Resize the window, set the title
		this->resize(1200, 400);
		this->setWindowTitle("DICOM Reader");





		// S======================= WIDGETS ======================= //

		// Create the "central" (primary) widget for the window
		QWidget* widget = new QWidget();

		// Add two menu itmes to QMainWindow's in-built menu bar 
		QAction* load_dset1_action = new QAction("Load DICOM dataset 1");
		QAction* load_dset2_action = new QAction("Load DICOM dataset 2");
		auto fileMenu = menuBar()->addMenu("&File");
		fileMenu->addAction(load_dset1_action);
		fileMenu->addAction(load_dset2_action);

		// initialize Qt viewports and VTK render windows
		for (int i = 0; i < NUM_VIEWPORTS; i++) {
			// initialize Qt viewports (that will show VTK render window)
			viewport_arr[i] = new QVTKOpenGLNativeWidget();
			viewport_arr[i]->enableHiDPI();

			// initialize VTK render windows
			window_arr[i] = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

			// set QT viewports' render windows to the VTK render windows
			viewport_arr[i]->SetRenderWindow(window_arr[i]);

			// set a minimum size for the viewports
			viewport_arr[i]->setMinimumSize(400, 400);

		}

		// initialize sliders for each of the 3 slice planes
		for (int i = 1; i < NUM_VIEWPORTS; i++) {
			slider_arr[i] = new QSlider();
			slider_arr[i]->setOrientation(Qt::Vertical);
		}

		// initialize slice labels for the 3 planes
		for (int i = 1; i < NUM_VIEWPORTS; i++) {
			slider_label_arr[i] = new QLabel(slice_label_texts[i]);
			//slider_label_arr[i]->setFixedSize(60, 20);
			slider_label_arr[i]->setAlignment(Qt::AlignCenter);
			slider_label_arr[i]->setStyleSheet("padding: 2px");
		}

		// dataset header labels
		col0_heading = new QLabel("Dataset 1: <no data loaded>");
		col0_heading->setObjectName("col0_heading");
		col0_heading->setAlignment(Qt::AlignCenter);

		col1_heading = new QLabel("Dataset 2: <no data loaded>");
		col1_heading->setObjectName("col1_heading");
		col1_heading->setAlignment(Qt::AlignCenter);

		// initialize opacity slider labels
		opacity_label0 = new QLabel("Opacity: 100");
		opacity_label0->setObjectName("opacity_label0");

		opacity_label1 = new QLabel("Opacity: 100");
		opacity_label1->setObjectName("opacity_label1");

		// initialize opacity sliders
		opacity_slider0 = new QSlider();
		opacity_slider0->setOrientation(Qt::Horizontal);
		opacity_slider0->setRange(0, 100);
		opacity_slider0->setValue(100);

		opacity_slider1 = new QSlider();
		opacity_slider1->setOrientation(Qt::Horizontal);
		opacity_slider1->setRange(0, 100);
		opacity_slider1->setValue(100);




		// S================ CREATE LAYOUTs ================ //
		QVBoxLayout* layout_vertical_main = new QVBoxLayout();
		QHBoxLayout* layout_row0 = new QHBoxLayout();
		QHBoxLayout* layout_row1 = new QHBoxLayout();
		QHBoxLayout* layout_row2 = new QHBoxLayout();

		// 2 vertical layouts for row0 for the two datasets
		QVBoxLayout* layout_col0 = new QVBoxLayout();
		QVBoxLayout* layout_col1 = new QVBoxLayout();

		QVBoxLayout* layout_slice_label_arr[4];

		// initialize layouts for slice labels for the 3 planes
		for (int i = 1; i < NUM_VIEWPORTS; i++) {
			layout_slice_label_arr[i] = new QVBoxLayout();
		}




		// S========== ADD WIDGETS/LAYOUTS TO LAYOUTs ========= //
		this->setCentralWidget(widget);
		widget->setLayout(layout_vertical_main);

		layout_vertical_main->addLayout(layout_row0);
		layout_vertical_main->addLayout(layout_row1);
		layout_vertical_main->addLayout(layout_row2);

		// populate row0
		layout_row0->addLayout(layout_col0);
		layout_row0->addLayout(layout_col1);
		layout_row0->addSpacing(15);

		// populate col0, col1
		layout_col0->addWidget(col0_heading, Qt::AlignCenter);
		layout_col0->addWidget(opacity_label0);
		layout_col0->addWidget(opacity_slider0);

		layout_col1->addWidget(col1_heading, Qt::AlignCenter);
		layout_col1->addWidget(opacity_label1);
		layout_col1->addWidget(opacity_slider1);

		// populate row1
		layout_row1->addSpacing(22); // no slider for volume view
		layout_row1->addWidget(viewport_arr[VOLUME]);

		layout_row1->addWidget(slider_arr[AXIAL]);
		layout_row1->addWidget(viewport_arr[AXIAL]);

		// populate row2
		layout_row2->addWidget(slider_arr[CORONAL]);
		layout_row2->addWidget(viewport_arr[CORONAL]);

		layout_row2->addWidget(slider_arr[SAGITTAL]);
		layout_row2->addWidget(viewport_arr[SAGITTAL]);

		// populate slider labels for each of 3 planes
		for (int i = 1; i < NUM_VIEWPORTS; i++) {
			viewport_arr[i]->setLayout(layout_slice_label_arr[i]);
			layout_slice_label_arr[i]->addWidget(slider_label_arr[i]);
			layout_slice_label_arr[i]->addStretch();
		}




		// S========== CONNECT SIGNALS TO SLOTS ========= //

		// file menu: load DICOM dataset actions
		connect(load_dset1_action, SIGNAL(triggered()),
			this, SLOT(load_dset1()));
		connect(load_dset2_action, SIGNAL(triggered()),
			this, SLOT(load_dset2()));

		// connect slice sliders
		connect(slider_arr[AXIAL], SIGNAL(valueChanged(int)),
			this, SLOT(slice_slider_changed(int)));
		connect(slider_arr[CORONAL], SIGNAL(valueChanged(int)),
			this, SLOT(slice_slider_changed(int)));
		connect(slider_arr[SAGITTAL], SIGNAL(valueChanged(int)),
			this, SLOT(slice_slider_changed(int)));

		// connect opacity sliders
		connect(opacity_slider0, SIGNAL(valueChanged(int)),
			this, SLOT(opacity_slider_changed(int)));
		connect(opacity_slider1, SIGNAL(valueChanged(int)),
			this, SLOT(opacity_slider_changed(int)));

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

	void load_DICOM_image(QDir dicom_dir, int plane_idx) {

		// rmb, 0 is a dummy idx to account for the volume viewport
		if (plane_idx == 0) {
			cout << "umm plane_idx should never be 0\n";
			return;
		}

		is_data1_loaded = false;
		cout << "loading data\n";

		// Read all the DICOM files in the specified directory.
		vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();

		// TODO: figure out a better way to convert QString to a char * to pass to SetDirectoryName
		reader->SetDirectoryName(dicom_dir.absolutePath().toStdString().c_str());

		// Force update, since we need to get information about the data dimensions
		reader->Update();

		int* dims = reader->GetOutput()->GetDimensions(); // Get the data dimensions
		double* range = reader->GetOutput()->GetScalarRange(); // Get the range of intensity values

		reslice_axes_arr[plane_idx] = vtkSmartPointer<vtkMatrix4x4>::New();
		reslice_axes_arr[plane_idx]->DeepCopy(plane_arr[plane_idx]);

		// vtkImageReslice is the filter that does the slicing (slices a 3D dataset to become 2D)
		reslice_arr[plane_idx] = vtkSmartPointer<vtkImageReslice>::New();
		reslice_arr[plane_idx]->SetInputConnection(reader->GetOutputPort()); // connect the reader to this filter
		reslice_arr[plane_idx]->SetOutputDimensionality(2);
		reslice_arr[plane_idx]->SetResliceAxes(reslice_axes_arr[plane_idx]); // tell it what plane to slice with
		reslice_arr[plane_idx]->SetInterpolationModeToLinear();
		reslice_arr[plane_idx]->Update();

		// Create an actor for the image. You'll notice we skipped the mapper step. This is because
		// image actors have a default mapper we can use as is if we don't want to change 
		// the colormap etc.
		iactor_arr[plane_idx] = vtkSmartPointer<vtkImageActor>::New();

		// Connect the reslice filter to the mapper
		iactor_arr[plane_idx]->GetMapper()->SetInputConnection(reslice_arr[plane_idx]->GetOutputPort());

		// Renderer
		renderer_arr[plane_idx] = vtkSmartPointer<vtkRenderer>::New();
		renderer_arr[plane_idx]->AddActor(iactor_arr[plane_idx]); // Add the actor to the renderer

		// Add the renderer to the render window
		window_arr[plane_idx]->AddRenderer(renderer_arr[plane_idx]);

		// Render (display the image)
		window_arr[plane_idx]->Render();

		// Ensure we aren't clipping any of the image (cameras have a front and back plane that 
		// clips for performance)
		renderer_arr[plane_idx]->ResetCameraClippingRange();

		int map[] = { -1, 2, 1, 0 };

		// Tell our slider widget what the min/max slice numbers are
		slider_arr[plane_idx]->setRange(0, dims[map[plane_idx]] - 1);

		cout << "finished loading data\n\n";
		is_data1_loaded = true;

	}

	/*
	Render DICOM as volume
	*/
	void load_DICOM_volume(QDir dicom_dir) {

		is_data1_loaded = false;
		cout << "loading data\n";

		//QDir dicom_dir = choose_directory();

		// Read all the DICOM files in the specified directory.
		vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();

		// TODO: figure out a better way to convert QString to a char * to pass to SetDirectoryName
		reader->SetDirectoryName(dicom_dir.absolutePath().toStdString().c_str());

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
		window_arr[0]->AddRenderer(renderer);
		window_arr[0]->Render();

		cout << "finished loading data\n";
		is_data1_loaded = true;
	}

public slots:


	void load_dset1() {

		// QDir dicom_dir = choose_directory();
		QDir dicom_dir = QDir("../data/VHF-Pelvis");

		load_DICOM_image(dicom_dir, AXIAL);
		load_DICOM_image(dicom_dir, CORONAL);
		load_DICOM_image(dicom_dir, SAGITTAL);
		load_DICOM_volume(dicom_dir);

		col0_heading->setText("Dataset 1: " + dicom_dir.dirName());
	}

	void load_dset2() {
		return;
	}


	void slice_slider_changed(int value) {

		if (!is_data1_loaded) {
			cout << "data not loaded yet!\n";
			return;
		}

		QObject* caller = sender();
		int plane_idx;

		if (caller == slider_arr[AXIAL])
			plane_idx = 1;
		else if (caller == slider_arr[CORONAL])
			plane_idx = 2;
		else if (caller == slider_arr[SAGITTAL])
			plane_idx = 3;

		int map[] = { -1, 2, 1, 0 };

		// Set the slice
		reslice_axes_arr[plane_idx]->SetElement(map[plane_idx], 3, value);
		reslice_arr[plane_idx]->Modified();

		// Update the slice label
		slider_label_arr[plane_idx]->setText(
			slice_label_texts[plane_idx] + QString::number(value)
		);

		// Re-render the image data
		window_arr[plane_idx]->Render();

	}

	void opacity_slider_changed(int value) {


		QObject* caller = sender();

		if (caller == opacity_slider0) {
			if (!is_data1_loaded) {
				cout << "dset1 not loaded yet!\n";
				return;
			
			}
			opacity_label0->setText("Opacity: " + QString::number(value));

			// change opacity for all image/slice actors
			for (int i = 1; i < NUM_VIEWPORTS; i++) {
				double opacity = ((double) value)/100;
				cout << std::to_string(opacity);
				iactor_arr[i]->SetOpacity(opacity);
				window_arr[i]->Render();
			}
		}

		else if (caller == opacity_slider1) {
			if (!is_data2_loaded) {
				cout << "dset2 not loaded yet!\n";
				return;
			}

			opacity_label1->setText("Opacity: " + QString::number(value));
		}
	}

};
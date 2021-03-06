/*
Author: Musab Shakeel
Date: 5/2022

This (header) file contains bulk of the code for a Qt application - a 
general purpose DICOM reader. The VTK library is used to implement
the visualization in this application. 

Notable features:
- Ability to load and view two separate DICOM datasets. The user is able to 
choose a directory for each dataset.
- The second dataset overlays on top of the first dataset.
- For each dataset, the axial/coronal/sagittal slices are shown in 3 separate 
viewports. 
- The volume rendering is also visualized in a separate viewport.
- User is able to change the opacity of the slice renderings.
- User is able to change the colormap for the volume renderings.
- The patient name (pulled from the DICOM data) is displayed for each dataset.

Pressing improvements/TODOs:
- Add ability to change window/level for the slice views.
- Add ability to specify custom color maps (for volume and slice views).
- Add ability to translate/move one DICOM dataset in a viewport (for some datasets, 
it becomes difficult to observe differences when both datasets are overlaid *exactly*
on top of each other).

Expected behavior:
- Dataset 1 and dataset 2 are loaded once, in whatever order. If dataset 1 is uploaded 
multiple times, behavior is undefined.
- The application is robust enough to never crash even with unexpected usage.

*/



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
#include <vtkImageMapper3D.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>

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
#include <QComboBox.h>


// Class that represents the main window for our application
class ui : public QMainWindow {

	Q_OBJECT
public:
	// constants
	int VOLUME = 0;
	int AXIAL = 1;    // xy- (setElement is 2,3)
	int CORONAL = 2;  // x-z (setElement is 1,3)
	int SAGITTAL = 3; // -yz (setElement is 0,1)

	double DSET2_OPACITY = 0.7;

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

	// vtk reslice filter, actors for dataset 2
	vtkSmartPointer<vtkImageReslice> reslice_arr2[NUM_VIEWPORTS];
	vtkSmartPointer<vtkImageActor> iactor_arr2[NUM_VIEWPORTS];

	// vtk volume property for dataset 1, 2
	vtkSmartPointer<vtkVolumeProperty> volume_property_arr[2];

	// colormap comboboxes
	QComboBox* color_combobox0, * color_combobox1;

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

	// LUTs for slice color maps
	vtkNew<vtkLookupTable> highContrastLut;
	vtkNew<vtkLookupTable> rainbowRedBlueLut;
	vtkNew<vtkLookupTable> rainbowBlueRedLut;
	vtkSmartPointer<vtkLookupTable> classExampleLut = vtkSmartPointer<vtkLookupTable>::New();
	vtkNew<vtkLookupTable> vtkExampleLut;
	vtkNew<vtkLookupTable> grayScaleLut;
	vtkNew<vtkLookupTable> customLut;

	// ctfs for volume color maps
	vtkSmartPointer<vtkColorTransferFunction> magma_ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
	vtkSmartPointer<vtkColorTransferFunction> viridis_ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
	vtkSmartPointer<vtkColorTransferFunction> class_example_ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
	vtkSmartPointer<vtkColorTransferFunction> grayscale_ctf = vtkSmartPointer<vtkColorTransferFunction>::New();

	// Populate the color transfer function VTK objects required for the 3D VTKVolume color maps.
	void populate_ctfs() {
		// magma
	   /* magma_ctf->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
		magma_ctf->AddRGBPoint(0.14, 0.094, 0.007, 0.286);
		magma_ctf->AddRGBPoint(0.28, 0.309, 0.0, 0.478);
		magma_ctf->AddRGBPoint(0.46, 0.576, 0.071, 0.502);
		magma_ctf->AddRGBPoint(0.63, 0.843, 0.271, 0.384);
		magma_ctf->AddRGBPoint(0.8, 0.976, 0.604, 0.388);
		magma_ctf->AddRGBPoint(1.0, 0.973, 1.0, 0.729);*/
		magma_ctf->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
		magma_ctf->AddRGBPoint(100, 0.094, 0.007, 0.286);
		magma_ctf->AddRGBPoint(150, 0.309, 0.0, 0.478);
		magma_ctf->AddRGBPoint(350, 0.576, 0.071, 0.502);
		magma_ctf->AddRGBPoint(400, 0.843, 0.271, 0.384);
		magma_ctf->AddRGBPoint(500, 0.976, 0.604, 0.388);
		magma_ctf->AddRGBPoint(550, 0.973, 1.0, 0.729);

		// viridis
		viridis_ctf->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
		viridis_ctf->AddRGBPoint(100, 0.28, 0.035, 0.396);
		viridis_ctf->AddRGBPoint(200, 0.223, 0.325, 0.556);
		viridis_ctf->AddRGBPoint(350, 0.058, 0.635, 0.529);
		viridis_ctf->AddRGBPoint(500, 1.0, 0.913, 0.0);

		// class_example_ctf
		class_example_ctf->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
		class_example_ctf->AddRGBPoint(300.0, 1.0, 0.0, 0.0);
		class_example_ctf->AddRGBPoint(555.0, 1.0, 1.0, 1.0);

		// grayscale_ctf
		grayscale_ctf->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
		grayscale_ctf->AddRGBPoint(100, 0.094, 0.094, 0.094);
		grayscale_ctf->AddRGBPoint(150, 0.309, 0.309, 0.309);
		grayscale_ctf->AddRGBPoint(350, 0.576, 0.576, 0.576);
		grayscale_ctf->AddRGBPoint(400, 0.843, 0.843, 0.843);
		grayscale_ctf->AddRGBPoint(500, 0.976, 0.976, 0.976);
		grayscale_ctf->AddRGBPoint(550, 0.963, 0.963, 0.963);
	}

	// Populate the VTK lookup tables (member variables) required for the 2D ImageActor color maps.
	void populate_luts() {

		// highContrastLut
		highContrastLut->SetNumberOfColors(256);
		highContrastLut->Build();

		for (int l = 0; l < 16; ++l)
		{
			highContrastLut->SetTableValue(l * 16, 1, 0, 0, 1);
			highContrastLut->SetTableValue(l * 16 + 1, 0, 1, 0, 1);
			highContrastLut->SetTableValue(l * 16 + 2, 0, 0, 1, 1);
			highContrastLut->SetTableValue(l * 16 + 3, 0, 0, 0, 1);
		}

		// ranbowRedBluelut
		rainbowRedBlueLut->SetNumberOfColors(256);
		rainbowRedBlueLut->SetHueRange(0.0, 0.667);
		rainbowRedBlueLut->Build();

		// rainbowBlueRedlut
		rainbowBlueRedLut->SetNumberOfColors(256);
		rainbowBlueRedLut->SetHueRange(0.667, 0.0);
		rainbowBlueRedLut->Build();

		// classExampleLut
		classExampleLut->SetNumberOfColors(256);
		classExampleLut->Build();
		for (int i = 0; i < 256; i++)
			classExampleLut->SetTableValue(i, (double)i / 255.0, 
				(double)i / 255.0, 
				(double)i / 255.0, 1.0); // value, red, green, blue, opacity
		classExampleLut->SetRange(0, 255);

		// vtkExampleLut
		vtkExampleLut->SetNumberOfTableValues(256);
		vtkExampleLut->SetRange(0.0, 255.0);
		vtkExampleLut->Build();

		// grayScaleLut
		grayScaleLut->SetHueRange(0, 0);
		grayScaleLut->SetSaturationRange(0, 0);
		grayScaleLut->SetValueRange(0.2, 1.0);
		grayScaleLut->SetNumberOfColors(256);
		grayScaleLut->SetHueRange(0.0, 0.667);
		grayScaleLut->Build();

		// customLut
		double m_mask_opacity = 1;
		customLut->SetRange(0, 4);
		customLut->SetRampToLinear();
		customLut->SetValueRange(0, 1);
		customLut->SetHueRange(0, 0);
		customLut->SetSaturationRange(0, 0);

		customLut->SetNumberOfTableValues(10);
		customLut->SetTableRange(0, 9);
		customLut->SetTableValue(0, 0, 0, 0, 0);
		customLut->SetTableValue(1, 1, 0, 0, m_mask_opacity);
		customLut->SetTableValue(2, 0, 1, 0, m_mask_opacity);
		customLut->SetTableValue(3, 1, 1, 0, m_mask_opacity);
		customLut->SetTableValue(4, 0, 0, 1, m_mask_opacity);
		customLut->SetTableValue(5, 1, 0, 1, m_mask_opacity);
		customLut->SetTableValue(6, 0, 1, 1, m_mask_opacity);
		customLut->SetTableValue(7, 1, 0.5, 0.5, m_mask_opacity);
		customLut->SetTableValue(8, 0.5, 1, 0.5, m_mask_opacity);
		customLut->SetTableValue(9, 0.5, 0.5, 1, m_mask_opacity);
		customLut->Build();

	};

	// Constructor (happens when created)
	ui() {

		// S==================== METADATA ==================== //

		// Resize the window, set the title
		this->setWindowTitle("DICOM Reader");
		//this->setGeometry(35, 35, 1200, 900);
		this->setWindowState(Qt::WindowMaximized);
		this->setMinimumSize(1200, 900);

		populate_ctfs();
		populate_luts();



		// S======================= WIDGETS ======================= //

		// Create the "central" (primary) widget for the window
		QWidget* widget = new QWidget();
		widget->setObjectName("central_widget");

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

			// initialize the vol + slice renderers
			renderer_arr[i] = vtkSmartPointer<vtkRenderer>::New();

		}

		// initialize sliders for each of the 3 slice planes
		for (int i = 1; i < NUM_VIEWPORTS; i++) {
			slider_arr[i] = new QSlider();
			slider_arr[i]->setOrientation(Qt::Vertical);

			// initialize the reslice_axes
			reslice_axes_arr[i] = vtkSmartPointer<vtkMatrix4x4>::New();
			reslice_axes_arr[i]->DeepCopy(plane_arr[i]);

			// initialize slice labels for the 3 planes
			slider_label_arr[i] = new QLabel(slice_label_texts[i]);
			slider_label_arr[i]->setAlignment(Qt::AlignCenter);
			slider_label_arr[i]->setStyleSheet(
				"padding: 2px; background-color: white; color: black; border-radius:3px"
			);
		}


		// dataset header labels
		col0_heading = new QLabel("Dataset 1: <no data loaded>");
		col0_heading->setObjectName("col0_heading");
		col0_heading->setAlignment(Qt::AlignCenter);

		col1_heading = new QLabel("Dataset 2: <no data loaded>");
		col1_heading->setObjectName("col1_heading");
		col1_heading->setAlignment(Qt::AlignCenter);

		// initialize opacity slider labels
		opacity_label0 = new QLabel("Slice Opacity: -");

		opacity_label1 = new QLabel("Slice Opacity: -");

		// initialize opacity sliders
		opacity_slider0 = new QSlider();
		opacity_slider0->setOrientation(Qt::Horizontal);
		opacity_slider0->setRange(0, 100);
		opacity_slider0->setValue(100);

		opacity_slider1 = new QSlider();
		opacity_slider1->setOrientation(Qt::Horizontal);
		opacity_slider1->setRange(0, 100);
		opacity_slider1->setValue(70);

		// initialize colormap comboboxes
		color_combobox0 = new QComboBox();
		color_combobox0->addItem("Map 1");
		color_combobox0->addItem("Viridis");
		color_combobox0->addItem("Magma");
		color_combobox0->addItem("Grayscale");


		color_combobox1 = new QComboBox();
		color_combobox1->addItem("Map 1");
		color_combobox1->addItem("Viridis");
		color_combobox1->addItem("Magma");
		color_combobox1->addItem("Grayscale");

		// initialize colormap combobox labels
		QLabel* color_combobox_label0 = new QLabel("Volume Color Map:");
		QLabel* color_combobox_label1 = new QLabel("Volume Color Map:");

		// initialize volume properties
		volume_property_arr[0] = vtkSmartPointer<vtkVolumeProperty>::New();
		volume_property_arr[1] = vtkSmartPointer<vtkVolumeProperty>::New();





		// S================ CREATE LAYOUTs ================ //
		QVBoxLayout* layout_vertical_main = new QVBoxLayout();
		QHBoxLayout* layout_row0 = new QHBoxLayout();
		QHBoxLayout* layout_row1 = new QHBoxLayout();
		QHBoxLayout* layout_row2 = new QHBoxLayout();

		// 2 vertical layouts for row0 for the two datasets
		QVBoxLayout* layout_col0 = new QVBoxLayout();
		QVBoxLayout* layout_col1 = new QVBoxLayout();

		// 2 horizontal layouts for opacity slider rows
		QHBoxLayout* layout_opacity_row0 = new QHBoxLayout();
		QHBoxLayout* layout_opacity_row1 = new QHBoxLayout();

		// 2 horizontal layouts for color combobx rows
		QHBoxLayout* layout_combobox_row0 = new QHBoxLayout();
		QHBoxLayout* layout_combobox_row1 = new QHBoxLayout();

		QVBoxLayout* layout_slice_label_arr[NUM_VIEWPORTS];

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

		// populate col0, col1
		layout_col0->addWidget(col0_heading, Qt::AlignCenter);
		layout_col0->addLayout(layout_opacity_row0);
		layout_col0->addLayout(layout_combobox_row0);
		layout_opacity_row0->addStretch();
		layout_opacity_row0->addWidget(opacity_label0);
		layout_opacity_row0->addWidget(opacity_slider0);
		layout_opacity_row0->addStretch();
		layout_combobox_row0->addStretch();
		layout_combobox_row0->addWidget(color_combobox_label0);
		layout_combobox_row0->addWidget(color_combobox0);
		layout_combobox_row0->addStretch();

		layout_col1->addWidget(col1_heading, Qt::AlignCenter);
		layout_col1->addLayout(layout_opacity_row1);
		layout_col1->addLayout(layout_combobox_row1);
		layout_opacity_row1->addStretch();
		layout_opacity_row1->addWidget(opacity_label1);
		layout_opacity_row1->addWidget(opacity_slider1);
		layout_opacity_row1->addStretch();
		layout_combobox_row1->addStretch();
		layout_combobox_row1->addWidget(color_combobox_label1);
		layout_combobox_row1->addWidget(color_combobox1);
		layout_combobox_row1->addStretch();

		// populate row1
		layout_row1->addSpacing(25); // no slider for volume view
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

		// connect combo boxes
		connect(color_combobox0, SIGNAL(currentIndexChanged(int)),
			this, SLOT(combobox_changed(int)));
		connect(color_combobox1, SIGNAL(currentIndexChanged(int)),
			this, SLOT(combobox_changed(int)));

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

	/*
	Used to render slices of the DICOM data.

	Args:
		dset_num: (int) either 1 or 2 (indicate whether loading dset1 or dset2)

	if dset_num = 2 then the following is a list of reused/new VTK pointers
	(new)
	- reslice_arr2
	- iactor_arr2

	(old)
	- reslice_axes_arr
	- renderer_arr
	- window_arr
	*/
	void load_DICOM_image(QDir dicom_dir, int plane_idx, int dset_num) {

		// S================== CHECK ARGUMENTS =================== //
		// rmb, 0 is a dummy idx to account for the volume viewport
		if (plane_idx == 0) {
			cout << "umm plane_idx should never be 0\n";
			return;
		}

		// sanity checking input
		if (!(dset_num == 1 || dset_num == 2)) {
			cout << "umm dset_num should be 1 or 2";
			return;
		}
		cout << "loading data\n";
		// E================== CHECK ARGUMENTS =================== //


		// S================== VTK PIPELINE =================== //
		// Reader -> ImageReslice -> ImageActor -> (ColorMapper) -> Renderer -> RenderWindow

		vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();

		// TODO: figure out a better way to convert QString to a char * to pass to SetDirectoryName
		reader->SetDirectoryName(dicom_dir.absolutePath().toStdString().c_str());
		reader->Update(); // Force update, since we need to get information about the data dimensions

		int* dims = reader->GetOutput()->GetDimensions(); // Get the data dimensions
		double* range = reader->GetOutput()->GetScalarRange(); // Get the range of intensity values

		// Create local pointers for the arrays containing the vtk reslice and actor objects. Then, based 
		// on whether dset1 or dset2 is being loaded, set these pointers appropriately (dset2 uses separate 
		// reslice and actor objects).
		vtkSmartPointer<vtkImageReslice>* curr_reslice_arr;
		vtkSmartPointer<vtkImageActor>* curr_iactor_arr;

		// maps plane_idx to the "missing" axis ( e.g. axial (plane_idx=1) misses z (2) )
		int map[] = { -1, 2, 1, 0 };

		if (dset_num == 1) {
			curr_reslice_arr = reslice_arr;
			curr_iactor_arr = iactor_arr;
			is_data1_loaded = false;

			// Tell our slice slider widget what the min/max slice numbers are (only once, for dset1)
			slider_arr[plane_idx]->setRange(0, dims[map[plane_idx]] - 1);
		}
		else {
			curr_reslice_arr = reslice_arr2;
			curr_iactor_arr = iactor_arr2;
			is_data2_loaded = false;
		}

		// Create an actor for the image. You'll notice we skipped the mapper step. This is because
		// image actors have a default mapper we can use as is if we don't want to change 
		// the colormap etc.
		curr_iactor_arr[plane_idx] = vtkSmartPointer<vtkImageActor>::New();

		// vtkImageReslice is the filter that does the slicing (slices a 3D dataset to become 2D)
		curr_reslice_arr[plane_idx] = vtkSmartPointer<vtkImageReslice>::New();
		curr_reslice_arr[plane_idx]->SetInputConnection(reader->GetOutputPort()); // connect the reader to this filter
		curr_reslice_arr[plane_idx]->SetOutputDimensionality(2);
		curr_reslice_arr[plane_idx]->SetResliceAxes(reslice_axes_arr[plane_idx]); // tell it what plane to slice with
		curr_reslice_arr[plane_idx]->SetInterpolationModeToLinear();
		curr_reslice_arr[plane_idx]->Update();


		// colormap mapper
		vtkSmartPointer<vtkImageMapToColors> imapper = vtkSmartPointer<vtkImageMapToColors>::New();
		imapper->PassAlphaToOutputOn();
		if (dset_num == 1) {
			grayScaleLut->SetRange(range);
			imapper->SetLookupTable(grayScaleLut);
		}
		else { // dset2
			customLut->SetRange(range);
			imapper->SetLookupTable(customLut);

			// set default opacity for dset2 slices
			curr_iactor_arr[plane_idx]->SetOpacity(DSET2_OPACITY);
		}

		imapper->SetInputConnection(curr_reslice_arr[plane_idx]->GetOutputPort());
		imapper->Update();

		// VTKMapper -> VTKImageActor
		curr_iactor_arr[plane_idx]->GetMapper()->SetInputConnection(imapper->GetOutputPort());


		// VTKImageActor -> VTKRenderer (initialized in constructor)
		renderer_arr[plane_idx]->AddActor(curr_iactor_arr[plane_idx]); // Add the actor to the renderer

		// VTKRenderer > VTKOpenGLRenderWindow
		window_arr[plane_idx]->AddRenderer(renderer_arr[plane_idx]);

		// Render (display the image)
		window_arr[plane_idx]->Render();

		// Ensure we aren't clipping any of the image (cameras have a front and back plane that 
		// clips for performance)
		renderer_arr[plane_idx]->ResetCameraClippingRange();
		renderer_arr[plane_idx]->ResetCamera();

		cout << "finished loading data\n\n";

		if (dset_num == 1) {
			is_data1_loaded = true;
		}
		else {
			is_data2_loaded = true;
		}

	}

	/*
	Render DICOM as volume
	*/
	void load_DICOM_volume(QDir dicom_dir, int dset_num) {

		// sanity checking input
		if (!(dset_num == 1 || dset_num == 2)) {
			cout << "umm dset_num should be 1 or 2";
			return;
		}

		cout << "loading data\n";

		if (dset_num == 1) {
			is_data1_loaded = false;
		}
		else {
			is_data2_loaded = false;
		}

		// Read all the DICOM files in the specified directory.
		vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
		reader->SetDirectoryName(dicom_dir.absolutePath().toStdString().c_str());
		reader->Update(); // Force update, since we need to get information about the data dimensions

		QString dset_name;

		if (dset_num == 1) {
			is_data1_loaded = false;
			dset_name = "Dataset 1: " + QString(reader->GetPatientName());
			col0_heading->setText(dset_name); // display patient name as dset name in GUI
		}
		else {
			is_data2_loaded = false;
			dset_name = "Dataset 2: " + QString(reader->GetPatientName());
			col1_heading->setText(dset_name);
		}

		/* Code taken from in-class example */

		// volume mapper
		vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
		volumeMapper->SetBlendModeToComposite(); // composite
		volumeMapper->SetInputConnection(reader->GetOutputPort());
		//volumeMapper->SetRequestedRenderModeToRayCast();
		volumeMapper->SetRequestedRenderModeToGPU();


		// volume properties
		volume_property_arr[dset_num-1]->ShadeOff();
		volume_property_arr[dset_num - 1]->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

		// opacity
		vtkSmartPointer<vtkPiecewiseFunction> opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
		opacity->AddPoint(0.0, 0.0);
		opacity->AddPoint(90.0, 0.05);
		opacity->AddPoint(555.0, 0.1);
		volume_property_arr[dset_num - 1]->SetScalarOpacity(opacity);

		// colormap
		if (dset_num == 1) {
			volume_property_arr[dset_num - 1]->SetColor(grayscale_ctf);
		}
		else {
			volume_property_arr[dset_num - 1]->SetColor(magma_ctf);
		}
		

		// VolumeMapper, VolumeProperty -> Volume
		vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
		volume->SetMapper(volumeMapper);
		volume->SetProperty(volume_property_arr[dset_num - 1]);

		// Volume -> Renderer
		renderer_arr[0]->AddViewProp(volume);
		renderer_arr[0]->ResetCamera();

		// Renderer -> VTKOpenGLRenderWindow
		window_arr[0]->AddRenderer(renderer_arr[0]);
		window_arr[0]->Render();

		cout << "finished loading data\n";

		if (dset_num == 1) {
			is_data1_loaded = true;
		}
		else {
			is_data2_loaded = true;
		}
	}

	// Check that directory is valid.
	bool is_valid(QDir dicom_dir) {


		QTextStream qt_cout(stdout);
		qt_cout << dicom_dir.path();

		if (!dicom_dir.exists() || dicom_dir.count() == 0) {
			cout << "wrong directory!";
			return false;
		}

		// directory not changed/dialog box closed
		if (QString::compare(dicom_dir.path(), ".") == 0) {
			return false;
		}

		return true;
	}



public slots:


	void load_dset1() {

		QDir dicom_dir = choose_directory();
		//QDir dicom_dir = QDir("../data/hw2-dataset");

		if (!is_valid(dicom_dir))
			return;


		load_DICOM_image(dicom_dir, AXIAL, 1);
		load_DICOM_image(dicom_dir, CORONAL, 1);
		load_DICOM_image(dicom_dir, SAGITTAL, 1);
		load_DICOM_volume(dicom_dir, 1);

		// ==== Restore UI elements to default positions after loading data 
		// (useful in case user messed with UI elements before loading data)

		// slice sliders
		for (int i = 1; i < NUM_VIEWPORTS; i++) {
			slider_arr[i]->setValue(0);
		}

		// opacity sliders
		opacity_label0->setText("Slice Opacity: 100");
		opacity_slider0->setValue(100);

		// colormap combobox
		color_combobox0->setCurrentIndex(3); //grayscale

	}

	void load_dset2() {

		QDir dicom_dir = choose_directory();
		//QDir dicom_dir = QDir("../data/VHF-Pelvis");

		if (!is_valid(dicom_dir))
			return;

		load_DICOM_image(dicom_dir, AXIAL, 2);
		load_DICOM_image(dicom_dir, CORONAL, 2);
		load_DICOM_image(dicom_dir, SAGITTAL, 2);
		load_DICOM_volume(dicom_dir, 2);

		// ==== Restore UI elements to default positions after loading data 
		// (useful in case user messed with UI elements before loading data)
		opacity_label1->setText("Slice Opacity: " + QString::number(DSET2_OPACITY * 100));
		opacity_slider1->setValue(DSET2_OPACITY * 100);

		// colormap combobox
		color_combobox1->setCurrentIndex(2); // magma
	}


	void slice_slider_changed(int value) {

		if (!is_data1_loaded) {
			cout << "data not loaded yet!\n";
			return;
		}

		QObject* caller = sender(); // determine axial/coronal/sagittal slice
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

	/*
	Defines behavior for when opacity slider values are changed by dragging the slider.
	Note that each of dset1 and dset2 have their own opacity slider. This fxn is called
	when either of the two sliders are changed. The caller slider is checked, and based on
	the caller, the appropriate ImageActor's opacity is changed:

		ImageActor->SetOpacity(0.5);

	As usual, the appropriate VTKWindow needs to be re-rendered:

		VtkWindow->Render();
	*/
	void opacity_slider_changed(int value) {


		QObject* caller = sender(); // determine dset1/dset2 opacity slider

		if (caller == opacity_slider0) {
			if (!is_data1_loaded) {
				cout << "dset1 not loaded yet!\n";
				return;

			}
			opacity_label0->setText("Slice Opacity: " + QString::number(value));

			// change opacity for all image/slice actors
			for (int i = 1; i < NUM_VIEWPORTS; i++) {
				double opacity = ((double)value) / 100;
				iactor_arr[i]->SetOpacity(opacity);
				window_arr[i]->Render();
			}
		}

		else if (caller == opacity_slider1) {
			if (!is_data2_loaded) {
				cout << "dset2 not loaded yet!\n";
				return;

			}
			opacity_label1->setText("Slice Opacity: " + QString::number(value));

			// change opacity for all image/slice actors
			for (int i = 1; i < NUM_VIEWPORTS; i++) {
				double opacity = ((double)value) / 100;
				iactor_arr2[i]->SetOpacity(opacity);
				window_arr[i]->Render();
			}
		}
	}

	void combobox_changed(int new_index) {

		QObject* caller = sender(); // determine dset1/dset2 opacity slider

		int idx = (caller == color_combobox0) ? 0 : 1;

		vtkSmartPointer<vtkColorTransferFunction> map[4] = {
			class_example_ctf, viridis_ctf, magma_ctf, grayscale_ctf };

		volume_property_arr[idx]->SetColor(map[new_index]);

		window_arr[0]->Render();
	}


};
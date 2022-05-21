
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

// OpenCV header files
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace cv; // This means we don't have to write cv:: for every object or function in opencv that we use

// Class that represents the main window for our application
class ui : public QMainWindow
{
	Q_OBJECT
public:
	// Declare three QVTKOpenGLNativeWidget here
	QVTKOpenGLNativeWidget* viewport1, * viewport2, * viewport3;

	// Declare buttons as member variables
	QPushButton* button1, * button2, * button3;

	// Declare slider as member variable
	QSlider* slider1, * slider2;
	int threshold_val1 = 40; // default 40
	int threshold_val2 = 40;

	// Declare label as member variable
	QLabel* label1, * label2;

	// Delcare three vtkGenericOpenGLRenderWindow here
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> window1, window2, window3;

	// Declare layouts
	QHBoxLayout* layout_horizontal, * layout_horizontal2, * layout_horizontal3;
	QVBoxLayout* layout_col1_load_img, * layout_vertical2, * layout_vertical3;

	// Threshold filter
	vtkSmartPointer<vtkImageThreshold> threshold_filter;

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

		// Create buttons
		button1 = new QPushButton("Load Data");
		button2 = new QPushButton("Threshold");
		button3 = new QPushButton("Find Circles");
		QPushButton* button_choose_dir = new QPushButton("Choose Directory");

		// Create sliders
		slider1 = new QSlider();
		slider1->setOrientation(Qt::Horizontal);
		slider1->setRange(0, 255);
		slider1->setValue(threshold_val1);

		slider2 = new QSlider();
		slider2->setOrientation(Qt::Horizontal);
		slider2->setRange(0, 255);
		slider2->setValue(threshold_val2);

		// Create a text label widgets
		label1 = new QLabel(QString::number(threshold_val1));
		label1->setFixedSize(60, 20);

		// Create a text label widget
		label2 = new QLabel(QString::number(threshold_val2));
		label2->setFixedSize(60, 20);

		// Create viewports and windows
		viewport1 = new QVTKOpenGLNativeWidget();
		viewport1->enableHiDPI(); // support for high DPI screens
		window1 = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		viewport1->SetRenderWindow(window1);
		viewport1->setMinimumSize(400, 400);

		viewport2 = new QVTKOpenGLNativeWidget();
		viewport2->enableHiDPI(); // support for high DPI screens
		window2 = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		viewport2->SetRenderWindow(window2);
		viewport2->setMinimumSize(400, 400);

		viewport3 = new QVTKOpenGLNativeWidget();
		viewport3->enableHiDPI(); // support for high DPI screens
		window3 = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		viewport3->SetRenderWindow(window3);
		viewport3->setMinimumSize(400, 400);



		// S================ CREATE LAYOUTs ================ //

		QVBoxLayout* layout_vertical_main = new QVBoxLayout();
		widget->setLayout(layout_vertical_main);

		QHBoxLayout* layout_row1 = new QHBoxLayout();
		QHBoxLayout* layout_row2 = new QHBoxLayout();

		layout_horizontal2 = new QHBoxLayout();
		layout_horizontal3 = new QHBoxLayout();;

		layout_col1_load_img = new QVBoxLayout();
		layout_vertical2 = new QVBoxLayout();
		layout_vertical3 = new QVBoxLayout();

		// S========== ADD WIDGETS/LAYOUTS TO LAYOUTs ========= //
		layout_vertical_main->addLayout(layout_row1);
		layout_vertical_main->addLayout(layout_row2);

		layout_row1->addLayout(layout_col1_load_img);
		layout_row1->addLayout(layout_vertical2);
		layout_row1->addLayout(layout_vertical3);

		layout_row2->addWidget(button_choose_dir);

		layout_col1_load_img->addWidget(viewport1);
		layout_col1_load_img->addWidget(button1);

		layout_vertical2->addWidget(viewport2);
		layout_vertical2->addWidget(button2);
		layout_vertical2->addLayout(layout_horizontal2);

		layout_horizontal2->addWidget(label1);
		layout_horizontal2->addWidget(slider1);

		layout_horizontal3->addWidget(label2);
		layout_horizontal3->addWidget(slider2);

		layout_vertical3->addWidget(viewport3);
		layout_vertical3->addWidget(button3);
		layout_vertical3->addLayout(layout_horizontal3);

		// Connect widget signals to your slots (functions)
		connect(button1, SIGNAL(released()), this, SLOT(load_data()));
		connect(button2, SIGNAL(released()), this, SLOT(threshold_slot()));
		connect(button3, SIGNAL(released()), this, SLOT(find_circles()));
		connect(slider1, SIGNAL(valueChanged(int)), this, SLOT(slider1_changed(int)));
		connect(slider2, SIGNAL(valueChanged(int)), this, SLOT(slider2_changed(int)));

		connect(button_choose_dir, SIGNAL(released()), this, SLOT(choose_directory()));

		// Display the window
		this->show();
	}

	void load_data_generic(char* path, vtkSmartPointer<vtkGenericOpenGLRenderWindow> window) {

		cout << "loading data\n";

		// Read image file from specified directory
		vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();

		// Specify where the image file lives
		reader->SetFileName(path);

		// Create an actor for the image, connect reader's output to actor's input
		vtkSmartPointer<vtkImageActor> iactor = vtkSmartPointer<vtkImageActor>::New();
		iactor->GetMapper()->SetInputConnection(reader->GetOutputPort());

		// vtkRenderer
		vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
		renderer->AddActor(iactor);

		// Add the renderer to the render window
		window->AddRenderer(renderer);

		// Render (display the image)
		window->Render();

		// Ensure we aren't clipping any of the image (cameras have a front and back plane that 
		// clips for performance)
		renderer->ResetCameraClippingRange();

		cout << "finished loading data\n";

	}

public slots:

	void load_data() {
		load_data_generic("../data/image.png", window1);
	}

	void threshold_slot() {
		cout << "loading data\n";

		// Read image file from specified directory
		vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();

		// Specify where the image file lives
		reader->SetFileName("../data/image.png");

		// Threshold the image data - in this case we are saying any value above 40 will be 
		// set to 255
		vtkSmartPointer<vtkImageThreshold> threshold_filter = vtkSmartPointer<vtkImageThreshold>::New();
		threshold_filter->SetInputConnection(reader->GetOutputPort());
		threshold_filter->ThresholdByUpper(threshold_val1);
		threshold_filter->ReplaceInOn();
		threshold_filter->SetInValue(255);

		// Create an actor for the image, connect reader's output to actor's input
		vtkSmartPointer<vtkImageActor> iactor = vtkSmartPointer<vtkImageActor>::New();
		iactor->GetMapper()->SetInputConnection(threshold_filter->GetOutputPort());

		// vtkRenderer
		vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
		renderer->AddActor(iactor);

		// Add the renderer to the render window
		window2->AddRenderer(renderer);

		// Render (display the image)
		window2->Render();

		// Ensure we aren't clipping any of the image (cameras have a front and back plane that 
		// clips for performance)
		renderer->ResetCameraClippingRange();

		cout << "finished loading data\n";

	}

	void find_circles() {

		// Load the PNG image in OpenCV format (Mat)
		Mat image = imread("../data/image.png", cv::IMREAD_COLOR);

		// Threshold the image using OpenCV - any value above 40 will be set to 255
		Mat image_th;
		cv::threshold(image, image_th, threshold_val2, 255, THRESH_BINARY);

		/* Find circles in the image */

		// Setup the blob (circle) detector
		SimpleBlobDetector::Params params;
		params.filterByArea = true;
		params.minArea = 4;
		params.maxArea = 100000;
		params.filterByColor = true;
		params.blobColor = 255;
		params.filterByCircularity = false;
		params.minCircularity = 0.75;
		params.filterByConvexity = false;
		params.minConvexity = 0.65;
		params.minDistBetweenBlobs = 0.0;

		Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

		// Run the detector
		std::vector<KeyPoint> blobs;
		detector->detect(image_th, blobs);

		// Draw a circle around each one that's found
		for (int i = 0; i < blobs.size(); ++i)
		{
			Point center = blobs[i].pt; // This is the center of the circle
			int radius = blobs[i].size / 2.0; // This is the radius
			circle(image_th, center, radius, Scalar(255, 0, 255), 8, LINE_AA);
			// Draw a purple circle on the image
		}

		// Save the image to a new PNG file
		imwrite("../data/circles.png", image_th);

		// Open and display the new PNG file in the third viewport
		load_data_generic("../data/circles.png", window3);

	}

	void slider1_changed(int value) {
		threshold_val1 = value;
		label1->setText(QString::number(value));
	}

	void slider2_changed(int value) {
		threshold_val2 = value;
		label2->setText(QString::number(value));
	}

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

};
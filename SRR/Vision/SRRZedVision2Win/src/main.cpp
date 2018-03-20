/**************************************************************************************************
 ** This sample demonstrates how to grab images and depth/disparity map with the ZED SDK          **
 ** Both images and depth/disparity map are displayed with OpenCV                                 **
 ** Most of the functions of the ZED SDK are linked with a key press event (using opencv)         **
 ***************************************************************************************************/

#include "stdafx.h"

//My stuff
#include "OCVCamera.h"
#include "ZEDCamera.h"
#include "ThresholdDetecter.h"

#include "../SmartDashboard/SmartDashboard_import.h"
#include "SmartDashbrdMode.h"

/** forward declarations **/
void detectHookSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);
void detectBeacon(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);
void printInfo(ThresholdDetecter &, ZEDCamera &);
void printHelp();

extern cv::CascadeClassifier hook_cascade;

#define FRONT_CAM_URL ""
//#define FRONT_CAM_URL "http://ctetrick.no-ip.org/videostream.cgi?user=guest&pwd=watchme&resolution=32&rate=0"
//#define FRONT_CAM_RUL "rtsp://root:root@192.168.0.90/axis-media/media.amp"


//Define the structure and callback for mouse event
typedef struct mouseOCVStruct {
	cv::Mat image;
	cv::Size _resize;
	int3 low, high;
	int hit_x, hit_y;
	bool update = false;
} mouseOCV;

mouseOCV mouseStruct;

static void onMouseCallback(int32_t event, int32_t x, int32_t y, int32_t flag, void * param) {

	if (param == NULL) return;

	mouseOCVStruct* data = (mouseOCVStruct*)param;

	if ((data->_resize.width <= 0 || data->_resize.height <= 0) ||
		(data->image.cols == 0 || data->image.rows == 0))
		return;

	int x_int = (x * data->image.cols / data->_resize.width);
	int y_int = (y * data->image.rows / data->_resize.height);

	if (flag == CV_EVENT_FLAG_MBUTTON)
	{	// middle button sets hit location
		if (x_int < 0) x_int = -1;
		if (x_int > data->image.cols) x_int = -1; // data->image.cols;
		if (y_int < 0) y_int = -1;
		if (y_int > data->image.rows) y_int = -1; // data->image.rows;

		data->hit_x = x_int;
		data->hit_y = y_int;
	}
	else
	{
		data->hit_x = -1;
		data->hit_y = -1;
	}

	if (flag == (CV_EVENT_FLAG_LBUTTON | CV_EVENT_FLAG_RBUTTON))
	{	// reset if both down
		data->low.x = 255; data->low.y = 255; data->low.z = 255;
		data->high.x = 0;  data->high.y = 0;  data->high.z = 0;
		std::cout << "mouse pick reset." << std::endl;
		data->update = true;
	}
	else if (event == CV_EVENT_LBUTTONDOWN) 
	{	// make the range wider
		//convert the img from color to hsv
		cv::Mat hsv;
		cv::cvtColor(data->image, hsv, CV_BGR2HSV);

		// find our 9x9 area
		int x_low = x_int - 4;
		int x_high = x_int + 4;
		int y_low = y_int - 4;
		int y_high = y_int + 4;

		if (x_low < 0) x_low = 0;
		if (x_high > data->image.cols) x_high = data->image.cols;
		if (y_low < 0) y_low = 0;
		if (y_high > data->image.rows) y_high = data->image.rows;

		// there has to be a more efficient way to do this
		for (int i = y_low; i < y_high; i++)
		{
			for (int j = x_low; j < x_high; j++)
			{	// find low high values
				cv::Vec3b intensity = hsv.at<cv::Vec3b>(i, j);
				if ((int)intensity.val[0] < data->low.x)  data->low.x = (int)intensity.val[0];
				if ((int)intensity.val[1] < data->low.y)  data->low.y = (int)intensity.val[1];
				if ((int)intensity.val[2] < data->low.z)  data->low.z = (int)intensity.val[2];

				if ((int)intensity.val[0] > data->high.x)  data->high.x = (int)intensity.val[0];
				if ((int)intensity.val[1] > data->high.y)  data->high.y = (int)intensity.val[1];
				if ((int)intensity.val[2] > data->high.z)  data->high.z = (int)intensity.val[2];
			}
		}

		std::cout << "H: " << data->low.x << " - " << data->high.x << std::endl;
		std::cout << "S: " << data->low.y << " - " << data->high.y << std::endl;
		std::cout << "V: " << data->low.z << " - " << data->high.z << std::endl;
		std::cout << std::endl;
		data->update = true;
	}
	else if (flag == CV_EVENT_FLAG_RBUTTON)
	{	// show value at click
		//convert the img from color to hsv
		cv::Mat hsv;
		cv::cvtColor(data->image, hsv, CV_BGR2HSV);

		if (x_int < 0) x_int = 0;
		if (x_int > data->image.cols) x_int = data->image.cols;
		if (y_int < 0) y_int = 0;
		if (y_int > data->image.rows) y_int = data->image.rows;

		cv::Vec3b intensity = hsv.at<cv::Vec3b>(y_int, x_int);
		std::cout << "H: " << (int)intensity.val[0] << std::endl;
		std::cout << "S: " << (int)intensity.val[1] << std::endl;
		std::cout << "V: " << (int)intensity.val[2] << std::endl;
		std::cout << std::endl;
	}
}

// trackbar callbacks
static void on_HLowChange(int val, void* param)
{
	mouseOCVStruct* data = (mouseOCVStruct*)param;

	data->low.x = val;
	data->update = true;
}

static void on_HHighChange(int val, void* param)
{
	mouseOCVStruct* data = (mouseOCVStruct*)param;

	data->high.x = val;
	data->update = true;
}

static void on_SLowChange(int val, void* param)
{
	mouseOCVStruct* data = (mouseOCVStruct*)param;

	data->low.y = val;
	data->update = true;
}

static void on_SHighChange(int val, void* param)
{
	mouseOCVStruct* data = (mouseOCVStruct*)param;

	data->high.y = val;
	data->update = true;
}

static void on_VLowChange(int val, void* param)
{
	mouseOCVStruct* data = (mouseOCVStruct*)param;
	
	data->low.z = val;
	data->update = true;
}

static void on_VHighChange(int val, void* param)
{
	mouseOCVStruct* data = (mouseOCVStruct*)param;

	data->high.z = val;
	data->update = true;
}


float GetDistanceAtPoint(sl::Mat depth, size_t x, size_t y)
{
	sl::float1 dist;
	depth.getValue(x, y, &dist);
	return dist;
}

enum Camera_Mode{
	Idle, 
	FindHook,
	FindRock,
	FindBeacon,
	PassThrough
};

enum ActiveCam {
	No_Cam,
	Stereo_Cam,
	Front_Cam
};

std::string filename1;
std::string filename2;
ActiveCam activeCamera = No_Cam;
bool SmallWindow = true;
int cam1_op_mode = FindRock;
int cam2_op_mode = PassThrough;
size_t SmartDashboard_Mode = 0;
size_t width = 1280;
size_t height = 720;


/** Cascade classifire data */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
std::string hook_cascade_name = "bin/data/SRR Samples/cascades/hook_cascade_gpu.xml";


//main  function
//mode 0 = robot
//mode 1 = simulation
//mode 2 = stand alone (runs directly with SmartDashboard UI)
//SRRZedVision.exe [mode=0] [filename *.svo for stereo cam, *.mpg or other supported format for front cam.]
int main(int argc, char **argv) {

    //-- 1. Load the cascades
    if (!hook_cascade.load(hook_cascade_name)){	
		std::cout << "--(!)Error loading cascade data" << std::endl; 
		return -1; 
    };

	int3 low;
	int3 high;
	low.x = low.y = low.z = 255;
	high.x = high.y = high.z = 0;

	// set up detectors
	int3 HSV_low;
	int3 HSV_high;
	HSV_low.x = 122; HSV_low.y = 50; HSV_low.z = 90;
	HSV_high.x = 155; HSV_high.y = 255; HSV_high.z = 255;
	ThresholdDetecter ThresholdDet(HSV_low, HSV_high);

	int key = ' ';
	int count = 0;

	filename1 = "";
	filename2 = FRONT_CAM_URL;

	if (argc == 3)
	{
		filename1 = argv[2];
		// if this is not a Zed SVO file, use it for the other camera.
		if (filename1.find(".svo") == std::string::npos)
		{
			filename2 = filename1;
			filename1 = "";
		}
	}

	std::cout << "Initializing OCV Camera." << std::endl;
	OCVCamera FrontCam = OCVCamera(filename2.c_str());

	std::cout << "Initializing ZED Camera." << std::endl;
	ZEDCamera StereoCam = ZEDCamera(filename1.c_str());

	if (!FrontCam.IsOpen && !StereoCam.IsOpen)
	{
		std::cout << "No Cameras!" << std::endl;
		Sleep(5000);
		return -1;
	}

	if (argc > 1)
		SmartDashboard_Mode = atoi(argv[1]);
	switch (SmartDashboard_Mode)
	{
	case 0:
		SmartDashboard::SetClientMode();
		SmartDashboard::SetIPAddress("10.34.81.99");  //robot
		break;
	case 1:
		//But set as client if testing with a robot server (e.g. robot simulation)
		SmartDashboard::SetClientMode();
		SmartDashboard::SetIPAddress("127.0.0.1");   //localhost
		break;
	case 2:
		//Run as a server to test directly to SmartDashboard UI locally
		SmartDashboard::SetIPAddress("127.0.0.1");   //localhost
		break;
	}
	SmartDashboard::init();

	if (StereoCam.IsOpen)
	{
		width = StereoCam.image_size.width;
		height = StereoCam.image_size.height;
	}
	else if (FrontCam.IsOpen)
	{
		width = FrontCam.width;
		height = FrontCam.height;
	}
	cv::Size displaySize((int)(SmallWindow ? width / 2 : width), (int)(SmallWindow ? height / 2 : height));

	// these are for stereo.
	cv::Mat anaplyph((int)height, (int)width, CV_8UC4);
	cv::Mat confidencemap((int)height, (int)width, CV_8UC4);
	sl::Mat depth;
	sl::Mat point_cloud;

	//create Opencv Window and mouse handler
	cv::namedWindow("VIEW", cv::WINDOW_AUTOSIZE);
	mouseStruct._resize = displaySize;
	mouseStruct.low = low;
	mouseStruct.high = high;
	mouseStruct.hit_x = mouseStruct.hit_y = -1;
	cv::setMouseCallback("VIEW", onMouseCallback, (void*)&mouseStruct);

	// make a controls window
	cv::namedWindow("Controls", cv::WINDOW_AUTOSIZE);
	cv::createTrackbar("H Low", "Controls", &mouseStruct.low.x, 255, on_HLowChange, &mouseStruct);
	cv::createTrackbar("H High", "Controls", &mouseStruct.high.x, 255, on_HHighChange, &mouseStruct);
	cv::createTrackbar("S Low", "Controls", &mouseStruct.low.y, 255, on_SLowChange, &mouseStruct);
	cv::createTrackbar("S High", "Controls", &mouseStruct.high.y, 255, on_SHighChange, &mouseStruct);
	cv::createTrackbar("V Low", "Controls", &mouseStruct.low.z, 255, on_VLowChange, &mouseStruct);
	cv::createTrackbar("V High", "Controls", &mouseStruct.high.z, 255, on_VHighChange, &mouseStruct);

	if (StereoCam.IsOpen)
	{
		// Mouse callback initialization
		activeCamera = Stereo_Cam;
		StereoCam.GrabDepth();
		mouseStruct.image = StereoCam.frame;
	}
	else
	{
		if (FrontCam.IsOpen)
		{
			mouseStruct.image = FrontCam.GrabFrame();
			activeCamera = Front_Cam;
		}
	}

	// Print help in console
	printHelp();

    //loop until 'q' is pressed
    while (key != 'q') 
	{
		/***** main video loop *****/
		if (activeCamera == Stereo_Cam)
		{	
			static SmartDashboard_ModeManager s_SmartDashboard_ModeManager(cam1_op_mode,"ZedMode",cam1_op_mode);
			s_SmartDashboard_ModeManager();  //update

  		    // update
			if (cam1_op_mode != Idle)
			{
				StereoCam.GrabFrameAndDapth();
				anaplyph = StereoCam.frame;
				depth = StereoCam.depth;
				point_cloud = StereoCam.point_cloud;
			}
			displaySize.height = (int)(SmallWindow ? height / 2 : height);
			displaySize.width = (int)(SmallWindow ? width / 2 : width);
			mouseStruct.image = anaplyph;
			mouseStruct._resize = displaySize;

			// Get frames and launch the computation
			if (StereoCam.bHaveFrame)
			{
				/***************  PROCESS:  ***************/
				switch (cam1_op_mode)
				{
				case FindHook:
					detectHookSample(anaplyph, depth, point_cloud);
					break;
				case FindRock:
				{
					if (mouseStruct.update)
					{
						ThresholdDet.setThreshold(mouseStruct.low, mouseStruct.high);
						cv::setTrackbarPos("H Low", "Controls", mouseStruct.low.x);
						cv::setTrackbarPos("H High", "Controls", mouseStruct.high.x);
						cv::setTrackbarPos("S Low", "Controls", mouseStruct.low.y);
						cv::setTrackbarPos("S High", "Controls", mouseStruct.high.y);
						cv::setTrackbarPos("V Low", "Controls", mouseStruct.low.z);
						cv::setTrackbarPos("V High", "Controls", mouseStruct.high.z);
						mouseStruct.update = false;
					}

					cv::Point mhit(mouseStruct.hit_x, mouseStruct.hit_y);
					ThresholdDet.detectRockSample(anaplyph, depth, point_cloud, mhit, SmallWindow);
					break;
				}
				case FindBeacon:
					detectBeacon(anaplyph, depth, point_cloud);
					break;
				default:
					break;
				}

				/***************  DISPLAY:  ***************/
				if (cam1_op_mode != Idle)
				{
					if (SmallWindow)
						resize(anaplyph, anaplyph, displaySize);
					imshow("VIEW", anaplyph);
				}
			}
		}

		if (activeCamera == Front_Cam)
		{
			cv::Mat frame = FrontCam.GrabFrame();
			displaySize.height = (int)(SmallWindow ? height / 2 : height);
			displaySize.width = (int)(SmallWindow ? width / 2 : width);
			mouseStruct.image = frame;
			mouseStruct._resize = displaySize;

			/***************  PROCESS:  ***************/
			switch (cam2_op_mode)
			{
			case FindHook:
				detectHookSample(frame, depth, point_cloud);	// TODO: no point cloud or depth for front cam.
				break;
			case FindRock:
			{
				if (mouseStruct.update)
				{
					ThresholdDet.setThreshold(mouseStruct.low, mouseStruct.high);
					cv::setTrackbarPos("H Low", "Controls", mouseStruct.low.x);
					cv::setTrackbarPos("H High", "Controls", mouseStruct.high.x);
					cv::setTrackbarPos("S Low", "Controls", mouseStruct.low.y);
					cv::setTrackbarPos("S High", "Controls", mouseStruct.high.y);
					cv::setTrackbarPos("V Low", "Controls", mouseStruct.low.z);
					cv::setTrackbarPos("V High", "Controls", mouseStruct.high.z);
					mouseStruct.update = false;
				}

				cv::Point mhit(mouseStruct.hit_x, mouseStruct.hit_y);
				ThresholdDet.detectRockSample(frame, depth, point_cloud, mhit, SmallWindow);
				break;
			}
			case FindBeacon:
				detectBeacon(frame, depth, point_cloud);
				break;
			default:
				break;
			}

			/***************  DISPLAY:  ***************/
			if (cam1_op_mode != Idle && frame.rows > 0 && frame.cols > 0)
			{
				if (SmallWindow)
					resize(frame, frame, displaySize);
				cv::imshow("VIEW", frame);
			}
		}

		/** end of main video loop **/

        key = cv::waitKey(5);
 
        // Keyboard shortcuts
        switch (key) {
			// common keys
			case 'h':
				printHelp();
				break;

			case '?':
				printInfo(ThresholdDet, StereoCam);
				break;

			case 'z':
				SmallWindow = !SmallWindow;
				break;

			case 'c':
				switch (activeCamera)
				{
				case No_Cam:
					std::cout << "No camera enabled" << std::endl;
					break;
				case Stereo_Cam:
					if (FrontCam.IsOpen)
					{
						activeCamera = Front_Cam;
						std::cout << "Front camera enabled" << std::endl;
					}
					break;
				case Front_Cam:
					if (StereoCam.IsOpen)
					{
						activeCamera = Stereo_Cam;
						std::cout << "Stereo camera enabled" << std::endl;
					}
					break;
				default:
					break;
				}
				break;

			// threshold values
			case 'i':	// increase increment
			case 'I':	// decrease increment
			case 'S':	// setting
			case 'R':	// reset
			case '<':	// decrease
			case '>':	// increase
			case 'Z':	// full range (0 - 255)
			case 0x00700000:	// function keys - 1st 4 save, 2nd 4 load
			case 0x00710000:
			case 0x00720000:
			case 0x00730000:
			case 0x00740000:
			case 0x00750000:
			case 0x00760000:
			case 0x00770000:
				// only change values if we are in that mode.
				if (cam1_op_mode == FindRock || cam2_op_mode == FindRock)
				{
					ThresholdDet.updateThresholdSettings(key);
					std::pair<int3, int3> threshold = ThresholdDet.getThreshold();
					mouseStruct.low = threshold.first;
					mouseStruct.high = threshold.second;
					cv::setTrackbarPos("H Low", "Controls", mouseStruct.low.x);
					cv::setTrackbarPos("H High", "Controls", mouseStruct.high.x);
					cv::setTrackbarPos("S Low", "Controls", mouseStruct.low.y);
					cv::setTrackbarPos("S High", "Controls", mouseStruct.high.y);
					cv::setTrackbarPos("V Low", "Controls", mouseStruct.low.z);
					cv::setTrackbarPos("V High", "Controls", mouseStruct.high.z);
				}
				break;

			// ZED
			//re-compute stereo alignment
			case 'a':
				if( activeCamera == Stereo_Cam)
					StereoCam.ResetCalibration();
				break;

				//Change camera settings 
			case 's':	// setting
			case 'r':	// reset (all)
			case '+':	// increase
			case '-':	// decrease
			case 0x00780000:
			case 0x00790000:
				if (activeCamera == Stereo_Cam)
					StereoCam.updateCameraSettings(key);
				break;

				// ______________  VIEW __________________
            case '0':	
				if (activeCamera == Stereo_Cam)
					StereoCam.ViewID = sl::VIEW_LEFT; 
				break;
            case '1':
				if (activeCamera == Stereo_Cam)
					StereoCam.ViewID = sl::VIEW_RIGHT; 
				break;
            case '2':	
				if (activeCamera == Stereo_Cam)
					StereoCam.ViewID = sl::VIEW_LEFT_UNRECTIFIED; 
				break;
            case '3':	
				if (activeCamera == Stereo_Cam)
					StereoCam.ViewID = sl::VIEW_RIGHT_UNRECTIFIED; 
				break;
            case '4':	
				if (activeCamera == Stereo_Cam)
					StereoCam.ViewID = sl::VIEW_DEPTH; 
				break;
			case '5':	
				if (activeCamera == Stereo_Cam)
					StereoCam.ViewID = sl::VIEW_CONFIDENCE;	
				break;
			case '6':	
				if (activeCamera == Stereo_Cam)
					StereoCam.ViewID = sl::VIEW_NORMALS; 
				break;

				//______________ SAVE ______________
			case 'w': // image
				if (activeCamera == Stereo_Cam)
					StereoCam.saveSbSimage(std::string("ZEDImage") + std::to_string(count) + std::string(".png"));
				count++;
				break;

			case 'd':
				if (activeCamera == Stereo_Cam)
					StereoCam.runtime_parameters.sensing_mode = sl::SENSING_MODE_STANDARD;
				std::cout << "SENSING_MODE: Standard" << std::endl;
				break;

			case 'f':
				if (activeCamera == Stereo_Cam)
					StereoCam.runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL;
				std::cout << "SENSING_MODE: FILL" << std::endl;
				break;

				// ______________  Search mode _____________________________
			case 'm':
				if (activeCamera == Stereo_Cam)
				{
					cam1_op_mode++;
					if (cam1_op_mode > PassThrough)
						cam1_op_mode = 0;
					switch (cam1_op_mode) {
					case 0: printf("mode NONE\n"); break;
					case 1: printf("mode Find Hook\n"); break;
					case 2: printf("mode Find Rock\n"); break;
					case 3: printf("mode Find Beacon\n"); cv::destroyWindow("Masked"); break;
					case 4: printf("mode passthrough\n"); break;
					}
				}
				else if (activeCamera == Front_Cam)
				{
					cam2_op_mode++;
					if (cam2_op_mode > PassThrough)
						cam2_op_mode = 0;
					switch (cam2_op_mode) {
					case 0: printf("mode NONE\n"); break;
					case 1: printf("mode Find Hook\n"); break;
					case 2: printf("mode Find Rock\n"); break;
					case 3: printf("mode Find Beacon\n"); cv::destroyWindow("Masked"); break;
					case 4: printf("mode passthrough\n"); break;
					}
				}
				break;
        }
    }

	std::cout << "shutting down SmartDashboard..." << std::endl;
	SmartDashboard::shutdown();

	return 0;
}

/**
This function display current settings and values.
**/
void printInfo(ThresholdDetecter &ThresholdDet, ZEDCamera &StereoCam)
{
	std::cout << std::endl;

	if (activeCamera == Stereo_Cam)
	{
		std::cout << "Stereo cam mode: ";
		switch (cam1_op_mode) {
		case 0: std::cout << "None" << std::endl; break;
		case FindHook: std::cout << "FindHook" << std::endl; break;
		case FindRock: std::cout << "FindRock" << std::endl; break;
		case FindBeacon: std::cout << "FindBeakon" << std::endl; break;
		}
		if (filename1.size()) std::cout << "video from file: " << filename1 << std::endl;
		std::cout << "Stereo cam view: ";
		switch (StereoCam.ViewID) {
		case sl::VIEW_LEFT: std::cout << "VIEW_LEFT" << std::endl; break;
		case sl::VIEW_RIGHT: std::cout << "VIEW_RIGHT" << std::endl; break;
		case sl::VIEW_LEFT_UNRECTIFIED: std::cout << "VIEW_LEFT_UNRECTIFIED" << std::endl; break;
		case sl::VIEW_RIGHT_UNRECTIFIED: std::cout << "VIEW_RIGHT_UNRECTIFIED" << std::endl; break;
		case sl::VIEW_DEPTH: std::cout << "VIEW_DEPTH" << std::endl; break;
		case sl::VIEW_CONFIDENCE: std::cout << "VIEW_CONFIDENCE" << std::endl;	break;
		case sl::VIEW_NORMALS: std::cout << "VIEW_NORMALS" << std::endl; break;
		}
		std::cout << "Stereo cam sensing mode: ";
		if (StereoCam.runtime_parameters.sensing_mode == sl::SENSING_MODE_STANDARD)
			std::cout << "Standard" << std::endl;
		if (StereoCam.runtime_parameters.sensing_mode == sl::SENSING_MODE_FILL)
			std::cout << "Fill" << std::endl;
		StereoCam.printCameraSettings();
	}

	if (activeCamera == Front_Cam)
	{
		std::cout << "Front cam mode: ";
		switch (cam2_op_mode) {
		case 0: std::cout << "None" << std::endl; break;
		case FindHook: std::cout << "FindHook" << std::endl; break;
		case FindRock: std::cout << "FindRock" << std::endl; break;
		case FindBeacon: std::cout << "FindBeakon" << std::endl; break;
		}
	}
	std::cout << "small display window: " << SmallWindow << std::endl;
	std::cout << "SmartDashboard mode: " << SmartDashboard_Mode << std::endl;
	std::cout << std::endl;
	ThresholdDet.printThreshold();
	std::cout << std::endl;
}

/**
This function displays help
**/
void printHelp() {
	std::cout << std::endl;
	std::cout << "Help (this):                                 'h'" << std::endl;
	std::cout << "info:                                        '?'" << std::endl;
	std::cout << "window size:                                 'z'" << std::endl;
	std::cout << std::endl;
	std::cout << "switch cameras:                              'c'" << std::endl;
	std::cout << "change cameera mode:                         'm'" << std::endl;
	std::cout << std::endl;
	std::cout << "Mouse:" << std::endl;
	std::cout << "hold both buttons to reset HSV values" << std::endl;
	std::cout << "LButton - Additive selection for HSV" << std::endl;
	std::cout << "RButton - show HSV value at pointer" << std::endl;
	std::cout << "Exit : 'q'" << std::endl;
	std::cout << std::endl;
	std::cout << "HSV values adjustment: " << std::endl;
	std::cout << "  Increase increment:                        'i'" << std::endl;
	std::cout << "  Decrease increment:                        'I'" << std::endl;
	std::cout << "  Increase HSV value:                        '>'" << std::endl;
	std::cout << "  Decrease HSV value:                        '<'" << std::endl;
	std::cout << "  Toggle HSV settings:                       'S'" << std::endl;
	std::cout << "  Reset HSV to default:                      'R'" << std::endl;
	std::cout << "  Reset HSV to full range:                   'Z'" << std::endl;
	std::cout << "  Save HSV values:           Function keys F1-F4" << std::endl;
	std::cout << "  Load HSV values:           Function keys F5-F8" << std::endl;
	std::cout << std::endl;
	std::cout << "Zed camera only" << std::endl;
	std::cout << std::endl;
	std::cout << "Camera controls hotkeys: " << std::endl;
	std::cout << "  Increase camera settings value:            '+'" << std::endl;
	std::cout << "  Decrease camera settings value:            '-'" << std::endl;
	std::cout << "  Toggle camera settings:                    's'" << std::endl;
	std::cout << "  Reset all parameters:                      'r'" << std::endl;
	std::cout << "  Save camera values:            Function key F9" << std::endl;
	std::cout << "  Load camera values:            Function key F10" << std::endl;
	std::cout << std::endl;
	std::cout << "recalibrate camera:                          'a'" << std::endl;
	std::cout << std::endl;
	std::cout << "views:" << std::endl;
	std::cout << "  left:                                      '0'" << std::endl;
	std::cout << "  right:                                     '1'" << std::endl;
	std::cout << "  left unrectified:                          '2'" << std::endl;
	std::cout << "  right unrectified:                         '3'" << std::endl;
	std::cout << "  depth:                                     '4'" << std::endl;
	std::cout << "  confidence:                                '5'" << std::endl;
	std::cout << "  normals:                                   '6'" << std::endl;
	std::cout << std::endl;
	std::cout << "write png image:                             'w'" << std::endl;
	std::cout << "sending mode standard:                       'd'" << std::endl;
	std::cout << "sensing mode fill:                           'f'" << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
}

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

/**
This function displays help
**/
void printHelp() {
	std::cout << std::endl;
	std::cout << "Camera controls hotkeys: " << std::endl;
	std::cout << "  Increase camera settings value:            '+'" << std::endl;
	std::cout << "  Decrease camera settings value:            '-'" << std::endl;
	std::cout << "  Toggle camera settings:                    's'" << std::endl;
	std::cout << "  Reset all parameters:                      'r'" << std::endl;
	std::cout << "  Save camera values:            Function key F9" << std::endl;
	std::cout << "  Load camera values:            Function key F10" << std::endl;
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
	std::cout << "Help (this):                                 'h'" << std::endl;
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
	std::cout << "window size:                                 'z'" << std::endl;
	std::cout << "write png image:                             'w'" << std::endl;
	std::cout << "sending mode standard:                       'd'" << std::endl;
	std::cout << "sensing mode fill:                           'f'" << std::endl;
	std::cout << std::endl;
	std::cout << "toggle front camera:                         'c'" << std::endl;
	std::cout << "toggle stereo camera:                        'C'" << std::endl;
	std::cout << std::endl;
	std::cout << "Exit : 'q'" << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
}

//Define the structure and callback for mouse event
typedef struct mouseOCVStruct {
	cv::Mat image;
	cv::Size _resize;
	int3 low, high;
	bool update = false;
} mouseOCV;

mouseOCV mouseStruct;

static void onMouseCallback(int32_t event, int32_t x, int32_t y, int32_t flag, void * param) {

	mouseOCVStruct* data = (mouseOCVStruct*)param;

	if (flag == (CV_EVENT_FLAG_LBUTTON | CV_EVENT_FLAG_RBUTTON))
	{	// reset if both down
		data->low.x = 255; data->low.y = 255; data->low.z = 255;
		data->high.x = 0;  data->high.y = 0;  data->high.z = 0;
		std::cout << "mouse pick reset." << std::endl;
		data->update = true;
	}
	else if (event == CV_EVENT_LBUTTONDOWN) 
	{
		//convert the img from color to hsv
		cv::Mat hsv;
		cv::cvtColor(data->image, hsv, CV_BGR2HSV);

		int x_int = (x * data->image.cols / data->_resize.width);
		int y_int = (y * data->image.rows / data->_resize.height);

		// find our 9x9 area
		int x_low = x_int - 4;
		int x_high = x_int + 4;
		int y_low = y_int - 4;
		int y_high = y_int + 4;
		if (x_low < 0) x_low = 0;
		if (x_high > 255) x_high = 255;
		if (y_low < 0) y_low = 0;
		if (y_high > 255) y_high = 255;

		int cn = hsv.channels();
		cv::Rect rc(cv::Point(x_low, y_low), cv::Point(x_high, y_high));
		cv::Mat roi(hsv, rc);

		// there has to be a more efficient way to do this
		for (int i = 0; i < roi.rows; i++)
		{
			for (int j = 0; j < roi.cols; j++)
			{	// find low high values
				cv::Vec3b intensity = roi.at<cv::Vec3b>(i, j);
				if (intensity.val[0] < data->low.x)  data->low.x = intensity.val[0];
				if (intensity.val[1] < data->low.y)  data->low.y = intensity.val[1];
				if (intensity.val[2] < data->low.z)	 data->low.z = intensity.val[2];

				if (intensity.val[0] > data->high.x)  data->high.x = intensity.val[0];
				if (intensity.val[1] > data->high.y)  data->high.y = intensity.val[1];
				if (intensity.val[2] > data->high.z)  data->high.z = intensity.val[2];
			}
		}

		std::cout << "H: " << data->low.x << " - " << data->high.x << std::endl;
		std::cout << "S: " << data->low.y << " - " << data->high.y << std::endl;
		std::cout << "V: " << data->low.z << " - " << data->high.z << std::endl;
		std::cout << std::endl;
		data->update = true;
	}
	else if (event == CV_EVENT_RBUTTONDOWN)
	{

		data->update = true;
	}
}

float GetDistanceAtPoint(sl::Mat depth, size_t x, size_t y)
{
	sl::float1 dist;
	depth.getValue(x, y, &dist);
	return dist;
}

#define FindHook 1
#define FindRock 2
#define FindBeacon 3

std::string filename;
bool FrontCamEnabled = false;
bool StereoCamEnabled = true;
bool SmallWindow = true;
int cam1_op_mode = FindHook;
int cam2_op_mode = FindHook;
int frameCount = 0;
size_t SmartDashboard_Mode = 0;

/** Functions **/
void detectHookSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);
void detectBeacon(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);
void printInfo(ThresholdDetecter &, ZEDCamera &);

/** Cascade classifire data */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
std::string hook_cascade_name = "bin/data/SRR Samples/cascades/hook_cascade_gpu.xml";

#define FRONT_CAM_URL ""
//#define FRONT_CAM_URL "http://ctetrick.no-ip.org/videostream.cgi?user=guest&pwd=watchme&resolution=32&rate=0"
//#define FRONT_CAM_RUL "rtsp://root:root@192.168.0.90/axis-media/media.amp"


extern cv::CascadeClassifier hook_cascade;

//main  function
//mode 0 = robot
//mode 1 = simulation
//mode 2 = stand alone (runs directly with SmartDashboard UI)
//SRRZedVision.exe [mode=0] [run svo file]
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

	if (argc == 3)
		filename = argv[2];
	else
		filename = "";

	std::cout << "Initializing OCV Camera." << std::endl;
	OCVCamera FrontCam = OCVCamera(FRONT_CAM_URL);

	std::cout << "Initializing ZED Camera." << std::endl;
	ZEDCamera StereoCam = ZEDCamera(filename.c_str());

	if (!FrontCam.IsOpen && !StereoCam.IsOpen)
	{
		std::cout << "No Cameras!" << std::endl;
		return -1;
	}

	size_t width = StereoCam.image_size.width;
	size_t height = StereoCam.image_size.height;

	cv::Mat anaplyph((int)height, (int)width, CV_8UC4);
	cv::Mat confidencemap((int)height, (int)width, CV_8UC4);

	sl::Mat depth;
	sl::Mat point_cloud;
	cv::Size displaySize((int)(SmallWindow ? width / 2 : width), (int)(SmallWindow ? height / 2 : height));

	if (StereoCam.IsOpen)
	{
		// Mouse callback initialization
		StereoCam.GrabDepth();

		//create Opencv Windows
		cv::namedWindow("VIEW", cv::WINDOW_AUTOSIZE);
		mouseStruct.image = StereoCam.frame;
		mouseStruct._resize = displaySize;
		mouseStruct.low = low;
		mouseStruct.high = high;
		cv::setMouseCallback("VIEW", onMouseCallback, (void*)&mouseStruct);
	}
	else
	{
		StereoCamEnabled = false;
		if (FrontCam.IsOpen)
			FrontCamEnabled = true;
		else
		{
			std::cout << "No Cameras!" << std::endl;
			return -1;
		}
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

	// Print help in console
	printHelp();

    //loop until 'q' is pressed
    while (key != 'q') 
	{
		/***** main video loop *****/
		if (FrontCamEnabled)
		{
			cv::Mat frame = FrontCam.GrabFrame();

			if (cam2_op_mode == FindHook)
				detectHookSample(frame, depth, point_cloud);	// TODO: no point cloud or depth for fromt cam.
			else if (cam2_op_mode == FindRock)
				ThresholdDet.detectRockSample(frame, depth, point_cloud, SmallWindow);
			else if (cam2_op_mode == FindBeacon)
				detectBeacon(frame, depth, point_cloud);

			cv::imshow("camera", frame);
		}

		if (StereoCamEnabled)
		{	// update
			StereoCam.GrabFrameAndDapth();
			anaplyph = StereoCam.frame;
			depth = StereoCam.depth;
			point_cloud = StereoCam.point_cloud;

			displaySize.height = (int)(SmallWindow ? height / 2 : height);
			displaySize.width = (int)(SmallWindow ? width / 2 : width);
			mouseStruct.image = anaplyph;
			mouseStruct._resize = displaySize;

			// Get frames and launch the computation
			if (StereoCam.bHaveFrame)
			{
				/***************  DISPLAY:  ***************/
				if (cam1_op_mode == FindHook)
					detectHookSample(anaplyph, depth, point_cloud);
				else if (cam1_op_mode == FindRock)
				{
#if 0	// need to confirm resonable numbers
					if (mouseStruct.update)
					{
						ThresholdDet.setThreshold(mouseStruct.low, mouseStruct.high);
						mouseStruct.update = false;
					}
#endif
					ThresholdDet.detectRockSample(anaplyph, depth, point_cloud, SmallWindow);
				}
				else if (cam1_op_mode == FindBeacon)
					detectBeacon(anaplyph, depth, point_cloud);

				if (SmallWindow)
					resize(anaplyph, anaplyph, displaySize);
				imshow("VIEW", anaplyph);
			}

			frameCount++;
		}
		/** end of main video loop **/

        key = cv::waitKey(5);
 
        // Keyboard shortcuts
        switch (key) {
			// ZED
			//re-compute stereo alignment
            case 'a':	
				StereoCam.ResetCalibration();
                break;

			case 'h':
				printHelp();
				break;

			case '?':
				printInfo(ThresholdDet, StereoCam);
				break;

            //Change camera settings 
			case 's':	// setting
			case 'r':	// reset (all)
			case '+':	// increase
			case '-':	// decrease
			case 0x00780000:
			case 0x00790000:
				StereoCam.updateCameraSettings(key);
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
					ThresholdDet.updateThresholdSettings(key);
				break;

				// ______________  VIEW __________________
            case '0':	StereoCam.ViewID = sl::VIEW_LEFT; break;
            case '1':	StereoCam.ViewID = sl::VIEW_RIGHT; break;
            case '2':	StereoCam.ViewID = sl::VIEW_LEFT_UNRECTIFIED; break;
            case '3':	StereoCam.ViewID = sl::VIEW_RIGHT_UNRECTIFIED; break;
            case '4':	StereoCam.ViewID = sl::VIEW_DEPTH; break;
			case '5':	StereoCam.ViewID = sl::VIEW_CONFIDENCE;	break;
			case '6':	StereoCam.ViewID = sl::VIEW_NORMALS; break;

			case 'z':
				SmallWindow = !SmallWindow;
				break;

				//______________ SAVE ______________
			case 'w': // image
				StereoCam.saveSbSimage(std::string("ZEDImage") + std::to_string(count) + std::string(".png"));
				count++;
				break;

			case 'd':
				StereoCam.runtime_parameters.sensing_mode = sl::SENSING_MODE_STANDARD;
				std::cout << "SENSING_MODE: Standard" << std::endl;
				break;

			case 'f':
				StereoCam.runtime_parameters.sensing_mode = sl::SENSING_MODE_FILL;
				std::cout << "SENSING_MODE: FILL" << std::endl;
				break;

			case 'c':
				if (FrontCam.IsOpen)
					FrontCamEnabled = !FrontCamEnabled;
				std::cout << "Front Camera " << (FrontCamEnabled ? "enabled" : "disabled") << std::endl;
				if (!FrontCamEnabled)
					cv::destroyWindow("camera");
				break;

			case 'C':
				if (StereoCam.IsOpen)
					StereoCamEnabled = !StereoCamEnabled;
				std::cout << "Stereo Camera " << (StereoCamEnabled ? "enabled" : "disabled") << std::endl;
				if (!FrontCamEnabled)
					cv::destroyWindow("VIEW");
				break;

				// ______________  Search mode _____________________________
			case 'm':
				cam1_op_mode++;
				if (cam1_op_mode > FindBeacon)
					cam1_op_mode = 0;
				switch (cam1_op_mode) {
					case 0: printf("mode NONE\n"); break;
					case 1: printf("mode Find Hook\n"); break;
					case 2: printf("mode Find Rock\n"); break;
					case 3: printf("mode Find Beacon\n"); cv::destroyWindow("Masked"); break;
				}
				break;

			case 'M':
				cam2_op_mode++;
				if (cam2_op_mode > FindBeacon)
					cam2_op_mode = 0;
				switch (cam2_op_mode) {
				case 0: printf("mode NONE\n"); break;
				case 1: printf("mode Find Hook\n"); break;
				case 2: printf("mode Find Rock\n"); break;
				case 3: printf("mode Find Beacon\n"); cv::destroyWindow("Masked"); break;
				}
				break;
        }
    }

	SmartDashboard::shutdown();

	return 0;
}

/**
This function display current settings and values.
**/
void printInfo(ThresholdDetecter &ThresholdDet, ZEDCamera &StereoCam)
{
	std::cout << std::endl;
	std::cout << "Stereo cam enabled: " << StereoCamEnabled << std::endl;
	if (StereoCamEnabled)
	{
		std::cout << "Stereo cam mode: ";
		switch (cam1_op_mode) {
		case 0: std::cout << "None" << std::endl; break;
		case FindHook: std::cout << "FindHook" << std::endl; break;
		case FindRock: std::cout << "FindRock" << std::endl; break;
		case FindBeacon: std::cout << "FindBeakon" << std::endl; break;
		}
		if (filename.size()) std::cout << "video from file: " << filename << std::endl;
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

	std::cout << "Front cam enabled: " << FrontCamEnabled << std::endl;
	if (FrontCamEnabled)
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


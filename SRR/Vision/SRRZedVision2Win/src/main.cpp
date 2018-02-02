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
	std::cout << std::endl;
	std::cout << "Camera controls hotkeys: " << std::endl;
	std::cout << "  Increase camera settings value:            '+'" << std::endl;
	std::cout << "  Decrease camera settings value:            '-'" << std::endl;
	std::cout << "  Toggle camera settings:                    's'" << std::endl;
	std::cout << "  Reset all parameters:                      'r'" << std::endl;
	std::cout << std::endl;
	std::cout << "Exit : 'q'" << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
}

//Define the structure and callback for mouse event
typedef struct mouseOCVStruct {
	sl::Mat depth;
	cv::Size _resize;
} mouseOCV;

mouseOCV mouseStruct;

static void onMouseCallback(int32_t event, int32_t x, int32_t y, int32_t flag, void * param) {
	if (event == CV_EVENT_LBUTTONDOWN) {
		mouseOCVStruct* data = (mouseOCVStruct*)param;
		size_t y_int = (y * data->depth.getHeight() / data->_resize.height);
		size_t x_int = (x * data->depth.getWidth() / data->_resize.width);

		sl::float1 dist;
		data->depth.getValue(x_int, y_int, &dist);

		std::cout << std::endl;
		if (isValidMeasure(dist))
			std::cout << "Depth at (" << x_int << "," << y_int << ") : " << dist << "m";
		else {
			std::string depth_status;
			if (dist == sl::TOO_FAR) depth_status = ("Depth is too far.");
			else if (dist == sl::TOO_CLOSE) depth_status = ("Depth is too close.");
			else depth_status = ("Depth not available");
			std::cout << depth_status;
		}
		std::cout << std::endl;
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
#define FindObsticals 4

std::string filename;
bool FrontCamEnabled = false;
bool StereoCamEnabled = true;
int cam1_op_mode = FindHook;
int cam2_op_mode = FindHook;
int frameCount = 0;

/** Functions **/
void detectHookSample(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);
void detectBeacon(cv::Mat frame, sl::Mat depth, sl::Mat point_cloud);

/** Cascade classifire data */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
std::string hook_cascade_name = "bin/data/SRR Samples/cascades/hook_cascade_gpu.xml";

#define FRONT_CAM_URL ""
//#define FRONT_CAM_URL "http://ctetrick.no-ip.org/videostream.cgi?user=guest&pwd=watchme&resolution=32&rate=0"
//#define FRONT_CAM_RUL "rtsp://root:root@192.168.0.90/axis-media/media.amp"


extern cv::CascadeClassifier hook_cascade;

/** threishold values **/
extern int ThreshInc;

//main  function
int main(int argc, char **argv) {

    if (argc > 2) {
        std::cout << "Only the path of a SVO can be passed in arg" << std::endl;
        return -1;
    }

    //-- 1. Load the cascades
    if (!hook_cascade.load(hook_cascade_name)){	
		std::cout << "--(!)Error loading cascade data" << std::endl; 
		return -1; 
    };

	// set up detectors
	int3 HSV_low;
	int3 HSV_high;
	HSV_low.x = 122; HSV_low.y = 50; HSV_low.z = 90;
	HSV_high.x = 155; HSV_high.y = 255; HSV_high.z = 255;
	ThresholdDetecter ThresholdDet(HSV_low, HSV_high);

	char key = ' ';
	int count = 0;

	char * filearg;
	if (argc == 2)
		filearg = argv[1];
	else
		filearg = NULL;

	std::cout << "Initializing OCV Camera." << std::endl;
	OCVCamera FrontCam = OCVCamera(FRONT_CAM_URL);

	std::cout << "Initializing ZED Camera." << std::endl;
	ZEDCamera StereoCam = ZEDCamera(filearg);

	if (!FrontCam.IsOpen && !StereoCam.IsOpen)
	{
		std::cout << "No Cameras!" << std::endl;
		return -1;
	}

	size_t width = StereoCam.image_size.width;
	size_t height = StereoCam.image_size.height;

	bool displayConfidenceMap = false;

	cv::Mat disp((int)height, (int)width, CV_8UC4);
	cv::Mat anaplyph((int)height, (int)width, CV_8UC4);
	cv::Mat confidencemap((int)height, (int)width, CV_8UC4);

	sl::Mat depth;
	sl::Mat point_cloud;

	if (StereoCam.IsOpen)
	{
		// Mouse callback initialization
		cv::Size displaySize((int)width, (int)height);
		StereoCam.GrabDepth();
		mouseStruct.depth = StereoCam.depth;
		mouseStruct._resize = displaySize;

		//create Opencv Windows
		cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);
		cv::setMouseCallback("Depth", onMouseCallback, (void*)&mouseStruct);
		cv::namedWindow("VIEW", cv::WINDOW_AUTOSIZE);
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

	SmartDashboard::SetIPAddress("127.0.0.1");
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
				ThresholdDet.detectRockSample(frame, depth, point_cloud);
			else if (cam2_op_mode == FindBeacon)
				detectBeacon(frame, depth, point_cloud);

			cv::imshow("camera", frame);
		}
		if (StereoCamEnabled)
		{
			StereoCam.GrabFrameAndDapth();
			anaplyph = StereoCam.frame;
			depth = StereoCam.depth;
			point_cloud = StereoCam.point_cloud;
			mouseStruct.depth = depth;
			// TODO: optional depth and disparity display below.
			// Get frames and launch the computation
			if (StereoCam.bHaveFrame)
			{

				/***************  DISPLAY:  ***************/
				// Normalize the DISPARITY / DEPTH map in order to use the full color range of grey level image
				StereoCam.GetNormDepth();
				disp = StereoCam.cvNormDepth;

				// To get the depth at a given position, click on the DISPARITY / DEPTH map image
				imshow("Depth", disp);

				if (displayConfidenceMap) {
					StereoCam.GetNormConfidence();
					confidencemap = StereoCam.cvConfidence;
					imshow("confidence", confidencemap);
				}

				if (cam1_op_mode == FindHook)
					detectHookSample(anaplyph, depth, point_cloud);
				else if (cam1_op_mode == FindRock)
					ThresholdDet.detectRockSample(anaplyph, depth, point_cloud);
				else if (cam1_op_mode == FindBeacon)
					detectBeacon(anaplyph, depth, point_cloud);

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

            //Change camera settings (here --> gain)
			case 's':
			case 'r':
			case '+':
			case '-':
				StereoCam.updateCameraSettings(key);

				// ______________  VIEW __________________
            case '0': // left
                StereoCam.ViewID = 0;
                break;
            case '1': // right
				StereoCam.ViewID = 1;
                break;
            case '2': // anaglyph
				StereoCam.ViewID = 2;
                break;
            case '3': // gray scale diff
				StereoCam.ViewID = 3;
                break;
            case '4': // Side by side
				StereoCam.ViewID = 4;
                break;
            case '5': // overlay
				StereoCam.ViewID = 5;
                break;

				// ______________  Display Confidence Map __________________
			case 'p':
				displayConfidenceMap = !displayConfidenceMap;
				break;

				//______________ SAVE ______________
			case 'w': // image
				StereoCam.saveSbSimage(std::string("ZEDImage") + std::to_string(count) + std::string(".png"));
				count++;
				break;

			case 'v': // disparity
			{
				std::string filename = std::string(("ZEDDisparity") + std::to_string(count) + std::string(".png"));
				cv::Mat dispSnapshot;
				disp.copyTo(dispSnapshot);
				cv::imshow("Saving Disparity", dispSnapshot);
				cv::imwrite(filename, dispSnapshot);
				count++;
				break;
			}

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
				if (cam1_op_mode > FindObsticals )
					cam1_op_mode = 0;
				switch (cam1_op_mode) {
					case 0: printf("mode NONE\n");break;
					case 1: printf("mode Find Hook\n");break;
					case 2: printf("mode Find Rock\n");break;
					case 3: printf("mode Find Beacon\n");break;
					case 4: printf("mode Find Obsticals\n");break;
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
				case 3: printf("mode Find Beacon\n"); break;
				}
				break;

			//TODO: add to class
			case 'i':
				if (ThreshInc == 1)
					ThreshInc = 5;
				else
					ThreshInc += 5;
				if (ThreshInc > 20) ThreshInc = 20;
				printf("ThreshInc: %d\n", ThreshInc);
				break;
				
			case 'I':
				if (ThreshInc == 5)
					ThreshInc = 1;
				else
					ThreshInc -= 5;
				if (ThreshInc <= 0) ThreshInc = 1;
				printf("ThreshInc: %d\n", ThreshInc);
				break;

			case 'S':
			case 'R':
			case '<':
			case '>':
				ThresholdDet.updateThresholdSettings(key);
				break;
        }
    }

	SmartDashboard::shutdown();

	return 0;
}

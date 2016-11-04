/**************************************************************************************************
 ** This sample demonstrates how to grab images and depth/disparity map with the ZED SDK          **
 ** Both images and depth/disparity map are displayed with OpenCV                                 **
 ** Most of the functions of the ZED SDK are linked with a key press event (using opencv)         **
 ***************************************************************************************************/

#include "stdafx.h"

//My stuff
#include "OCVCamera.h"
#include "ZEDCamera.h"

#include "../SmartDashboard/SmartDashboard_import.h"

//Define the structure and callback for mouse event
typedef struct mouseOCVStruct {
    float* data;
    uint32_t step;
    cv::Size _image;
    cv::Size _resize;
    std::string name;
} mouseOCV;

mouseOCV mouseStruct;

static void onMouseCallback(int32_t event, int32_t x, int32_t y, int32_t flag, void * param) {
    if (event == CV_EVENT_LBUTTONDOWN) {
        mouseOCVStruct* data = (mouseOCVStruct*) param;

        int y_int = (y /* * data->_image.height / data->_resize.height*/);
        int x_int = (x /* * data->_image.width / data->_resize.width*/);

        float* ptr_image_num = (float*) ((int8_t*) data->data + y_int * data->step);
        float dist = ptr_image_num[x_int] / 1000.f;

        if (dist > 0.)
            printf("\n%s : %2.2f m : %2.2f ft\n", data->name.c_str(), dist, dist * 3.37);
        else
            printf("\n : NAN\n");
    }
}

float GetDistanceAtPoint(sl::zed::Mat depth, int x, int y)
{
    float* ptr_image_num = (float*) (depth.data + y * depth.step);
    float dist = ptr_image_num[x] / 1000.f;

	return dist;
}

#define FindHook 1
#define FindRock 2
#define FindBeacon 3
#define FindObsticals 4

std::string filename;
bool FrontCamEnabled = false;
bool StereoCamEnabled = true;
int cam1_op_mode = 0;
int cam2_op_mode = 0;
int frameCount = 0;

/** Functions **/
void detectHookSample(cv::Mat frame, sl::zed::Mat depth);
void detectRockSample(cv::Mat frame, sl::zed::Mat depth);
void detectBeacon(cv::Mat frame, sl::zed::Mat depth);

/** Cascade classifire data */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
std::string hook_cascade_name = "data/SRR Samples/cascades/hook_cascade_gpu.xml";

extern cv::CascadeClassifier hook_cascade;

/** threishold values **/
extern int H_low;
extern int H_high;
extern int S_low;
extern int S_high;
extern int V_low;
extern int V_high;
extern int ThreshInc;

//main  function
int main(int argc, char **argv) {

    if (argc > 2) {
        std::cout << "Only the path of a SVO can be passed in arg" << std::endl;
        return -1;
    }

    //-- 1. Load the cascades
    if (!hook_cascade.load(hook_cascade_name)){	
		std::cout << "--(!)Error loading" << std::endl; 
		return -1; 
    };

	char key = ' ';
	int count = 0;

	char * filearg;
	if (argc == 2)
		filearg = argv[1];
	else
		filearg = NULL;

//	OCVCamera FrontCam = OCVCamera("http://ctetrick.no-ip.org/videostream.cgi?user=guest&pwd=watchme&resolution=32&rate=0");
	OCVCamera FrontCam = OCVCamera("rtsp://root:root@192.168.0.90/axis-media/media.amp");

	cv::Mat frame;

	ZEDCamera StereoCam = ZEDCamera(filearg);
    StereoCam.dm_type = sl::zed::STANDARD;

	int width = StereoCam.width;
	int height = StereoCam.height;

	bool DisplayDisp = true;
	bool displayConfidenceMap = false;

	cv::Mat disp(height, width, CV_8UC4);
	cv::Mat anaplyph(height, width, CV_8UC4);
	cv::Mat confidencemap(height, width, CV_8UC4);

	sl::zed::Mat depth;

	if (StereoCam.IsOpen)
	{
		/* Init mouse callback */
		depth = StereoCam.GrabDepth();

		// Set the structure
		mouseStruct._image = cv::Size(width, height);
		mouseStruct.data = (float*)depth.data;
		mouseStruct.step = depth.step;
		mouseStruct.name = "DEPTH";

		//create Opencv Windows
		cv::namedWindow(mouseStruct.name, cv::WINDOW_AUTOSIZE);
		cv::setMouseCallback(mouseStruct.name, onMouseCallback, (void*)&mouseStruct);
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

	SmartDashboard::init();

	std::cout << "Press 'q' to exit." << std::endl;

    //loop until 'q' is pressed
    while (key != 'q') 
	{
		/***** main video loop *****/
		if (FrontCamEnabled)
		{
			frame = FrontCam.GrabFrame();

			if (cam2_op_mode == FindHook)
				detectHookSample(frame, depth);
			else if (cam2_op_mode == FindRock)
				detectRockSample(frame, depth);
			else if (cam2_op_mode == FindBeacon)
				detectBeacon(frame, depth);

			cv::imshow("camera", frame);
		}
		if (StereoCamEnabled)
		{
			anaplyph = StereoCam.GrabFrameAndDapth();
			depth = StereoCam.depth;
			// TODO: optional depth and disparity display below.
			// Get frames and launch the computation
			if (!StereoCam.bNoFrame)
			{

				/***************  DISPLAY:  ***************/
				// Normalize the DISPARITY / DEPTH map in order to use the full color range of grey level image
				if (DisplayDisp)
					disp = StereoCam.GetNormDisparity();
				else
					disp = StereoCam.GetNormDepth();

				// To get the depth at a given position, click on the DISPARITY / DEPTH map image
				imshow(mouseStruct.name, disp);

				if (displayConfidenceMap) {
					confidencemap = StereoCam.GetNormConfidence();
					imshow("confidence", confidencemap);
				}

				if (cam1_op_mode == FindHook)
					detectHookSample(anaplyph, depth);
				else if (cam1_op_mode == FindRock)
					detectRockSample(anaplyph, depth);
				else if (cam1_op_mode == FindBeacon)
					detectBeacon(anaplyph, depth);

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
            case 'g': //increase gain of 1
            {
				int current_gain = StereoCam.GetGain() + 1;
				StereoCam.SetGain(current_gain);
                std::cout << "set Gain to " << current_gain << std::endl;
            }
                break;

            case 'h': //decrease gain of 1
            {
				int current_gain = StereoCam.GetGain() + 1;
				StereoCam.SetGain(current_gain);
				std::cout << "set Gain to " << current_gain << std::endl;
            }
                break;

			case 'e': //increase exposure of 1
			{
				int current_exp = StereoCam.GetExposure() + 1;
				StereoCam.SetExposure(current_exp);
				std::cout << "set Exposure to " << current_exp << std::endl;
			}
			break;

			case 'E': //decrease exposure of 1
			{
				int current_exp = StereoCam.GetExposure() + 1;
				StereoCam.SetExposure(current_exp);
				std::cout << "set Exposure to " << current_exp << std::endl;
			}
			break;

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
			case 's':
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

			case 'r':
				StereoCam.dm_type = sl::zed::SENSING_MODE::STANDARD;
				std::cout << "SENSING_MODE: Standard" << std::endl;
				break;

			case 'f':
				StereoCam.dm_type = sl::zed::SENSING_MODE::FILL;
				std::cout << "SENSING_MODE: FILL" << std::endl;
				break;

			case 'd':
				DisplayDisp = !DisplayDisp;
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

			case '+':
				if (ThreshInc == 1)
					ThreshInc = 5;
				else
					ThreshInc += 5;
				if (ThreshInc > 20) ThreshInc = 20;
				printf("ThreshInc: %d\n", ThreshInc);
				break;
				
			case '-':
				if (ThreshInc == 5)
					ThreshInc = 1;
				else
					ThreshInc -= 5;
				if (ThreshInc <= 0) ThreshInc = 1;
				printf("ThreshInc: %d\n", ThreshInc);
				break;
				
            case '[':
            	if ((H_low -= ThreshInc) < 0) H_low = 0;
            	printf("H low: %d\n", H_low);
            	break;
            	
            case ']':
            	if ((H_low += ThreshInc) > 255) H_low = 255;
            	printf("H low: %d\n", H_low);
            	break;
            	
            case ';':
            	if ((S_low -= ThreshInc) < 0) S_low = 0;
            	printf("S low: %d\n", S_low);
            	break;
            	
            case '\'':
            	if ((S_low += ThreshInc) > 255) S_low = 255;
            	printf("S low: %d\n", S_low);
            	break;
            	
            case ',':
            	if ((V_low -= ThreshInc) < 0) V_low = 0;
            	printf("V low: %d\n", V_low);
            	break;
            	
            case '.':
            	if ((V_low += ThreshInc) > 255) V_low = 255;
            	printf("V low: %d\n", V_low);
            	break;
                
            case '{':
            	if ((H_high -= ThreshInc) < 0) H_high = 0;
            	printf("H high: %d\n", H_high);
            	break;
            	
            case '}':
            	if ((H_high += ThreshInc) > 255) H_high = 255;
            	printf("H high: %d\n", H_high);
            	break;
            	
            case ':':
            	if ((S_high -= ThreshInc) < 0) S_high = 0;
            	printf("S high: %d\n", S_high);
            	break;
            	
            case '\"':
            	if ((S_high += ThreshInc) > 255) S_high = 255;
            	printf("S high: %d\n", S_high);
            	break;
            	
            case '<':
            	if ((V_high -= ThreshInc) < 0) V_high = 0;
            	printf("V high: %d\n", V_high);
            	break;
            	
            case '>':
            	if ((V_high += ThreshInc) > 255) V_high = 255;
            	printf("V high: %d\n", V_high);
            	break;
        }
    }

	SmartDashboard::shutdown();

	return 0;
}

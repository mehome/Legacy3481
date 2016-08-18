/**************************************************************************************************
 ** This sample demonstrates how to grab images and depth/disparity map with the ZED SDK          **
 ** Both images and depth/disparity map are displayed with OpenCV                                 **
 ** Most of the functions of the ZED SDK are linked with a key press event (using opencv)         **
 ***************************************************************************************************/

//standard includes
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <chrono>

//opencv includes
#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/calib3d/calib3d.hpp>

//ZED Includes
#include <zed/Camera.hpp>
#include <zed/utils/GlobalDefine.hpp>

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


enum histo_mode
{
	h_original,
	h_equalize,
	h_clahe
};

#define FindHook 1
#define FindRock 2
#define FindBeacon 3
#define FindObsticals 4

std::string filename;
enum histo_mode mode = h_original;
bool FrontCamEnabled = false;
bool StereoCamEnabled = true;
int cam1_op_mode = 0;
int cam2_op_mode = 0;
bool bShowImg = false;
int frameCount = 0;

/** Function Headers */
void detectHookSample(cv::Mat frame, sl::zed::Mat depth);
void detectRockSample(cv::Mat frame, sl::zed::Mat depth);
void detectBeacon(cv::Mat frame, sl::zed::Mat depth);

/** Global variables */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
std::string hook_cascade_name = "data/SRR Samples/cascades/hook_cascade_gpu.xml";
cv::CascadeClassifier hook_cascade;

std::vector<cv::Rect> hooks;

/**
* @function detectHookSample
*/
void detectHookSample(cv::Mat frame, sl::zed::Mat depth)
{
	cv::Mat frame_gray;

	cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
	if (mode == h_original && bShowImg)
		cv::imshow("original gray", frame_gray);

	if (mode == h_equalize)
	{
		cv::equalizeHist(frame_gray, frame_gray);
		if (bShowImg)
			cv::imshow("equalized", frame_gray);
	}

	if (mode == h_clahe)
	{
		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		//clahe->setTilesGridSize(cv::Size(8, 8));
		clahe->apply(frame_gray, frame_gray);
		if (bShowImg)
			cv::imshow("clahe", frame_gray);
	}

	//-- detect hook sample
	hook_cascade.detectMultiScale(frame_gray, hooks, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(15, 15));

	int height = 0;
	int XRes = frame_gray.cols;
	int YRes = frame_gray.rows;

	for (size_t i = 0; i < hooks.size(); i++)
	{
		float Distance = GetDistanceAtPoint(depth, hooks[i].x, hooks[i].y);	
		std::cout << "hook found at: " << hooks[i].x << ", " << hooks[i].y << " Dist: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;

		cv::Point p1(hooks[i].x, hooks[i].y);
		cv::Point p2(hooks[i].x + hooks[i].width, hooks[i].y + hooks[i].height);
		rectangle(frame, p1, p2, cv::Scalar(255, 0, 255), 2, 8, 0);
		
		double x_target = hooks[i].x + hooks[i].width / 2;
		double y_target = hooks[i].y + hooks[i].height / 2;
		//SmartDashboard::PutNumber("X Position", x_target);
		//SmartDashboard::PutNumber("Y Position", y_target);
	}
}

int H_low = 122;
int H_high = 155;
int S_low = 50;
int S_high = 255;
int V_low = 90;
int V_high = 255;

int ThreshInc = 10;

void detectRockSample(cv::Mat frame, sl::zed::Mat depth)
{
	cv::Scalar color = cv::Scalar( 255, 0, 255 );

	cv::Mat hsv, binary, masked;

    //convert the img from color to hsv
    cv::cvtColor(frame, hsv, CV_BGR2HSV);
    
    //cv::blur(hsv, hsv, cv::Size(3,3));
	cv::GaussianBlur( hsv, hsv, cv::Size( 5, 5 ), 0, 0 );

    //process the image - threshold
    cv::inRange(hsv, cv::Scalar(H_low, S_low, V_low), cv::Scalar(H_high, S_high, V_high), binary);

	int erosion_size = 3;
	cv::Mat element = cv::getStructuringElement( cv::MORPH_RECT,
                                       cv::Size( 2*erosion_size + 1, 2*erosion_size + 1 ),
                                       cv::Point( erosion_size, erosion_size ) );	
	// this eliminates small artifacts.
	cv::erode( binary, binary, element );
	cv::dilate( binary, binary, element );                                       

	// countours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	
	/// Find contours
	cv::findContours( binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

	// mask and display   
	frame.copyTo(masked, binary);   

	/// moments
	std::vector<cv::Moments> mu(contours.size() );
	/// mass centers
	std::vector<cv::Point2f> mc(contours.size() );
	/// rotated rectangles 
	std::vector<cv::RotatedRect> minRect(contours.size() );

	for( int i = 0; i< contours.size(); i++ )
	{
		/// Get the moments
		mu[i] = moments( contours[i], false );
		///  Get the mass centers:
		mc[i] = cv::Point2f( (float)(mu[i].m10/mu[i].m00) , (float)(mu[i].m01/mu[i].m00) );
		/// Find the rotated rectangles for each contour
		minRect[i] = cv::minAreaRect( cv::Mat(contours[i]) );

		float Distance = GetDistanceAtPoint(depth, (int)mc[i].x, (int)mc[i].y);	
     
		if ((contourArea(contours[i]) > 150) && 
			(minRect[i].size.width > 10) && 
			(minRect[i].size.height > 10))
		{
			std::cout << "rock found at " << mc[i].x << ", " << mc[i].y << " distance: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;

			/// Draw contours
			cv::drawContours( frame, contours, i, color, 2, 8, hierarchy, 0, cv::Point() );
			cv::circle( frame, mc[i], 4, color, -1, 8, 0 );
			// rotated rectangle
			cv::Point2f rect_points[4]; minRect[i].points( rect_points );
			for( int j = 0; j < 4; j++ )
				cv::line( frame, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
		}	
	}

	cv::imshow("Maksed", masked);    
}

int count = 0;
std::vector<cv::Point2f> pointBuf;

// use calibration target for a beacon.
void detectBeacon(cv::Mat view, sl::zed::Mat depth)
{
    cv::Size boardSize( 9, 6 );            // The size of the board -> Number of items by width and height
    cv::Mat cameraMatrix, distCoeffs;
    cv::Size imageSize;

    imageSize = view.size();  // Format input image.

	std::cout << count++ << " calling findchessboardcorners." << std::endl;
    bool found = cv::findChessboardCorners( view, boardSize, pointBuf,
        			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
        			
	std::cout << "findcorners returned " << found << std::endl;
    if (found)                // If done with success,
    {
        // improve the found corners' coordinate accuracy for chessboard
		cv::Mat viewGray;
		cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
        cv::cornerSubPix( viewGray, pointBuf, cv::Size(11,11),
            cv::Size(-1,-1), cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
            
        /// min point
        cv::Point2f minp( (float)imageSize.width, (float)imageSize.height );

		/// max point
		cv::Point2f maxp( 0, 0 );
            
		/// center
		cv::Point2f center;

		for( int i = 0; i < pointBuf.size(); i++)
		{
			if (pointBuf[i].x < minp.x) minp.x = pointBuf[i].x;
			if (pointBuf[i].y < minp.y) minp.y = pointBuf[i].y;
			if (pointBuf[i].x > maxp.x) maxp.x = pointBuf[i].x; 
			if (pointBuf[i].y > maxp.y) maxp.y = pointBuf[i].y;
		}
		center.x = minp.x + (maxp.x - minp.x) / 2;
		center.y = minp.y + (maxp.y - minp.y) / 2;

        // Draw the corners.
        cv::drawChessboardCorners( view, boardSize, cv::Mat(pointBuf), found );
        
   		float Distance = GetDistanceAtPoint(depth, (int)center.x, (int)center.y);	
		std::cout << "beacon found at " << center.x << ", " << center.y << " distance: " << Distance << " m " << Distance * 3.37 << " ft" << std::endl;
    }
}

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

	OCVCamera FrontCam = OCVCamera("http://ctetrick.no-ip.org/videostream.cgi?user=guest&pwd=watchme&resolution=32&rate=0");

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

	std::cout << "Press 'q' to exit, hoser!" << std::endl;

    //loop until 'q' is pressed
    while (key != 'q') 
	{
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

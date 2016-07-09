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
//#include "SmartDashboard/SmartDashboard_Import.h"

//ZED Includes
#include <zed/Camera.hpp>


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

        int y_int = (y * data->_image.height / data->_resize.height);
        int x_int = (x * data->_image.width / data->_resize.width);

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

// save function using opencv
void saveSbSimage(sl::zed::Camera* zed, std::string filename) {
    sl::zed::resolution imSize = zed->getImageSize();

    cv::Mat SbS(imSize.height, imSize.width * 2, CV_8UC4);
    cv::Mat leftIm(SbS, cv::Rect(0, 0, imSize.width, imSize.height));
    cv::Mat rightIm(SbS, cv::Rect(imSize.width, 0, imSize.width, imSize.height));

    slMat2cvMat(zed->retrieveImage(sl::zed::SIDE::LEFT)).copyTo(leftIm);
    slMat2cvMat(zed->retrieveImage(sl::zed::SIDE::RIGHT)).copyTo(rightIm);

    cv::imshow("Saving Image", SbS);
    cv::cvtColor(SbS, SbS, CV_RGBA2RGB);

    cv::imwrite(filename, SbS);
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
int op_mode = 0;
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

/**
* @function detectHookSample
*/
void detectHookSample(cv::Mat frame, sl::zed::Mat depth)
{
	std::vector<cv::Rect> hooks;
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

    cv::Size DisplaySize(720, 404);
    cv::Mat maskDisplay(DisplaySize, CV_8UC4);

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
	cv::vector<cv::vector<cv::Point> > contours;
	cv::vector<cv::Vec4i> hierarchy;
	
	/// Find contours
	cv::findContours( binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

	// mask and display   
	frame.copyTo(masked, binary);   

	/// moments
	cv::vector<cv::Moments> mu(contours.size() );
	/// mass centers
	cv::vector<cv::Point2f> mc(contours.size() );
	/// rotated rectangles 
	cv::vector<cv::RotatedRect> minRect(contours.size() );

	for( int i = 0; i< contours.size(); i++ )
	{
		/// Get the moments
		mu[i] = moments( contours[i], false );
		///  Get the mass centers:
		mc[i] = cv::Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
		/// Find the rotated rectangles for each contour
		minRect[i] = cv::minAreaRect( cv::Mat(contours[i]) );

		float Distance = GetDistanceAtPoint(depth, mc[i].x, mc[i].y);	
     
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

	cv::resize(masked, maskDisplay, DisplaySize);
	cv::imshow("Maksed", maskDisplay);    
}

int count = 0;

// use calibration target for a beacon.
void detectBeacon(cv::Mat view, sl::zed::Mat depth)
{
    cv::Size boardSize( 9, 6 );            // The size of the board -> Number of items by width and height
    cv::Mat cameraMatrix, distCoeffs;
    cv::Size imageSize;

    imageSize = view.size();  // Format input image.

    cv::vector<cv::Point2f> pointBuf;
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
        cv::Point2f min( imageSize.width, imageSize.height );

		/// max point
		cv::Point2f max( 0, 0 );
            
		/// center
		cv::Point2f center;

		for( int i = 0; i < pointBuf.size(); i++)
		{
			if (pointBuf[i].x < min.x) min.x = pointBuf[i].x;
			if (pointBuf[i].y < min.y) min.y = pointBuf[i].y;
			if (pointBuf[i].x > max.x) max.x = pointBuf[i].x; 
			if (pointBuf[i].y > max.y) max.y = pointBuf[i].y;
		}
		center.x = min.x + (max.x - min.x) / 2;
		center.y = min.y + (max.y - min.y) / 2;

        // Draw the corners.
        cv::drawChessboardCorners( view, boardSize, cv::Mat(pointBuf), found );
        
   		float Distance = GetDistanceAtPoint(depth, center.x, center.y);	
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

    sl::zed::SENSING_MODE dm_type = sl::zed::FULL;
    sl::zed::Camera* zed;

    if (argc == 1) // Use in Live Mode
        zed = new sl::zed::Camera(sl::zed::HD720, 15);
    else // Use in SVO playback mode
        zed = new sl::zed::Camera(argv[1]);

    int width = zed->getImageSize().width;
    int height = zed->getImageSize().height;

    //init WITH self-calibration (- last parameter to false -)
    sl::zed::ERRCODE err = zed->init(sl::zed::MODE::PERFORMANCE, 0, true, false, false);

    // ERRCODE display
    std::cout << "Error code : " << sl::zed::errcode2str(err) << std::endl;

    // Quit if an error occurred
    if (err != sl::zed::SUCCESS) {
        delete zed;
        return 1;
    }

    char key = ' ';
    int ViewID = 0;
    int count = 0;

    bool DisplayDisp = true;
    bool displayConfidenceMap = false;

	long long last_current_ts=0;
	long long last_current_cts=0;

    cv::Mat disp(height, width, CV_8UC4);
    cv::Mat anaplyph(height, width, CV_8UC4);
    cv::Mat confidencemap(height, width, CV_8UC4);

    cv::Size DisplaySize(720, 404);
    cv::Mat dispDisplay(DisplaySize, CV_8UC4);
    cv::Mat anaplyphDisplay(DisplaySize, CV_8UC4);
    cv::Mat confidencemapDisplay(DisplaySize, CV_8UC4);
    
    cv::Mat hsvDisplay(DisplaySize, CV_8UC4);
    cv::Mat binaryDisplay(DisplaySize, CV_8UC4);

    /* Init mouse callback */
    sl::zed::Mat depth;
    zed->grab(dm_type);
    depth = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH); // Get the pointer
    // Set the structure
    mouseStruct._image = cv::Size(width, height);
    mouseStruct._resize = DisplaySize;
    mouseStruct.data = (float*) depth.data;
    mouseStruct.step = depth.step;
    mouseStruct.name = "DEPTH";
    /***/

    //create Opencv Windows
    cv::namedWindow(mouseStruct.name, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(mouseStruct.name, onMouseCallback, (void*) &mouseStruct);
    cv::namedWindow("VIEW", cv::WINDOW_AUTOSIZE);

    std::cout << "Press 'q' to exit, hoser!" << std::endl;

    //Jetson only. Execute the calling thread on core 2
    sl::zed::Camera::sticktoCPUCore(2);

    sl::zed::ZED_SELF_CALIBRATION_STATUS old_self_calibration_status = sl::zed::SELF_CALIBRATION_NOT_CALLED;
#undef usecam
#ifdef usecam
	cv::VideoCapture capture;
	cv::Mat frame;
	capture.open("http://ctetrick.no-ip.org/videostream.asf?user=guest&pwd=watchme&resolution=32");
   	if (!capture.isOpened())
		printf("Camera NOT opened.\n");
#endif

    //loop until 'q' is pressed
    while (key != 'q') {
#ifdef usecam
    	if (capture.isOpened())
		{
			capture >> frame;
    		cv::transpose(frame, frame);  
		    cv::flip(frame, frame, 0); //transpose+flip(1)=CW			
			cv::imshow("camera", frame);
		}
#endif
        zed->setConfidenceThreshold(100);

        // Get frames and launch the computation
        bool res = zed->grab(dm_type);

        if (!res) {
            // Estimated rotation :
            if (old_self_calibration_status != zed->getSelfCalibrationStatus()) {
                std::cout << "Self Calibration Status : " << sl::zed::statuscode2str(zed->getSelfCalibrationStatus()) << std::endl;
                old_self_calibration_status = zed->getSelfCalibrationStatus();
            }

            depth = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH); // Get the pointer

            // The following is the best way to save a disparity map/ Image / confidence map in Opencv Mat.
            // Be Careful, if you don't save the buffer/data on your own, it will be replace by a next retrieve (retrieveImage, NormalizeMeasure, getView....)
            // !! Disparity, Depth, confidence are in 8U,C4 if normalized format !! //
            // !! Disparity, Depth, confidence are in 32F,C1 if only retrieve !! //


            /***************  DISPLAY:  ***************/
            // Normalize the DISPARITY / DEPTH map in order to use the full color range of grey level image
            if (DisplayDisp)
                slMat2cvMat(zed->normalizeMeasure(sl::zed::MEASURE::DISPARITY)).copyTo(disp);
            else
                slMat2cvMat(zed->normalizeMeasure(sl::zed::MEASURE::DEPTH)).copyTo(disp);


            // To get the depth at a given position, click on the DISPARITY / DEPTH map image
            cv::resize(disp, dispDisplay, DisplaySize);
            imshow(mouseStruct.name, dispDisplay);

            if (displayConfidenceMap) {
                slMat2cvMat(zed->normalizeMeasure(sl::zed::MEASURE::CONFIDENCE)).copyTo(confidencemap);
                cv::resize(confidencemap, confidencemapDisplay, DisplaySize);
                imshow("confidence", confidencemapDisplay);
            }

            //Even if Left and Right images are still available through getView() function, it's better since v0.8.1 to use retrieveImage for cpu readback because GPU->CPU is done async during depth estimation.
            // Therefore :
            // -- if disparity estimation is enabled in grab function, retrieveImage will take no time because GPU->CPU copy has already been done during disp estimation
            // -- if disparity estimation is not enabled, GPU->CPU copy is done in retrieveImage fct, and this function will take the time of copy.
            if (ViewID == sl::zed::STEREO_LEFT || ViewID == sl::zed::STEREO_RIGHT)
                slMat2cvMat(zed->retrieveImage(static_cast<sl::zed::SIDE> (ViewID))).copyTo(anaplyph);
            else
                slMat2cvMat(zed->getView(static_cast<sl::zed::VIEW_MODE> (ViewID))).copyTo(anaplyph);

			long long current_cts = zed->getCameraTimestamp();
			std::cout << "* Camera TimeStamp : " << (current_cts - last_current_cts)/1000000 << std::endl;
			long long current_ts = zed->getCurrentTimestamp();
			std::cout << "* Current TimeStamp : " << (current_ts - last_current_ts)/1000000 << std::endl;
			last_current_cts = current_cts;
			last_current_ts = current_ts;

			frameCount++;
			if (op_mode == FindHook)
				detectHookSample(anaplyph, depth);
			else if (op_mode == FindRock)
				detectRockSample(anaplyph, depth);
			else if (op_mode == FindBeacon)
				detectBeacon(anaplyph, depth);
        }

        cv::resize(anaplyph, anaplyphDisplay, DisplaySize);
        imshow("VIEW", anaplyphDisplay);

        key = cv::waitKey(5);

        // Keyboard shortcuts
        switch (key) {
                //re-compute stereo alignment
            case 'a':
                zed->resetSelfCalibration();
                break;

                //Change camera settings (here --> gain)
            case 'g': //increase gain of 1
            {
                int current_gain = zed->getCameraSettingsValue(sl::zed::ZED_GAIN) + 1;
                zed->setCameraSettingsValue(sl::zed::ZED_GAIN, current_gain);
                std::cout << "set Gain to " << current_gain << std::endl;
            }
                break;

            case 'h': //decrease gain of 1
            {
                int current_gain = zed->getCameraSettingsValue(sl::zed::ZED_GAIN) - 1;
                zed->setCameraSettingsValue(sl::zed::ZED_GAIN, current_gain);
                std::cout << "set Gain to " << current_gain << std::endl;
            }
                break;


                // ______________  VIEW __________________
            case '0': // left
                ViewID = 0;
                break;
            case '1': // right
                ViewID = 1;
                break;
            case '2': // anaglyph
                ViewID = 2;
                break;
            case '3': // gray scale diff
                ViewID = 3;
                break;
            case '4': // Side by side
                ViewID = 4;
                break;
            case '5': // overlay
                ViewID = 5;
                break;

				// ______________  Search mode _____________________________
			case 'm':
				op_mode++;
				if (op_mode > FindObsticals )
					op_mode = 0;
				switch (op_mode) {
					case 0: printf("mode NONE\n");break;
					case 1: printf("mode Find Hook\n");break;
					case 2: printf("mode Find Rock\n");break;
					case 3: printf("mode Find Beacon\n");break;
					case 4: printf("mode Find Obsticals\n");break;
				}
				break;

                // ______________  Display Confidence Map __________________
            case 's':
                displayConfidenceMap = !displayConfidenceMap;
                break;

                //______________ SAVE ______________
            case 'w': // image
                saveSbSimage(zed, std::string("ZEDImage") + std::to_string(count) + std::string(".png"));
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
                dm_type = sl::zed::SENSING_MODE::RAW;
                std::cout << "SENSING_MODE: Raw" << std::endl;
                break;
                
            case 'f':
                dm_type = sl::zed::SENSING_MODE::FULL;
                std::cout << "SENSING_MODE: FULL" << std::endl;
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
				
            case 'd':
                DisplayDisp = !DisplayDisp;
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

    delete zed;
    return 0;
}

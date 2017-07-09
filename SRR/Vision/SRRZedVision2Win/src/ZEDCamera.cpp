#include "stdafx.h"
#include "ZEDCamera.h"

ZEDCamera::ZEDCamera(const char *file)
	: dm_type(sl::SENSING_MODE_STANDARD),
	  old_self_calibration_state (sl::SELF_CALIBRATION_STATE_NOT_STARTED),
	  confidenceLevel(100),
	  ViewID(0),
	  width(0),
	  height(0),
	  bNoFrame(false),
	  IsOpen(false)
{
	zed = new sl::Camera();

	// Setup configuration parameters for the ZED
	sl::InitParameters initParameters;
	if (file != NULL) initParameters.svo_input_filename = file;
	initParameters.camera_resolution = sl::RESOLUTION_HD720;
	initParameters.depth_mode = sl::DEPTH_MODE_PERFORMANCE; //need quite a powerful graphic card in QUALITY
	initParameters.coordinate_units = sl::UNIT_MILLIMETER; // set meter as the OpenGL world will be in meters
	initParameters.coordinate_system = sl::COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP; // OpenGL's coordinate system is right_handed
	initParameters.sdk_verbose = 1;
	initParameters.camera_disable_self_calib = false;

	// Open the ZED
	sl::ERROR_CODE err = zed->open(initParameters);
	if (err != sl::SUCCESS) {
		std::cout << errorCode2str(err) << std::endl;
		zed->close();
		return; // Quit if an error occurred
	}

	IsOpen = true;

	sl::Resolution res = zed->getResolution();

	width = res.width;
	height = res.height;
	
	//Jetson only. Execute the calling thread on core 2
#ifdef __arm__ //only for Jetson K1/X1    
	sl::zed::Camera::sticktoCPUCore(2);
#endif
}

ZEDCamera::~ZEDCamera() 
{
	if (zed)
	{
		zed->close();
		delete zed;
	}
}

cv::Mat ZEDCamera::GrabFrameAndDapth(void)
{
	zed->setConfidenceThreshold(confidenceLevel);

	bNoFrame = zed->grab(dm_type);

	sl::ERROR_CODE res;

	if (!bNoFrame) {
		// Estimated rotation :
		if (old_self_calibration_state != zed->getSelfCalibrationState()) {
			std::cout << "Self Calibration Status : " << sl::statusCode2str(zed->getSelfCalibrationState()) << std::endl;
			old_self_calibration_state = zed->getSelfCalibrationState();
		}

		res = zed->retrieveMeasure(depth ,sl::MEASURE_DEPTH, sl::MEM_CPU); // Get the pointer

		//Even if Left and Right images are still available through getView() function, it's better since v0.8.1 to use retrieveImage for cpu readback because GPU->CPU is done async during depth estimation.
		// Therefore :
		// -- if disparity estimation is enabled in grab function, retrieveImage will take no time because GPU->CPU copy has already been done during disp estimation
		// -- if disparity estimation is not enabled, GPU->CPU copy is done in retrieveImage fct, and this function will take the time of copy.
		if (ViewID == sl::VIEW_LEFT || ViewID == sl::VIEW_RIGHT)
			slMat2cvMat(zed->retrieveImage(static_cast<sl::SIDE> (ViewID))).copyTo(frame);
		else
			slMat2cvMat(zed->getView(static_cast<sl::VIEW_MODE> (ViewID))).copyTo(frame);
	}

	return frame;
}

sl::Mat ZEDCamera::GrabDepth(void)
{
	zed->setConfidenceThreshold(confidenceLevel);

	if (!zed->grab(dm_type))
		depth = zed->retrieveMeasure(sl::MEASURE_DEPTH); // Get the pointer

	return depth;
}

// The following is the best way to save a disparity map/ Image / confidence map in Opencv Mat.
// Be Careful, if you don't save the buffer/data on your own, it will be replace by a next retrieve (retrieveImage, NormalizeMeasure, getView....)
// !! Disparity, Depth, confidence are in 8U,C4 if normalized format !! //
// !! Disparity, Depth, confidence are in 32F,C1 if only retrieve !! //

cv::Mat ZEDCamera::GetNormDisparity(void)
{
	if (!bNoFrame)
		slMat2cvMat(zed->normalizeMeasure(sl::MEASURE_DISPARITY)).copyTo(cvDisparity);

	return cvDisparity;
}

cv::Mat ZEDCamera::GetNormDepth(void)
{
	if (!bNoFrame)
		slMat2cvMat(zed->normalizeMeasure(sl::MEASURE_DEPTH)).copyTo(cvDepth);

	return cvDepth;
}

cv::Mat ZEDCamera::GetNormConfidence(void)
{
	if (!bNoFrame)
		slMat2cvMat(zed->normalizeMeasure(sl::MEASURE_CONFIDENCE)).copyTo(cvConfidence);

	return cvConfidence;
}

void ZEDCamera::ResetCalibration(void)
{
	zed->resetSelfCalibration();
}

int ZEDCamera::GetGain(void)
{
	return zed->getCameraSettingsValue(sl::ZED_GAIN);
}

void ZEDCamera::SetGain(int gain)
{
	//zed->setCameraSettingsValue(sl::zed::ZED_EXPOSURE, 0);	// disable, but don't change the value.
	zed->setCameraSettingsValue(sl::ZED_GAIN, gain);
}

int ZEDCamera::GetExposure(void)
{
	return zed->getCameraSettingsValue(sl::ZED_EXPOSURE);
}

void ZEDCamera::SetExposure(int exp)
{
	zed->setCameraSettingsValue(sl::ZED_EXPOSURE, exp);
}

// save function using opencv
void ZEDCamera::saveSbSimage(std::string filename) 
{
	sl::resolution imSize = zed->getImageSize();

	cv::Mat SbS(imSize.height, imSize.width * 2, CV_8UC4);
	cv::Mat leftIm(SbS, cv::Rect(0, 0, imSize.width, imSize.height));
	cv::Mat rightIm(SbS, cv::Rect(imSize.width, 0, imSize.width, imSize.height));

	slMat2cvMat(zed->retrieveImage(sl::SIDE::LEFT)).copyTo(leftIm);
	slMat2cvMat(zed->retrieveImage(sl::SIDE::RIGHT)).copyTo(rightIm);

	cv::imshow("Saving Image", SbS);
	cv::cvtColor(SbS, SbS, CV_RGBA2RGB);

	cv::imwrite(filename, SbS);
}

cv::Mat slMat2cvMat(sl::Mat& input)
{

	//convert MAT_TYPE to CV_TYPE
	int cv_type = -1;
	switch (input.getDataType())
	{
	case sl::MAT_TYPE_32F_C1: cv_type = CV_32FC1; break;
	case sl::MAT_TYPE_32F_C2: cv_type = CV_32FC2; break;
	case sl::MAT_TYPE_32F_C3: cv_type = CV_32FC3; break;
	case sl::MAT_TYPE_32F_C4: cv_type = CV_32FC4; break;
	case sl::MAT_TYPE_8U_C1: cv_type = CV_8UC1; break;
	case sl::MAT_TYPE_8U_C2: cv_type = CV_8UC2; break;
	case sl::MAT_TYPE_8U_C3: cv_type = CV_8UC3; break;
	case sl::MAT_TYPE_8U_C4: cv_type = CV_8UC4; break;
	default: break;
	}

	// cv::Mat data requires a uchar* pointer. Therefore, we get the uchar1 pointer from sl::Mat (getPtr<T>())
	//cv::Mat and sl::Mat will share the same memory pointer
	return cv::Mat(input.getHeight(), input.getWidth(), cv_type, input.getPtr<sl::uchar1>(MEM_CPU));
}
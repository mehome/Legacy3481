#include "stdafx.h"
#include "ZEDCamera.h"

ZEDCamera::ZEDCamera(const char *file)
	: old_self_calibration_state (sl::SELF_CALIBRATION_STATE_NOT_STARTED),
	  confidenceLevel(100),
	  ViewID(0),
	  bHaveFrame(true),
	  IsOpen(false)
{
	zed = new sl::Camera();

	// Setup configuration parameters for the ZED
	sl::InitParameters initParameters;
	if (file != NULL) initParameters.svo_input_filename = file;
	initParameters.camera_resolution = sl::RESOLUTION_HD720;
	initParameters.depth_mode = sl::DEPTH_MODE_PERFORMANCE; //need quite a powerful graphic card in QUALITY
	initParameters.coordinate_units = sl::UNIT_METER; // set meter as the OpenGL world will be in meters
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

	// disable tracking until we put the camera on a thread.
	zed->disableTracking("");
	zed->disableSpatialMapping();

	IsOpen = true;

	image_size = zed->getResolution();

	runtime_parameters.sensing_mode = sl::SENSING_MODE_STANDARD;

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

sl::ERROR_CODE ZEDCamera::GrabFrameAndDapth(void)
{
	zed->setConfidenceThreshold(confidenceLevel);

	bHaveFrame = (zed->grab(runtime_parameters) == sl::SUCCESS);

	sl::ERROR_CODE res = sl::SUCCESS;

	if (bHaveFrame) {
		// Estimated rotation :
		if (old_self_calibration_state != zed->getSelfCalibrationState()) {
			std::cout << "Self Calibration Status : " << sl::statusCode2str(zed->getSelfCalibrationState()) << std::endl;
			old_self_calibration_state = zed->getSelfCalibrationState();
		}

		res = zed->retrieveMeasure(depth ,sl::MEASURE_DEPTH, sl::MEM_CPU); // Get the pointer
		res = zed->retrieveImage(zedFrame, static_cast<sl::VIEW> (ViewID));
#ifdef USE_POINT_CLOUD 
		zed->retrieveMeasure(point_cloud, sl::MEASURE_XYZRGBA);
#endif
		frame = slMat2cvMat(zedFrame);
	}

	return res;
}

sl::ERROR_CODE ZEDCamera::GrabDepth(void)
{
	sl::ERROR_CODE res = sl::SUCCESS;

	zed->setConfidenceThreshold(confidenceLevel);

	if (zed->grab(runtime_parameters) == sl::SUCCESS)
		res = zed->retrieveMeasure(depth, sl::MEASURE_DEPTH); // Get the pointer

	return res;
}

sl::ERROR_CODE ZEDCamera::GetNormDepth(void)
{
	sl::ERROR_CODE res = sl::SUCCESS;

	if (bHaveFrame)
	{
		res = zed->retrieveImage(Disparity, sl::VIEW_DEPTH);
		cvNormDepth = slMat2cvMat(Disparity);
	}

	return res;
}

sl::ERROR_CODE ZEDCamera::GetNormConfidence(void)
{
	sl::ERROR_CODE res = sl::SUCCESS;

	if (bHaveFrame)
	{
		res = zed->retrieveImage(Confidence, sl::VIEW_CONFIDENCE);
		cvConfidence = slMat2cvMat(Confidence);
	}

	return res;
}

void ZEDCamera::ResetCalibration(void)
{
	zed->resetSelfCalibration();
}

int ZEDCamera::GetGain(void)
{
	return zed->getCameraSettings(sl::CAMERA_SETTINGS_GAIN);
}

void ZEDCamera::SetGain(int gain)
{
	zed->setCameraSettings(sl::CAMERA_SETTINGS_GAIN, gain);
}

int ZEDCamera::GetExposure(void)
{
	return zed->getCameraSettings(sl::CAMERA_SETTINGS_EXPOSURE);
}

void ZEDCamera::SetExposure(int exp)
{
	zed->setCameraSettings(sl::CAMERA_SETTINGS_EXPOSURE, exp);
}

// save function using opencv
void ZEDCamera::saveSbSimage(std::string filename) 
{
	sl::Resolution imSize = zed->getResolution();

	cv::Mat SbS((int)imSize.height, (int)imSize.width * 2, CV_8UC4);
	cv::Mat leftIm(SbS, cv::Rect(0, 0, (int)imSize.width, (int)imSize.height));
	cv::Mat rightIm(SbS, cv::Rect((int)imSize.width, 0, (int)imSize.width, (int)imSize.height));

	sl::Mat left;
	sl::Mat right;
	zed->retrieveImage(left, sl::VIEW::VIEW_LEFT);
	leftIm = slMat2cvMat(left);
	zed->retrieveImage(right, sl::VIEW::VIEW_RIGHT);
	rightIm = slMat2cvMat(right);

	cv::imshow("Saving Image", SbS);
	cv::cvtColor(SbS, SbS, CV_RGBA2RGB);

	cv::imwrite(filename, SbS);
}

cv::Mat ZEDCamera::slMat2cvMat(sl::Mat& input)
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
	return cv::Mat((int)input.getHeight(), (int)input.getWidth(), cv_type, input.getPtr<sl::uchar1>(sl::MEM_CPU));
}
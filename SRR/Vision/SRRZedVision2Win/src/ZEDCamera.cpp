#include "stdafx.h"
#include "ZEDCamera.h"

ZEDCamera::ZEDCamera(const char *file)
	: dm_type (sl::zed::STANDARD),
	  old_self_calibration_status (sl::zed::SELF_CALIBRATION_NOT_CALLED),
	  confidenceLevel(100),
	  ViewID(0),
	  width(0),
	  height(0),
	  bNoFrame(false),
	  IsOpen(false)
{
	if (file == NULL) // Use in Live Mode
		zed = new sl::zed::Camera(sl::zed::HD720, 30);
	else // Use in SVO playback mode
		zed = new sl::zed::Camera(file);

	sl::zed::InitParams parameters;
	parameters.mode = sl::zed::PERFORMANCE;
	parameters.unit = sl::zed::MILLIMETER;
	parameters.verbose = 1;
	parameters.disableSelfCalib = false;

	sl::zed::ERRCODE err = zed->init(parameters);
	std::cout << errcode2str(err) << std::endl;
	if (err != sl::zed::SUCCESS) {
		delete zed;
		zed = NULL;
		return;
	}

	IsOpen = true;

	width = zed->getImageSize().width;
	height = zed->getImageSize().height;
	
	//Jetson only. Execute the calling thread on core 2
#ifdef __arm__ //only for Jetson K1/X1    
	sl::zed::Camera::sticktoCPUCore(2);
#endif
}

ZEDCamera::~ZEDCamera() 
{
	if (zed)
		delete zed;
}

cv::Mat ZEDCamera::GrabFrameAndDapth(void)
{
	zed->setConfidenceThreshold(confidenceLevel);

	bNoFrame = zed->grab(dm_type);

	if (!bNoFrame) {
		// Estimated rotation :
		if (old_self_calibration_status != zed->getSelfCalibrationStatus()) {
			std::cout << "Self Calibration Status : " << sl::zed::statuscode2str(zed->getSelfCalibrationStatus()) << std::endl;
			old_self_calibration_status = zed->getSelfCalibrationStatus();
		}

		depth = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH); // Get the pointer

		//Even if Left and Right images are still available through getView() function, it's better since v0.8.1 to use retrieveImage for cpu readback because GPU->CPU is done async during depth estimation.
		// Therefore :
		// -- if disparity estimation is enabled in grab function, retrieveImage will take no time because GPU->CPU copy has already been done during disp estimation
		// -- if disparity estimation is not enabled, GPU->CPU copy is done in retrieveImage fct, and this function will take the time of copy.
		if (ViewID == sl::zed::STEREO_LEFT || ViewID == sl::zed::STEREO_RIGHT)
			slMat2cvMat(zed->retrieveImage(static_cast<sl::zed::SIDE> (ViewID))).copyTo(frame);
		else
			slMat2cvMat(zed->getView(static_cast<sl::zed::VIEW_MODE> (ViewID))).copyTo(frame);
	}

	return frame;
}

sl::zed::Mat ZEDCamera::GrabDepth(void)
{
	zed->setConfidenceThreshold(confidenceLevel);

	if (!zed->grab(dm_type))
		depth = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH); // Get the pointer

	return depth;
}

// The following is the best way to save a disparity map/ Image / confidence map in Opencv Mat.
// Be Careful, if you don't save the buffer/data on your own, it will be replace by a next retrieve (retrieveImage, NormalizeMeasure, getView....)
// !! Disparity, Depth, confidence are in 8U,C4 if normalized format !! //
// !! Disparity, Depth, confidence are in 32F,C1 if only retrieve !! //

cv::Mat ZEDCamera::GetNormDisparity(void)
{
	if (!bNoFrame)
		slMat2cvMat(zed->normalizeMeasure(sl::zed::MEASURE::DISPARITY)).copyTo(cvDisparity);

	return cvDisparity;
}

cv::Mat ZEDCamera::GetNormDepth(void)
{
	if (!bNoFrame)
		slMat2cvMat(zed->normalizeMeasure(sl::zed::MEASURE::DEPTH)).copyTo(cvDepth);

	return cvDepth;
}

cv::Mat ZEDCamera::GetNormConfidence(void)
{
	if (!bNoFrame)
		slMat2cvMat(zed->normalizeMeasure(sl::zed::MEASURE::CONFIDENCE)).copyTo(cvConfidence);

	return cvConfidence;
}

void ZEDCamera::ResetCalibration(void)
{
	zed->resetSelfCalibration();
}

int ZEDCamera::GetGain(void)
{
	return zed->getCameraSettingsValue(sl::zed::ZED_GAIN);
}

void ZEDCamera::SetGain(int gain)
{
	//zed->setCameraSettingsValue(sl::zed::ZED_EXPOSURE, 0);	// disable, but don't change the value.
	zed->setCameraSettingsValue(sl::zed::ZED_GAIN, gain);
}

int ZEDCamera::GetExposure(void)
{
	return zed->getCameraSettingsValue(sl::zed::ZED_EXPOSURE);
}

void ZEDCamera::SetExposure(int exp)
{
	zed->setCameraSettingsValue(sl::zed::ZED_EXPOSURE, exp);
}

// save function using opencv
void ZEDCamera::saveSbSimage(std::string filename) 
{
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


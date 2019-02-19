#include "stdafx.h"
#include "ZEDCamera.h"

ZEDCamera::ZEDCamera()
	: old_self_calibration_state(sl::SELF_CALIBRATION_STATE_NOT_STARTED),
 	  confidenceLevel(100),
	  ViewID(0),
	  IsOpen(false),
	  quit(false),
	  zed(NULL)
{
}

ZEDCamera::ZEDCamera(const char *file)
	: old_self_calibration_state (sl::SELF_CALIBRATION_STATE_NOT_STARTED),
	  confidenceLevel(100),
	  ViewID(0),
	  IsOpen(false),
   	  quit(false),
	  zed(NULL)
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
		std::cout << toString(err) << std::endl;
		zed->close();
		return; // Quit if an error occurred
	}

	// Set positional tracking parameters
	sl::TrackingParameters trackingParameters;
	trackingParameters.initial_world_transform = sl::Transform::identity();
	trackingParameters.enable_spatial_memory = true;

	// Start motion tracking
	zed->enableTracking(trackingParameters);

	IsOpen = true;

	image_size = zed->getResolution();

	runtime_parameters.sensing_mode = sl::SENSING_MODE_STANDARD;
	updateCameraSettings('r');
	loadSettings();

	zed->setConfidenceThreshold(confidenceLevel);

	// Print camera information
	printf("ZED Model                 : %s\n", sl::toString(zed->getCameraInformation().camera_model).c_str());
	printf("ZED Serial Number         : %d\n", zed->getCameraInformation().serial_number);
	printf("ZED Firmware              : %d\n", zed->getCameraInformation().firmware_version);
	printf("ZED Camera Resolution     : %dx%d\n", (int)zed->getResolution().width, (int)zed->getResolution().height);
	printf("ZED Camera FPS            : %d\n", (int)zed->getCameraFPS());

	grab_thread = std::thread(&ZEDCamera::grab_run, this);
}

ZEDCamera::~ZEDCamera() 
{
	close();
}

void ZEDCamera::close(void)
{
	IsOpen = false;
	quit = true;
	if(grab_thread.joinable())
		grab_thread.join();
	if (zed)
	{
		zed->close();
		delete zed;
	}
}

void ZEDCamera::grab_run()
{
	// Get the distance between the center of the camera and the left eye
	float translation_left_to_center = zed->getCameraInformation().calibration_parameters.T.x * 0.5f;

	while (!quit)
	{
		if (zed->grab(runtime_parameters) == sl::SUCCESS) {
			std::lock_guard<std::mutex> guard(mtx);

			// Estimated rotation :
			if (old_self_calibration_state != zed->getSelfCalibrationState()) {
				std::cout << "Self Calibration Status : " << sl::toString(zed->getSelfCalibrationState()) << std::endl;
				old_self_calibration_state = zed->getSelfCalibrationState();
			}

			// Get the position of the camera in a fixed reference frame (the World Frame)
			sl::TRACKING_STATE tracking_state = zed->getPosition(camera_pose, sl::REFERENCE_FRAME_WORLD);

			if (tracking_state == sl::TRACKING_STATE_OK) {
				// getPosition() outputs the position of the Camera Frame, which is located on the left eye of the camera.
				// To get the position of the center of the camera, we transform the pose data into a new frame located at the center of the camera.
				// The generic formula used here is: Pose(new reference frame) = M.inverse() * Pose (camera frame) * M, where M is the transform between two frames.
				transformPose(camera_pose.pose_data, translation_left_to_center); // Get the pose at the center of the camera (baseline/2 on X axis)
				if (pose_queue.size() >= max_queue_aize)
					pose_queue.pop();

				pose_queue.push(camera_pose);
			}

			zed->retrieveMeasure(depth, sl::MEASURE_DEPTH, sl::MEM_CPU); // Get the pointer
			zed->retrieveImage(zedFrame, static_cast<sl::VIEW> (ViewID));
			zed->retrieveMeasure(point_cloud, sl::MEASURE_XYZRGBA);

			if (frame_queue.size() >= max_queue_aize)
				frame_queue.pop();

			frame_queue.push(zedFrame);

			if (depth_queue.size() >= max_queue_aize)
				depth_queue.pop();

			depth_queue.push(depth);

			if (pointcl_queue.size() > max_queue_aize)
				pointcl_queue.pop();

			pointcl_queue.push(point_cloud);
		}
		else sl::sleep_ms(1);
	}
}

/**
 **  Trasnform pose to create a Tracking Frame located in a separate location from the Camera Frame
 **/
void ZEDCamera::transformPose(sl::Transform &pose, float tx) 
{
	sl::Transform transform_;
	transform_.setIdentity();
	// Move the tracking frame by tx along the X axis
	transform_.tx = tx;
	// Apply the transformation
	pose = sl::Transform::inverse(transform_) * pose * transform_;
}

sl::Mat ZEDCamera::GetFrame(void)
{
	// wait for a frame if request is ahead of the camera
	while (frame_queue.empty());

	std::lock_guard<std::mutex> guard(mtx);

	sl::Mat frame;
	do {	// get most recent frames, discard old
		frame = frame_queue.front();
		frame_queue.pop();
	} while (frame.timestamp < zed->getTimestamp(sl::TIME_REFERENCE_IMAGE) && frame_queue.size() > 0);

	return frame;
}

sl::Mat ZEDCamera::GetDepth(void)
{
	// wait for a frame if request is ahead of the camera
	while (depth_queue.empty());
	
	std::lock_guard<std::mutex> guard(mtx);

	sl::Mat depth;
	do {	// get most recent frames, discard old
		depth = depth_queue.front();
		depth_queue.pop();
	} while (depth.timestamp < zed->getTimestamp(sl::TIME_REFERENCE_IMAGE) && depth_queue.size() > 0);

	return depth;
}

sl::Mat ZEDCamera::GetPointCloud(void)
{
	// wait for a frame if request is ahead of the camera
	while (pointcl_queue.empty());
	
	std::lock_guard<std::mutex> guard(mtx);

	sl::Mat pointcl;
	do {	// get most recent frames, discard old
		pointcl = pointcl_queue.front();
		pointcl_queue.pop();
	} while (pointcl.timestamp < zed->getTimestamp(sl::TIME_REFERENCE_IMAGE) && pointcl_queue.size() > 0);

	return pointcl;
}

bool ZEDCamera::HavePose(void)
{
	return !pose_queue.empty();
}

sl::Pose ZEDCamera::GetPose(void)
{
	// wait for a pose update if request is ahead of the camera
	while (pose_queue.empty());

	std::lock_guard<std::mutex> guard(mtx);

	sl::Pose cam_pose;
	do {
		cam_pose = pose_queue.front();
		pose_queue.pop();
	} while (cam_pose.timestamp < zed->getTimestamp(sl::TIME_REFERENCE_IMAGE) && pose_queue.size() > 0);

	return camera_pose;
}

cv::Mat ZEDCamera::GetView(void)
{
	return slMat2cvMat(GetFrame());
}

void ZEDCamera::ResetCalibration(void)
{
	zed->resetSelfCalibration();
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
	// cv::Mat and sl::Mat will share the same memory pointer
	return cv::Mat((int)input.getHeight(), (int)input.getWidth(), cv_type, input.getPtr<sl::uchar1>(sl::MEM_CPU));
}

/**
This function saves current camera settings
**/
void ZEDCamera::saveSettings(void)
{
	std::ofstream myfile;
	myfile.open("camera_settings");
	myfile << zed->getCameraSettings(sl::CAMERA_SETTINGS_BRIGHTNESS) << std::endl;
	myfile << zed->getCameraSettings(sl::CAMERA_SETTINGS_CONTRAST) << std::endl;
	myfile << zed->getCameraSettings(sl::CAMERA_SETTINGS_HUE) << std::endl;
	myfile << zed->getCameraSettings(sl::CAMERA_SETTINGS_SATURATION) << std::endl;
	myfile << zed->getCameraSettings(sl::CAMERA_SETTINGS_GAIN) << std::endl;
	myfile << zed->getCameraSettings(sl::CAMERA_SETTINGS_EXPOSURE) << std::endl;
	myfile << zed->getCameraSettings(sl::CAMERA_SETTINGS_WHITEBALANCE) << std::endl;
	myfile.close();
	std::cout << "Camera settings saved." << std::endl;
}

/**
This function loads camera settings
**/
bool ZEDCamera::loadSettings(void)
{
	bool ret = false;
	std::string line;
	std::string::size_type sz;   // alias of size_t

	std::ifstream myfile("camera_settings");
	if (myfile.is_open())
	{
		for (int i = 0; i < sl::CAMERA_SETTINGS_LAST; i++)
		{
			if (getline(myfile, line))
			{
				int setting = std::stoi(line, &sz);
				zed->setCameraSettings((sl::CAMERA_SETTINGS)i, setting);
			}
		}
		myfile.close();
		std::cout << "camera settings loaded." << std::endl;

		ret = true;
	}
	else std::cout << "Unable to open camera settings" << std::endl;

	return ret;
}

/**
This function updates camera settings
**/
void ZEDCamera::updateCameraSettings(int key) {
	int current_value;

	// Keyboard shortcuts
	switch (key) {

		// Switch to the next camera parameter
	case 's':
		switchCameraSettings();
		break;

		// Increase camera settings value ('+' key)
	case '+':
		current_value = zed->getCameraSettings(camera_settings_);
		zed->setCameraSettings(camera_settings_, current_value + step_camera_setting);
		std::cout << str_camera_settings << ": " << current_value + step_camera_setting << std::endl;
		break;

		// Decrease camera settings value ('-' key)
	case '-':
		current_value = zed->getCameraSettings(camera_settings_);
		if (current_value >= 1) {
			zed->setCameraSettings(camera_settings_, current_value - step_camera_setting);
			std::cout << str_camera_settings << ": " << current_value - step_camera_setting << std::endl;
		}
		break;

		// Reset to default parameters
	case 'r':
		std::cout << "Reset all settings to default" << std::endl;
		zed->setCameraSettings(sl::CAMERA_SETTINGS_BRIGHTNESS, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_CONTRAST, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_HUE, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_SATURATION, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_GAIN, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_EXPOSURE, -1, true);
		zed->setCameraSettings(sl::CAMERA_SETTINGS_WHITEBALANCE, -1, true);
		break;

	case 0x00780000:	saveSettings(); break;
	case 0x00790000:	loadSettings(); break;

	}
}

/**
This function toggles between camera settings
**/
void ZEDCamera::switchCameraSettings(void) {
	step_camera_setting = 1;
	switch (camera_settings_) {
	case sl::CAMERA_SETTINGS_BRIGHTNESS:
		camera_settings_ = sl::CAMERA_SETTINGS_CONTRAST;
		str_camera_settings = "Contrast";
		std::cout << "Camera Settings: CONTRAST" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_CONTRAST:
		camera_settings_ = sl::CAMERA_SETTINGS_HUE;
		str_camera_settings = "Hue";
		std::cout << "Camera Settings: HUE" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_HUE:
		camera_settings_ = sl::CAMERA_SETTINGS_SATURATION;
		str_camera_settings = "Saturation";
		std::cout << "Camera Settings: SATURATION" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_SATURATION:
		camera_settings_ = sl::CAMERA_SETTINGS_GAIN;
		str_camera_settings = "Gain";
		std::cout << "Camera Settings: GAIN" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_GAIN:
		camera_settings_ = sl::CAMERA_SETTINGS_EXPOSURE;
		str_camera_settings = "Exposure";
		std::cout << "Camera Settings: EXPOSURE" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_EXPOSURE:
		camera_settings_ = sl::CAMERA_SETTINGS_WHITEBALANCE;
		str_camera_settings = "White Balance";
		step_camera_setting = 100;
		std::cout << "Camera Settings: WHITE BALANCE" << std::endl;
		break;

	case sl::CAMERA_SETTINGS_WHITEBALANCE:
		camera_settings_ = sl::CAMERA_SETTINGS_BRIGHTNESS;
		str_camera_settings = "Brightness";
		std::cout << "Camera Settings: BRIGHTNESS" << std::endl;
		break;
	}
}

void ZEDCamera::printCameraSettings(void)
{
	if (IsOpen)
	{
		std::cout << std::endl;
		std::cout << "Camera settings: " << std::endl;
		std::cout << "BRIGHTNESS " << zed->getCameraSettings(sl::CAMERA_SETTINGS_BRIGHTNESS) << std::endl;
		std::cout << "CONTRAST " << zed->getCameraSettings(sl::CAMERA_SETTINGS_CONTRAST) << std::endl;
		std::cout << "HUE " << zed->getCameraSettings(sl::CAMERA_SETTINGS_HUE) << std::endl;
		std::cout << "SATURATION " << zed->getCameraSettings(sl::CAMERA_SETTINGS_SATURATION) << std::endl;
		std::cout << "GAIN " << zed->getCameraSettings(sl::CAMERA_SETTINGS_GAIN) << std::endl;
		std::cout << "EXPOSURE " << zed->getCameraSettings(sl::CAMERA_SETTINGS_EXPOSURE) << std::endl;
		std::cout << "WHITEBALANCE " << zed->getCameraSettings(sl::CAMERA_SETTINGS_WHITEBALANCE) << std::endl;
		std::cout << std::endl;
	}
}

#pragma once

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

#pragma warning (disable: 4251)		// TODO: see if there is a fix that doesn't require modifying zed includes.
#pragma warning (disable: 4756)
#pragma warning (disable: 4056)

//ZED Includes
#include <sl/Camera.hpp>

#define USE_POINT_CLOUD

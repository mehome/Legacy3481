#pragma once

//standard includes
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <chrono>
#include <windows.h>

//opencv includes
#include <opencv2\cvconfig.h>
#include <opencv2/opencv.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/calib3d.hpp>
#if defined(HAVE_CUDA)
#include "opencv2/cudaobjdetect.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudawarping.hpp"
#endif

#pragma warning (disable: 4251)		// TODO: see if there is a fix that doesn't require modifying zed includes.
#pragma warning (disable: 4756)
#pragma warning (disable: 4056)

//ZED Includes
#include <sl/Camera.hpp>

//#define OLDSCHOOL_TIMER

This is a Windows version of the Zed camera code for the SRR robot.
This is based on the Zed SDK 2.3.0.
Cascade data is moved to the bin folder where it is needed.

installing the latest sdk seems to wipe environment variables.
set them as below.
(probably better to base them from ZED_SDK_ROOT_DIR which is what the installer sets.)

Environment variables:
includes - 
OPENCV_DIR = C:\Program Files (x86)\ZED SDK\dependencies\opencv_3.1.0
CUDA_PATH - set when installing latest cuda.
ZED_INCLUDE_DIRS = C:\Program Files (x86)\ZED SDK\include
library dirs - 
ZED_LIBRARY_DIR = C:\Program Files (x86)\ZED SDK\lib
additional deps - 
ZED_LIBRARIES_64 = sl_core64.lib;sl_input64.lib;sl_zed64.lib

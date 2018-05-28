This is a Windows version of the Zed camera code for the SRR robot.
This is based on the Zed SDK 2.4.0.
Cascade data is moved to the bin folder where it is needed.

installing the latest sdk seems to wipe environment variables.
set them as below.

Environment variables:
includes - 
openCV is found in deps folder. the path depends on the config.
CUDA_PATH - set when installing latest cuda.
ZED_SDK_ROOT_DIR\include
library dirs - 
ZED_SDK_ROOT_DIR\lib


Other build considerations:
The bin directory contains a folder called deps which has the latest version of the zed sdk, 
and has opencv 3.1.0 (including a version for vs 2013) which is the same as the one included in the Zed sdk, and opencv 3.4.0.

great post on how to build opencv 3.4 with cuda support
https://jamesbowley.co.uk/build-compile-opencv-3-4-in-windows-with-cuda-9-0-and-intel-mkl-tbb/

GPUImageHarrisCornerDetectionFilter - for findcorners replacement?
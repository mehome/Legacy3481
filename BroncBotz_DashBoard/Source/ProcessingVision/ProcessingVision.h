#pragma once

#ifdef PROCESSINGVISION_EXPORTS
#define PROCESSINGVISION_API __declspec(dllexport)
#else
#define PROCESSINGVISION_API __declspec(dllimport)
#endif

struct Bitmap_Frame
{
	Bitmap_Frame(PBYTE memory,size_t xres,size_t yres,size_t stride) : Memory(memory),XRes(xres),YRes(yres),Stride(stride) {}
	PBYTE Memory;  //A pointer to memory of the frame... this can be written over for compositing effects
	size_t XRes,YRes;  //X and Y res in pixels, these may also change but typically should stay the same (and return same memory)
	size_t Stride;  //This is >= Xres some memory buffers need extra room for processing... this can be changed (and might need to be)
};

/// \param Frame this will contain incoming frames from a stream (e.g. a camera) and can analyze the frames for targeting inquiry.
/// \ret This gives the ability to control what gets shown on the video feed (i.e. preview) This makes it possible to make composite overlaying 
/// (e.g. targeting reticle or bound box feedback of acquired target).  You can return the same frame sent or create a new one that has
/// different properties typically the stride may be different depending on the kind of buffers you need to allocate
/// The format color space of the memory is RGB 24.  This does not change and must remain this for frames returned
extern "C" PROCESSINGVISION_API Bitmap_Frame *ProcessFrame_RGB24(Bitmap_Frame *Frame);

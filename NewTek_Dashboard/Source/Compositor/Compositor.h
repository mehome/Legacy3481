#pragma once
#include "../Dashboard/Dashboard_Interfaces.h"

#ifdef COMPOSITER_EXPORTS
#define COMPOSITER_API __declspec(dllexport)
#else
#define COMPOSITER_API __declspec(dllimport)
#endif

// No C library depreciation warnings
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

extern "C" 
{
	/// \param IPAddress- this may be null if user does not wish to send out UDP packets
	COMPOSITER_API void Callback_SmartCppDashboard_Initialize(const char *IPAddress,Dashboard_Framework_Interface *DashboardHelper);
	/// This is called when we are about to close will all for creation and destruction of classes
	COMPOSITER_API void Callback_SmartCppDashboard_Shutdown();
	COMPOSITER_API Plugin_Controller_Interface *Callback_CreatePluginControllerInterface();
	COMPOSITER_API void Callback_DestroyPluginControllerInterface(Plugin_Controller_Interface *plugin);

	/// \param Frame this will contain incoming frames from a stream (e.g. a camera) and can analyze the frames for targeting inquiry.
	/// \ret This gives the ability to control what gets shown on the video feed (i.e. preview) This makes it possible to make composite overlaying 
	/// (e.g. targeting reticle or bound box feedback of acquired target).  You can return the same frame sent or create a new one that has
	/// different properties typically the stride may be different depending on the kind of buffers you need to allocate
	/// The format color space of the memory is BGRA 32.  This does not change and must remain this for frames returned
	/// \note The color space BGRA 32 has been chosen since this is what NI_Vision uses, we can however change this to some other format (e.g YUV 4.2.2)
	/// if necessary.  The m1011 camera in h264 mode produces YUV 420p by default, and this gets converted to desired colorspace.
	COMPOSITER_API Bitmap_Frame *ProcessFrame_UYVY(Bitmap_Frame *Frame);
};

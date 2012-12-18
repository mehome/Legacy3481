#ifndef TEST_H
#define TEST_H
#include "DVCam.h"

class testclass {

	public:
		HWND window;
		class DVCamlib *dvcamlib;

		testclass();
		~testclass();
		handletest(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void createwindow(HWND w_ptr);
		void startup();
		void shutdown();
		void resize();
		};

#endif /* TEST_H */

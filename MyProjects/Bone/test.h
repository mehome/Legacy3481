#ifndef TEST_H
#define TEST_H

class testclass : public messagebase {

	public:
		HWND window;

		testclass();
		~testclass();
		Callback(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void createwindow(HWND w_ptr);
		void startup();
		void shutdown();
		void resize();
		};

#endif /* TEST_H */

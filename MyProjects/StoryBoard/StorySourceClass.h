#ifndef STORYSOURCECLASS_H
#define STORYSOURCECLASS_H
#include "StorySourceBase.h"

class storysourceclass : public messagebase {
	public:

		class mediaclass : public storysourcebase {
			public:
				//This group initializes the media class
				mediaclass();
				void createwindow(class storyboardclass *storyboard,HWND w_ptr);
				~mediaclass();
			} media;

		class dvesourceclass : public storysourcebase {
			public:
				dvesourceclass();
				void createwindow(class storyboardclass *storyboard,HWND w_ptr);
				~dvesourceclass();
			} fxobject;

		class filtersourceclass : public storysourcebase {
			public:
				filtersourceclass();
				void createwindow(class storyboardclass *storyboard,HWND tabwindow);
				~filtersourceclass();
			} filtersourceobj;

		class audiosourceclass : public storysourcebase {
			public:
				audiosourceclass();
				void createwindow(class storyboardclass *storyboard,HWND tabwindow);
				~audiosourceclass();
			} audiosourceobj;

		HWND tabwindow,window;

		storysourceclass();
		int Callback(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void createwindow(class storyboardclass *storyboard,HWND w_ptr);
		void startup();
		void resize();
		void shutdown();
		~storysourceclass();

	private:
		HBITMAP smartrefreshclip;
	};

#endif /* STORYSOURCECLASS_H */

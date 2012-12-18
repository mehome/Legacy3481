#ifndef BMPLISTBOX_H
#define BMPLISTBOX_H

class bmplistbox {

	public:
		HWND bmplstwindow,bmplstframe;
		HBITMAP itembmp_ptr[1000];
		
		bmplistbox();
		void measureitem(LPARAM lParam);
		int drawitem(LPARAM lParam);
		void addbmp();
		void shutdown();
		~bmplistbox();

	protected:
		void additem (HWND w_ptr,char *itemname,HBITMAP bmp_ptr);
		LPMEASUREITEMSTRUCT lpmis;
		HBITMAP hbmppicture,hbmpold;
		UWORD itemindex;
	};

class transitionclass:public bmplistbox {
	public:
		transitionclass();
		handletransition(class dragthisclass *dragthis,HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void startup(HWND w_ptr);
		~transitionclass();
	};


#endif /* BMPLISTBOX_H */

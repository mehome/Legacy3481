#ifndef DRAGOBJECT_H
#define DRAGOBJECT_H


class dragthisclass : public messagebase {
	public:
	UINT dragmsg;
	HWND dragwindow;
	struct imagelist *dragimage;
	UWORD dragorigin;
	BOOL dragtoggle;

		dragthisclass();
		void dragthisclass::updateimage(struct imagelist *dragimage,UWORD originator);
		void createwindow(HWND w_ptr);
		void shutdown();
		~dragthisclass();

	private:
		int Callback(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		HBITMAP smartrefreshclip;
	};

#endif /* DRAGOBJECT_H */

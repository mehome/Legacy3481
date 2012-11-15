#ifndef PROGRESS_H
#define PROGRESS_H

struct progressparms {
	//Window stuff
	char *name;
	int x,y,width,height;
	HWND parent;
	//skins
	HBITMAP backbmp,barbmp; //most likely already created
	POINT backloc,barloc;
	};

class progressclass : public messagebase {
	public:
		HWND window;

		progressclass(progressparms *parms);
		~progressclass();
		int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);

		void updateimage(int num,int den);
	private:
		int x,y,width,height,progress;
		POINT backloc,barloc;
		HBITMAP backbmp,barbmp;
	};

#endif /* PROGRESS_H */

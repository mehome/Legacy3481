#ifndef MEDIA_H
#define MEDIA_H

#define MD_TXTOFFSET 10
#define MD_OFFSETX 20
#define MD_OFFSETY 16
#define MD_YSIZE HALFYBITMAP+16
#define MD_XSIZE HALFXBITMAP+MD_TXTOFFSET+md_txt_xsize-MD_OFFSETX


class mediaclass {
	public:
		struct imagelist *imagelisthead,*imageptr;
		HWND mediawindow;

		mediaclass();
		handlemedia(class dragthisclass *dragthis,HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void makeglow(UWORD y);
		void startup(HWND w_ptr);
		void resize(HWND parent);
		void configuresize();
		void tempsetlist();
		void updateimagelist();
		void shutdown();
		~mediaclass();

	private:
		HBITMAP smartrefreshclip;
		UBYTE numcolumns,numrows;
		UWORD md_txt_xsize;
		short olditem; //For Focus
	};

#endif /* MEDIA_H */

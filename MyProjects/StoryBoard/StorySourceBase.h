#ifndef STORYSOURCEBASE_H
#define STORYSOURCEBASE_H

#include "selectimage.h"

#define MD_TXTOFFSET 10
#define MD_OFFSETX 20
#define MD_OFFSETY 30 //4+TOOLBUTTONY+6
#define MD_YSIZE HALFYBITMAP+16
#define MD_XSIZE HALFXBITMAP+MD_TXTOFFSET+md_txt_xsize-MD_OFFSETX
#define MAXDRVBUFSIZE 256
#define TB_IDCMEDIAOFFSET 104  //Ownerdrawn Media toolbar blt

class storysourcebase : public messagebase {
	public:
		struct cachefile {
			struct cachefile *next;
			struct cachefile *prev;
			DWORD filetype;
			char *filename;
			DWORD sizeh;
			DWORD sizel;
			FILETIME date;
			UWORD filenumber;
			union {
				mediatypes mediatype;
				audiosourcetypes audiotype;
				};
			UWORD frames;
			};
		struct cachedir {
			struct cachedir *next;
			struct cachedir *prev;
			char *dirpath;
			struct cachefile *dirlist;
			struct cachefile *filelist;
		} *dirlisthead;
		HWND window;
		HWND toolwindow;
		class selectimageclass *selectimageobj;
		struct imagelist *imagelisthead,*scrollpos;
		static HBITMAP folder;
		static HBITMAP errorimage;
		static HBITMAP audioimage;
		static HBITMAP audiohalfimage;
		static char drvspecbuf[];
		static DWORD drvbufsize;

		storysourcebase();
		int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		int handlestorysource(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		int handletools(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void resize(HWND parent);
		void updateimagelist();
		void closedirectory(struct imagelist *index);
		void startup();
		void shutdown();
		~storysourcebase();
		void changedir(class storyboardclass *storyboard,char *newdir);

	protected:
		struct nodevars *medianodeobject;
		HBITMAP smartrefreshclip;
		struct imagelist *imageindex;
		char *currentdir;
		UWORD md_txt_xsize;
		short olditem; //For Focus
		UWORD oldy;
		imageidentifier id;
		UBYTE numcolumns,numrows;

		//This group manages the cache
		void initdirlist(char *initpath);
		void updatefilelist(struct cachedir *dirlist);
		struct imagelist *cachemanager(class storyboardclass *storyboard,char *path);
		mediatypes getmediatype(char *filename);
		mediatypes getdvetype(char *filename);
		mediatypes getfiltertype(char *filename);
		audiosourcetypes getaudiotype(char *filename);

	private:
		HWND drvspeccombo;

		void makeglow(UWORD y);
		void configuresize();
	};

#endif /* STORYSOURCEBASE_H */

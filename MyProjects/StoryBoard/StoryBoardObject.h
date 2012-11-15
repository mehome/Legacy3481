#ifndef STORYBOARDOBJECT_H
#define STORTBOARDOBJECT_H

#define TB_NUMOFBUTTONS 5
#define TB_IDCOFFSET 1010
enum pointermodes {selection,dragdrop};

class storyboardclass : public messagebase {
	public:
		class storyboardtools : public messagebase {
			public:
				HWND toolwindow,inwindow,outwindow,durwindow;
				class miniscrub *miniscrubobj;

				void createwindow(HWND w_ptr);
				void startup();
				void shutdown();
				void updatetoolbar(struct imagelist *streamptr);
			private:
				int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
			} tools;

		class selectimageclass *selectimageobj;
		struct imagelist *imagelisthead,*imagelisttail,*scrollpos;
		HWND storywindow;
		HBITMAP fillerimage;
		struct imagelist *oldstreamptr;
		int imagecount,scrolloffset;
		UWORD oldglowpos;
		pointermodes pointermode;
		ULONG framecounter;
		UBYTE numcolumns,numrows;

		static HBITMAP glowbmp;

		storyboardclass();
		int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		long clipduration(struct imagelist *index,struct imagelist *tail,struct imagelist *dragptr);
		BOOL insertimage(struct imagelist *dragptr,struct imagelist *index,struct imagelist *tail);
		void removeimage(struct imagelist *index, BOOL andcloseit);
		void closeimage(struct imagelist *index);
		short getitem(UWORD x,UWORD y,struct imagelist **index,struct imagelist **tail);
		void makeglow(UWORD x,UWORD y,short offsetx,UWORD offsety);
		void resetframecounter();
		void adjustscrollbar();
		void createwindow(HWND w_ptr);
		void startup();
		void resize();
		void configuresize();
		void updateglow(UWORD glowpos,struct imagelist *streamptr);
		void updateimagelist();
		void shutdown();
		void closeproject();
		void premediafx(struct imagelist *index);
		void postmediafx(struct imagelist *index);
		~storyboardclass();

	private:
		HBITMAP smartrefreshclip;
		BLENDFUNCTION bf;
		struct imagelist *removethis;
		short olditem,newitem;
		UWORD oldx,oldy;
		UBYTE sb_offsetx,sb_offsety;
		UBYTE freezeglow;

	};

#endif /* STORYBOARDOBJECT_H */

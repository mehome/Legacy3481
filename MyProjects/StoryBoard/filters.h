#ifndef FILTERS_H
#define FILTERS_H

#include "textedit.h"

#define FT_OFFSETX 10
#define FT_OFFSETY 20
#define FT_YSIZE 50
#define FT_XSIZE 100
#define CG_CLOSE		200
#define CG_IN			201
#define CG_OUT			202
#define CG_DUR			203
#define CG_XY			204
#define CG_XP			205
#define CG_X			206
#define CG_YP			207
#define CG_Y			208
#define CG_XYRANGEBT	209
#define CG_XYRANGEED	210
#define CG_DEPTHTB	211
#define CG_DEPTHED	212
#define CG_DEPTHRCHK	213
#define CG_DEPTHRBT	214
#define CG_DEPTHPLOT	215
#define CG_XYRCHK		216
#define CG_XYRBT		217
#define CG_XYPLOT		218


class filtersclass : public messagebase {
	public:
		class filterscommon : public messagebase {
			public:
			struct listofptrs *windowhead;
			struct listofptrs *windowindex;
			struct listofptrs *windowprev;
			ULONG windowlistsize;

			filterscommon();
			~filterscommon();
			virtual HWND getnewgui(struct filterswindowlist *CGwindowptr,HWND filterwindow,ULONG count)=0;
			virtual void initcontrols(struct filterswindowlist *CGwindowptr,struct filternode *filterptr)=0;
			virtual void closefilter(struct filternode *filter)=0;
			};

		class CGclass : public filterscommon {
			public:
			struct CGgui {
				struct filterswindowlist windownode;
				HWND caption;
				HWND inwindow;
				HWND outwindow;
				HWND durwindow;
				HWND x;
				HWND y;
				HWND depthed;
				HWND depthscrub;
				HWND depthrchk;
				HWND xyrchk;
				class textedit *depthred;
				class textedit *xyred;
				};

			CGclass();
			~CGclass();
			int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
			void xytext2points (struct filterCG *filter,struct imagelist *media);
			void depthtext2points (struct filterCG *filter,struct imagelist *media);
			char *makexytext (struct filterCG *filter);
			char *makedepthtext (struct filterCG *filter);
			void updatexyrange (struct filterCG *filter,struct imagelist *media);
			void initxyrange (struct filterCG *filter,struct CGgui *CGwindowindex);
			void updatedepthrange (struct filterCG *filter,struct imagelist *media);
			void initdepthrange (struct filterCG *filter,struct CGgui *CGwindowindex);
			void closefilter(struct filternode *filter);
			void initcontrols(struct filterswindowlist *CGwindowptr,struct filternode *filterptr);
			void getcommonCGgui(struct CGgui *CGwindowindex,HWND filterwindow,HWND maincontrol,int tooly);
			HWND getnewgui(struct filterswindowlist *CGwindowptr,HWND filterwindow,ULONG count);
			struct filterCG  *initfilterCG(struct imagelist *dragimage,struct imagelist *streamptr);

			protected:
			ULONG *alphayuv(char *decodedbits,int x,int y,BOOL upsidedown,UBYTE **alpha,UWORD *width,UWORD *height);

			private:
			BOOL CGtoggle;

			ULONG *opentga(char *filesource,UBYTE **alpha,UWORD *width,UWORD *height);
			char *gettoken(char token,char *textindex,char *textmax);
			char *getnumber(char *textindex,char *textmax,int *number);
			void clearxypoints(struct filterCG *filter);
			void cleardepthpoints(struct filterCG *filter);
			void adddepthpoint(struct filterCG *filter,ULONG mediatime,UBYTE trans);
			void addxypoint(struct filterCG *filter,ULONG mediatime,short x,short y);
			} CGobj;

		class titlerCGclass : public CGclass {
			public:
			struct titlergui {
				struct CGgui basegui;
				HWND titleED;
				};

			titlerCGclass();
			~titlerCGclass();
			struct filterCG  *initfiltertitlerCG(struct imagelist *dragimage,struct imagelist *streamptr);
			void initcontrols(struct filterswindowlist *CGwindowptr,struct filternode *filterptr);
			HWND getnewgui(struct filterswindowlist *CGwindowptr,HWND filterwindow,ULONG count);

			private:
			int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
			ULONG *fontyuv(struct titlerCG *source,UBYTE **alpha,UWORD *width,UWORD *height);
			char *font2raw (struct titlerCG *source,UWORD *x,UWORD *y);
			} titlerCGobj;


		struct filterswindowlist *windowlist;
		class miniscrub *miniscrubobj;

		HWND window;
		int Callback(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		filtersclass();
		void createwindow(HWND w_ptr);
		void resize();
		void resizefilter(HWND filterwindow,ULONG y,ULONG height);
		void updateimagelist();
		void startup();
		void closefilters(struct imagelist *media);
		void shutdown();
		~filtersclass();
		void filterimage(ULONG *videobuf,struct imagelist *mediaptr,long mediatime);
		struct filternode *getnode(HWND w_ptr,struct filterswindowlist **CGwindowindex);

	private:
	};

#endif /* FILTERS_H */

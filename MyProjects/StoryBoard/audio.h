#ifndef AUDIO_H
#define AUDIO_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

//Amiga version MakeID(a,b,c,d) ((a)<<24|(b)<<16|(c)<<8|(d))
//PC version
#define MakeID(d,c,b,a) ((a)<<24|(b)<<16|(c)<<8|(d))

#define ID_RIFF	MakeID('R','I','F','F')
#define ID_WAVE	MakeID('W','A','V','E')
#define ID_fmt		MakeID('f','m','t',' ')
#define ID_data	MakeID('d','a','t','a')

#define AD_OFFSETX 10
#define AD_OFFSETY 20
#define AD_YSIZE 50
#define AD_XSIZE 100
#define AD_CLOSE		200
#define AD_IN			201
#define AD_OUT			202
#define AD_DUR			203
#define AD_XP			204
#define AD_OFFSETTB	205
#define AD_YP			206
#define AD_OFFSETED	207

class audioclass : public messagebase {
	public:
		class playerclass {
			public:
			LPDIRECTSOUND lpDS;  //public for avi's

			playerclass();
			~playerclass();
			BOOL streamwav();
			void playwav(struct wavinfo *wavobj);
			BOOL openmedia(struct wavinfo *wavobj);
			void closevoices();
			BOOL startup();
			void shutdown();

			private:
			struct wavvoicelist *voicehead;
			struct wavvoicelist *voicetail;
			DSBUFFERDESC dsbdesc;
			LPDIRECTSOUNDBUFFER lpdsbPrimary;

			void addwavvoice(struct wavinfo *wavobj);
			void removewavvoice(struct wavvoicelist *voice);
			BOOL openWAV(struct wavinfo *wavobj);
			int frame2audiobytes(int frame,struct wavinfo *wavobj);
			UINT readwave(struct wavinfo *wavobj,void *dsbuf1,DWORD dsbuflen1);
			void closeWAV (struct wavinfo *wavobj);
			BOOL initFail(char *reason);
			} player;

		class audioguiclass : public messagebase {
			public:
			struct audiogui{
				struct filterswindowlist windownode;
				HWND caption;
				HWND inwindow;
				HWND outwindow;
				HWND durwindow;
				HWND audioed;
				HWND audioscrub;
				};
			void initcontrols(struct filterswindowlist *audiowindowptr,struct audionode *audioptr);
			HWND getnewgui(struct filterswindowlist *audiowindowptr,HWND audiowindow,ULONG count);
			ULONG getwindowlistsize();
			struct wavinfo *getnode(HWND w_ptr,struct filterswindowlist **audiowindowindex);
			void closeaudio(struct wavinfo *audiowav);

			private:
			int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
			} audioguiobj;

		HWND window;
		struct wavvoicelist *playqueuehead;
		struct wavvoicelist *playqueuetail;
		struct filterswindowlist *windowlist; //filters are the same kind
		class miniscrub *miniscrubobj;

		int Callback(HWND w_ptr,UINT uMsg,WPARAM wParam,LPARAM lParam);
		audioclass();
		~audioclass();
		void startup();
		void shutdown();
		void createwindow(HWND w_ptr);
		void resize();
		struct wavinfo *addwavtomedia (struct imagelist *streamptr,char *filename);
		void addvoicetoplay(struct wavinfo *wavobj,struct imagelist *media);
		void remvoicefromplay(struct wavvoicelist *voice);
		void closeaudio(struct imagelist *media);  //this will check for null
		void updateimagelist();
	private:
	};

#endif /* AUDIO_H */

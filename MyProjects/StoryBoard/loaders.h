#ifndef LOADERS_H
#define LOADERS_H

#define RTVCACHEUNIT 32
#define RTVMAXCACHE RTVCACHEUNIT*3
#define RTVCACHEHALF RTVCACHEUNIT/2
#define RTVREFRESHRATE RTVCACHEHALF-1
#define RTVNEGRATE -RTVCACHEHALF

class loadersclass {
	public:
		class loaderscommon {
			protected:
				void makegraypalette (char *dest);
				BOOL raw2yuv(struct imagelist *media,LPBITMAPINFOHEADER lpBitmapInfoHeader,char *decodedbits,BOOL upsidedown);
			};

		class aviclass {
			public:
			//HBITMAP avicontainer;
			aviclass();
			~aviclass();
			void streamaudio(int notifypos);
			BOOL beginavi(struct imagelist *media);
			void endavi(struct imagelist *media);
			UWORD gettotalavi(struct imagelist *media);
			HBITMAP getthumbavi(struct imagelist *mediaptr,ULONG framenum,LPBITMAPINFOHEADER *bitmapinfo);
			BOOL openavi(struct imagelist *media,BOOL thumb);
			void closeavi(struct imagelist *media,BOOL thumb);
			BOOL getframeavi(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);

			private:
			//Note the vars declared in here mean all media apply to it
			struct avivarlist *avivarlisthead,*avivarlisttail;

			BOOL renderframe(ULONG *videobuf,class avihandle *avivar);
			BOOL queueaudio(struct imagelist *mediaptr,ULONG framenum,ULONG samplesize);
			BOOL openaudio(class avihandle *avivar);
			} aviobj;

		class pcxclass : public loaderscommon {
			public:
			HBITMAP getthumbpcx(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo);
			BOOL openpcx(struct imagelist *media);
			void closepcx(struct imagelist *media);
			char *PCX2raw (char *source,long size,LPBITMAPINFOHEADER *bitmapinfo);
			private:
			char *decompress (char *source,char *dest,DWORD maincount,char *eos,char *eod);
			void convertpalette (char *source,char *dest,int numofcolors);
			void consolidatebitplanes(char *scanline,char *dest,int planesize);
			} pcxobj;

		class tifclass : public loaderscommon {
			public:
			BOOL amiga;

			HBITMAP getthumbtif(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo);
			BOOL opentif(struct imagelist *media);
			void closetif(struct imagelist *media);
			char *TIF2raw (char *source,long size,LPBITMAPINFOHEADER *bitmapinfo);

			private:
			UWORD Getword(UWORD *x);
			ULONG Getlong(ULONG *x);
			void bgr2rgb(char *dest,ULONG SizeImage);
			void skiptotag(unsigned int *tag,unsigned int desttag,ULONG *IFDcount,char **sourceindex);
			void advancetag(unsigned int *tag,ULONG *IFDcount,char **sourceindex);
			} tifobj;

		class jpgclass : public loaderscommon {
			public:
			HBITMAP getthumbjpg(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo);
			BOOL openjpg(struct imagelist *media);
			void closejpg(struct imagelist *media);
			char *JPG2raw (char *source,long size,LPBITMAPINFOHEADER *bitmapinfo);
			} jpgobj;

		class bmpclass : public loaderscommon {
			public:
			BOOL openbmp(struct imagelist *media);
			void closebmp(struct imagelist *media);
			//BOOL getframebmp(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
			HBITMAP getthumbbmp(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo);
			} bmpobj;

		class iffclass {
			public:
			HBITMAP getthumbiff(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo);
			BOOL openiff(struct imagelist *media);
			void closeiff(struct imagelist *media);
			BOOL getframeiff(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
			char *ILBM2raw (char *source,long size,UWORD *x,UWORD *y);
			//These are IFF internals
			private:
			UWORD unpackiffbyterun (char *source,char *dest,char *eof,short xbytes);
			} iffobj;

		class tgaclass : public loaderscommon {
			public:
			HBITMAP getthumbtga(struct imagelist *mediaptr,LPBITMAPINFOHEADER *bitmapinfo);
			BOOL opentga(struct imagelist *media);
			void closetga(struct imagelist *media);
			BOOL getframetga(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
			char *TGA2raw (char *source,long size,UWORD *x,UWORD *y);
			//These are TGA internals
			private:
			void unpack (char *source,char *dest,char *eof,char *deof,UBYTE depth);
			} tgaobj;

		class rtvclass {
			public:
			rtvclass();
			~rtvclass();
			UWORD gettotalrtv(struct imagelist *media);
			HBITMAP getthumbrtv(struct imagelist *mediaptr,ULONG framenum,LPBITMAPINFOHEADER *bitmapinfo);
			void beginrtv(struct imagelist *media);
			BOOL openrtv(struct imagelist *media);
			void closertv(struct imagelist *media);
			void endrtv(struct imagelist *media);
			BOOL getframertv(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
			BOOL checkrtvstream(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
			BOOL getoneframertv(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
			BOOL getframertvstill(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
			ULONG *rtvcache[RTVMAXCACHE]; // made public for toaster capture
			void streamrtv();
			private:
			struct streammsg {
				struct streammsg *next;
				struct imagelist *media;
				long rtvframecache;
				int cachelength;
				};
			char **m_AudioPaths; //these load an .ini file and enumerate to see if a matching audio file exist
			unsigned m_items;
			struct streammsg *streamreqhead,*streamreqtail;
			ULONG *videobuffers;
			ULONG rtvidcache[3];
			struct imagelist *nextmedia;
			int prestartnexttime,singlestreamlength;
			char updatestreamrequest;
			} rtvobj;

		struct imagelist *mediaptr,*prevmediaptr,*nextmediaptr;
		BOOL (loadersclass::*getframefunc)(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL (loadersclass::*nextgetframefunc)(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL (loadersclass::*prevgetframefunc)(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL toggleimagestream;

		BOOL mediamanager(ULONG *videobuf,struct imagelist *streamptr,ULONG mediatimer,UBYTE mode);
		void flushusedmedia(struct imagelist *index);
		DWORD streamfunc(void);
		HBITMAP openthumb(struct imagelist *buffer,UWORD *totalframes,LPBITMAPINFOHEADER *bitmapinfo);
		void closethumb(struct imagelist *buffer,LPBITMAPINFOHEADER bitmapinfo);
		BOOL openmedia(struct imagelist *media,BOOL (loadersclass::**getframefunc)(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage));
		void closemedia(struct imagelist *media);
		void beginmedia(struct imagelist *media);
		void endmedia(struct imagelist *media);
		//This group initializes the media filters
		loadersclass();
		~loadersclass();
		void shutdown();

	private:
		struct imagelist *oldstreamptr;
		ULONG addtomediatimer;
		ULONG *videobuffers;
		ULONG *imagea,*imageb;
		UBYTE playmode;

		//This group loads media to controls for display
		void blackvideo(ULONG *videobuf);
		void setnextmediaptr(struct imagelist *streamptr);
		void setprevmediaptr(struct imagelist *streamptr);
		//These here are for getting the directory
		UWORD gettotalframes(struct imagelist *media);
		UWORD gettotalstills(struct imagelist *media);
		//Here is the getframe filter section
		BOOL getframestills(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL getframeavi(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL getframeiff(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL getframetga(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL getframertv(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
		BOOL getframertvstill(ULONG *videobuf,struct imagelist *mediaptr,ULONG framenum,BOOL bimage);
	};

#endif /* LOADERS_H */

#ifndef TOASTER_H
#define TOASTER_H

class toasterclass {
	public:
	struct RTMEOutput *rtmeoutput;
	struct MediaProbe	*probe;
	struct toastercapvars *toastercapobj;
	ULONG *NoToaster;
	long videobufid;
	long videobufsize;
	void *videobuf;

	toasterclass();
	~toasterclass();
	ULONG *AllocFrame(long *videobufid);
	void SendFrame(long videobufid);
	void resync();
	void sendRGBframe(HBITMAP RGBbitmap);
	void initmasterframe(char *file);
	BOOL startupcapture();
	void shutdowncapture();
	void startcapture();
	void writertv();
	void stopcapture();

	private:
	struct RTMEConnection *rtc;
	};

#endif /* TOASTER_H */

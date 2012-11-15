/*
 * Toaster Interface
 */
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif
*/
#include "handlemain.h"

#pragma comment(lib, "../lib/I386/beethoven")
#pragma comment(lib, "../lib/I386/newtekrtme")
#pragma comment(lib, "../lib/I386/proof")


#ifndef PROOF_CLASSUSR_H
#include <proof/classusr.h>
#endif

#ifndef PROOF_STREAM_H
#include <proof/stream.h>
#endif

#ifndef NEWTEKRTME_MSG_H
#include <newtek/newtekrtme_msg.h>
#endif

#ifndef NEWTEK_HUBMSG_H
#include <newtek/hubmsg.h>
#endif

#ifndef NEWTEK_HUBMEDIA_H
#include <newtek/hubmedia.h>
#endif

#ifndef NEWTEK_VID_D1_H
#include <newtek/vid_d1.h>
#endif

#include <proof/proof_protos.h>
#include <newtek/beethoven_protos.h>
#include <newtek/newtekrtme_protos.h>
#include <newtek/rtme_hub_names.h>

#include "../include/newtek/RTVlib.h"

#include "Toaster.h"
#include "loaders.h"


void consume_mediagram (void *userdata,void *media_buffer,const union MediaGram *mg) {
	struct toastercapvars *toastercapobj=toaster->toastercapobj;
	struct HubHandle *hh =mediaprobe_GetHubHandle(toaster->probe);
	long field_num;
	long size_of_data;

	field_num = rtms_WhichVideoField( &mg->media.rtms );

	if ((field_num==0)||(field_num==1)) {
		/* looks like video, otherwise field_num < 0 */

		size_of_data = mg->media.rtms.rtms_chunk_size; //should be 345600 in NTSC
		/* here's the size (in bytes) of the 4:2:2 data you've just been
		 * passed, do not read beyond (media_buffer + size_of_data)
		 */

		if (D1_NTSC_RAW_WIDTH == mg->media.rtms.type.vid.yuv_422.captured_modulo) {
			/* NTSC signal is coming in */
			//This signal is all we support at this time
			if (debug) print("N");
			}

		else if (D1_PAL_RAW_WIDTH == mg->media.rtms.type.vid.yuv_422.captured_modulo) {
			/* PAL signal is coming in */
			if (debug) print("P");
			goto skipnoprocessframe;
			}
		else {
			/* unknown (to me) video format */
			if (debug) print("?");
			goto skipnoprocessframe;
			}
		//make sure we have the right field from last time
		if (field_num==0) if (toastercapobj->lastfield==0) goto skipnoprocessframe;
		if (field_num==1) if ((toastercapobj->lastfield==1)||(toastercapobj->lastfield==-1)) goto skipnoprocessframe;
		if (toastercapobj->lastfield==-1) ((class captureclass::captoaster *)capture->vcaptureptr)->startaudiocapture();
		toastercapobj->lastfield=field_num;
		/* a video MediaProbe will recieve "fields", not frames */
		//if (debug) printc("(%ld:%ld)",field_num,mg->media.source_time);
		//if (debug) printc("(%ld:%lx)",field_num,media_buffer);
		//Here we have a valid captured media field
		//printc("(%ld:%lx)",field_num,((char *)(medialoaders->rtvobj.rtvcache[toastercapobj->frameindex]))+(field_num*size_of_data));
		memcpy(((char *)(medialoaders->rtvobj.rtvcache[toastercapobj->frameindex]))+(field_num*size_of_data),media_buffer,size_of_data);
		if (field_num==1) {
			toastercapobj->frameindex++;
			if (toastercapobj->frameindex==RTVCACHEHALF) {
				//printc("Writelow");
				toastercapobj->frameshigh=0;
				SetEvent(arrayofevents[EVENT_STREAM]);
				}
			if (toastercapobj->frameindex>=RTVCACHEUNIT) {
				toastercapobj->frameindex=0;
				//printc("Writehigh");
				toastercapobj->frameshigh=1;
				SetEvent(arrayofevents[EVENT_STREAM]);
				}
			}
		}

skipnoprocessframe:
	// Return this to its source
	hh_SendMessage(hh,mg->media.done_msg_id);
	}


toasterclass::toasterclass() {
	//initialize our class vars
	rtc=NULL;
	rtmeoutput=NULL;
	NoToaster=NULL;
	probe=NULL;
	videobufid=0;
	videobufsize=691200;
	toastercapobj=(struct toastercapvars *)newnode(nodeobject,sizeof(struct toastercapvars));
	//TODO fix BIOS issue on pentium III serial
	//Security is currently commented out due to newer copy protection
	//if (security->pass()) {
/**/
		if (rtc=rtc_InitializeRTME()) { //We'll let storyboard run w/o toaster
			rtmeoutput=rtme_output_Create("Apps.Exodus.StoryBoard",0,0,false);
			//What is the padding size suppose to be?
			//rtme_output_SetAllocPadding(rtmeoutput,8,8);
			initmasterframe("resources\\startup.rtv");
			//and now lets set up a callback function
			}
/**/
	//	}
	if (!rtmeoutput) {
		unsigned long *videobufindex;
		long t;

		videobuf=(void *)(NoToaster=new ULONG[172800]);
		//This section here is to init the preview
		if (!(ReadRTVFile("resources\\startup.rtv",0,(void *)videobuf))) {
			//fill the buffer with YUYV here
			videobufindex=(unsigned long *)videobuf;
			t=691200;
			do {
				*videobufindex++=0x10801080; //black
				t-=4;
				} while (t);
			}
		}
	}

toasterclass::~toasterclass() {
	//Cleanup resources here
	//if (videobufid) rtme_output_FreeFrame(rtmeoutput,videobufid,691200);
	if (probe) shutdowncapture();
	Sleep(100);
	if (rtmeoutput) rtme_output_Delete(rtmeoutput);
	if (rtc) rtc_ShutdownRTME(rtc);
	if (NoToaster) delete [] NoToaster;
	}


BOOL toasterclass::startupcapture() {
	char producer_name[512];

	streamthreadmode=1;
	wsprintf(producer_name,RTME_TOASTER_VID_IN_PRODUCER_FMT_STRING,0);
	if (probe=mediaprobe_Create(rtc_GetHub(rtc),"media.apps.storyboard.probe")) {
	/* Connect to to program monitor out */
		if (mediaprobe_Connect(probe,producer_name)) {
			return (TRUE);
			}
		}
	return (FALSE);
	}


void toasterclass::shutdowncapture() {
	mediaprobe_Connect(probe,NULL);
	mediaprobe_Delete(probe);
	streamthreadmode=0;
	}

void toasterclass::startcapture() {
	//init videocapvars
	toastercapobj->frameindex=0;
	toastercapobj->lastfield=-1;
	BuildRTVFile(toastercapobj->videofilename,2,0,720,240,29.97f);
	if (debug) printc(toastercapobj->videofilename);
	/* Here we go */
	if (probe) mediaprobe_BeginStreaming(probe,NULL,consume_mediagram);
	}

void toasterclass::writertv() {
	int t,start;
	if (debug) printc("writing video %d",toastercapobj->frameshigh);
	start=toastercapobj->frameshigh*RTVCACHEHALF;
	EnterCriticalSection(&csglobal);
/**/
	for (t=start;t<start+RTVCACHEHALF;t++) {
		//print("%lx,",medialoaders->rtvobj.rtvcache[t]);
		WriteRTVFile(toastercapobj->videofilename,medialoaders->rtvobj.rtvcache[t]);
		}
/**/
	LeaveCriticalSection(&csglobal);
	}

void toasterclass::stopcapture() {
	UINT t,start;
	if (probe) mediaprobe_StopStreaming(probe);
	//Get the remaining frames in cache
	//printc("frameindexlaston %ld",toastercapobj->frameindex);
	if (toastercapobj->frameindex>RTVCACHEHALF) start=RTVCACHEHALF;
	else start=0;
/**/
	for (t=start;t<toastercapobj->frameindex;t++) {
		WriteRTVFile(toastercapobj->videofilename,medialoaders->rtvobj.rtvcache[t]);
		}
/**/
	CloseRTVFile(toastercapobj->videofilename);
	}

ULONG *toasterclass::AllocFrame(long *videobufid) {
	char retry=100;
	do {
		if (*videobufid=rtme_output_AllocFrame(rtmeoutput,&videobuf,&videobufsize)) {
			//wsprintf(string,"%lx",videobufid);printc(string);
			return((ULONG *)videobuf);
			}
		retry--;
		Sleep(1);
		} while (retry);
	return(FALSE);
	}

void toasterclass::SendFrame(long videobufid) {
	rtme_output_SendFrame(rtmeoutput,videobufid,691200);
	}


void toasterclass::resync() {
		if (rtmeoutput) rtme_output_SetTimingCallback(rtmeoutput,NULL,toastercallback);
		//rtme_output_SetDisplayCallback(rtmeoutput,NULL,toasterdcallback);
		}


void toasterclass::sendRGBframe(HBITMAP RGBbitmap) {
	unsigned long *videobufindex;
	long t,x,y;
	ULONG UYVY;
	ULONG R,G,B,R1,G1,B1;

	videobufid=rtme_output_AllocFrame(rtmeoutput,&videobuf,&videobufsize);
	if (rtc&&rtmeoutput&&videobufid) {

		//fill the buffer with YUYV here
		videobufindex=(unsigned long *)videobuf;
		t=691200;
		x=0;y=0;
		do {
			R=128;G=128;B=128;
			R1=128;G1=128;B1=128;

			UYVY=(((28770*B-19071*G-9699*R)>>16)+128)+\
				(((16843*R+33030*G+6423*B)>>8&65280)+4096)+\
				(((28770*R1-24117*G1-4653*B1)&16711680)+8388608)+\
				(((16843*R1+33030*G1+6423*B1)<<8&4278190080)+268435456);

			*videobufindex++=UYVY; //Our two pixels here
			t-=4;
			x+=2;
			if (y>=480) y=1;
			if (x>=720) {
				x=0;
				y+=2;
				}
			} while (t);
		rtme_output_SendFrame(rtmeoutput,videobufid,691200);
		}
	}


void toasterclass::initmasterframe(char *file) {
	//Temp declaration vars here
	unsigned long *videobufindex;
	long t;
	
	videobufid=rtme_output_AllocFrame(rtmeoutput,&videobuf,&videobufsize);
	if (rtc&&rtmeoutput&&videobufid) {
		if (!(ReadRTVFile(file,0,(void *)videobuf))) {
			//fill the buffer with YUYV here
			videobufindex=(unsigned long *)videobuf;
			t=691200;
			do {
				*videobufindex++=0x10801080; //black
				t-=4;
				} while (t);
			}
		rtme_output_SendFrame(rtmeoutput,videobufid,691200);
		}
	}

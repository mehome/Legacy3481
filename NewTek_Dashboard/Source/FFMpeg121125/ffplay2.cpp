
#include "stdafx.h"
#include "../FrameWork/FrameWork.h"
#include "FrameGrabber.h"
#include "WinHttpUtils.h"

#ifndef _LIB

#endif

#undef __Test_Queue_Picture__

#undef __ShowFrameInfo__
#undef __ShowProcessingDelta__

/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * simple media player based on the FFmpeg libraries
 */

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS 1
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif

#define CONFIG_RTSP_DEMUXER 0

extern "C"
{
#include "include/libavutil/avutil.h"
#include "include/libavutil/avstring.h"
#include "include/libavutil/crc.h"

#include "include/libavcodec/avcodec.h"
//#include "include/libavcodec/audioconvert.h"

#include "include/libavformat/avformat.h"
//#include "include/libavformat/url.h"

#include "include/libswscale/swscale.h"
}

//#include "config.h"

#include "include/inttypes.h"
#include <math.h>
#include <limits.h>
#include <signal.h>
extern "C"
{
#include "include/libavutil/avstring.h"
#include "include/libavutil/colorspace.h"
#include "include/libavutil/mathematics.h"
#include "include/libavutil/pixdesc.h"
#include "include/libavutil/imgutils.h"
#include "include/libavutil/dict.h"
#include "include/libavutil/parseutils.h"
#include "include/libavutil/samplefmt.h"
#include "include/libavutil/avassert.h"
#include "include/libavutil/time.h"
#include "include/libavformat/avformat.h"
#include "include/libavdevice/avdevice.h"
#include "include/libswscale/swscale.h"
#include "include/libavutil/opt.h"
#include "include/libavcodec/avfft.h"
#include "include/libswresample/swresample.h"
}

#include "SDL/include/SDL.h"
#include "SDL/include/SDL_thread.h"

#include <assert.h>
#ifndef _LIB
#pragma comment (lib , "lib/avcodec.lib")
#pragma comment (lib , "lib/avdevice.lib")
#pragma comment (lib , "lib/avfilter.lib")
#pragma comment (lib , "lib/avformat.lib")
#pragma comment (lib , "lib/avutil.lib")
//#pragma comment (lib , "lib/postproc.lib")
#pragma comment (lib , "lib/swresample.lib")
#pragma comment (lib , "lib/swscale.lib")
#pragma comment (lib , "SDL/lib/x86/SDL.lib")
#pragma comment (lib , "SDL/lib/x86/SDLmain.lib")
#endif

//Not defined in visual studio so we'll make a simple one here
//  [11/26/2012 JamesK]
__inline const int rint(float x){
	return (int)(x+0.5);
}

static void ShowTimeDelta(char label[]="",bool UsePrintF=true)
{
	using namespace FrameWork;
	static time_type LastTime=(0.0);
	time_type currenttime=time_type::get_current_time();
	time_type delta = currenttime-LastTime;
	if (UsePrintF)
		printf("%s-> %f\n",label,(double)delta*1000.0);
	else
		DebugOutput("%s-> %f\n",label,(double)delta*1000.0);
	LastTime=currenttime;
}

const char program_name[] = "ffplay";
const int program_birth_year = 2003;

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 5

/* SDL audio buffer size, in samples. Should be small to have precise
   A/V sync as SDL does not have hardware buffer fullness info. */
#define SDL_AUDIO_BUFFER_SIZE 1024

/* no AV sync correction is done if below the AV sync threshold */
#define AV_SYNC_THRESHOLD 0.01
/* no AV correction is done if too big error */
const double c_av_nosync_threshold=10.0;

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

static int sws_flags = SWS_BICUBIC;

typedef struct MyAVPacketList {
    AVPacket pkt;
    struct MyAVPacketList *next;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue {
    MyAVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

const int c_video_picture_queue_size=4;
#define SUBPICTURE_QUEUE_SIZE 4

typedef struct VideoPicture {
    double pts;             // presentation timestamp for this picture
    int64_t pos;            // byte position in file
    int skip;
	//TODO replace with framework bitmaps if we use this
	SDL_Overlay *bmp;
    int width, height; /* source height & width */
    AVRational sample_aspect_ratio;
    int allocated;
    int reallocate;
    int serial;

} VideoPicture;

typedef struct SubPicture {
    double pts; /* presentation time stamp for this picture */
    AVSubtitle sub;
} SubPicture;

typedef struct AudioParams {
    int freq;
    int channels;
    int channel_layout;
    enum AVSampleFormat fmt;
} AudioParams;

enum AV_Sync
{
    eAV_SYNC_AUDIO_MASTER, // default choice 
    eAV_SYNC_VIDEO_MASTER,
    eAV_SYNC_EXTERNAL_CLOCK // synchronize to an external clock 
};

struct FF_Play_Reader {

	public:
		virtual ~FF_Play_Reader();

		int stream_component_open(int stream_index);	// open a given stream. Return 0 if OK 

		void stream_seek(int64_t pos, int64_t rel, int seek_by_bytes);  // seek in the stream
		void stream_toggle_pause();  // pause or resume the video

		//These are to be moved to protected once we are weened off of SDL
		int video_thread();
		int subtitle_thread();
		//This is similar to legacy ffmpeg where it will fill in a buffer, but there is a reading packet queue to be considerate for
		void sdl_audio_callback(Uint8 *stream, int len);	// prepare a new audio buffer 
	protected:
		void stream_close();
		double get_audio_clock() const;   // get the current audio clock value
		double get_video_clock() const;	// get the current video clock value 
		double get_external_clock() const;	// get the current external clock value
		AV_Sync get_master_sync_type() const;
		//This will call one of the clocks above depending on the sync type
		double get_master_clock() const;  // get the current master clock value
		void update_external_clock_pts(double pts);
		void check_external_clock_sync(double pts);
		void update_external_clock_speed(double speed);
		void check_external_clock_speed(); 
		double compute_target_delay(double delay) const;

		void update_video_pts(double pts, int64_t pos, int serial);
		//TODO see if we can use this or dispatch
		int queue_picture(AVFrame *src_frame, double pts1, int64_t pos, int serial);
		int dispatch_picture(AVFrame *src_frame, double pts1, int64_t pos, int serial);
		int get_video_frame(AVFrame *frame, int64_t *pts, AVPacket *pkt, int *serial);

		// return the wanted number of samples to get better sync if sync_type is video or external master clock
		int synchronize_audio(int nb_samples);
		int audio_decode_frame(double *pts_ptr);  // decode one audio frame and returns its uncompressed size 

	private:
	FrameWork::Outstream_Interface *m_Preview;  //Temp for now
	Processing::FX::procamp::Procamp_Manager *m_procamp;
    SDL_Thread *m_read_tid;
    SDL_Thread *m_video_tid;
    AVInputFormat *m_iformat;
	int m_stopped; // used to avoid crash on repeating dispatch
    int m_no_background;
    int m_abort_request;
    int m_paused;
    int m_last_paused;
    int m_que_attachments_req;
    int m_seek_req;
    int m_seek_flags;
    int64_t m_seek_pos;
    int64_t m_seek_rel;
    int m_read_pause_return;
    AVFormatContext *m_ic;
    int m_realtime;

    int m_audio_stream;

    int m_av_sync_type;
    double m_external_clock;                   ///< external clock base
    double m_external_clock_drift;             ///< external clock base - time (av_gettime) at which we updated external_clock
    int64_t m_external_clock_time;             ///< last reference time
    double m_external_clock_speed;             ///< speed of the external clock

    double m_audio_clock;
    double m_audio_diff_cum; /* used for AV difference average computation */
    double m_audio_diff_avg_coef;
    double m_audio_diff_threshold;
    int m_audio_diff_avg_count;
    AVStream *m_audio_st;
    PacketQueue m_audioq;
    int m_audio_hw_buf_size;
	//TODO probably use a new operation on this variable and then make it a member
    DECLARE_ALIGNED(16,uint8_t,g_audio_buf2)[AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
    uint8_t m_silence_buf[SDL_AUDIO_BUFFER_SIZE];
    uint8_t *m_audio_buf;
    uint8_t *m_audio_buf1;
    unsigned int m_audio_buf_size; /* in bytes */
    int m_audio_buf_index; /* in bytes */
    int m_audio_write_buf_size;
    AVPacket m_audio_pkt_temp;
    AVPacket m_audio_pkt;
    int m_audio_pkt_temp_serial;
    AudioParams m_audio_src;
    AudioParams m_audio_tgt;
    SwrContext *m_swr_ctx;
    double m_audio_current_pts;
    double m_audio_current_pts_drift;
    int m_frame_drops_early;
    int m_frame_drops_late;
    AVFrame *m_frame;

	//TODO get rid of this
    enum ShowMode 
	{
		eSHOW_MODE_NONE = -1, 
		eSHOW_MODE_VIDEO = 0, 
		eSHOW_MODE_WAVES, 
		eSHOW_MODE_RDFT, 
		eSHOW_MODE_NB
    } show_mode;

    int16_t m_sample_array[SAMPLE_ARRAY_SIZE];
    int m_sample_array_index;
    int m_last_i_start;
    RDFTContext *m_rdft;
    int m_rdft_bits;
    FFTSample *m_rdft_data;
    int m_xpos;

    SDL_Thread *m_subtitle_tid;
    int m_subtitle_stream;
    int m_subtitle_stream_changed;
    AVStream *m_subtitle_st;
    PacketQueue m_subtitleq;
    SubPicture m_subpq[SUBPICTURE_QUEUE_SIZE];
    int m_subpq_size, m_subpq_rindex, m_subpq_windex;
    SDL_mutex *m_subpq_mutex;
    SDL_cond *m_subpq_cond;

    double m_frame_timer;
    double m_frame_last_pts;
    double m_frame_last_duration;
    double m_frame_last_dropped_pts;
    double m_frame_last_returned_time;
    double m_frame_last_filter_delay;
    int64_t m_frame_last_dropped_pos;
    double m_video_clock;             // pts of last decoded frame / predicted pts of next decoded frame
    int m_video_stream;
    AVStream *m_video_st;
    PacketQueue m_videoq;
    double m_video_current_pts;       // current displayed pts (different from video_clock if frame fifos are used)
    double m_video_current_pts_drift; // video_current_pts - time (av_gettime) at which we updated video_current_pts - used to have running video pts
    int64_t m_video_current_pos;      // current displayed file pos
    VideoPicture m_pictq[c_video_picture_queue_size];
    int m_pictq_size, m_pictq_rindex, m_pictq_windex;
    SDL_mutex *m_pictq_mutex;
    SDL_cond *m_pictq_cond;
    struct SwsContext *m_img_convert_ctx;
    SDL_Rect m_last_display_rect;

    char m_filename[1024];
    int m_width, m_height, m_xleft, m_ytop;
    int m_step;

    int m_last_video_stream, m_last_audio_stream, m_last_subtitle_stream;

    SDL_cond *m_continue_read_thread;
};

/* options specified by the user */
static AVInputFormat *file_iformat;
static std::string m_URL;
static const char *input_filename;
static const char *window_title;
static int fs_screen_width;
static int fs_screen_height;
static int audio_disable;
static int video_disable;
static int wanted_stream[AVMEDIA_TYPE_NB] = {0,-1,-1,0,-1};
static int seek_by_bytes = -1;
static int display_disable;
#define __ShowStatus__
//static int show_status = 1;
//static int av_sync_type = AV_SYNC_AUDIO_MASTER;
static int64_t start_time = AV_NOPTS_VALUE;
static int64_t duration = AV_NOPTS_VALUE;
static int workaround_bugs = 1;
static int fast = 0;
static int genpts = 0;
static int lowres = 0;
static int idct = FF_IDCT_AUTO;
static enum AVDiscard skip_frame       = AVDISCARD_DEFAULT;
static enum AVDiscard skip_idct        = AVDISCARD_DEFAULT;
static enum AVDiscard skip_loop_filter = AVDISCARD_DEFAULT;
static int error_concealment = 3;
static int decoder_reorder_pts = -1;
static int autoexit;
static int exit_on_keydown;
static int exit_on_mousedown;
static int loop = 1;
static int framedrop = -1;
static int infinite_buffer = -1;
//static enum VideoState::ShowMode show_mode = VideoState::SHOW_MODE_NONE;
static const char *audio_codec_name;
static const char *subtitle_codec_name;
static const char *video_codec_name;
static int rdftspeed = 20;

/* current context */
static int is_full_screen;
static int64_t audio_callback_time;

static AVPacket flush_pkt;

#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

static SDL_Surface *screen;

static int packet_queue_put(PacketQueue *q, AVPacket *pkt);

static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
    MyAVPacketList *pkt1;

    if (q->abort_request)
       return -1;

    pkt1 = (MyAVPacketList *)av_malloc(sizeof(MyAVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    if (pkt == &flush_pkt)
        q->serial++;
    pkt1->serial = q->serial;

    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size + sizeof(*pkt1);
    /* XXX: should duplicate packet data in DV case */
    SDL_CondSignal(q->cond);
    return 0;
}

static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    int ret;

    /* duplicate the packet */
    if (pkt != &flush_pkt && av_dup_packet(pkt) < 0)
        return -1;

    SDL_LockMutex(q->mutex);
    ret = packet_queue_put_private(q, pkt);
    SDL_UnlockMutex(q->mutex);

    if (pkt != &flush_pkt && ret < 0)
        av_free_packet(pkt);

    return ret;
}

/* packet queue handling */
static void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
    q->abort_request = 1;
}

static void packet_queue_flush(PacketQueue *q)
{
    MyAVPacketList *pkt, *pkt1;

    SDL_LockMutex(q->mutex);
    for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
    SDL_UnlockMutex(q->mutex);
}

static void packet_queue_destroy(PacketQueue *q)
{
    packet_queue_flush(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

static void packet_queue_abort(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);

    q->abort_request = 1;

    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
}

static void packet_queue_start(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    packet_queue_put_private(q, &flush_pkt);
    SDL_UnlockMutex(q->mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial)
{
    MyAVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size + sizeof(*pkt1);
            *pkt = pkt1->pkt;
            if (serial)
                *serial = pkt1->serial;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

#define ALPHA_BLEND(a, oldp, newp, s)\
((((oldp << s) * (255 - (a))) + (newp * (a))) / (255 << s))

#define RGBA_IN(r, g, b, a, s)\
{\
    unsigned int v = ((const uint32_t *)(s))[0];\
    a = (v >> 24) & 0xff;\
    r = (v >> 16) & 0xff;\
    g = (v >> 8) & 0xff;\
    b = v & 0xff;\
}

#define YUVA_IN(y, u, v, a, s, pal)\
{\
    unsigned int val = ((const uint32_t *)(pal))[*(const uint8_t*)(s)];\
    a = (val >> 24) & 0xff;\
    y = (val >> 16) & 0xff;\
    u = (val >> 8) & 0xff;\
    v = val & 0xff;\
}

#define YUVA_OUT(d, y, u, v, a)\
{\
    ((uint32_t *)(d))[0] = (a << 24) | (y << 16) | (u << 8) | v;\
}


#define BPP 1

static void free_subpicture(SubPicture *sp)
{
    avsubtitle_free(&sp->sub);
}

static inline int compute_mod(int a, int b)
{
    return a < 0 ? a%b + b : a%b;
}

void FF_Play_Reader::stream_close()
{
    VideoPicture *vp;
    int i;
    /* XXX: use a special url_shutdown call to abort parse cleanly */
    m_abort_request = 1;
    SDL_WaitThread(m_read_tid, NULL);
    packet_queue_destroy(&m_videoq);
    packet_queue_destroy(&m_audioq);
    packet_queue_destroy(&m_subtitleq);

    /* free all pictures */
    for (i = 0; i < c_video_picture_queue_size; i++) 
	{
        vp = &m_pictq[i];
        if (vp->bmp) 
		{
            SDL_FreeYUVOverlay(vp->bmp);
            vp->bmp = NULL;
        }
    }
    SDL_DestroyMutex(m_pictq_mutex);
    SDL_DestroyCond(m_pictq_cond);
    SDL_DestroyMutex(m_subpq_mutex);
    SDL_DestroyCond(m_subpq_cond);
    SDL_DestroyCond(m_continue_read_thread);
    if (m_img_convert_ctx)
        sws_freeContext(m_img_convert_ctx);
	//this is no longer needed
    //av_free(is);
}

FF_Play_Reader::~FF_Play_Reader()
{
    stream_close();
    av_lockmgr_register(NULL);
    avformat_network_deinit();
	#ifdef __ShowStatus__
	printf("\n");
	#endif
    SDL_Quit();
    av_log(NULL, AV_LOG_QUIET, "%s", "");
}

double FF_Play_Reader::get_audio_clock() const
{
	double ret=m_audio_current_pts_drift + av_gettime() / 1000000.0;
    if (m_paused) 
        ret=m_audio_current_pts;
    return ret;
}

double FF_Play_Reader::get_video_clock() const
{
	double ret=m_video_current_pts_drift + av_gettime() / 1000000.0;
    if (m_paused) 
        ret=m_video_current_pts;
	return ret;
}


double FF_Play_Reader::get_external_clock() const
{
	double time = av_gettime() / 1000000.0;
	double ret=m_external_clock_drift + time - (time - m_external_clock_time / 1000000.0) * (1.0 - m_external_clock_speed);
    if (m_paused) 
        ret=m_external_clock;
	return ret;
}

AV_Sync FF_Play_Reader::get_master_sync_type() const
{
	AV_Sync ret=eAV_SYNC_EXTERNAL_CLOCK;
    if (m_av_sync_type == eAV_SYNC_VIDEO_MASTER) 
		ret=(m_video_st)? eAV_SYNC_VIDEO_MASTER:eAV_SYNC_AUDIO_MASTER;
	else
	{
		if (m_av_sync_type == eAV_SYNC_AUDIO_MASTER) 
			ret=(m_audio_st)? eAV_SYNC_AUDIO_MASTER:eAV_SYNC_EXTERNAL_CLOCK;
	}
	return ret;
}


double FF_Play_Reader::get_master_clock() const
{
    double val;

    switch (get_master_sync_type()) {
        case eAV_SYNC_VIDEO_MASTER:
            val = get_video_clock();
            break;
        case eAV_SYNC_AUDIO_MASTER:
            val = get_audio_clock();
            break;
        default:
            val = get_external_clock();
            break;
    }
    return val;
}

void FF_Play_Reader::update_external_clock_pts( double pts)
{
   m_external_clock_time = av_gettime();
   m_external_clock = pts;
   m_external_clock_drift = pts - m_external_clock_time / 1000000.0;
}

void FF_Play_Reader::check_external_clock_sync(double pts) 
{
    if (fabs(get_external_clock() - pts) > c_av_nosync_threshold) 
        update_external_clock_pts(pts);
}

void FF_Play_Reader::update_external_clock_speed(double speed) 
{
    update_external_clock_pts( get_external_clock());
    m_external_clock_speed = speed;
}

void FF_Play_Reader::check_external_clock_speed()
{
	if (m_video_stream >= 0 && m_videoq.nb_packets <= MIN_FRAMES / 2 ||  m_audio_stream >= 0 && m_audioq.nb_packets <= MIN_FRAMES / 2) 
		update_external_clock_speed( FFMAX(EXTERNAL_CLOCK_SPEED_MIN, m_external_clock_speed - EXTERNAL_CLOCK_SPEED_STEP));
	else if ((m_video_stream < 0 || m_videoq.nb_packets > MIN_FRAMES * 2) && (m_audio_stream < 0 || m_audioq.nb_packets > MIN_FRAMES * 2)) 
       update_external_clock_speed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX, m_external_clock_speed + EXTERNAL_CLOCK_SPEED_STEP));
	else 
	{
		double speed = m_external_clock_speed;
		if (speed != 1.0)
			update_external_clock_speed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
	}
}


void FF_Play_Reader::stream_seek(int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!m_seek_req) {
        m_seek_pos = pos;
        m_seek_rel = rel;
        m_seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes)
            m_seek_flags |= AVSEEK_FLAG_BYTE;
        m_seek_req = 1;
    }
}

void FF_Play_Reader::stream_toggle_pause()
{
    if (m_paused) 
	{
        m_frame_timer += av_gettime() / 1000000.0 + m_video_current_pts_drift - m_video_current_pts;
        if (m_read_pause_return != AVERROR(ENOSYS)) 
            m_video_current_pts = m_video_current_pts_drift + av_gettime() / 1000000.0;
        
        m_video_current_pts_drift = m_video_current_pts - av_gettime() / 1000000.0;
    }
    update_external_clock_pts( get_external_clock());
    m_paused = !m_paused;
}

double FF_Play_Reader::compute_target_delay(double delay) const
{
    double sync_threshold, diff;

    // update delay to follow master synchronization source 
    if (get_master_sync_type() != eAV_SYNC_VIDEO_MASTER) 
	{
        // if video is slave, we try to correct big delays by
        //duplicating or deleting a frame 
        diff = get_video_clock() - get_master_clock();

        // skip or repeat frame. We take into account the
        //   delay to compute the threshold. I still don't know
        //   if it is the best guess 
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay);
        if (fabs(diff) < c_av_nosync_threshold) 
		{
            if (diff <= -sync_threshold)
                delay = 0;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }

    av_dlog(NULL, "video: delay=%0.3f A-V=%f\n", delay, -diff);

    return delay;
}

void FF_Play_Reader::update_video_pts(double pts, int64_t pos, int serial) 
{
    double time = av_gettime() / 1000000.0;
    /* update current video pts */
    m_video_current_pts = pts;
    m_video_current_pts_drift = m_video_current_pts - time;
    m_video_current_pos = pos;
    m_frame_last_pts = pts;
    if (m_videoq.serial == serial)
        check_external_clock_sync(m_video_current_pts);
}

int FF_Play_Reader::queue_picture(AVFrame *src_frame, double pts1, int64_t pos, int serial)
{
    VideoPicture *vp;
    double frame_delay, pts = pts1;

    // compute the exact PTS for the picture if it is omitted in the stream
     // pts1 is the dts of the pkt / pts of the frame 

    if (pts != 0)  // update video clock with pts, if present 
        m_video_clock = pts;
	else 
        pts = m_video_clock;
    
    // update video clock for next frame 
    frame_delay = av_q2d(m_video_st->codec->time_base);
    // for MPEG2, the frame can be repeated, so we update the
      // clock accordingly 
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    m_video_clock += frame_delay;

	#if defined(DEBUG_SYNC) && 0
    printf("frame_type=%c clock=%0.3f pts=%0.3f\n",
           av_get_picture_type_char(src_frame->pict_type), pts, pts1);
	#endif

    // wait until we have space to put a new picture 
    SDL_LockMutex(m_pictq_mutex);

    // keep the last already displayed picture in the queue 
    while (m_pictq_size >= c_video_picture_queue_size - 2 && !m_videoq.abort_request) 
        SDL_CondWait(m_pictq_cond, m_pictq_mutex);
    
    SDL_UnlockMutex(m_pictq_mutex);

	//ShowTimeDelta("thiers");

    if (m_videoq.abort_request)
        return -1;

    vp = &m_pictq[m_pictq_windex];

    vp->sample_aspect_ratio = av_guess_sample_aspect_ratio(m_ic, m_video_st, src_frame);

    // alloc or resize hardware picture buffer 
    if (!vp->bmp || vp->reallocate || !vp->allocated ||
        vp->width  != src_frame->width ||
        vp->height != src_frame->height) 
	{
        SDL_Event event;

        vp->allocated  = 0;
        vp->reallocate = 0;
        vp->width = src_frame->width;
        vp->height = src_frame->height;

        // the allocation must be done in the main thread to avoid
           //locking problems. 
        event.type = FF_ALLOC_EVENT;
        event.user.data1 = this;
        SDL_PushEvent(&event);

        // wait until the picture is allocated 
        SDL_LockMutex(m_pictq_mutex);
        while (!vp->allocated && !m_videoq.abort_request) 
            SDL_CondWait(m_pictq_cond, m_pictq_mutex);
        
        /* if the queue is aborted, we have to pop the pending ALLOC event or wait for the allocation to complete */
        if (m_videoq.abort_request && SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENTMASK(FF_ALLOC_EVENT)) != 1) 
		{
            while (!vp->allocated) 
                SDL_CondWait(m_pictq_cond, m_pictq_mutex);
            
        }
        SDL_UnlockMutex(m_pictq_mutex);

        if (m_videoq.abort_request)
            return -1;
    }

    /* if the frame is not skipped, then display it */
    if (vp->bmp) 
	{
        AVPicture pict = { { 0 } };

        /* get a pointer on the bitmap */
        SDL_LockYUVOverlay (vp->bmp);

        pict.data[0] = vp->bmp->pixels[0];
        pict.data[1] = vp->bmp->pixels[2];
        pict.data[2] = vp->bmp->pixels[1];

        pict.linesize[0] = vp->bmp->pitches[0];
        pict.linesize[1] = vp->bmp->pitches[2];
        pict.linesize[2] = vp->bmp->pitches[1];

        //sws_flags = (int)av_get_int(sws_opts, "sws_flags", NULL);
		sws_flags = 0;
        m_img_convert_ctx = sws_getCachedContext(m_img_convert_ctx, vp->width, vp->height, (AVPixelFormat)src_frame->format, vp->width, vp->height,
            AV_PIX_FMT_YUV420P, sws_flags, NULL, NULL, NULL);
        if (m_img_convert_ctx == NULL) 
		{
            fprintf(stderr, "Cannot initialize the conversion context\n");
            assert(false);
		}
        sws_scale(m_img_convert_ctx, src_frame->data, src_frame->linesize, 0, vp->height, pict.data, pict.linesize);

        // update the bitmap content 
        SDL_UnlockYUVOverlay(vp->bmp);

        vp->pts = pts;
        vp->pos = pos;
        vp->skip = 0;
        vp->serial = serial;

        // now we can update the picture count 
        if (++m_pictq_windex == c_video_picture_queue_size)
            m_pictq_windex = 0;
        SDL_LockMutex(m_pictq_mutex);
        m_pictq_size++;
        SDL_UnlockMutex(m_pictq_mutex);
    }
    return 0;
}

int FF_Play_Reader::dispatch_picture(AVFrame *src_frame, double pts1, int64_t pos, int serial)
{
	#if 0
	{
		static size_t counter=0;
		if (counter++>30)
		{
			FrameWork::DebugOutput("vq=%d\n",is->videoq.nb_packets),
			counter=0;
		}
	}
	#endif
    double frame_delay, pts = pts1;

    // compute the exact PTS for the picture if it is omitted in the stream
     // pts1 is the dts of the pkt / pts of the frame 
    if (pts != 0) // update video clock with pts, if present 
        m_video_clock = pts;
    else 
        pts = m_video_clock;
    
    /* update video clock for next frame */
    frame_delay = av_q2d(m_video_st->codec->time_base);
    /* for MPEG2, the frame can be repeated, so we update the
       clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    m_video_clock += frame_delay;

    if (m_videoq.abort_request)
        return -1;


	//always dispatch
    {
		FrameWork::Bitmaps::bitmap_bgra_u8 bitmap(src_frame->width,src_frame->height);
		bitmap.set_interleaved(src_frame->interlaced_frame!=0);
		AVPicture pict = { { 0 } };
		pict.data[0] = (uint8_t *) bitmap();
		pict.data[1] = 0;
		pict.data[2] = 0;

		pict.linesize[0] = bitmap.stride_in_bytes();
		pict.linesize[1] = 0;
		pict.linesize[2] = 0;

		#if 1
		sws_flags = 4;
		#else
        sws_flags = (int)av_get_int(sws_opts, "sws_flags", NULL);
		#endif
		if (m_procamp->Get_Procamp_Matrix()==NULL)
		{
			m_img_convert_ctx = sws_getCachedContext(m_img_convert_ctx,
				src_frame->width, src_frame->height, (AVPixelFormat)src_frame->format,bitmap.xres(), bitmap.yres(),
				AV_PIX_FMT_BGRA, sws_flags, NULL, NULL, NULL);

			assert(m_img_convert_ctx != NULL);

			sws_scale(m_img_convert_ctx, src_frame->data, src_frame->linesize,
					  0, src_frame->height, pict.data, pict.linesize);
		}
		else
		{
			//unfortunately the procamp is in UYVY so we'll have to convert it to BGRA
			FrameWork::Bitmaps::bitmap_ycbcr_u8 bitmap_ycbcr(src_frame->width,src_frame->height);
			pict.data[0] = (uint8_t *) bitmap_ycbcr();
			pict.linesize[0] = bitmap_ycbcr.stride_in_bytes();

			m_img_convert_ctx = sws_getCachedContext(m_img_convert_ctx,
				src_frame->width, src_frame->height, (AVPixelFormat)src_frame->format,bitmap_ycbcr.xres(), bitmap_ycbcr.yres(),
				AV_PIX_FMT_UYVY422, sws_flags, NULL, NULL, NULL);

			assert(m_img_convert_ctx != NULL);

			sws_scale(m_img_convert_ctx, src_frame->data, src_frame->linesize,
				0, src_frame->height, pict.data, pict.linesize);
			//Apply the procamp
			(*m_procamp)(bitmap_ycbcr);
			//yuck... the conversion copy
			bitmap=bitmap_ycbcr;
		}

		const int Playback_MaxQueue=8;
		{
			#ifdef __ShowProcessingDelta__
			using namespace FrameWork;
			time_type StartTime=time_type::get_current_time();
			#endif
			//Don't let the queue grow by skipping the processing when it is too large
			if (m_videoq.nb_packets < (m_realtime ? 2 : Playback_MaxQueue) )
				m_Preview->process_frame(&bitmap);

			#ifdef __ShowProcessingDelta__
			DebugOutput("%d time delta=%.1f\n",m_videoq.nb_packets,(double)(time_type::get_current_time()-StartTime) * 1000.0);
			#endif
		}

		//For files... we use a timing mechanism to determine the amount to sleep... I have two different solutions the first shows the way it works in the video refresh where it
		//works with the pts values to determine the delay, and the other uses a constant frame rate from the av_stream info... the big difference between these are noticeable where
		//dropped frames would have occurred, and in one case the frame rate wasn't really what the PTS timing was showing.  The first choice is more accurate.
		//  [12/10/2012 James]

		//Note the nb_packets<Playback_MaxQueue... it is possible for queue to be large (eg. 80 fames) to fall behind... yet it will still sleep in this loop causing no frames to be sent... 
		//this check will ensure that it goes as fast as possible to get the queue back down below max... this must also be the same value as the queue so that it doesn't play the frames
		//too fast
		if ((!m_realtime) && (m_videoq.nb_packets<Playback_MaxQueue))
		{
			#if 1
			// compute nominal last_duration
			double last_duration = pts - m_frame_last_pts;
			if (last_duration > 0 && last_duration < 10.0) {
				/* if duration of the last frame was sane, update last_duration in video state */
				m_frame_last_duration = last_duration;
			}
			double delay = compute_target_delay(m_frame_last_duration);

			double time= av_gettime()/1000000.0;
			//Having delay means that our frames are not ahead of time
			if (delay > 0)
				m_frame_timer += delay * FFMAX(1, floor((time-m_frame_timer) / delay));

			//At this point the frame timer and timer can be used to determine sync
			const double frame_timer=m_frame_timer;
			if (time < frame_timer)
			{
				double diff=frame_timer - time;
				//FrameWork::DebugOutput("%.1f= %.3f - %.3f time=%.3f\n",diff*1000.0,delay,frame_timer,time);
				if (diff>0.002)
					if (diff<0.100)
						Sleep((DWORD)(diff*1000.0)); 
					else
						m_frame_timer=time;
			}

			//FrameWork::DebugOutput("p=%.3f a=%.3f diff=%.3f\n",frame_timer,time,(m_frame_timer) - time);

			//ShowTimeDelta("dispatch_picture",false);
			update_video_pts(pts, pos, serial);

			#else
			//double diff=delay - (time-m_frame_timer);
			double TimeDelta;
			TimeDelta=get_external_clock(is);
			double diff=(1.0/av_q2d(m_video_st->r_frame_rate)) - TimeDelta;
			//FrameWork::DebugOutput("%f\n",TimeDelta);

			if (diff>0.0)
			{
				//FrameWork::DebugOutput("%f\n",diff);
				Sleep((DWORD)(diff*1000.0)); 
			}
			update_external_clock_pts(is,pts);
			#endif
		}
    }
    return 0;
}

int FF_Play_Reader::get_video_frame(AVFrame *frame, int64_t *pts, AVPacket *pkt, int *serial)
{
    int got_picture, i;

    if (packet_queue_get(&m_videoq, pkt, 1, serial) < 0)
        return -1;

    if (pkt->data == flush_pkt.data) 
	{
        avcodec_flush_buffers(m_video_st->codec);

        SDL_LockMutex(m_pictq_mutex);
        // Make sure there are no long delay timers (ideally we should just flush the que but thats harder)
        for (i = 0; i < c_video_picture_queue_size; i++) 
            m_pictq[i].skip = 1;
        
        while (m_pictq_size && !m_videoq.abort_request) 
            SDL_CondWait(m_pictq_cond, m_pictq_mutex);
        
        m_video_current_pos = -1;
        m_frame_last_pts = (double)AV_NOPTS_VALUE;
        m_frame_last_duration = 0;
        m_frame_timer = (double)av_gettime() / 1000000.0;
        m_frame_last_dropped_pts = (double)AV_NOPTS_VALUE;
        SDL_UnlockMutex(m_pictq_mutex);

        return 0;
    }

    if(avcodec_decode_video2(m_video_st->codec, frame, &got_picture, pkt) < 0)
        return 0;

    if (got_picture) 
	{
        int ret = 1;

        if (decoder_reorder_pts == -1) 
            *pts = av_frame_get_best_effort_timestamp(frame);
        else if (decoder_reorder_pts) 
            *pts = frame->pkt_pts;
        else 
            *pts = frame->pkt_dts;
        

        if (*pts == AV_NOPTS_VALUE) 
            *pts = 0;
        

		//do not delay if we are real time here -JamesK
        if ((framedrop>0 || (framedrop && get_master_sync_type() != eAV_SYNC_VIDEO_MASTER)) &&	(!m_realtime))		
		{
            SDL_LockMutex(m_pictq_mutex);
            if (m_frame_last_pts != AV_NOPTS_VALUE && *pts) 
			{
                double clockdiff = get_video_clock() - get_master_clock();
                double dpts = av_q2d(m_video_st->time_base) * *pts;
                double ptsdiff = dpts - m_frame_last_pts;
                if (fabs(clockdiff) < c_av_nosync_threshold &&
                     ptsdiff > 0 && ptsdiff < c_av_nosync_threshold &&
                     clockdiff + ptsdiff - m_frame_last_filter_delay < 0) 
				{
                    m_frame_last_dropped_pos = pkt->pos;
                    m_frame_last_dropped_pts = dpts;
                    m_frame_drops_early++;
                    ret = 0;
                }
            }
            SDL_UnlockMutex(m_pictq_mutex);
        }

        return ret;
    }
	//use to measure codec latency
	#if 0
	else
	{
		static size_t counter=0;
		FrameWork::DebugOutput("Codec got picture fails count=%d\n",counter++);
	}
	#endif
    return 0;
}

int FF_Play_Reader::video_thread()
{
    AVPacket pkt = { 0 };
    AVFrame *frame = avcodec_alloc_frame();
    int64_t pts_int = AV_NOPTS_VALUE, pos = -1;
    double pts;
    int ret;
    int serial = 0;

    for (;;) 
	{
        while (m_paused && !m_videoq.abort_request)
		{
			//Not sure yet if we want to repeat pic... probably not
			#if 0
            SDL_Delay(10);
			#else
			if (!m_stopped)
			{
				SDL_Delay(33);
				ret = dispatch_picture(frame, pts, pkt.pos, serial);
			}
			else
				SDL_Delay(10);
			#endif
		}

        avcodec_get_frame_defaults(frame);
        av_free_packet(&pkt);

        ret = get_video_frame(frame, &pts_int, &pkt, &serial);
        if (ret < 0)
            goto the_end;

        if (!ret)
            continue;

        pts = pts_int * av_q2d(m_video_st->time_base);
		#ifdef __Test_Queue_Picture__
        ret = queue_picture(frame, pts, pkt.pos, serial);
		#else
		ret = dispatch_picture(frame, pts, pkt.pos, serial);
		#endif

        if (ret < 0)
            goto the_end;

        if (m_step)
            stream_toggle_pause();
    }
 the_end:
    avcodec_flush_buffers(m_video_st->codec);
    av_free_packet(&pkt);
    avcodec_free_frame(&frame);
    return 0;
}

//TODO nuke once we switch to our threads
static int video_thread(void *arg)
{
	FF_Play_Reader *is = (FF_Play_Reader *)arg;
	is->video_thread();
}

int FF_Play_Reader::subtitle_thread()
{
    SubPicture *sp;
    AVPacket pkt1, *pkt = &pkt1;
    int got_subtitle;
    double pts;
    int i, j;
    int r, g, b, y, u, v, a;

    for (;;) 
	{
        while (m_paused && !m_subtitleq.abort_request) 
            SDL_Delay(10);
        
        if (packet_queue_get(&m_subtitleq, pkt, 1, NULL) < 0)
            break;

        if (pkt->data == flush_pkt.data) 
		{
            avcodec_flush_buffers(m_subtitle_st->codec);
            continue;
        }
        SDL_LockMutex(m_subpq_mutex);
        while (m_subpq_size >= SUBPICTURE_QUEUE_SIZE && !m_subtitleq.abort_request) 
            SDL_CondWait(m_subpq_cond, m_subpq_mutex);
        
        SDL_UnlockMutex(m_subpq_mutex);

        if (m_subtitleq.abort_request)
            return 0;

        sp = &m_subpq[m_subpq_windex];

       // NOTE: ipts is the PTS of the _first_ picture beginning in this packet, if any 
        pts = 0;
        if (pkt->pts != AV_NOPTS_VALUE)
            pts = av_q2d(m_subtitle_st->time_base) * pkt->pts;

        avcodec_decode_subtitle2(m_subtitle_st->codec, &sp->sub, &got_subtitle, pkt);
        if (got_subtitle && sp->sub.format == 0) 
		{
            if (sp->sub.pts != AV_NOPTS_VALUE)
                pts = sp->sub.pts / (double)AV_TIME_BASE;
            sp->pts = pts;

            for (i = 0; i < (int)sp->sub.num_rects; i++)
            {
                for (j = 0; j < sp->sub.rects[i]->nb_colors; j++)
                {
                    RGBA_IN(r, g, b, a, (uint32_t*)sp->sub.rects[i]->pict.data[1] + j);
                    y = RGB_TO_Y_CCIR(r, g, b);
                    u = RGB_TO_U_CCIR(r, g, b, 0);
                    v = RGB_TO_V_CCIR(r, g, b, 0);
                    YUVA_OUT((uint32_t*)sp->sub.rects[i]->pict.data[1] + j, y, u, v, a);
                }
            }

            /* now we can update the picture count */
            if (++m_subpq_windex == SUBPICTURE_QUEUE_SIZE)
                m_subpq_windex = 0;
            SDL_LockMutex(m_subpq_mutex);
            m_subpq_size++;
            SDL_UnlockMutex(m_subpq_mutex);
        }
        av_free_packet(pkt);
    }
    return 0;
}

static int subtitle_thread(void *arg)
{
	FF_Play_Reader *is = (FF_Play_Reader *)arg;
	is->subtitle_thread();
}

int FF_Play_Reader::synchronize_audio(int nb_samples)
{
    int wanted_nb_samples = nb_samples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (get_master_sync_type() != eAV_SYNC_AUDIO_MASTER) 
	{
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = get_audio_clock() - get_master_clock();

        if (fabs(diff) < c_av_nosync_threshold) 
		{
            m_audio_diff_cum = diff + m_audio_diff_avg_coef * m_audio_diff_cum;
            if (m_audio_diff_avg_count < AUDIO_DIFF_AVG_NB) // not enough measures to have a correct estimate 
                m_audio_diff_avg_count++;
             else // estimate the A-V difference 
			 {
                
                avg_diff = m_audio_diff_cum * (1.0 - m_audio_diff_avg_coef);

                if (fabs(avg_diff) >= m_audio_diff_threshold) 
				{
                    wanted_nb_samples = nb_samples + (int)(diff * m_audio_src.freq);
                    min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = FFMIN(FFMAX(wanted_nb_samples, min_nb_samples), max_nb_samples);
                }
                av_dlog(NULL, "diff=%f adiff=%f sample_diff=%d apts=%0.3f vpts=%0.3f %f\n",
                        diff, avg_diff, wanted_nb_samples - nb_samples,m_audio_clock, m_video_clock, m_audio_diff_threshold);
            }
        } 
		else 
		{
            // too big difference : may be initial PTS errors, so reset A-V filter 
            m_audio_diff_avg_count = 0;
            m_audio_diff_cum       = 0;
        }
    }

    return wanted_nb_samples;
}


int FF_Play_Reader::audio_decode_frame(double *pts_ptr)
{
    AVPacket *pkt_temp = &m_audio_pkt_temp;
    AVPacket *pkt = &m_audio_pkt;
    AVCodecContext *dec = m_audio_st->codec;
    int len1, len2, data_size, resampled_data_size;
    int64_t dec_channel_layout;
    int got_frame;
    double pts;
    int new_packet = 0;
    int flush_complete = 0;
    int wanted_nb_samples;

    for (;;) 
	{
        // NOTE: the audio packet can contain several frames 
        while (pkt_temp->size > 0 || (!pkt_temp->data && new_packet)) 
		{
            if (!m_frame) 
			{
                if (!(m_frame = avcodec_alloc_frame()))
                    return AVERROR(ENOMEM);
            } 
			else
                avcodec_get_frame_defaults(m_frame);

            if (m_paused)
                return -1;

            if (flush_complete)
                break;
            new_packet = 0;
            len1 = avcodec_decode_audio4(dec, m_frame, &got_frame, pkt_temp);
            if (len1 < 0) 
			{
                /* if error, we skip the frame */
                pkt_temp->size = 0;
                break;
            }

            pkt_temp->data += len1;
            pkt_temp->size -= len1;

            if (!got_frame) 
			{
                /* stop sending empty packets if the decoder is finished */
                if (!pkt_temp->data && dec->codec->capabilities & CODEC_CAP_DELAY)
                    flush_complete = 1;
                continue;
            }
            data_size = av_samples_get_buffer_size(NULL, (int)m_frame->channels,(int)m_frame->nb_samples,(AVSampleFormat)m_frame->format, 1);

            dec_channel_layout =
                (m_frame->channel_layout && m_frame->channels == av_get_channel_layout_nb_channels(m_frame->channel_layout)) ?
                m_frame->channel_layout : av_get_default_channel_layout((int)m_frame->channels);
            wanted_nb_samples = synchronize_audio(m_frame->nb_samples);

            if (m_frame->format        != m_audio_src.fmt            ||
                dec_channel_layout     != m_audio_src.channel_layout ||
                m_frame->sample_rate   != m_audio_src.freq           ||
                (wanted_nb_samples     != m_frame->nb_samples && !m_swr_ctx)) 
			{
                swr_free(&m_swr_ctx);
                m_swr_ctx = swr_alloc_set_opts(NULL,
                                                 m_audio_tgt.channel_layout,      m_audio_tgt.fmt, m_audio_tgt.freq,
                                                 dec_channel_layout,(AVSampleFormat)m_frame->format, m_frame->sample_rate,
                                                 0, NULL);
                if (!m_swr_ctx || swr_init(m_swr_ctx) < 0) 
				{
                    fprintf(stderr, "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                        m_frame->sample_rate,   av_get_sample_fmt_name((AVSampleFormat)m_frame->format), (int)m_frame->channels,
                        m_audio_tgt.freq, av_get_sample_fmt_name(m_audio_tgt.fmt), m_audio_tgt.channels);
                    break;
                }
                m_audio_src.channel_layout = (int)dec_channel_layout;
                m_audio_src.channels = (int)m_frame->channels;
                m_audio_src.freq = m_frame->sample_rate;
                m_audio_src.fmt = (AVSampleFormat) m_frame->format;
            }

            if (m_swr_ctx) 
			{
                const uint8_t **in = (const uint8_t **)m_frame->extended_data;
                uint8_t *out[] = {g_audio_buf2};
                int out_count = sizeof(g_audio_buf2) / m_audio_tgt.channels / av_get_bytes_per_sample(m_audio_tgt.fmt);
                if (wanted_nb_samples != m_frame->nb_samples) 
				{
                    if (swr_set_compensation(m_swr_ctx, (wanted_nb_samples - m_frame->nb_samples) * m_audio_tgt.freq / m_frame->sample_rate,
                                                wanted_nb_samples * m_audio_tgt.freq / m_frame->sample_rate) < 0) 
					{
                        fprintf(stderr, "swr_set_compensation() failed\n");
                        break;
                    }
                }
                len2 = swr_convert(m_swr_ctx, out, out_count, in, m_frame->nb_samples);
                if (len2 < 0) 
				{
                    fprintf(stderr, "swr_convert() failed\n");
                    break;
                }
                if (len2 == out_count) 
				{
                    fprintf(stderr, "warning: audio buffer is probably too small\n");
                    swr_init(m_swr_ctx);
                }
                m_audio_buf = g_audio_buf2;
                resampled_data_size = len2 * m_audio_tgt.channels * av_get_bytes_per_sample(m_audio_tgt.fmt);
            } 
			else 
			{
                m_audio_buf = m_frame->data[0];
                resampled_data_size = data_size;
            }

            /* if no pts, then compute it */
            pts = m_audio_clock;
            *pts_ptr = pts;
            m_audio_clock += (double)data_size / (m_frame->channels * m_frame->sample_rate * av_get_bytes_per_sample((AVSampleFormat)m_frame->format));
#ifdef DEBUG
            {
                static double last_clock;
                printf("audio: delay=%0.3f clock=%0.3f pts=%0.3f\n",
                       m_audio_clock - last_clock,
                       m_audio_clock, pts);
                last_clock = m_audio_clock;
            }
#endif
            return resampled_data_size;
        }

        /* free the current packet */
        if (pkt->data)
            av_free_packet(pkt);
        memset(pkt_temp, 0, sizeof(*pkt_temp));

        if (m_paused || m_audioq.abort_request) {
            return -1;
        }

        if (m_audioq.nb_packets == 0)
            SDL_CondSignal(m_continue_read_thread);

        /* read next packet */
        if ((new_packet = packet_queue_get(&m_audioq, pkt, 1, &m_audio_pkt_temp_serial)) < 0)
            return -1;

        if (pkt->data == flush_pkt.data) 
		{
            avcodec_flush_buffers(dec);
            flush_complete = 0;
        }

        *pkt_temp = *pkt;

        /* if update the audio clock with the pts */
        if (pkt->pts != AV_NOPTS_VALUE) 
            m_audio_clock = av_q2d(m_audio_st->time_base)*pkt->pts;
        
    }
}

void FF_Play_Reader::sdl_audio_callback(Uint8 *stream, int len)
{
    int audio_size, len1;
    int bytes_per_sec;
    int frame_size = av_samples_get_buffer_size(NULL, m_audio_tgt.channels, 1, m_audio_tgt.fmt, 1);
    double pts;

    audio_callback_time = av_gettime();

    while (len > 0) 
	{
        if (m_audio_buf_index >= (int)m_audio_buf_size) 
		{
           audio_size = audio_decode_frame(&pts);
           if (audio_size < 0) 
		   {
                /* if error, just output silence */
               m_audio_buf      = m_silence_buf;
               m_audio_buf_size = sizeof(m_silence_buf) / frame_size * frame_size;
           } 
		   else
               m_audio_buf_size = audio_size;

           m_audio_buf_index = 0;
        }
        len1 = m_audio_buf_size - m_audio_buf_index;
        if (len1 > len)
            len1 = len;
        memcpy(stream, (uint8_t *)m_audio_buf + m_audio_buf_index, len1);
        len -= len1;
        stream += len1;
        m_audio_buf_index += len1;
    }
    bytes_per_sec = m_audio_tgt.freq * m_audio_tgt.channels * av_get_bytes_per_sample(m_audio_tgt.fmt);
    m_audio_write_buf_size = m_audio_buf_size - m_audio_buf_index;
    /* Let's assume the audio driver that is used by SDL has two periods. */
    m_audio_current_pts = m_audio_clock - (double)(2 * m_audio_hw_buf_size + m_audio_write_buf_size) / bytes_per_sec;
    m_audio_current_pts_drift = m_audio_current_pts - audio_callback_time / 1000000.0;
    if (m_audioq.serial == m_audio_pkt_temp_serial)
        check_external_clock_sync( m_audio_current_pts);
}

void sdl_audio_callback(void *opaque,Uint8 *stream, int len)
{
	FF_Play_Reader *is = (FF_Play_Reader *)opaque;
	is->sdl_audio_callback(stream,len);
}

static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params)
{
    SDL_AudioSpec wanted_spec, spec;
    const char *env;
    const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};

    env = SDL_getenv("SDL_AUDIO_CHANNELS");
    if (env) {
        wanted_nb_channels = atoi(env);
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
    }
    if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
    wanted_spec.channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.freq = wanted_sample_rate;
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        fprintf(stderr, "Invalid sample rate or channel count!\n");
        return -1;
    }
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = opaque;
    while (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
        fprintf(stderr, "SDL_OpenAudio (%d channels): %s\n", wanted_spec.channels, SDL_GetError());
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            fprintf(stderr, "No more channel combinations to try, audio open failed\n");
            return -1;
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }
    if (spec.format != AUDIO_S16SYS) {
        fprintf(stderr, "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }
    if (spec.channels != wanted_spec.channels) {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            fprintf(stderr, "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }

    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;
    audio_hw_params->channel_layout = (int)wanted_channel_layout;
    audio_hw_params->channels =  spec.channels;
    return spec.size;
}

int FF_Play_Reader::stream_component_open(int stream_index)
{
    AVFormatContext *ic = m_ic;
    AVCodecContext *avctx;
    AVCodec *codec;
    AVDictionary *opts;
    AVDictionaryEntry *t = NULL;

    if (stream_index < 0 || stream_index >= (int)ic->nb_streams)
        return -1;
    avctx = ic->streams[stream_index]->codec;

    codec = avcodec_find_decoder(avctx->codec_id);

    switch(avctx->codec_type)
	{
        case AVMEDIA_TYPE_AUDIO   : m_last_audio_stream    = stream_index; if(audio_codec_name   ) codec= avcodec_find_decoder_by_name(   audio_codec_name); break;
        case AVMEDIA_TYPE_SUBTITLE: m_last_subtitle_stream = stream_index; if(subtitle_codec_name) codec= avcodec_find_decoder_by_name(subtitle_codec_name); break;
        case AVMEDIA_TYPE_VIDEO   : m_last_video_stream    = stream_index; if(video_codec_name   ) codec= avcodec_find_decoder_by_name(   video_codec_name); break;
    }
    if (!codec)
        return -1;

    avctx->workaround_bugs   = workaround_bugs;
    avctx->lowres            = lowres;
    if(avctx->lowres > codec->max_lowres)
	{
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
                codec->max_lowres);
        avctx->lowres= codec->max_lowres;
    }
    avctx->idct_algo         = idct;
    avctx->skip_frame        = skip_frame;
    avctx->skip_idct         = skip_idct;
    avctx->skip_loop_filter  = skip_loop_filter;
    avctx->error_concealment = error_concealment;

    if(avctx->lowres) avctx->flags |= CODEC_FLAG_EMU_EDGE;
    if (fast)   avctx->flags2 |= CODEC_FLAG2_FAST;
    if(codec->capabilities & CODEC_CAP_DR1)
        avctx->flags |= CODEC_FLAG_EMU_EDGE;

    //opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
	opts = NULL;
    if (!av_dict_get(opts, "threads", NULL, 0))
        av_dict_set(&opts, "threads", "auto", 0);
    if (avcodec_open2(avctx, codec, &opts) < 0)
        return -1;
    if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) 
	{
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        return AVERROR_OPTION_NOT_FOUND;
    }

    /* prepare audio output */
    if (avctx->codec_type == AVMEDIA_TYPE_AUDIO) 
	{
        int audio_hw_buf_size = audio_open(this, avctx->channel_layout, avctx->channels, avctx->sample_rate, &m_audio_src);
        if (audio_hw_buf_size < 0)
            return -1;
        m_audio_hw_buf_size = audio_hw_buf_size;
        m_audio_tgt = m_audio_src;
    }

    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type) 
	{
    case AVMEDIA_TYPE_AUDIO:
        m_audio_stream = stream_index;
        m_audio_st = ic->streams[stream_index];
        m_audio_buf_size  = 0;
        m_audio_buf_index = 0;

        /* init averaging filter */
        m_audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
        m_audio_diff_avg_count = 0;
        /* since we do not have a precise anough audio fifo fullness,
           we correct audio sync only if larger than this threshold */
        m_audio_diff_threshold = 2.0 * m_audio_hw_buf_size / av_samples_get_buffer_size(NULL, m_audio_tgt.channels, m_audio_tgt.freq, m_audio_tgt.fmt, 1);

        memset(&m_audio_pkt, 0, sizeof(m_audio_pkt));
        memset(&m_audio_pkt_temp, 0, sizeof(m_audio_pkt_temp));
        packet_queue_start(&m_audioq);
        SDL_PauseAudio(0);
        break;
    case AVMEDIA_TYPE_VIDEO:
        m_video_stream = stream_index;
        m_video_st = ic->streams[stream_index];

        packet_queue_start(&m_videoq);
		m_video_tid = SDL_CreateThread(::video_thread, this);
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        m_subtitle_stream = stream_index;
        m_subtitle_st = ic->streams[stream_index];
        packet_queue_start(&m_subtitleq);

		m_subtitle_tid = SDL_CreateThread(::subtitle_thread, this);
        break;
    default:
        break;
    }
    return 0;
}

static void stream_component_close(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;

    if (stream_index < 0 || stream_index >= (int)ic->nb_streams)
        return;
    avctx = ic->streams[stream_index]->codec;

    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        packet_queue_abort(&is->audioq);

        SDL_CloseAudio();

        packet_queue_flush(&is->audioq);
        av_free_packet(&is->audio_pkt);
        swr_free(&is->swr_ctx);
        av_freep(&is->audio_buf1);
        is->audio_buf = NULL;
        avcodec_free_frame(&is->frame);

        if (is->rdft) {
            av_rdft_end(is->rdft);
            av_freep(&is->rdft_data);
            is->rdft = NULL;
            is->rdft_bits = 0;
        }
        break;
    case AVMEDIA_TYPE_VIDEO:
        packet_queue_abort(&is->videoq);

        /* note: we also signal this mutex to make sure we deblock the
           video thread in all cases */
        SDL_LockMutex(is->pictq_mutex);
        SDL_CondSignal(is->pictq_cond);
        SDL_UnlockMutex(is->pictq_mutex);

        SDL_WaitThread(is->video_tid, NULL);

        packet_queue_flush(&is->videoq);
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        packet_queue_abort(&is->subtitleq);

        /* note: we also signal this mutex to make sure we deblock the
           video thread in all cases */
        SDL_LockMutex(is->subpq_mutex);
        is->subtitle_stream_changed = 1;

        SDL_CondSignal(is->subpq_cond);
        SDL_UnlockMutex(is->subpq_mutex);

        SDL_WaitThread(is->subtitle_tid, NULL);

        packet_queue_flush(&is->subtitleq);
        break;
    default:
        break;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    avcodec_close(avctx);
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_st = NULL;
        is->audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_st = NULL;
        is->video_stream = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_st = NULL;
        is->subtitle_stream = -1;
        break;
    default:
        break;
    }
}

static int decode_interrupt_cb(void *ctx)
{
    VideoState *is = (VideoState *)ctx;
    return is->abort_request;
}

static int is_realtime(AVFormatContext *s)
{
    if(   !strcmp(s->iformat->name, "rtp")
       || !strcmp(s->iformat->name, "rtsp")
       || !strcmp(s->iformat->name, "sdp")
	   || !strnicmp(s->filename, "http", 4)  //TODO see about determining http being realtime or not -James
    )
        return 1;

    if(s->pb && (   !strncmp(s->filename, "rtp:", 4)
                 || !strncmp(s->filename, "udp:", 4)
                )
    )
        return 1;
    return 0;
}

static void print_error(const char *filename, int err)
{
	char errbuf[128];
	const char *errbuf_ptr = errbuf;

	if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
		errbuf_ptr = strerror(AVUNERROR(err));
	av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}

/* this thread gets the stream from the disk or the network */
static int read_thread(void *arg)
{
    VideoState *is = (VideoState *)arg;
    AVFormatContext *ic = NULL;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    int eof = 0;
    int pkt_in_play_range = 0;
    //AVDictionaryEntry *t;
    AVDictionary **opts;
    int orig_nb_streams;
    SDL_mutex *wait_mutex = SDL_CreateMutex();

    memset(st_index, -1, sizeof(st_index));
    is->last_video_stream = is->video_stream = -1;
    is->last_audio_stream = is->audio_stream = -1;
    is->last_subtitle_stream = is->subtitle_stream = -1;

    ic = avformat_alloc_context();
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;
    //err = avformat_open_input(&ic, is->filename, is->iformat, &format_opts);
	err = avformat_open_input(&ic, is->filename, is->iformat, NULL);
    if (err < 0) {
        print_error(is->filename, err);
        ret = -1;
        goto fail;
    }
    //if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
    //    av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
    //    ret = AVERROR_OPTION_NOT_FOUND;
    //    goto fail;
    //}
    is->ic = ic;

    if (genpts)
        ic->flags |= AVFMT_FLAG_GENPTS;

    //opts = setup_find_stream_info_opts(ic, codec_opts);
	opts=NULL;
    orig_nb_streams = ic->nb_streams;

    err = avformat_find_stream_info(ic, opts);
    if (err < 0) {
        fprintf(stderr, "%s: could not find codec parameters\n", is->filename);
        ret = -1;
        goto fail;
    }
    //for (i = 0; i < orig_nb_streams; i++)
    //    av_dict_free(&opts[i]);
    //av_freep(&opts);

    if (ic->pb)
        ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use url_feof() to test for the end

    if (seek_by_bytes < 0)
        seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT);

    /* if seeking requested, we execute it */
    if (start_time != AV_NOPTS_VALUE) {
        int64_t timestamp;

        timestamp = start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0) {
            fprintf(stderr, "%s: could not seek to position %0.3f\n",
                    is->filename, (double)timestamp / AV_TIME_BASE);
        }
    }

    is->realtime = is_realtime(ic);

    for (i = 0; i < (int)ic->nb_streams; i++)
        ic->streams[i]->discard = AVDISCARD_ALL;
    if (!video_disable)
	{
        st_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                wanted_stream[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
		//hack not sure why we must have wanted stream... but for clips that put video stream on 1 (e.g. BadApple) we'll need to try to get it
		//from this index
		//  [11/27/2012 JamesK]
		if (st_index[AVMEDIA_TYPE_VIDEO]!=wanted_stream[AVMEDIA_TYPE_VIDEO])
			st_index[AVMEDIA_TYPE_VIDEO] =	av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,	wanted_stream[AVMEDIA_TYPE_AUDIO], -1, NULL, 0);

	}
    if (!audio_disable)
        st_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                wanted_stream[AVMEDIA_TYPE_AUDIO],
                                st_index[AVMEDIA_TYPE_VIDEO],
                                NULL, 0);
    if (!video_disable)
        st_index[AVMEDIA_TYPE_SUBTITLE] =
            av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                wanted_stream[AVMEDIA_TYPE_SUBTITLE],
                                (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                 st_index[AVMEDIA_TYPE_AUDIO] :
                                 st_index[AVMEDIA_TYPE_VIDEO]),
                                NULL, 0);
		#ifdef __ShowStatus__
        av_dump_format(ic, 0, is->filename, 0);
		#endif

    is->show_mode = show_mode;

    /* open the streams */
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO]);
    }

    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        ret = stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO]);
    }

    if (is->show_mode == VideoState::SHOW_MODE_NONE)
        is->show_mode = ret >= 0 ? VideoState::SHOW_MODE_VIDEO : VideoState::SHOW_MODE_RDFT;

    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
        stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE]);
    }

    if (is->video_stream < 0 && is->audio_stream < 0) {
        fprintf(stderr, "%s: could not open codecs\n", is->filename);
        ret = -1;
        goto fail;
    }

    if (infinite_buffer < 0 && is->realtime)
        infinite_buffer = 1;

    for (;;) {
        if (is->abort_request)
            break;
        if (is->paused != is->last_paused) {
            is->last_paused = is->paused;
            if (is->paused)
                is->read_pause_return = av_read_pause(ic);
            else
                av_read_play(ic);
        }
#if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
        if (is->paused &&
                (!strcmp(ic->iformat->name, "rtsp") ||
                 (ic->pb && !strncmp(input_filename, "mmsh:", 5)))) {
            /* wait 10 ms to avoid trying to get another packet */
            /* XXX: horrible */
            SDL_Delay(10);
            continue;
        }
#endif
        if (is->seek_req) {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;
// FIXME the +-2 is due to rounding being not done in the correct direction in generation
//      of the seek_pos/seek_rel variables

            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0) {
                fprintf(stderr, "%s: error while seeking\n", is->ic->filename);
            } else {
                if (is->audio_stream >= 0) {
                    packet_queue_flush(&is->audioq);
                    packet_queue_put(&is->audioq, &flush_pkt);
                }
                if (is->subtitle_stream >= 0) {
                    packet_queue_flush(&is->subtitleq);
                    packet_queue_put(&is->subtitleq, &flush_pkt);
                }
                if (is->video_stream >= 0) {
                    packet_queue_flush(&is->videoq);
                    packet_queue_put(&is->videoq, &flush_pkt);
                }
            }
            update_external_clock_pts(is, (seek_target + ic->start_time) / (double)AV_TIME_BASE);
            is->seek_req = 0;
            eof = 0;
        }
        if (is->que_attachments_req) {
            avformat_queue_attached_pictures(ic);
            is->que_attachments_req = 0;
        }

        /* if the queue are full, no need to read more */
        if (infinite_buffer<1 &&
              (is->audioq.size + is->videoq.size + is->subtitleq.size > MAX_QUEUE_SIZE
            || (   (is->audioq   .nb_packets > MIN_FRAMES || is->audio_stream < 0 || is->audioq.abort_request)
                && (is->videoq   .nb_packets > MIN_FRAMES || is->video_stream < 0 || is->videoq.abort_request)
                && (is->subtitleq.nb_packets > MIN_FRAMES || is->subtitle_stream < 0 || is->subtitleq.abort_request)))) {
            /* wait 10 ms */
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        if (eof) {
            if (is->video_stream >= 0) {
                av_init_packet(pkt);
                pkt->data = NULL;
                pkt->size = 0;
                pkt->stream_index = is->video_stream;
                packet_queue_put(&is->videoq, pkt);
            }
            if (is->audio_stream >= 0 &&
                is->audio_st->codec->codec->capabilities & CODEC_CAP_DELAY) {
                av_init_packet(pkt);
                pkt->data = NULL;
                pkt->size = 0;
                pkt->stream_index = is->audio_stream;
                packet_queue_put(&is->audioq, pkt);
            }
            SDL_Delay(10);
            if (is->audioq.size + is->videoq.size + is->subtitleq.size == 0) {
                if (loop != 1 && (!loop || --loop)) {
                    stream_seek(is, start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
                } else if (autoexit) {
                    ret = AVERROR_EOF;
                    goto fail;
                }
            }
            eof=0;
            continue;
        }
        ret = av_read_frame(ic, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF || url_feof(ic->pb))
                eof = 1;
            if (ic->pb && ic->pb->error)
                break;
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                (pkt->pts - ic->streams[pkt->stream_index]->start_time) *
                av_q2d(ic->streams[pkt->stream_index]->time_base) -
                (double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000
                <= ((double)duration / 1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range) {
            packet_queue_put(&is->audioq, pkt);
        } else if (pkt->stream_index == is->video_stream && pkt_in_play_range) {
            packet_queue_put(&is->videoq, pkt);
        } else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range) {
            packet_queue_put(&is->subtitleq, pkt);
        } else {
            av_free_packet(pkt);
        }
    }
    /* wait until the end */
    while (!is->abort_request) {
        SDL_Delay(100);
    }

    ret = 0;
 fail:
    /* close each stream */
    if (is->audio_stream >= 0)
        stream_component_close(is, is->audio_stream);
    if (is->video_stream >= 0)
        stream_component_close(is, is->video_stream);
    if (is->subtitle_stream >= 0)
        stream_component_close(is, is->subtitle_stream);
    if (is->ic) {
        avformat_close_input(&is->ic);
    }

    if (ret != 0) {
        SDL_Event event;

        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    SDL_DestroyMutex(wait_mutex);
    return 0;
}

static VideoState *stream_open(const char *filename, AVInputFormat *iformat,FrameWork::Outstream_Interface * Outstream)
{
    VideoState *is;

    is = (VideoState *)av_mallocz(sizeof(VideoState));
    if (!is)
        return NULL;
    av_strlcpy(is->filename, filename, sizeof(is->filename));
    is->iformat = iformat;
    is->ytop    = 0;
    is->xleft   = 0;

    /* start video display */
    is->pictq_mutex = SDL_CreateMutex();
    is->pictq_cond  = SDL_CreateCond();

    is->subpq_mutex = SDL_CreateMutex();
    is->subpq_cond  = SDL_CreateCond();

    packet_queue_init(&is->videoq);
    packet_queue_init(&is->audioq);
    packet_queue_init(&is->subtitleq);

    is->continue_read_thread = SDL_CreateCond();

    update_external_clock_pts(is, 0.0);
    update_external_clock_speed(is, 1.0);
    is->audio_current_pts_drift = -av_gettime() / 1000000.0;
    is->video_current_pts_drift = is->audio_current_pts_drift;
    is->av_sync_type = av_sync_type;
	assert(Outstream);
	is->Preview=Outstream;
    is->read_tid     = SDL_CreateThread(read_thread, is);
    if (!is->read_tid) {
        av_free(is);
        return NULL;
    }
    return is;
}

static void stream_cycle_channel(VideoState *is, int codec_type)
{
    AVFormatContext *ic = is->ic;
    int start_index, stream_index;
    int old_index;
    AVStream *st;

    if (codec_type == AVMEDIA_TYPE_VIDEO) {
        start_index = is->last_video_stream;
        old_index = is->video_stream;
    } else if (codec_type == AVMEDIA_TYPE_AUDIO) {
        start_index = is->last_audio_stream;
        old_index = is->audio_stream;
    } else {
        start_index = is->last_subtitle_stream;
        old_index = is->subtitle_stream;
    }
    stream_index = start_index;
    for (;;) {
        if (++stream_index >= (int)is->ic->nb_streams)
        {
            if (codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                stream_index = -1;
                is->last_subtitle_stream = -1;
                goto the_end;
            }
            if (start_index == -1)
                return;
            stream_index = 0;
        }
        if (stream_index == start_index)
            return;
        st = ic->streams[stream_index];
        if (st->codec->codec_type == codec_type) {
            /* check that parameters are OK */
            switch (codec_type) {
            case AVMEDIA_TYPE_AUDIO:
                if (st->codec->sample_rate != 0 &&
                    st->codec->channels != 0)
                    goto the_end;
                break;
            case AVMEDIA_TYPE_VIDEO:
            case AVMEDIA_TYPE_SUBTITLE:
                goto the_end;
            default:
                break;
            }
        }
    }
 the_end:
    stream_component_close(is, old_index);
    stream_component_open(is, stream_index);
    if (codec_type == AVMEDIA_TYPE_VIDEO)
        is->que_attachments_req = 1;
}


static void toggle_pause(VideoState *is)
{
    stream_toggle_pause(is);
    is->step = 0;
}

static void step_to_next_frame(VideoState *is)
{
    /* if the stream is paused unpause it, then step */
    if (is->paused)
        stream_toggle_pause(is);
    is->step = 1;
}

static int opt_format(void *optctx, const char *opt, const char *arg)
{
    file_iformat = av_find_input_format(arg);
    if (!file_iformat) {
        fprintf(stderr, "Unknown input format: %s\n", arg);
        return AVERROR(EINVAL);
    }
    return 0;
}

static int opt_sync(void *optctx, const char *opt, const char *arg)
{
    if (!strcmp(arg, "audio"))
        av_sync_type = AV_SYNC_AUDIO_MASTER;
    else if (!strcmp(arg, "video"))
        av_sync_type = AV_SYNC_VIDEO_MASTER;
    else if (!strcmp(arg, "ext"))
        av_sync_type = AV_SYNC_EXTERNAL_CLOCK;
    else {
        fprintf(stderr, "Unknown value for %s: %s\n", opt, arg);
        exit(1);
    }
    return 0;
}

static int opt_codec(void *o, const char *opt, const char *arg)
{
    switch(opt[strlen(opt)-1]){
    case 'a' :    audio_codec_name = arg; break;
    case 's' : subtitle_codec_name = arg; break;
    case 'v' :    video_codec_name = arg; break;
    }
    return 0;
}

static int dummy;

static int lockmgr(void **mtx, enum AVLockOp op)
{
   switch(op) {
      case AV_LOCK_CREATE:
          *mtx = SDL_CreateMutex();
          if(!*mtx)
              return 1;
          return 0;
      case AV_LOCK_OBTAIN:
          return !!SDL_LockMutex((SDL_mutex *)(*mtx));
      case AV_LOCK_RELEASE:
          return !!SDL_UnlockMutex((SDL_mutex *)(*mtx));
      case AV_LOCK_DESTROY:
          SDL_DestroyMutex((SDL_mutex *)(*mtx));
          return 0;
   }
   return 1;
}

static void ffm_logger(void* ptr, int level, const char* fmt, va_list vl)
{
#ifndef NDEBUG
	if (level > av_log_get_level() || strstr(fmt, "%td"))
		return;

	char Temp[2048];
	vsprintf_s(Temp, 2048, fmt, vl);
	OutputDebugStringA(Temp);
#endif
}


  /***************************************************************************************************************/
 /*												FrameGrabber_FFMpeg												*/
/***************************************************************************************************************/

size_t FrameGrabber_FFMpeg::split_arguments(const std::string& str, std::vector<std::string>& arguments)
{
	arguments.clear();

	if (str.empty())
		return 0;

	const std::string whitespace = " \t\n\r";
	const char group_char = '"';
	bool in_argument = false;

	arguments.push_back(std::string());
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		if (*it == group_char)
			in_argument = !in_argument;
		else if (in_argument || whitespace.find(*it) == std::string::npos)
			arguments.back().push_back(*it);
		else if (!arguments.back().empty())
			arguments.push_back(std::string());
	}

	if (arguments.back().empty())
		arguments.pop_back();

	assert(!in_argument); // Uneven quotes?

	return arguments.size();
}

void FrameGrabber_FFMpeg::SetFileName(const wchar_t *IPAddress,IpURLConversion format)
{
	if (IPAddress[0]!=0)
		FrameWork::DebugOutput("FrameGrabber [%p] Ip Address=%ls\n",this,IPAddress);
	else
	{
		assert(false);  //User submitted nothing?  recovery... keep last file
		return;
	}

	#if 0
	input_filename="rtsp://FRC:FRC@10.28.1.11/axis-media/media.amp";
	#else
	{
		wchar2char(IPAddress);
		m_URL=wchar2char_pchar;
	}
	#endif

	//Now to evaluate if it is a number... we'll want to have some intelligent way to deal with the correct URL but for now just hard code the m1011's H264's URL
	if ((m_URL.c_str()[0]>='0')&&(m_URL.c_str()[0]<='9'))
	{
		char Buffer[1024];
		//this is lazy but effective... TODO parse numbers remove leading zero's as this will cause it to fail
		switch (format)
		{
			case eIpURL_MJPEG:
				sprintf_s(Buffer,1024,"http://FRC:FRC@%s/mjpg/video.mjpg",m_URL.c_str());
				break;
			default:
				sprintf_s(Buffer,1024,"rtsp://FRC:FRC@%s/axis-media/media.amp",m_URL.c_str());
		}
		audio_disable=1;
		m_URL=Buffer;
	}
	input_filename=m_URL.c_str();
}

FrameGrabber_FFMpeg::FrameGrabber_FFMpeg(FrameWork::Outstream_Interface *Preview,const wchar_t *IPAddress) : m_Outstream(Preview), m_VideoStream(NULL),m_TestPattern(Preview,IPAddress)
{
	//If we have no IPAddress we have no work to do
	if(IPAddress[0]==0)
		return;

	SetFileName(IPAddress);

	int flags;
	//VideoState *is;
	char dummy_videodriver[] = "SDL_VIDEODRIVER=dummy";

	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	av_log_set_callback(ffm_logger);

	/* register all codecs, demux and protocols */
	avcodec_register_all();
	av_register_all();
	avformat_network_init();

	flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
	if (audio_disable)
		flags &= ~SDL_INIT_AUDIO;

	if (display_disable)
		SDL_putenv(dummy_videodriver); /* For the event queue, we always need a video driver. */

	if (SDL_Init (flags)) {
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		fprintf(stderr, "(Did you set the DISPLAY variable?)\n");
		exit(1);
	}

	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	if (av_lockmgr_register(lockmgr)) {
		fprintf(stderr, "Could not initialize lock manager!\n");
		//do_exit(NULL);
	}

	av_init_packet(&flush_pkt);
	flush_pkt.data = (uint8_t *)((char *)(intptr_t)"FLUSH");
}

FrameGrabber_FFMpeg::~FrameGrabber_FFMpeg()
{
	StopStreaming();

	//TODO I should be able to call functions even if they were never open...
	//I'll need to ensure the code can handle that
	if (m_URL[0]==0)
		return;

	av_lockmgr_register(NULL);
	avformat_network_deinit();
	#ifdef __ShowStatus__
		printf("\n");
	#endif
	SDL_Quit();
	av_log(NULL, AV_LOG_QUIET, "%s", "");
}

void FrameGrabber_FFMpeg::SetOutstream_Interface(FrameWork::Outstream_Interface *Preview) 
{
	m_Outstream=Preview;
	m_TestPattern.SetOutstream_Interface(Preview);
}

bool FrameGrabber_FFMpeg::StartStreaming()
{
	if (m_VideoStream) return true;
	if (m_URL[0]==0)
		return m_TestPattern.StartStreaming();

	m_VideoStream = stream_open(input_filename, file_iformat, m_Outstream);
	if (!m_VideoStream) 
		fprintf(stderr, "Failed to initialize VideoState!\n");
	else
	{
		VideoState *is=(VideoState *)m_VideoStream;
		is->procamp=new Processing::FX::procamp::Procamp_Manager;
	}

	return m_VideoStream!=NULL;
}

void FrameGrabber_FFMpeg::StopStreaming()
{
	m_TestPattern.StopStreaming();  //this is fine to call checks implicitly
	if (!m_VideoStream) return;

	VideoState *is=(VideoState *)m_VideoStream;
	//grab pointer before we nuke structure
	Processing::FX::procamp::Procamp_Manager *procamp=is->procamp;
	stream_close((VideoState *)m_VideoStream);
	delete procamp;

	m_VideoStream=NULL;
}
  /***************************************************************************************************************/
 /*													FrameGrabber												*/
/***************************************************************************************************************/

FrameGrabber::FrameGrabber(FrameWork::Outstream_Interface *Preview,const wchar_t *IPAddress,ReaderFormat format) : m_VideoStream(NULL)
{
	if (IPAddress[0]!=0)
		FrameWork::DebugOutput("FrameGrabber [%p] Ip Address=%ls\n",this,IPAddress);

	//determine which reader to use
	if (IPAddress[0]==0)
		m_VideoStream=new FrameGrabber_TestPattern(Preview,IPAddress);
	else
	{
		std::wstring IPToUse=IPAddress;
		//Now to evaluate if it is a number... we'll want to have some intelligent way to deal with the correct URL but for now just hard code the m1011's H264's URL
		if ((IPToUse.c_str()[0]>='0')&&(IPToUse.c_str()[0]<='9'))
		{
			wchar_t Buffer[1024];
			//this is lazy but effective... TODO parse numbers remove leading zero's as this will cause it to fail
			if (format==eFFMPeg_Reader)
				swprintf(Buffer,1024,L"rtsp://FRC:FRC@%s/axis-media/media.amp",IPToUse.c_str());
			else
				swprintf(Buffer,1024,L"http://FRC:FRC@%s/mjpg/video.mjpg",IPToUse.c_str());
			IPToUse=Buffer;
		}
		switch (format)
		{
			case eTestPattern:
				m_VideoStream=new FrameGrabber_TestPattern(Preview,IPToUse.c_str());
				break;
			case eFFMPeg_Reader:
			case eHttpReader:
				m_VideoStream=new FFPlay_Controller(Preview,IPToUse.c_str(),
					format==eHttpReader?FrameGrabber_FFMpeg::eIpURL_MJPEG:FrameGrabber_FFMpeg::eIpURL_H264);
				break;
			case eHttpReader2:
				assert(false);
				break;
		}

	}
	assert(m_VideoStream);
}

FrameGrabber::~FrameGrabber()
{
	delete m_VideoStream;
	m_VideoStream=NULL;
}

  /***************************************************************************************************************/
 /*												FFPlay_Controller												*/
/***************************************************************************************************************/

void FFPlay_Controller::Flush() 
{

}
int FFPlay_Controller::Run (void)
{
	StartStreaming();
	VideoState * is=(VideoState *)m_VideoStream;
	if (is->realtime) return 0;
	is->stopped=0;
	if (is->paused)
		toggle_pause(is);
	return 0;
}

void FFPlay_Controller::SwitchFilename(const wchar_t FileToUse[])
{
	StopStreaming();
	SetFileName(FileToUse,m_IP_Format);
	StartStreaming();
}

void FFPlay_Controller::GetFileName(std::wstring &Output) const
{
	char2wchar(m_URL.c_str());
	Output=char2wchar_pwchar;
}

int FFPlay_Controller::Stop (void)
{
	VideoState * is=(VideoState *)m_VideoStream;
	if (is->realtime) return 0;
	is->stopped=1;
	if (!is->paused)
		toggle_pause(is);
	else
	{
		int seek_by_bytes=(::seek_by_bytes || is->ic->duration <= 0) ? 1:0;
		stream_seek(is,0,0,seek_by_bytes);
		//TODO this is a hack will need to have real seeking
		toggle_pause(is);
		Sleep(60);
		toggle_pause(is);
	}
	return 0;
}

int FFPlay_Controller::Pause (void)
{
	VideoState * is=(VideoState *)m_VideoStream;
	if (is->realtime) return 0;
	is->stopped=0;
	toggle_pause(is);
	return 0;
}

void FFPlay_Controller::Seek (double fraction)
{
	VideoState * cur_stream=(VideoState *)m_VideoStream;
	if (cur_stream->realtime) return;
	cur_stream->stopped=0;
	{
		int64_t ts;
		int ns, hh, mm, ss;
		int tns, thh, tmm, tss;
		tns  = (int)(cur_stream->ic->duration / 1000000LL);
		thh  = tns / 3600;
		tmm  = (tns % 3600) / 60;
		tss  = (tns % 60);
		ns   = (int)(fraction * tns);
		hh   = ns / 3600;
		mm   = (ns % 3600) / 60;
		ss   = (ns % 60);
		fprintf(stderr, "Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n", fraction*100,
			hh, mm, ss, thh, tmm, tss);
		ts = (int64_t)(fraction * cur_stream->ic->duration);
		if (cur_stream->ic->start_time != AV_NOPTS_VALUE)
			ts += cur_stream->ic->start_time;
		stream_seek(cur_stream, ts, 0, 0);
	}
	cur_stream->stopped=1;
}

int FFPlay_Controller::SetRate (int)
{
	return 0;
}

bool FFPlay_Controller::Set_ProcAmp(ProcAmp_enum ProcSetting,double value)
{
	VideoState * is=(VideoState *)m_VideoStream;
	return is->procamp->Set_ProcAmp(ProcSetting,value);
}

double FFPlay_Controller::Get_ProcAmp(ProcAmp_enum ProcSetting) const
{
	VideoState * is=(VideoState *)m_VideoStream;
	return is->procamp->Get_ProcAmp(ProcSetting);
}
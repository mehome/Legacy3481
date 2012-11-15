/**
 * $Id: ldrdv.cpp,v 1.0 2000/01/30 19:17:42 kalle Exp $
 *
 * Copyright 2000 by MainConcept GmbH
 */

// !!! attention: there are some accesses going via "*(long*)&" instead of UCHAR, these are CPU-dependend!

/*@/// includes */
#ifdef unix
#define USE_DV_CODEC 1
#endif

#ifdef WIN32
#define USE_DV_CODEC 1
#endif

//#define __MADEBUG__ 1
#ifdef __MADEBUG__
#include  <maoutput.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include  "typedefs.h"
#include  "Project.h"
#include  <string.h>
#include  <stdio.h>
#include  "ldrdv.h"

#include "filter_functions.h"
//#pragma comment(lib, "mcdvd_32")

/*@\\\0000000901*/
/*@/// protos */
// ULONG readBlock( void *buf, ULONG bufsize);
// ULONG SwpLng( ULONG Value );
/*
#ifdef USE_DV_CODEC
extern "C" ULONG __cdecl DecompressBuffer_DV (
                         PBYTE pbSrc, ULONG SrcBufLen,
                         PBYTE pbDst, int wRealLineSize, ULONG DstWidth, ULONG DstHeight,
                         ULONG ulFlags, ULONG ColorSpaceFourcc, ULONG optFlags, PBYTE extInfo);
#endif
*/
/*@\\\0000000501*/

/*@/// !project specific defines */
// #define UNKNOWN     12

// #define MAXJSAMPLE 255
// #define CENTERJSAMPLE 128
// #define RANGE_MASK  (MAXJSAMPLE * 4 + 3) /* 2 bits wider than legal samples */
/*@\\\0000000107*/
/*@/// !gvar */
// ULONG CompressMethod=0, indexOffset, useIndex, indexCounter, indexFullOffset;

// struct SoundBuffer *soundBuffer;
// struct ChunkHeader chead={0};
// struct FrameHeader fhead={0};
// struct AnimHeader ahead;

// struct MainAVIHeader AVIH;
// struct AVIStreamHeader strh;
// struct AVIBitMapHeader bmhd;
// struct AVIAudioHeader audiohdr;
// struct AVIIndexHeader *aviindex, *ind;

// struct FrameData *lastSoundFrame;
// ULONG framesCount=0, indexEntries, moviOffset, soundChunks, dataChunks, outChunks;

// double fpsError, fpsTimecode, fpsTotalError;
// int    fpsTiming;
/*@\\\*/

struct ErrorInfo gr_error_info;
/*@/// struct codecInfo gar_codec[] = {};  // !!! adapt content for selecting fields?!? */
struct codecInfo gar_codec[] = {
  {"DigitalVideo",     0x64737664},  // dvsd
  {"Unknown",          0}
};
/*@\\\0000000203*/


/*@/// void DecompressBuffer_DVAudio() */
#define DV_PACK_SIZE 5
#define DV_BLK_SIZE 80
#define DV_SUBC_SYB_OFS 3
#define DV_SUBC_SYB_NUM 6
#define DV_SYB_SIZE 8
#define DV_SYB_PACK_OFS 3
#define DV_SYB_PACK_NUM 1
#define DV_VAUX_PACK_OFS 3
#define DV_VAUX_PACK_NUM 15
#define DV_AAUX_PACK_OFS 3
#define DV_AAUX_PACK_NUM 1
#define DV_ADTA_OFS 8
#define DV_ADTA_SIZE 72
#define DV_VDTA_OFS 3
#define DV_VDTA_SIZE 77
// DIF: 150 blks: 1hdr,2subc,3vaux,9adta,135vdta
#define DV_DIF_SIZE 12000
// FR: 50: (25*)12*DIF, 60: (30*)10*DIF
#define DV_FR50_BLK_NUM 1800
#define DV_FR50_SIZE 144000
#define DV_FR50_CHN1 6
#define DV_FR60_BLK_NUM 1500
#define DV_FR60_SIZE 120000
#define DV_FR60_CHN1 5
#define DV_FR_MAX_BLK_NUM 1800
#define DV_FR_MAX_SIZE 144000
// one second of data
#define DV_SEC_SIZE 3600000

/*@/// struct tr_dv_aux */
struct tr_dv_aux
{
  unsigned char video_system;   // 0x00=525-60 0x01=SDL_525-60 0x02=1125-60 0x20=625-50 0x21=SDL_625-50 0x22=1250-50
  unsigned char frame_rate;     // 0=30fr/60fields/10DIFs 1=25fr/50fields/12DIFs
  unsigned char lp_sp;          // 0=LP 1=SP
  unsigned int resolution_x;    // 525-60: 720 625-50:720
  unsigned int resolution_y;    // 525-60: 480 625-50:576

  struct tr_dv_aux_titl // TITL
  {
    unsigned long time_code;    // BCD
  } titl;
  struct tr_dv_aux_aaux // AAUX
  {
    unsigned char locked;       // 0=locked 1=unlocked
    unsigned int au_fr_size;
    unsigned char stereo;         // 0=multi 1=lumped
    unsigned char channel_num;    // 0=one_chn_per_blk 1=two_chn_per_blk
    unsigned char pair;           // 0=pair 1=independent
    unsigned char au_mode;        // see table
/*@///                   au_mode */
/*

Audio Blk:   CH1   CH2
Trk Pos: 60  0..4  5..9
         50  0..5  6..b
Enc Mode:2Ch 48k   48k
             44k1  44k1
             32k   32k
         4Ch 32k-2 32k-2

SM  0        1
CHN 0  1     0/1
    12 ac bd 1   2   3   4
    34 eg fh a b c d e f g h
AM
 0  L  L  R  L   R   C   S
 1  R  M1 -  L   R   C   M
 2  M  M1 M2 L   R   C   -
 3  !  LS RS L   R   LS  RS
 4  !  C  S  Lm  Rm  T WoQ1Q2
 5  !  C  -  L R C WoLSRSLmRm
 6  !  C  M1 L R C WoLSRSLSRS
 7  !  !  !  L R C WoLSRSLCRC
 8  !  !  !  ! ! ! ! ! ! ! !
 9  !  !  !  ! ! ! ! ! ! ! !
 a  !  !  !  ! ! ! ! ! ! ! !
 b  !  !  !  ! ! ! ! ! ! ! !
 c  !  !  !  ! ! ! ! ! ! ! !
 d  !  !  !  ! ! ! ! ! ! ! !
 e  ?  ?  ?  ! ! ! ! ! ! ! !
 f  -  -  -  ! ! ! ! ! ! ! !

!:Res ?:indistinguishable -:noinfo
L:leftStereo R:rightStereo M:monaural C:center S:surround LS:leftSurround RS:rightSurround LC:leftCenter RC:rightCenter
Wo:woofer Lm:L+.7C+.7LS Rm:R+.7C+.7RS T:.7C Q1:.7LS+.7RS Q2:.7LS-.7RS


CHN=0    CH1   CH2
         PA AM PA AM
Stereo    0  0  0  1
2ch mono  1  2  1  2
1ch mono  1  2  1  f
indist    1  e  1  e
no info   1  f  1  f

CHN=1    CHab  CHcd
         PA AM PA AM
St+St     1  0  1  0
St+2Mn    1  0  1  2
St+1Mn    1  0  1  1
St        1  0  1  f
2Mn+St    1  2  1  0
4Mn       1  2  1  2
3Mn 1     1  2  1  1
2Mn 1     1  2  1  f
1Mn+St    1  1  1  0
3Mn 2     1  1  1  2
2Mn 2     1  1  1  1
1Mn       1  1  1  f
3/1St     0  0  0  4
3/0St+1Mn 0  0  0  6
3/0St     0  0  0  5
2/2St     0  0  0  3
indist    1  e  1  e
no info   1  f  1  f

*/
/*@\\\800000080D00080D*/
    unsigned char lang;           // 0=multi 1=single
    unsigned char emphasis;       // 0=on 1=off
    unsigned char time_emph;      // 0=res 1=50/15mysec
    unsigned char sample_rate;    // 0=48kHz 1=44.1kHz 2=32kHz
    unsigned char sample_size;    // 0=16bit_lin 1=12bit_nonlin 2=20bit_lin
    unsigned char compr_count;    // 0=once 1=twice 2=triple+ 3=noinfo
  } aaux[2];            // for the two channels separately: 50:0..5/6..b 60:0..4/5..9
  struct tr_dv_aux_vaux // VAUX
  {
    unsigned char bw;             // 0=b/w 1=col
    unsigned char col_fl_val;     // 0=valid 1=invalid
    unsigned char col_fl;         // 50: 0=a 1=b 60: 0=1_2 1=3_4 2=5_6 3=7_8
    unsigned char display_mode;   // 50: 0=4:3_full 1=16:9_LB(c) 2=16:9_full 60: 0=4:3_full 1=14:9_LB(c) 2=14:9_LB(t) 3=16:9_LB(c) 4=16:9_LB(t) 5=>16:9_LB(c) 6=14:9_full(c) 7=16:9_full
    unsigned char field_flags;    // 0x80: 1fl_twice/both_fl 0x40: fl1_first/fl2_first 0x20: no_chg/chg 0x10: non_il/il 0x08: still/std fl dist 0x04: still/norm cam pic 0x3: 0=EIAJ/EIA (NTSC) 1=ETS (PAL)
  } vaux;

  struct ProjectData *mpr_project_data;
};
/*@\\\0000001501*/

// 12DIF*9ADTA*72bytes
#define DV_AC_ADTA_SIZE 7776
#define DV_AC_ADTA_50_SIZE 7776
#define DV_AC_ADTA_60_SIZE 6480
// 6DIF*9ADTA*72bytes
#define DV_AC_ADTA_CHN_SIZE 3888
#define DV_AC_ADTA_50_CHN_SIZE 3888
#define DV_AC_ADTA_60_CHN_SIZE 3240

// deshuffle tables
/*@/// BYTE ac_dv_adta_60_ofs[] = {}; */
BYTE ac_dv_adta_60_ofs[] = {
   0,15,30,10,25,40, 5,20,35,
   3,18,33,13,28,43, 8,23,38,
   6,21,36, 1,16,31,11,26,41,
   9,24,39, 4,19,34,14,29,44,
  12,27,42, 7,22,37, 2,17,32
};
/*@\\\0000000703*/
/*@/// BYTE ac_dv_adta_50_ofs[] = {}; */
BYTE ac_dv_adta_50_ofs[] = {
   0,18,36,13,31,49, 8,26,44,
   3,21,39,16,34,52,11,29,47,
   6,24,42, 1,19,37,14,32,50,
   9,27,45, 4,22,40,17,35,53,
  12,30,48, 7,25,43, 2,20,38,
  15,33,51,10,28,46, 5,23,41
};
/*@\\\0000000201*/

/*@/// void unshuffle_adta_60_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_60_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i = 0; i<45; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_60_ofs[i]*2];
    for (int j = 36; j>0; --j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
//       pc_tgt[0] = pc_src[0];
//       pc_tgt[1] = pc_src[1];
      pc_src +=  2;
      pc_tgt += 90; // 45*2bytes
    }
  }
}
/*@\\\0000000B08*/
/*@/// void unshuffle_adta_50_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_50_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i = 0; i<54; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_50_ofs[i]*2];
    for (int j = 36; j>0; --j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
//       pc_tgt[0] = pc_src[0];
//       pc_tgt[1] = pc_src[1];
      pc_src +=   2;
      pc_tgt += 108;  // 54*2bytes
    }
  }
}
/*@\\\0000000901*/
/*@/// void unshuffle_adta_60_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_60_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i = 0; i<45; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_60_ofs[i]*3];
    for (int j = 24; j>0; --j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
//       pc_tgt[0] = pc_src[0];
//       pc_tgt[1] = pc_src[1];
      pc_tgt[2] = pc_src[2];
      pc_src +=   3;
      pc_tgt += 135; // 45*3bytes
    }
  }
}
/*@\\\0000000C01*/
/*@/// void unshuffle_adta_50_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_50_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i = 0; i<54; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_50_ofs[i]*3];
    for (int j = 24; j>0; --j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
//       pc_tgt[0] = pc_src[0];
//       pc_tgt[1] = pc_src[1];
      pc_tgt[2] = pc_src[2];
      pc_src +=   3;
      pc_tgt += 162; // 54*3bytes
    }
  }
}
/*@\\\0000000C01*/
/*@/// void analyze_packs(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_pack,int pack_num,int dv_chn_nr) */
void analyze_packs(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_pack,int pack_num,int dv_chn_nr)
{
  while (pack_num>0)
  {
    switch (pc_pack[0])  // pack_id
    {
/*@///       case 0x13: // TITL time code */
case 0x13:
{
  pr_dv_aux->titl.time_code = (unsigned long)(*(long*)(&pc_pack[1]) & 0x3f7f7f3f);
  break;
}
/*@\\\0000000352*/
/*@///       case 0x50: // AAUX src */
case 0x50:
{
  pr_dv_aux->aaux[dv_chn_nr].locked      = (unsigned char)( pc_pack[1]         >> 7);
  pr_dv_aux->aaux[dv_chn_nr].au_fr_size  = (unsigned int )( pc_pack[1] & 0x3f      );
  pr_dv_aux->aaux[dv_chn_nr].stereo      = (unsigned char)( pc_pack[2]         >> 7);
  pr_dv_aux->aaux[dv_chn_nr].channel_num = (unsigned char)((pc_pack[2] & 0x60) >> 5);
  pr_dv_aux->aaux[dv_chn_nr].pair        = (unsigned char)((pc_pack[2] & 0x10) >> 4);
  pr_dv_aux->aaux[dv_chn_nr].au_mode     = (unsigned char)( pc_pack[2] & 0x0f      );
  pr_dv_aux->aaux[dv_chn_nr].lang        = (unsigned char)((pc_pack[3] & 0x40) >> 6);
  pr_dv_aux->aaux[dv_chn_nr].emphasis    = (unsigned char)( pc_pack[4]         >> 7);
  pr_dv_aux->aaux[dv_chn_nr].time_emph   = (unsigned char)((pc_pack[4] & 0x40) >> 6);
  pr_dv_aux->aaux[dv_chn_nr].sample_rate = (unsigned char)((pc_pack[4] & 0x38) >> 3);
  pr_dv_aux->aaux[dv_chn_nr].sample_size = (unsigned char)( pc_pack[4] & 0x07      );

//   pr_dv_aux->frame_rate       = (unsigned char)((pc_pack[3] & 0x20) >> 5);
//   pr_dv_aux->video_system     = (unsigned char)( pc_pack[3] & 0x3f      );

  switch ((pr_dv_aux->frame_rate << 4) | pr_dv_aux->aaux[dv_chn_nr].sample_rate)
  {
    case 0x00:  pr_dv_aux->aaux[dv_chn_nr].au_fr_size += 1580; break;  // 60Hz/48kHz
    case 0x01:  pr_dv_aux->aaux[dv_chn_nr].au_fr_size += 1452; break;  // 60Hz/44.1kHz
    case 0x02:  pr_dv_aux->aaux[dv_chn_nr].au_fr_size += 1053; break;  // 60Hz/32kHz
    case 0x10:  pr_dv_aux->aaux[dv_chn_nr].au_fr_size += 1896; break;  // 50Hz/48kHz
    case 0x11:  pr_dv_aux->aaux[dv_chn_nr].au_fr_size += 1742; break;  // 50Hz/44.1kHz
    case 0x12:  pr_dv_aux->aaux[dv_chn_nr].au_fr_size += 1264; break;  // 50Hz/32kHz
  }
  break;
}
/*@\\\0000001032*/
/*@///       case 0x51: // AAUX src ctl */
case 0x51:
{
  pr_dv_aux->aaux[dv_chn_nr].compr_count = (unsigned char)((pc_pack[1] & 0x0c) >> 2);
  break;
}
/*@\\\000000033C*/
/*@///       case 0x60: // VAUX src */
case 0x60:
{
  pr_dv_aux->vaux.bw         = (unsigned char)( pc_pack[2]         >> 7);
  pr_dv_aux->vaux.col_fl_val = (unsigned char)((pc_pack[2] & 0x40) >> 6);
  pr_dv_aux->vaux.col_fl     = (unsigned char)((pc_pack[2] & 0x30) >> 4);

//   pr_dv_aux->frame_rate      = (unsigned char)((pc_pack[3] & 0x20) >> 5);
//   pr_dv_aux->video_system    = (unsigned char)( pc_pack[3] & 0x3f      );
  break;
}
/*@\\\000000084C*/
/*@///       case 0x61: // VAUX src ctl */
case 0x61:
{
  pr_dv_aux->vaux.display_mode = (unsigned char)( pc_pack[2] & 0x07);
  pr_dv_aux->vaux.field_flags  = (unsigned char)( pc_pack[3]       );
  break;
}
/*@\\\0000000301*/
    }
    pc_pack += DV_PACK_SIZE;
    --pack_num;
  }
}
/*@\\\0000000B15*/
/*@/// void analyze_subc(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_subc,int subc_num,int dv_chn_nr) */
void analyze_subc(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_subc,int subc_num,int dv_chn_nr)
{
  while (subc_num>0)
  {
    // check for subc data here
    analyze_packs(pr_dv_aux,&pc_subc[DV_SYB_PACK_OFS],DV_SYB_PACK_NUM,dv_chn_nr);
    pc_subc += DV_SYB_SIZE;
    --subc_num;
  }
}
/*@\\\000000061B*/
/*@/// void analyze_fr0(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_fr) */
DLLExportC void analyze_fr0(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_fr)
{
  unsigned char* pc_dv_blk = pc_dv_fr;
  unsigned int dv_blk_num = DV_FR_MAX_BLK_NUM;

  for (unsigned int i = 0; i<dv_blk_num; ++i)
  {
    unsigned char dv_blk_type = (unsigned char)(pc_dv_blk[0] >> 5);
//     unsigned char dv_fr_nr    = (unsigned char)(pc_dv_blk[0] & 0x0f);
    unsigned char dv_dif_nr   = (unsigned char)(pc_dv_blk[1] >> 4);
//     unsigned char dv_blk_nr   = (unsigned char)(pc_dv_blk[2]);
    unsigned char dv_chn_nr   = (unsigned char)((dv_dif_nr<((pr_dv_aux->frame_rate==1)?DV_FR50_CHN1:DV_FR60_CHN1))?0:1);
    switch (dv_blk_type)
    {
/*@///       case 0:  // HDR */
case 0:
{
  if ((pc_dv_blk[3] & 0x80) == 0x80)  // bf: 12 DIFs 3f: 10 DIFs
  {
    dv_blk_num = DV_FR50_BLK_NUM;
    pr_dv_aux->video_system = 0x20;  // 625-50
    pr_dv_aux->frame_rate = 1;  // 50
    pr_dv_aux->lp_sp = 1;  // SP
    pr_dv_aux->resolution_x = 720;
    pr_dv_aux->resolution_y = 576;
  }
  else
  {
    dv_blk_num = DV_FR60_BLK_NUM;
    pr_dv_aux->video_system = 0x00;  // 525-60
    pr_dv_aux->frame_rate = 0;  // 60
    pr_dv_aux->lp_sp = 1;  // SP
    pr_dv_aux->resolution_x = 720;
    pr_dv_aux->resolution_y = 480;
  }
  break;
}
/*@\\\0000000E01*/
      case 1: analyze_subc(pr_dv_aux,&pc_dv_blk[DV_SUBC_SYB_OFS],DV_SUBC_SYB_NUM,dv_chn_nr); break;
      case 2: analyze_packs(pr_dv_aux,&pc_dv_blk[DV_VAUX_PACK_OFS],DV_VAUX_PACK_NUM,dv_chn_nr); break;
      case 3: analyze_packs(pr_dv_aux,&pc_dv_blk[DV_AAUX_PACK_OFS],DV_AAUX_PACK_NUM,dv_chn_nr); /* memcpy(pc_dv_adta,&pc_dv_blk[DV_ADTA_OFS],DV_ADTA_SIZE); pc_dv_adta += DV_ADTA_SIZE; */ break;
      case 4: break;
      default:; // *** unknown block type
    }
    pc_dv_blk += DV_BLK_SIZE;
  }
}
/*@\\\0000001801*/
/*@/// void analyze_fr(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_adta,unsigned char* pc_dv_fr) */
void analyze_fr(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_adta,unsigned char* pc_dv_fr)
{
  unsigned char* pc_dv_blk = pc_dv_fr;
  unsigned int dv_blk_num = DV_FR_MAX_BLK_NUM;

  for (unsigned int i = 0; i<dv_blk_num; ++i)
  {
    unsigned char dv_blk_type = (unsigned char)(pc_dv_blk[0] >> 5);
//     unsigned char dv_fr_nr    = (unsigned char)(pc_dv_blk[0] & 0x0f);
    unsigned char dv_dif_nr   = (unsigned char)(pc_dv_blk[1] >> 4);
//     unsigned char dv_blk_nr   = (unsigned char)(pc_dv_blk[2]);
    unsigned char dv_chn_nr   = (unsigned char)((dv_dif_nr<((pr_dv_aux->frame_rate==1)?DV_FR50_CHN1:DV_FR60_CHN1))?0:1);
    switch (dv_blk_type)
    {
/*@///       case 0:  // HDR */
case 0:
{
  if ((pc_dv_blk[3] & 0x80) == 0x80)  // bf: 12 DIFs 3f: 10 DIFs
  {
    dv_blk_num = DV_FR50_BLK_NUM;
    pr_dv_aux->video_system = 0x20;  // 625-50
    pr_dv_aux->frame_rate = 1;  // 50
    pr_dv_aux->lp_sp = 1;  // SP
    pr_dv_aux->resolution_x = 720;
    pr_dv_aux->resolution_y = 576;
  }
  else
  {
    dv_blk_num = DV_FR60_BLK_NUM;
    pr_dv_aux->video_system = 0x00;  // 525-60
    pr_dv_aux->frame_rate = 0;  // 60
    pr_dv_aux->lp_sp = 1;  // SP
    pr_dv_aux->resolution_x = 720;
    pr_dv_aux->resolution_y = 480;
  }
  break;
}
/*@\\\*/
      case 1: analyze_subc(pr_dv_aux,&pc_dv_blk[DV_SUBC_SYB_OFS],DV_SUBC_SYB_NUM,dv_chn_nr); break;
      case 2: analyze_packs(pr_dv_aux,&pc_dv_blk[DV_VAUX_PACK_OFS],DV_VAUX_PACK_NUM,dv_chn_nr); break;
      case 3: analyze_packs(pr_dv_aux,&pc_dv_blk[DV_AAUX_PACK_OFS],DV_AAUX_PACK_NUM,dv_chn_nr); memcpy(pc_dv_adta,&pc_dv_blk[DV_ADTA_OFS],DV_ADTA_SIZE); pc_dv_adta += DV_ADTA_SIZE; break;
      case 4: break;
      default:; // *** unknown block type
    }
    pc_dv_blk += DV_BLK_SIZE;
  }
}
/*@\\\0000000F01*/

// 20lin: even: +=2 odd: +=3
/*@/// inline unsigned short decode_20lin(unsigned char* pc_pcm,bool is_even) */
inline unsigned short decode_20lin(unsigned char* pc_pcm,bool is_even)
{
  unsigned short result;
  if (is_even)  // even
  {
    result = (unsigned short)((pc_pcm[0]<<8)+(pc_pcm[1]));  // pc_pcm[2] msn is dropped  // !!! could use bswap instead
  }
  else  // odd
  {
    result = (unsigned short)((pc_pcm[1]<<8)+(pc_pcm[2]));  // pc_pcm[0] lsn is dropped  // !!! correct order of bytes?  // !!! could use bswap instead
  }
  return result;
}
/*@\\\000000063A*/
// 16lin: +=2
/*@/// inline unsigned short decode_16lin(unsigned char* pc_pcm) */
inline unsigned short decode_16lin(unsigned char* pc_pcm)
{
  unsigned short result;
  result = (unsigned short)((pc_pcm[0]<<8)+(pc_pcm[1]));  // !!! could use bswap instead
  if (result==0x8000) ++result;  // 0x8000 -> 0x8001
  return result;
}
/*@\\\000000040C*/
// 12nonlin: even: +=1 odd: +=2
/*@/// inline unsigned short decode_12nonlin(unsigned char* pc_pcm,bool is_even) */
inline unsigned short decode_12nonlin(unsigned char* pc_pcm,bool is_even)
{
  unsigned short result;
//   if (is_even)  // even
//   {
//     result = (unsigned short)(((pc_pcm[0]<<4) & 0x0ff0)|((pc_pcm[1]>>4) & 0x000f));
//   }
//   else  // odd
//   {
//     result = (unsigned short)(((pc_pcm[1]<<4) & 0x0ff0)|((pc_pcm[0]   ) & 0x000f));
//   }
  if (is_even)
  {
    result = (pc_pcm[0]<<4u) | (pc_pcm[2]>>4u);
  }
  else
  {
    result = (pc_pcm[0]<<4u) | (pc_pcm[1] & 0xfu);
  }
  // re-linearization tables             0x000   0x100   0x200   0x300   0x400   0x500   0x600   0x700    0x800   0x900   0xa00   0xb00   0xc00   0xd00   0xe00   0xf00
  static unsigned short a_12_add[] = { 0x0000u,0x0000u,0xff00u,0xfe00u,0xfd00u,0xfc00u,0xfb00u,0xfa00u, 0x0601u,0x0501u,0x0401u,0x1301u,0x3201u,0x7101u,0xf000u,0xf000u };
  static unsigned char  a_12_shl[] = {      0u,     0u,     1u,     2u,     3u,     4u,     5u,     6u,      6u,     5u,     4u,     3u,     2u,     1u,     0u,     0u };
  static unsigned char  a_12_sub[] = {      0u,     0u,     0u,     0u,     0u,     0u,     0u,     0u,      1u,     1u,     1u,     1u,     1u,     1u,     0u,     0u };
  unsigned char i = (unsigned char)((result >> 8u)&0x000fu); // first nibble: select
  if (result==0x800u) ++result;  // 0x800 -> 0x801
  result = (unsigned short)((((result&0x0fffu)+a_12_add[i])<<a_12_shl[i])-a_12_sub[i]);
  return result;
}
/*@\\\0000001C1E*/
/*@/// void mix_audio(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_audio_dest,unsigned char* pc_pcm_chn_1,unsigned char* pc_pcm_chn_2) */
void mix_audio(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_audio_dest,unsigned char* pc_pcm_chn_1,unsigned char* pc_pcm_chn_2,char mode)
{
  unsigned int
    i = 0,
    j = 0,
    d_num_0 = pr_dv_aux->aaux[0].au_fr_size,
    d_num_1 = pr_dv_aux->aaux[1].au_fr_size,
    chn_ss_0 = (pr_dv_aux->aaux[0].channel_num<<4) | (pr_dv_aux->aaux[0].sample_size),
    chn_ss_1 = (pr_dv_aux->aaux[1].channel_num<<4) | (pr_dv_aux->aaux[1].sample_size),
    sm_chn_am_0 = (pr_dv_aux->aaux[0].stereo<<8) | (pr_dv_aux->aaux[0].channel_num<<4) | (pr_dv_aux->aaux[0].au_mode),
    sm_chn_am_1 = (pr_dv_aux->aaux[1].stereo<<8) | (pr_dv_aux->aaux[1].channel_num<<4) | (pr_dv_aux->aaux[1].au_mode);
  unsigned short ca,cb,cc,cd,cl,cr;  // ca,cb=CHN1, cc,cd=CHN2, cl,cr=PCM
  bool
    is_even_1 = true,
    is_even_2 = true;

  do
  {
    ca = cb = cc = cd = cl = cr = 0;
    // 1st step: decode into channel short
    if (pr_dv_aux->aaux[0].au_mode!=0xf)
      switch (chn_ss_0)
      {
        case 0x00: // 16lin 1chn
          ca = decode_16lin   (pc_pcm_chn_1);                               pc_pcm_chn_1 += 2;
          break;
        case 0x01: // 12nonlin 1chn
          ca = decode_12nonlin(pc_pcm_chn_1,is_even_1); if (is_even_1) { pc_pcm_chn_1 += 1; } else { pc_pcm_chn_1 += 2; }
          break;
        case 0x02: // 20lin 1chn
          ca = decode_20lin   (pc_pcm_chn_1,is_even_1); if (is_even_1) { pc_pcm_chn_1 += 2; } else { pc_pcm_chn_1 += 3; }
          break;
        case 0x11: // 12nonlin 2chn
          ca = decode_12nonlin(pc_pcm_chn_1,is_even_1); if (is_even_1) { pc_pcm_chn_1 += 1; } else { pc_pcm_chn_1 += 2; } is_even_1 = !is_even_1;
          cb = decode_12nonlin(pc_pcm_chn_1,is_even_1); if (is_even_1) { pc_pcm_chn_1 += 1; } else { pc_pcm_chn_1 += 2; }
          break;
      }
    if (pr_dv_aux->aaux[1].au_mode!=0xf)
      switch (chn_ss_1)
      {
        case 0x00: // 16lin 1chn
          cc = decode_16lin   (pc_pcm_chn_2);                           pc_pcm_chn_2 += 2;
          break;
        case 0x01: // 12nonlin 1chn
          cc = decode_12nonlin(pc_pcm_chn_2,is_even_2); if (is_even_2) { pc_pcm_chn_2 += 1; } else { pc_pcm_chn_2 += 2; }
          break;
        case 0x02: // 20lin 1chn
          cc = decode_20lin   (pc_pcm_chn_2,is_even_2); if (is_even_2) { pc_pcm_chn_2 += 2; } else { pc_pcm_chn_2 += 3; }
          break;
        case 0x11: // 12nonlin 2chn
          cc = decode_12nonlin(pc_pcm_chn_2,is_even_2); if (is_even_2) { pc_pcm_chn_2 += 1; } else { pc_pcm_chn_2 += 2; } is_even_2 = !is_even_2;
          cd = decode_12nonlin(pc_pcm_chn_2,is_even_2); if (is_even_2) { pc_pcm_chn_2 += 1; } else { pc_pcm_chn_2 += 2; }
          break;
      }
    // 2nd step: decide if mono (ca,cb,cc,cd) or stereo (ca & cb,ca & cc), coming from which channels
    // for definiton of L R M ... see au_mode tables inside definition of tr_dv_aux
    switch (sm_chn_am_1)
    {
      case 0x000: cl = cc;          break; // 2ch L
      case 0x001:          cr = cc; break; // 2ch R
      case 0x002: cl = cc; cr = cc; break; // 2ch M
      case 0x010: cl = cc; cr = cd; break; // 4ch L R
      case 0x011: cl = cc; cr = cc; break; // 4ch M1 -
      case 0x012: cl = cc; cr = cd; break; // 4ch M1 M2
      case 0x013:                   break; // 4ch LS RS (L+R Surround, cc=LS cd=RS)
      case 0x014:                   break; // 4ch C S (C+S Surround, cc=C center (front) cd=S surround (rear))
      case 0x015:                   break; // 4ch C - (cc=C center cd=void)
      case 0x016:                   break; // 4ch C M1 (cc=C center cd=M1 extra channel mono)
      case 0x100:
      case 0x110:                   break; // 4ch (L R) C S (cc=C center (front) cd=S surround (rear))
      case 0x101:
      case 0x111:                   break; // 4ch (L R) C M (cc=C center cd=M extra channel mono)
      case 0x102:
      case 0x112:                   break; // 4ch (L R) C - (cc=C center cd=void)
      case 0x103:
      case 0x113:                   break; // 4ch (L R) LS RS (cc=LS left surround cd=RS right surround)
      case 0x104:
      case 0x114:                   break; // 2+4 (Lm Rm) T Wo Q1 Q2 -- don't know how to handle this, there's no docs on 8ch
      case 0x105:
      case 0x115:                   break; // 4+4 (L R C Wo) LS RS Lm Rm -- don't know how to handle this, there's no docs on 8ch
      case 0x106:
      case 0x116:                   break; // 4+4 (L R C Wo) LS RS LS RS -- don't know how to handle this, there's no docs on 8ch
      case 0x107:
      case 0x117:                   break; // 4+4 (L R C Wo) LS RS LC RC -- don't know how to handle this, there's no docs on 8ch
    }
    switch (sm_chn_am_0)
    {
      case 0x000: cl = ca;          break; // 2ch L
      case 0x001:          cr = ca; break; // 2ch R
      case 0x002: cl = ca; cr = ca; break; // 2ch M
      case 0x010: cl = ca; cr = cb; break; // 4ch L R
      case 0x011: cl = ca; cr = ca; break; // 4ch M1 -
      case 0x012: cl = ca; cr = cb; break; // 4ch M1 M2
      case 0x013:                   break; // 4ch LS RS (L+R Surround, cc=LS cd=RS)
      case 0x014:                   break; // 4ch C S (C+S Surround, cc=C center (front) cd=S surround (rear))
      case 0x015:                   break; // 4ch C - (cc=C center cd=void)
      case 0x016: cl = ca; cr = ca; break; // 4ch C M1
      case 0x100:
      case 0x110: cl = ca; cr = cb; break; // 4ch L R (C S) (ca=L cb=R)
      case 0x101:
      case 0x111: cl = ca; cr = cb; break; // 4ch L R (C M) (ca=L cb=R)
      case 0x102:
      case 0x112: cl = ca; cr = cb; break; // 4ch L R (C -) (ca=L cb=R)
      case 0x103:
      case 0x113: cl = ca; cr = cb; break; // 4ch L R (LS RS) (ca=L cb=R)
      case 0x104:
      case 0x114: cl = ca; cr = cb; break; // 2+4 Lm Rm (T Wo Q1 Q2) (ca=L+.7C+.7LS cb=R+.7C+.7RS)
      case 0x105:
      case 0x115:                   break; // 4+4 L R C Wo (LS RS Lm Rm) -- don't know how to handle this, there's no docs on 8ch
      case 0x106:
      case 0x116:                   break; // 4+4 L R C Wo (LS RS LS RS) -- don't know how to handle this, there's no docs on 8ch
      case 0x107:
      case 0x117:                   break; // 4+4 L R C Wo (LS RS LC RC) -- don't know how to handle this, there's no docs on 8ch
    }
    // 3rd step: write back the two channels: always return 48k/44.1k/32k/2ch
	 if (mode==1) {cl=ca;cr=cb;}
	 if (mode==2) {cl=cc;cr=cd;}
    ((unsigned short*)pc_audio_dest)[0] = cl;
    ((unsigned short*)pc_audio_dest)[1] = cr;
    pc_audio_dest += 4;
    ++i; ++j;  // i & j should be syncronous to go to d_num_*
    is_even_1 = !is_even_1;
    is_even_2 = !is_even_2;
  }
  while ((i<d_num_0) && (j<d_num_1));
}
/*@\\\0000004E01*/

/*@/// void DecompressBuffer_DVAudio() */
DLLExportC int decodeAudio(BYTE* pc_dv_fr /*,ULONG __dv_fr_size or __dv_aud_size*/ ,BYTE* pc_audio_dest,char mode)
/*,ULONG __sample_rate,ULONG __sample_size,ULONG __sample_channels,ULONG __unk1,ULONG __format,ULONG __unk2,ULONG __unk3)*/
{
  unsigned char ac_dv_adta[DV_AC_ADTA_SIZE];
  unsigned char ac_pcm_adta[DV_AC_ADTA_SIZE];
  struct tr_dv_aux r_dv_aux;
  analyze_fr(&r_dv_aux,&ac_dv_adta[0],pc_dv_fr);  // phase 1: collect data from AAUX/ADTA blocks into local buffer
  // unshuffle_audio();  // phase 2: "un-shuffle" data to natural alignment
  switch ((r_dv_aux.frame_rate<<4) | (r_dv_aux.aaux[0].channel_num))
  {
    case 0x00: unshuffle_adta_60_1ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
    case 0x01: unshuffle_adta_60_2ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
    case 0x10: unshuffle_adta_50_1ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
    case 0x11: unshuffle_adta_50_2ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
  }
  // unshuffle_audio();  // unshuffle second chn
  switch ((r_dv_aux.frame_rate<<4) | (r_dv_aux.aaux[1].channel_num))
  {
    case 0x00: unshuffle_adta_60_1ch(&ac_pcm_adta[DV_AC_ADTA_60_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_60_CHN_SIZE]); break;
    case 0x01: unshuffle_adta_60_2ch(&ac_pcm_adta[DV_AC_ADTA_60_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_60_CHN_SIZE]); break;
    case 0x10: unshuffle_adta_50_1ch(&ac_pcm_adta[DV_AC_ADTA_50_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_50_CHN_SIZE]); break;
    case 0x11: unshuffle_adta_50_2ch(&ac_pcm_adta[DV_AC_ADTA_50_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_50_CHN_SIZE]); break;
  }
  // mix_audio();  // phase 3: if necessary, decode 20b-lin/12b-nonlin into 16b-lin, mix l&r channels into tgt
  if (r_dv_aux.frame_rate==1)
    mix_audio(&r_dv_aux,pc_audio_dest,&ac_pcm_adta[0],&ac_pcm_adta[DV_AC_ADTA_50_CHN_SIZE],mode);
  else
    mix_audio(&r_dv_aux,pc_audio_dest,&ac_pcm_adta[0],&ac_pcm_adta[DV_AC_ADTA_60_CHN_SIZE],mode);
  return (r_dv_aux.aaux[0].au_fr_size);
}
/*@\\\0000001C01*/
/*@\\\4000003C01003001*/
/*@\\\0031000101000120000011*/

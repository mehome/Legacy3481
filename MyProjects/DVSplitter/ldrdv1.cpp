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
/*@\\\0000000901*/
/*@/// protos */
// ULONG readBlock( void *buf, ULONG bufsize);
// ULONG SwpLng( ULONG Value );

#ifdef USE_DV_CODEC
extern "C" ULONG __cdecl DecompressBuffer_DV (
                         PBYTE pbSrc, ULONG SrcBufLen,
                         PBYTE pbDst, int wRealLineSize, ULONG DstWidth, ULONG DstHeight,
                         ULONG ulFlags, ULONG ColorSpaceFourcc, ULONG optFlags, PBYTE extInfo);
#endif
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

/*@/// local */
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
#define DV_FR50_CHN1 6U
#define DV_FR60_BLK_NUM 1500
#define DV_FR60_SIZE 120000
#define DV_FR60_CHN1 5U
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

Audio Blk:   CH1  CH2
Trk Pos: 60  0..4 5..9
         50  0..5 6..b
Enc Mode:2Ch 48k  48k
             44k1 44k1
             32k  32k
         4Ch 32k-232k-2

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
/*@\\\0000000801*/
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
/*@\\\0000001502*/

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
/*@\\\*/

/*@/// void unshuffle_adta*() */
/*@/// void unshuffle_adta_60_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_60_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i=0; i<45; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_60_ofs[i]];
    for (int j=0; j<36; ++j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
      pc_src +=  2;
      pc_tgt += 90; // 45*2bytes
    }
  }
}
/*@\\\000000070D*/
/*@/// void unshuffle_adta_50_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_50_1ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i=0; i<54; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_50_ofs[i]];
    for (int j=0; j<36; ++j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
      pc_src +=   2;
      pc_tgt += 108;  // 54*2bytes
    }
  }
}
/*@\\\000000062E*/
/*@/// void unshuffle_adta_60_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_60_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i=0; i<45; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_60_ofs[i]];
    for (int j=0; j<24; ++j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
      pc_tgt[2] = pc_src[2];
      pc_src +=   3;
      pc_tgt += 135; // 45*3bytes
    }
  }
}
/*@\\\*/
/*@/// void unshuffle_adta_50_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta) */
void unshuffle_adta_50_2ch(unsigned char* pc_pcm_audio,unsigned char* pc_dv_adta)
{
  unsigned char *pc_src = pc_dv_adta,*pc_tgt;
  for (int i=0; i<54; ++i)
  {
    pc_tgt = &pc_pcm_audio[ac_dv_adta_50_ofs[i]];
    for (int j=0; j<24; ++j)
    {
      *(unsigned short*)pc_tgt = *(unsigned short*)pc_src;
      pc_tgt[2] = pc_src[2];
      pc_src +=   3;
      pc_tgt += 162; // 54*3bytes
    }
  }
}
/*@\\\0000000801*/
/*@\\\0000000201*/
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
  pr_dv_aux->titl.time_code = *(long*)(&pc_pack[1]) & 0x3f7f7f3f;
  break;
}
/*@\\\000000030E*/
/*@///       case 0x50: // AAUX src */
case 0x50:
{
  pr_dv_aux->aaux[dv_chn_nr].locked      =  pc_pack[1]         >> 7;
  pr_dv_aux->aaux[dv_chn_nr].au_fr_size  =  pc_pack[1] & 0x3f      ;
  pr_dv_aux->aaux[dv_chn_nr].stereo      =  pc_pack[2]         >> 7;
  pr_dv_aux->aaux[dv_chn_nr].channel_num = (pc_pack[2] & 0x60) >> 5;
  pr_dv_aux->aaux[dv_chn_nr].pair        = (pc_pack[2] & 0x10) >> 4;
  pr_dv_aux->aaux[dv_chn_nr].au_mode     =  pc_pack[2] & 0x0f      ;
  pr_dv_aux->aaux[dv_chn_nr].lang        = (pc_pack[3] & 0x40) >> 6;
  pr_dv_aux->aaux[dv_chn_nr].emphasis    =  pc_pack[4]         >> 7;
  pr_dv_aux->aaux[dv_chn_nr].time_emph   = (pc_pack[4] & 0x40) >> 6;
  pr_dv_aux->aaux[dv_chn_nr].sample_rate = (pc_pack[4] & 0x38) >> 3;
  pr_dv_aux->aaux[dv_chn_nr].sample_size =  pc_pack[4] & 0x07      ;

  pr_dv_aux->frame_rate       = (pc_pack[3] & 0x20) >> 5;
  pr_dv_aux->video_system     =  pc_pack[3] & 0x3f      ;

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
/*@\\\0000001252*/
/*@///       case 0x51: // AAUX src ctl */
case 0x51:
{
  pr_dv_aux->aaux[dv_chn_nr].compr_count = (pc_pack[1] & 0x0c) >> 2;
  break;
}
/*@\\\000000030E*/
/*@///       case 0x60: // VAUX src */
case 0x60:
{
  pr_dv_aux->vaux.bw         =  pc_pack[2]         >> 7;
  pr_dv_aux->vaux.col_fl_val = (pc_pack[2] & 0x40) >> 6;
  pr_dv_aux->vaux.col_fl     = (pc_pack[2] & 0x30) >> 4;

  pr_dv_aux->frame_rate      = (pc_pack[3] & 0x20) >> 5;
  pr_dv_aux->video_system    =  pc_pack[3] & 0x3f      ;
  break;
}
/*@\\\0000000338*/
/*@///       case 0x61: // VAUX src ctl */
case 0x61:
{
  pr_dv_aux->vaux.display_mode = pc_pack[2] & 0x07;
  pr_dv_aux->vaux.field_flags  = pc_pack[3]       ;
  break;
}
/*@\\\0000000313*/
    }
    pc_pack += DV_PACK_SIZE;
    --pack_num;
  }
}
/*@\\\0000000801*/
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
/*@\\\000000071B*/
/*@/// void analyze_fr0(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_fr) */
void analyze_fr0(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_fr)
{
  unsigned char* pc_dv_blk = pc_dv_fr;
  unsigned int dv_blk_num = DV_FR_MAX_BLK_NUM;

  for (unsigned int i = 0; i<dv_blk_num; ++i)
  {
    unsigned int dv_blk_type = pc_dv_blk[0] >> 5;
//     unsigned int dv_fr_nr    = pc_dv_blk[0] & 0x0f;
    unsigned int dv_dif_nr   = pc_dv_blk[1] >> 4;
//     unsigned int dv_blk_nr   = pc_dv_blk[2];
    unsigned int dv_chn_nr   = (dv_dif_nr<((pr_dv_aux->frame_rate==1)?DV_FR50_CHN1:DV_FR60_CHN1))?0:1;
		 //Warning: signed/unsigned mismatch
    switch (dv_blk_type)
    {
/*@///       case 0:  // HDR */
case 0:
{
  if ((pc_dv_blk[3] & 0x80) == 0x80)  // bf: 12 DIFs 3f: 10 DIFs
  {
    pr_dv_aux->video_system = 0x20;  // 625-50
    pr_dv_aux->frame_rate = 1;  // 50
    dv_blk_num = DV_FR50_BLK_NUM;
    pr_dv_aux->resolution_x = 720;
    pr_dv_aux->resolution_y = 576;
  }
  else
  {
    pr_dv_aux->video_system = 0x00;  // 525-60
    pr_dv_aux->frame_rate = 0;  // 60
    dv_blk_num = DV_FR60_BLK_NUM;
    pr_dv_aux->resolution_x = 720;
    pr_dv_aux->resolution_y = 480;
  }
  break;
}
/*@\\\0000000922*/
      case 1: analyze_subc(pr_dv_aux,&pc_dv_blk[DV_SUBC_SYB_OFS],DV_SUBC_SYB_NUM,dv_chn_nr); break;
      case 2: analyze_packs(pr_dv_aux,&pc_dv_blk[DV_VAUX_PACK_OFS],DV_VAUX_PACK_NUM,dv_chn_nr); break;
      case 3: analyze_packs(pr_dv_aux,&pc_dv_blk[DV_AAUX_PACK_OFS],DV_AAUX_PACK_NUM,dv_chn_nr); /* memcpy(pc_dv_adta,&pc_dv_blk[DV_ADTA_OFS],DV_ADTA_SIZE); pc_dv_adta += DV_ADTA_SIZE; */ break;
      case 4: break;
      default:; // *** unknown block type
    }
    //pc_dv_fr += DV_BLK_SIZE;
    pc_dv_blk += DV_BLK_SIZE;
  }
}
/*@\\\0000000E01*/
/*@/// void analyze_fr(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_adta,unsigned char* pc_dv_fr) */
void analyze_fr(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_adta,unsigned char* pc_dv_fr)
{
  unsigned char* pc_dv_blk = pc_dv_fr;
  unsigned int dv_blk_num = DV_FR_MAX_BLK_NUM;

  for (unsigned int i = 0; i<dv_blk_num; ++i)
  {
    unsigned int dv_blk_type = pc_dv_blk[0] >> 5;
//     unsigned int dv_fr_nr    = pc_dv_blk[0] & 0x0f;
    unsigned int dv_dif_nr   = pc_dv_blk[1] >> 4;
//     unsigned int dv_blk_nr   = pc_dv_blk[2];
    unsigned int dv_chn_nr   = (dv_dif_nr<((pr_dv_aux->frame_rate==1)?DV_FR50_CHN1:DV_FR60_CHN1))?0:1;
		 //Warning: signed/unsigned mismatch
    switch (dv_blk_type)
    {
/*@///       case 0:  // HDR */
case 0:
{
  if ((pc_dv_blk[3] & 0x80) == 0x80)  // bf: 12 DIFs 3f: 10 DIFs
  {
    pr_dv_aux->video_system = 0x20;  // 625-50
    pr_dv_aux->frame_rate = 1;  // 50
    dv_blk_num = DV_FR50_BLK_NUM;
    pr_dv_aux->resolution_x = 720;
    pr_dv_aux->resolution_y = 576;
  }
  else
  {
    pr_dv_aux->video_system = 0x00;  // 525-60
    pr_dv_aux->frame_rate = 0;  // 60
    dv_blk_num = DV_FR60_BLK_NUM;
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
    //pc_dv_fr +=  DV_BLK_SIZE;
    pc_dv_blk += DV_BLK_SIZE;
  }
}
/*@\\\0000001301*/

// 20lin: even: +=2 odd: +=3
/*@/// short decode_20lin(unsigned char* pc_pcm,bool is_even)  // !!! correct order of bytes? */
short decode_20lin(unsigned char* pc_pcm,bool is_even)
{
  short result;
  if (is_even)  // even
  {
    result = (pc_pcm[0]<<8)+(pc_pcm[1]);  // pc_pcm[2] msn is dropped
  }
  else  // odd
  {
    result = (pc_pcm[1]<<8)+(pc_pcm[2]);  // pc_pcm[0] lsn is dropped
  }
  return result;
}
/*@\\\0000000502*/
// 12lin: even: +=1 odd: +=2
/*@/// short decode_12nonlin(unsigned char* pc_pcm,bool is_even) */
short decode_12nonlin(unsigned char* pc_pcm,bool is_even)
{
	//short sel; <---unreferenced
  short result;
  if (is_even)  // even
  {
    result = (pc_pcm[0]<<4)+(pc_pcm[1]>>4);
  }
  else  // odd
  {
    result = (pc_pcm[1]<<4)+(pc_pcm[0]>>4);
  }
  // re-linearization tables
  static short a_12_add[] = { (short)0x0000,(short)0x0000,(short)0xff00,(short)0xfe00,(short)0xfd00,(short)0xfc00,(short)0xfb00,(short)0xfa00, (short)0x0601,(short)0x0501,(short)0x0401,(short)0x1301,(short)0x3201,(short)0x7101,(short)0xf000,(short)0xf000 };
  static short a_12_shl[] = {      0,     0,     1,     2,     3,     4,     5,     6,      6,     5,     4,     3,     2,     1,     0,     0 };
  static short a_12_sub[] = {      0,     0,     0,     0,     0,     0,     0,     0,      1,     1,     1,     1,     1,     1,     0,     0 };
  short i = result >> 8; // first nibble: select
  result = (((result+a_12_add[i])<<a_12_shl[i])-a_12_sub[i]);
  return result;
}
/*@\\\*/
/*@/// void mix_audio(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_audio_dest,unsigned char* pc_pcm_chn_1,unsigned char* pc_pcm_chn_2) */
void mix_audio(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_audio_dest,unsigned char* pc_pcm_chn_1,unsigned char* pc_pcm_chn_2)
{
  int
    d_num_0 = pr_dv_aux->aaux[0].au_fr_size,
    d_num_1 = pr_dv_aux->aaux[1].au_fr_size;
  int i = 0,j = 0;
  bool is_even = true;

  ULONG
    chn_ss_0 = (pr_dv_aux->aaux[0].channel_num<<4) | (pr_dv_aux->aaux[0].sample_size),
    chn_ss_1 = (pr_dv_aux->aaux[1].channel_num<<4) | (pr_dv_aux->aaux[1].sample_size),
    sm_chn_am_0 = (pr_dv_aux->aaux[0].stereo<<8) | (pr_dv_aux->aaux[0].channel_num<<4) | (pr_dv_aux->aaux[0].au_mode),
    sm_chn_am_1 = (pr_dv_aux->aaux[1].stereo<<8) | (pr_dv_aux->aaux[1].channel_num<<4) | (pr_dv_aux->aaux[1].au_mode);

  short ca,cb,cc,cd,cl,cr;  // ca=c1, cc=c2

next_sample:
  // 1st step: decode into channel short
  switch (chn_ss_0)
  {
    case 0x00: // 16lin 1chn
      ca = *(short*)pc_pcm_chn_1;                 pc_pcm_chn_1 += 2;
      cb = 0;
      break;
    case 0x01: // 12nonlin 1chn
      ca = decode_12nonlin(pc_pcm_chn_1,is_even); if (is_even) pc_pcm_chn_1 += 1; else pc_pcm_chn_1 += 2;
      cb = 0;
      break;
    case 0x02: // 20lin 1chn
      ca = decode_20lin(pc_pcm_chn_1,is_even);    if (is_even) pc_pcm_chn_1 += 2; else pc_pcm_chn_1 += 3;
      cb = 0;
      break;
    case 0x11: // 12nonlin 2chn
      ca = decode_12nonlin(pc_pcm_chn_1,is_even); if (is_even) pc_pcm_chn_1 += 1; else pc_pcm_chn_1 += 2; is_even = !is_even; ++i;
      cb = decode_12nonlin(pc_pcm_chn_1,is_even); if (is_even) pc_pcm_chn_1 += 1; else pc_pcm_chn_1 += 2;
      break;
  }
  switch (chn_ss_1)
  {
    case 0x00: // 16lin 1chn
      cc = *(short*)pc_pcm_chn_2;                 pc_pcm_chn_2 += 2;
      cd = 0;
      break;
    case 0x01: // 12nonlin 1chn
      cc = decode_12nonlin(pc_pcm_chn_2,is_even); if (is_even) pc_pcm_chn_2 += 1; else pc_pcm_chn_2 += 2;
      cd = 0;
      break;
    case 0x02: // 20lin 1chn
      cc = decode_20lin(pc_pcm_chn_2,is_even);    if (is_even) pc_pcm_chn_2 += 2; else pc_pcm_chn_2 += 3;
      cd = 0;
      break;
    case 0x11: // 12nonlin 2chn
      cc = decode_12nonlin(pc_pcm_chn_2,is_even); if (is_even) pc_pcm_chn_2 += 1; else pc_pcm_chn_2 += 2; is_even = !is_even; ++j;
      cd = decode_12nonlin(pc_pcm_chn_2,is_even); if (is_even) pc_pcm_chn_2 += 1; else pc_pcm_chn_2 += 2;
      break;
  }
  // 2nd step: decide if mono (ca,cb,cc,cd) or stereo (ca & cb,ca & cc), coming from which channels
  switch (sm_chn_am_1)
  {
    case 0x000: cl = cc;          break; // 2ch L
    case 0x001:          cr = cc; break; // 2ch R
    case 0x002: cl = cc; cr = cc; break; // 2ch M
    case 0x010: cl = cc; cr = cd; break; // 4ch LR
    case 0x011: cl = cc; cr = cc; break; // 4ch M1
    case 0x012: cl = cc; cr = cd; break; // 4ch M1 M2
  }
  switch ((pr_dv_aux->aaux[0].stereo<<8) | (pr_dv_aux->aaux[0].channel_num<<4) | (pr_dv_aux->aaux[0].au_mode))
  {
    case 0x000: cl = ca;          break; // 2ch L
    case 0x001:          cr = ca; break; // 2ch R
    case 0x002: cl = ca; cr = ca; break; // 2ch M
    case 0x010: cl = ca; cr = cb; break; // 4ch LR
    case 0x011: cl = ca; cr = ca; break; // 4ch M1
    case 0x012: cl = ca; cr = cb; break; // 4ch M1 M2
    case 0x016: cl = ca; cr = ca; break; // 4ch C M1
    case 0x100:
    case 0x110: cl = ca; cr = cb; break; // 4ch L R C S
    case 0x101:
    case 0x111: cl = ca; cr = cb; break; // 4ch L R C M
    case 0x102:
    case 0x112: cl = ca; cr = cb; break; // 4ch L R C -
    case 0x103:
    case 0x113: cl = ca; cr = cb; break; // 4ch L R LS RS
    case 0x104:
    case 0x114: cl = ca; cr = cb; break; // 4ch Lm Rm T Wo Q1 Q2
  }
  // 3rd step: write back the two channels: always return 48k/44.1k/32k/2ch
  *(short*)pc_audio_dest = cl;
  pc_audio_dest += 2;
  *(short*)pc_audio_dest = cr;
  pc_audio_dest += 2;
  ++i; ++j;  // i & j should be syncronous to go to d_num_*
  is_even = !is_even;
  if ((i<d_num_0) && (j<d_num_1))
    goto next_sample;
}
/*@\\\*/
/*@\\\0000003701*/
// pr_project_data->prPrivate[0] ::= new struct tr_dv_aux

/*@/// DLLExportC int isProject(FILE* f) */
DLLExportC int isProject(FILE* f)
{
  ULONG ul_chars_read;
  char ac_buf[8];

  DosSetFilePtr(f,0,FILE_BEGIN,&ul_chars_read);  // !!! 64bit
  DosRead(f,&ac_buf,8*sizeof(char),&ul_chars_read);  // !!! 64bit

  if (ul_chars_read == 8*sizeof(char))
  {
    if ((ac_buf[0]&0xe0) != 0x00) return 0;  // block type #0: HDR
    if ((ac_buf[3]&0x3f) != 0x3f) return 0;  // reserved, this is testable
    if (*(long*)&ac_buf[4] != 0x78787868) return 0;  // HDR data type info HDR[3..7] says it's DV (consumer) video
  }

  DosSetFilePtr(f,0,FILE_BEGIN,&ul_chars_read);  // !!! 64bit
  return 1;                                            /* No  == Return false */
}
/*@\\\0000000D01*/
/*@/// DLLExportC struct ErrorInfo* initProject(FILE* f,struct ProjectData* pr_project_data) */
DLLExportC struct ErrorInfo *initProject(FILE* f,struct ProjectData* pr_project_data)
{
#ifdef __MADEBUG__
maOpen( MAOUTPUT_WRITE, NULL, "ldrdv.dll");
#endif
  memset(&gr_error_info,0,sizeof(struct ErrorInfo));

  ULONG ul_f_pos,ul_chars_read;
  unsigned char ac_dv_fr[DV_FR50_SIZE];  // one frame is either 144000(50Hz) or 120000(60Hz), composed of 12|10 DIF sequences
  struct tr_dv_aux *pr_dv_aux;

  // initialize struct ProjectData: prCodecs(struct codecInfo codec[])
  strcpy(pr_project_data->prName,"DV");
  pr_project_data->prType = PR_TYPE_ANIMATION;
  pr_project_data->prFlags = 0;  // !!! what PR_FLAGS_* are valid for DV?
//   pr_project_data->prFlags = PR_FLAGS_SINGLEBUFFERED | PR_FLAGS_LINEARENCODED | PR_FLAGS_CACHING | PR_FLAGS_LOCALTIMECODES;
  // get file size
  DosSetFilePtr(f,0,FILE_END,&ul_f_pos);  // !!! 64bit
  DosSetFilePtr(f,0,FILE_CURRENT,&pr_project_data->prSize);  // !!! 64bit, where to put 64bit size?, or prSize = frame_num?
  // r_dv_aux
  pr_dv_aux = new struct tr_dv_aux;
  pr_project_data->prPrivate[0] = (long)pr_dv_aux;
  // analyze frame #0
  DosSetFilePtr(f,0,FILE_BEGIN,&ul_f_pos);  // !!! 64bit
  DosRead(f,ac_dv_fr,sizeof(ac_dv_fr),&ul_chars_read);  // !!! 64bit
  analyze_fr0(pr_dv_aux,ac_dv_fr);
  // set missing values from what has been found in frame #0
  pr_project_data->prWidth = pr_dv_aux->resolution_x;  // 720
  pr_project_data->prHeight = pr_dv_aux->resolution_y;  // 60/50: 480/576
//   pr_project_data->prGlobalTimecode = 100;  // !!!
  pr_project_data->prSoundIsValid = 0;
  if ((pr_dv_aux->aaux[0].stereo==0) && (pr_dv_aux->aaux[0].au_mode<0x03)) pr_project_data->prSoundIsValid = 1;
  if ((pr_dv_aux->aaux[1].stereo==0) && (pr_dv_aux->aaux[1].au_mode<0x07)) pr_project_data->prSoundIsValid = 1;
  if ((pr_dv_aux->aaux[0].stereo==1) && (pr_dv_aux->aaux[0].au_mode<0x08)) pr_project_data->prSoundIsValid = 1;
  if ((pr_dv_aux->aaux[1].stereo==1) && (pr_dv_aux->aaux[1].au_mode<0x08)) pr_project_data->prSoundIsValid = 1;
  pr_project_data->prDepth = 24;
  switch (pr_dv_aux->aaux[0].sample_rate)
  {
    case 0: pr_project_data->prSamplesPerSecond = 48000;
    case 1: pr_project_data->prSamplesPerSecond = 44100;
    case 2: pr_project_data->prSamplesPerSecond = 32000;
  }
  pr_project_data->prSampleChannels = 2;
  pr_project_data->prBitsPerSample = 16;

  // prFrames,prLoopFrames,prGlobalTimecode,prCaching,prCurrentFilePosition,prMaxDataSize,prCodecs,prMaxData,prSampleFormat,prAudioStreamSize,prMemory,prPrivate
  // what codecs?

  // reposition to beginning
  DosSetFilePtr(f,0,FILE_BEGIN,&ul_f_pos);  // !!! 64bit
  return &gr_error_info;
}
/*@\\\0000002E01*/
/*@/// DLLExportC int exNextFrame(FILE* f,struct ProjectData* pr_project_data,struct FrameData* pr_frame_data) */
DLLExportC int exNextFrame(FILE* f,struct ProjectData* pr_project_data,struct FrameData* pr_frame_data)
{
  //int result;  <---unreferenced
  ULONG ul_f_pos,ul_chars_read;
  unsigned char ac_dv_fr[DV_FR50_SIZE];  // one frame is either 144000(50Hz) or 120000(60Hz), composed of 12|10 DIF sequences
  struct tr_dv_aux *pr_dv_aux,*pr_dv_aux0;
  bool is_eof_reached = false;

  pr_dv_aux0 = (struct tr_dv_aux*)pr_project_data->prPrivate[0];
//   pr_frame_data->frPictureName = (char*)pr_project_data; ?
  pr_dv_aux = new struct tr_dv_aux;
  pr_dv_aux->mpr_project_data = pr_project_data;

  pr_frame_data->frPictureName = (char*)pr_dv_aux;
  pr_frame_data->frWidth = pr_project_data->prWidth;
  pr_frame_data->frHeight = pr_project_data->prHeight;
//   pr_frame_data->frTimecode = pr_project_data->prGlobalTimecode;

  DosSetFilePtr(f,0,FILE_CURRENT,&ul_f_pos);  // !!! 64bit
  pr_frame_data->frDataOffset = ul_f_pos;  // the whole frame is the data (video)
  pr_frame_data->frSampleOffset = ul_f_pos;  // the whole frame is the sample (audio)
  if (pr_dv_aux0->frame_rate==1)  // select the correct frame size
  {
    pr_frame_data->frDataSize = DV_FR50_SIZE;
    pr_frame_data->frSampleSize = DV_FR50_SIZE;
  }
  else
  {
    pr_frame_data->frDataSize = DV_FR60_SIZE;
    pr_frame_data->frSampleSize = DV_FR60_SIZE;
  }

  DosRead(f,ac_dv_fr,pr_frame_data->frDataSize,&ul_chars_read);  // !!! 64bit
  if ((ul_chars_read<pr_frame_data->frDataSize) || (feof(f))) is_eof_reached = true;  // !!! 64bit
  analyze_fr0(pr_dv_aux,ac_dv_fr);  // just as the frame #0,
  DosSetFilePtr(f,0,FILE_CURRENT,&pr_project_data->prCurrentFilePosition);  // !!! 64bit

// ul frSampleSize
// ul frColDataSize,frColDataOffset,frSampleOffset
// pb frData,frColData,frSampleData
// ul frTimecodeOffset
// us frDepth,frChangeColours
// ul frCompressMethod,frFlags

  return is_eof_reached?PR_EXNFRAME_ENDOFPROJECT:PR_EXNFRAME_OK;
}
/*@\\\0000002D40*/

static struct ProjectData *gpr_project_data;
static struct DisplayData *gpr_display_data;
static FILE *gf_dv;
static PBYTE gp_data;

/*@/// DLLExportC void initDecodeFrame(FILE* f,struct ProjectData* pr_project_data,struct DisplayData* pr_display_data) */
DLLExportC void initDecodeFrame(FILE* f,struct ProjectData* pr_project_data,struct DisplayData* pr_display_data)
{
  gf_dv = f;
  if ((pr_project_data!=NULL) && (pr_display_data!=NULL))
  {
    gpr_project_data = pr_project_data;
    gpr_display_data = pr_display_data;
//     Initialize_Buffer();  // !!! where?
//     Initialize_Decoder();
//     Initialize_Sequence();
  }
  else
  {
//     Deinitialize_Sequence();
  }
}
/*@\\\*/
/*@/// DLLExportC void decodeFrame(struct FrameData* pr_frame_data) */
DLLExportC void decodeFrame(struct FrameData* pr_frame_data)
{
  ULONG ul_action;

  if (gpr_project_data->prCaching)
  {
    gp_data = pr_frame_data->frData;
  }
  else
  {
    gp_data = gpr_project_data->prMaxData;

    DosSetFilePtr(gf_dv,pr_frame_data->frDataOffset,FILE_BEGIN,&ul_action);
    DosRead(gf_dv,gpr_project_data->prMaxData,pr_frame_data->frDataSize,&ul_action);
  }

  {
#ifdef USE_DV_CODEC
//     case RIFF_DVSD :
/*
    DecompressBuffer_DV((BYTE*)gp_data,pr_frame_data->frDataSize,
      gpr_display_data->dData,gpr_display_data->dModulo,gpr_display_data->dWidth,gpr_display_data->dHeight,
      0,FOURCC_BGR3,0,NULL);
*/
//     break;
#endif
  }
}
/*@\\\0000000107*/
/*@/// DLLExportC struct ModuleInfo* moduleInfo(void)  // !!! linux?!? */
DLLExportC struct ModuleInfo* moduleInfo(void)
{
  static struct ModuleInfo gr_module_info;

  gr_module_info.nextInfo = NULL;
  strcpy(gr_module_info.moduleName, "DV");
  strcpy(gr_module_info.version, __MAVERSION__);
  strcpy(gr_module_info.writtenby, "Michael Schwinnen");
  strcpy(gr_module_info.copyrightby, "MainConcept, GmbH");
  strcpy(gr_module_info.modulePattern, "*.DV");
  strcpy(gr_module_info.comments, "None");

  gr_module_info.modulePrType = PR_TYPE_ANIMATION;
  gr_module_info.moduleSupportsSound = 1;

#ifdef __WINDOWS__
  strcpy(gr_module_info.codec1,"Millions of Colors: Digital Video");
  gr_module_info.codec2[0] = '\0';
#endif
// #ifdef unix
//   strcpy(gr_module_info.codec1,"Millions of Colors: Digital Video");
//   gr_module_info.codec2[0] = '\0';
// #endif
#ifdef __OS2__
  strcpy(gr_module_info.codec1,"Millions of Colors: Digital Video");
  gr_module_info.codec2[0] = '\0';
#endif

  return &gr_module_info;
}
/*@\\\0000001E02*/
/*@/// .DLLExportC int setTimecode(char* nz,struct ProjectData* pr_project_data,struct FrameData* pr_frame_data,int timecode) */
DLLExportC int setTimecode(char* nz,struct ProjectData* pr_project_data,struct FrameData* pr_frame_data,int timecode)
{
  ULONG ul_buf;
  //ULONG ul_action; <----unreferenced
  FILE *f;

  if (pr_project_data->prSoundIsValid) return 1;

#ifdef __OS2__
  if (!(DosOpen(nz,(PHMODULE)&f,&ul_action,0,FILE_NORMAL,FILE_OPEN,OPEN_FLAGS_WRITE_THROUGH|OPEN_ACCESS_WRITEONLY|OPEN_SHARE_DENYNONE|OPEN_FLAGS_SEQUENTIAL,(PEAOP2)NULL)))
#endif
#ifdef WIN32
  f = (FILE*)CreateFile(nz,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (f!=INVALID_HANDLE_VALUE)
#endif
#ifdef unix
  f = open(nz,O_RDWR);
  if (f!=-1)
#endif
  {
    ULONG code = timecode * 1000;

    DosSetFilePtr(f,pr_project_data->prPrivate[1],FILE_BEGIN,&ul_buf);
//     DosWrite(f,&code,4,&ul_buf);  // !!! what file is nz? don't write "code" to dv dump like this!

    DosClose(f);
  }
  else
    return 1;
  return 0;
}
/*@\\\*/
/*@/// DLLExportC int decodeAudio(FILE* f,struct FrameData* pr_frame_data,BYTE* pc_audio_dest) */
//DLLExportC int decodeAudio(FILE* f,struct FrameData* pr_frame_data,BYTE* pc_audio_dest)
DLLExportC int decodeAudio(BYTE *ac_dv_fr,BYTE* pc_audio_dest)
{
  ULONG ul_bytes_read=0;  //ul_bytes_read is handled in avi parser... our return should be error
  //unsigned char ac_dv_fr[DV_FR_MAX_SIZE];
  unsigned char ac_dv_adta[DV_AC_ADTA_SIZE];
  unsigned char ac_pcm_adta[DV_AC_ADTA_SIZE];
  struct tr_dv_aux r_dv_aux;
/*
  DosSetFilePtr(f,pr_frame_data->frSampleOffset,FILE_BEGIN,&ul_bytes_read); // !!! 64bit
  DosRead(f,ac_dv_fr,pr_frame_data->frSampleSize,&ul_bytes_read);  // !!! 64bit // !!! should be taken from the already read video buffer
*/
  analyze_fr(&r_dv_aux,&ac_dv_adta[0],ac_dv_fr);  // phase 1: collect data from AAUX/ADTA blocks into local buffer
  // unshuffle_audio();  // phase 2: "un-shuffle" data to natural alignment
  switch ((r_dv_aux.frame_rate<<4) & (r_dv_aux.aaux[0].channel_num))
  {
    case 0x00: unshuffle_adta_60_1ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
    case 0x01: unshuffle_adta_60_2ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
    case 0x10: unshuffle_adta_50_1ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
    case 0x11: unshuffle_adta_50_2ch(&ac_pcm_adta[0],&ac_dv_adta[0]); break;
  }
  // unshuffle_audio();  // unshuffle second chn
  switch ((r_dv_aux.frame_rate<<4) & (r_dv_aux.aaux[1].channel_num))
  {
    case 0x00: unshuffle_adta_60_1ch(&ac_pcm_adta[DV_AC_ADTA_60_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_60_CHN_SIZE]); break;
    case 0x01: unshuffle_adta_60_2ch(&ac_pcm_adta[DV_AC_ADTA_60_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_60_CHN_SIZE]); break;
    case 0x10: unshuffle_adta_50_1ch(&ac_pcm_adta[DV_AC_ADTA_50_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_50_CHN_SIZE]); break;
    case 0x11: unshuffle_adta_50_2ch(&ac_pcm_adta[DV_AC_ADTA_50_CHN_SIZE],&ac_dv_adta[DV_AC_ADTA_50_CHN_SIZE]); break;
  }
  // mix_audio();  // phase 3: if necessary, decode 20b-lin/12b-nonlin into 16b-lin, mix l&r channels into tgt
  if (r_dv_aux.frame_rate==1)
    mix_audio(&r_dv_aux,pc_audio_dest,&ac_pcm_adta[0],&ac_pcm_adta[DV_AC_ADTA_50_CHN_SIZE]);
  else
    mix_audio(&r_dv_aux,pc_audio_dest,&ac_pcm_adta[0],&ac_pcm_adta[DV_AC_ADTA_60_CHN_SIZE]);

  return ul_bytes_read;
}
/*@\\\0000000901*/

/*@/// ULONG SwpLng(ULONG Value) */
ULONG SwpLng(ULONG Value)
{
  PBYTE bytes=(PBYTE) &Value;

  return( (ULONG) bytes[0] << 24 | (ULONG) bytes[1]<< 16 | (ULONG) bytes[2] << 8 | bytes[3] );
}
/*@\\\0000000602*/

/**
 * $Log: ldrdv.cpp,v $
 *
 */
/*@\\\8002000C04000C04000C04*/

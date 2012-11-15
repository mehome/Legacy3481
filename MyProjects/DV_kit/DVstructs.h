#ifndef DVSTRUCTS_H
#define DVSTRUCTS_H

#define DLLIMPORT __declspec(dllimport)
#define DV_AC_ADTA_SIZE 7776
#define DV_FR_MAX_SIZE 144000
//#define FOURCC_YUYV 0x56595559
//#define FOURCC_UYVY	MakeID('U','Y','U','V')

extern "C" DLLIMPORT int decodeAudio(BYTE *pc_dv_fr,BYTE* pc_audio_dest,char mode);
extern "C" DLLIMPORT void analyze_fr0(struct tr_dv_aux* pr_dv_aux,unsigned char* pc_dv_fr);
/*
extern "C" ULONG __cdecl DecompressBuffer_DV (
                         PBYTE pbSrc, ULONG SrcBufLen,
                         PBYTE pbDst, int wRealLineSize, ULONG DstWidth, ULONG DstHeight,
                         ULONG ulFlags, ULONG ColorSpaceFourcc, ULONG optFlags, PBYTE extInfo);
*/

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
/*@\\\000000131D*/
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

#endif //DVSTRUCTS_H

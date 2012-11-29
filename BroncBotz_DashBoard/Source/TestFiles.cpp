
#ifndef COUNTOF
#define COUNTOF(x) sizeof(x)/sizeof(*x)
#endif

struct FileStuff
{
	const char * const GroupID;
	const size_t NoSelections;
	size_t DefaultSelection;
	const char * const * const inputfile;
};
//Here is an easy way for us to share the same groups... just put your drive here:
#define __Drive__ "E:\\"

#define FileGroupListTemplate(n,x,y) \
const size_t n ## _ ## x ## _NoSelections=COUNTOF(n ## _ ## x ## _Filelist); \
	FileStuff n ## _ ## x ## _Group={ #x ,n ## _ ## x ## _NoSelections,y,n ## _ ## x ## _Filelist };

const char * const James_HTTP_Filelist[] = 
{
	"http://www.termstech.com/files/Short0.avi",
	"http://www.termstech.com/files/BadApple.mp4",
	"http://sirius/testing/cat.mxf",
	"http://sirius/testing/clock.avi",
	"http://sirius/testing/whirly.mpg",
	"http://sirius/testing/HDV1080i60-MotoX.m2t",
	"http://sirius/testing/3Play-Export-1080i.mpg",
	"http://sirius/testing/3Play-Export-1080i-Clock.mpg",
	"http://sirius/testing/AlamoTourSendoff0001.mpg",
	"http://sirius/testing/VictoriaSecretFashionShow2005.mpg",
	"http://sirius/testing/VictoriaSecretFashionShow2005.mpg"
};
FileGroupListTemplate(James,HTTP,0);

const char * const James_Problems_Filelist[] = 
{
	//Title claims 4 channels but only shows 2
	__Drive__"Media\\Video\\AVI\\AVI_todo\\4chdv002.avi",
	//These two one are slow
	__Drive__"Media\\Video\\AVI\\AVITodo\\SlowAVI\\FullCircle-Crime-MTV_Headbangers_Ball-Divx-Metal-DVDrip.avi",
	__Drive__"Media\\Video\\AVI\\AVITodo\\SlowAVI\\12fps.avi",
	//When using UYVY this one smears the x resolution as if it were incorrect (plays fine in windows media player)
	__Drive__"Media\\Video\\AVI\\Missle_AttackIndeo.avi"
};
FileGroupListTemplate(James,Problems,0);

const char * const James_MPG_Filelist[] = 
{
	__Drive__"Media\\Video\\MPEG\\snow[1].mpeg",
	__Drive__"Media\\Video\\MPEG\\Britney - Toxic {smg}.mpg",
	__Drive__"Media\\Video\\MPEG\\AlienSong.mpeg",
	//FFmpeg is not picking up any video stream (Example code same results)
	__Drive__"Media\\Video\\MPEG\\mpeg1_plays_on_VT_not_SE.mpg",
	__Drive__"Media\\Video\\MPEG2\\PS\\tartwar.mpg",
	//This one has a drifting audio stream
	__Drive__"Media\\Video\\MPEG2\\PS\\The Victoria's Secret Fashion Show.mpg",
	__Drive__"Media\\Video\\m2t\\HD\\HDV-JVC_Footage\\Cap0002(0001).m2t"
};
FileGroupListTemplate(James,MPG,4);

const char * const James_MPGTS_Filelist[] = 
{
	__Drive__"Media\\Video\\MPEG2\\TS\\051010170259.ts",
	__Drive__"Media\\Video\\MPEG2\\TS\\jump.ts",
	//This one has a problem with the correct frame rate
	__Drive__"Media\\Video\\m2t\\HD\\A-closeup.m2t",
	__Drive__"Media\\Video\\m2t\\HD\\CanopusHDV.m2t",
	__Drive__"Media\\Video\\m2t\\HD\\Clocks.m2t",
	//This one locks up because there are no audio packets available according to FFmpeg (needed a timeout)
	__Drive__"Media\\Video\\m2t\\HD\\GYHD100 Edited Dallas,Japan,Football.m2t",
	__Drive__"Media\\Video\\m2t\\HD\\HDV_ColorStripes.m2t",
	__Drive__"Media\\Video\\m2t\\HD\\Hot Air Ballons 1080i Sony HDV.m2t",
	__Drive__"Media\\Video\\m2t\\HD\\motoX_MainConcept MPEG-2_HDV 1080-60i[3].m2t",
	//This one has the audio out of sync by about 9 frames
	__Drive__"Media\\Video\\m2t\\AudioSync\\Canopus1080iCapture.m2t"
};
FileGroupListTemplate(James,MPGTS,7);

const char * const James_AVCHD_Filelist[] = 
{
	__Drive__"Media\\Video\\AVCHD\\FRC2012\\Alamo\\00206.MTS",
	__Drive__"Media\\Video\\AVCHD\\FRC2012\\Alamo\\00207.MTS",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00000.m2t",
	//Case 36326 Needed newer ffmpeg build to support pcm_bluray audio
	__Drive__"Media\\Video\\AVCHD\\00016.MTS",
	__Drive__"Media\\Video\\AVCHD\\Panasonic_DMC-GH1\\PRIVATE\\AVCHD\\BDMV\\STREAM\\00000.MTS",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00001.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00002.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00003.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00004.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00005.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00006.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00007.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\00008.m2t",
	__Drive__"Media\\Video\\AVCHD\\Canon_Vixia_HF200\\Sync_60i.M2T"
};
FileGroupListTemplate(James,AVCHD,0);

const char * const James_AVI_DV_Filelist[] = 
{
	__Drive__"Media\\Video\\AVI\\Short0.avi",
	__Drive__"Media\\Video\\AVI\\Piano0.avi",
	//This one is 14minutes good for random seek stress test
	__Drive__"Media\\Video\\AVI\\2GBDV_DirectShow1.avi",
	//Tests the 80108010 bad blocks of audio (listen for my favorite things this year is going to the "golf-ball")
	__Drive__"Media\\Video\\AVI\\Justin.avi",
	//Actually 2 channel type 2... with 4 channel embedded in IAVS stream
	__Drive__"Media\\Video\\AVI\\DV4channel32k12bit.avi",
	//Type 1 with 4 channels
	__Drive__"Media\\Video\\AVI\\Type1-4ch.avi",
	//PAL DV- with a bad video frame
	__Drive__"Media\\Video\\AVI\\capture0000.avi",
	//This appears to be 4:3 with bars... I don't think there is a way to zoom into it
	__Drive__"Media\\Video\\AVI\\DV_002.AVI",
	//This one is reverse fielded
	__Drive__"Media\\Video\\AVI\\testDV7.avi",
	//This one has dropped frames at the very beginning in the idx1.  Stresses have been added to compensate
	__Drive__"Media\\Video\\AVI\\DVCPro50_NTSC.avi",
};
FileGroupListTemplate(James,AVI_DV,0);

const char * const James_AVI_Filelist[] = 
{
	//This has a huge 3014704/3670066 aspect ratio which does not reduce well (had to lose some precision)
	__Drive__"Media\\Video\\AVI\\Truck_to_TriCaster_DiveIn_850.avi",
	//This has an alignment issue between strn and JUNK
	__Drive__"Media\\Video\\AVI\\Starship_Troopers_Pluto.avi",
	//This one has a very strange audio coding byte alignment (and compression)
	__Drive__"Media\\Video\\AVI\\405divx_lg_v2.avi",
	//The audio is mp3 using the VBR technique
	__Drive__"Media\\Video\\AVI\\165-star-oasis.avi",
	//This one had pts alignment issue for audio, but no longer
	__Drive__"Media\\Video\\AVI\\[A-Flux]_Nurse_Witch_Komugi_Chan_Magikarte_-_01.avi",
	__Drive__"Media\\Video\\AVI\\AC3avi\\ST_TNG-1x15-11001001.avi",
	__Drive__"Media\\Video\\AVI\\BorisFX.avi",
	//Here is a clip that has some dropped frames at 300 and 452
	__Drive__"Media\\Video\\AVI\\DroppedFrames\\S007.avi",
	//This codec takes advantage of the extradata in the BMI structure (just like how palette clips do)
	__Drive__"Media\\Video\\AVI\\Huffy.avi",
	//This one tests the avcodec_find_decoder() when it fails for video (this could change later)
	__Drive__"Media\\Video\\AVI\\capture.avi",
	//This one tests the bounds of both chunk indexes by providing bogus data any anything exceeding
	__Drive__"Media\\Video\\AVI\\wahlstrom_intro.avi",
	//This one has 8 bit pcm audio
	__Drive__"Media\\Video\\AVI\\Video1.avi",
	//Audio only stream
	__Drive__"Media\\Video\\AVI\\BigAVI_AudioOnly.avi",
	//These are Indeo (IV50) which needs a special codec to work.  
	//FFmpeg recognizes the tagID but does not have any codecs registered for it.  It will attempt to use VCM
	//works great once the ligos indeo codec is installed
	__Drive__"Media\\Video\\AVI\\1LeMenagerie_NextEpisodePt2.avi",
	__Drive__"Media\\Video\\AVI\\MavFlybyCarFade30b.avi"		//Video only stream
};
FileGroupListTemplate(James,AVI,0);

const char * const James_AVI_SpeedHQ_Filelist[] = 
{
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\ReboundRumbleField.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\TC_Powered_Alpha.avi", //case 36685 exposed crash when reading corrupt memory
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\061_JB_HD_a.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\AlphaDog_SHQ3.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\BSA102H2_nt25.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\Kiki_Window_Set002.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\PALSmoothSpeedHQ.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\TrainSpeedHQ.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\London_Bus.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\IBC_BUG.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\IBC_BUG2.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\IBC_BUG2_WS.avi",
	__Drive__"Media\\Video\\AVI\\KirksCodecs\\IBC_BUG_HD.avi",
	__Drive__"Media\\Video\\AVI\\CountUp.avi",  //Tests the frame range ends on 450
	__Drive__"Media\\Video\\AVI\\NTSC_HD_Test.avi"  //SpeedHQ... used for audio popping tests
};
FileGroupListTemplate(James,AVI_SpeedHQ,0);

const char * const James_AVI_Uncompressed_Filelist[] = 
{
	//uncompressed
	__Drive__"Media\\Video\\AVI\\Uncompressed\\LeaderFull.avi",
	__Drive__"Media\\Video\\AVI\\Uncompressed\\UYVY.avi",
	__Drive__"Media\\Video\\AVI\\Uncompressed\\YuY2.avi",
	//causes assertion error towards end of clip (only in application)
	__Drive__"Media\\Video\\AVI\\Uncompressed\\VideoToaster_Uncompressed.avi",
	//These work in FFmpeg tests but fail in the DDR
	__Drive__"Media\\Video\\AVI\\Uncompressed\\VT_uncompressed.avi",
	__Drive__"Media\\Video\\AVI\\Uncompressed\\IBMSpoofAVI.avi",
	__Drive__"Media\\Video\\AVI\\Uncompressed\\Alpha BGRA.avi",
	__Drive__"Media\\Video\\AVI\\Static.avi"
};
FileGroupListTemplate(James,AVI_Uncompressed,0);

const char * const James_WAV_Filelist[] = 
{
	//Length=1.06 8 bit unsigned PCM	Sample rate=11025 Channels=1
	__Drive__"Media\\Audio\\WAVE\\be_back.wav",
	//Length=838.35 (13:58:10) 16 bit	Sample rate=44100 Channels=2
	__Drive__"Media\\Audio\\WAVE\\Beethoven9thPart2.wav",
	//Length=4.79 8 bit unsigned PCM	Sample rate=24000 Channels=1
	__Drive__"Media\\Audio\\WAVE\\tadacrap.wav",
	//This is actually an embedded mp3
	//Length=160.60 (2:40:18)	mp3		Sample rate=12000 Channels 1
	__Drive__"Media\\Audio\\WAVE\\GoodBadUgly.wav",
	__Drive__"Media\\Audio\\WAVE\\DBTest\\0_16.wav",
	__Drive__"Media\\Audio\\MP3\\70\\Led Zeppelin\\Led Zeppelin IV\\Led Zeppelin-Four-04 - Stairway To Heaven.mp3"
};
FileGroupListTemplate(James,WAV,0);

const char * const James_DV_Filelist[] = 
{
	__Drive__"Media\\Video\\Others\\DIF-DVCPROHD\\Test.dv",
	__Drive__"Media\\Video\\Others\\DIF DVCPRO50\\black_test.dv",
	//Very clean switch (artificially made) between NTSC and PAL
	__Drive__"Media\\Video\\Others\\Crash\\NTSC2Pal.dv",
	//This one goes through several format changes before reaching PAL (Actually captured)
	__Drive__"Media\\Video\\Others\\Crash\\NTSC2PalTape.dv",
	//This one is brutal same place but many format changes
	__Drive__"Media\\Video\\Others\\Crash\\NTSCPAL3.dv",
	//This one starts a frame that has no audio, and then later frames have audio
	__Drive__"Media\\Video\\Others\\Crash\\NoAudio.dv",
	__Drive__"Media\\Video\\Others\\DIF DVCPRO50\\FALUNTEST1-16-9.dv",
	__Drive__"Media\\Video\\Others\\DIF DVCPRO50\\FALUNTEST2-16-9.dv",
	__Drive__"Media\\Video\\Others\\DIF DVCPRO50\\FALUNTEST3-4-3.dv",
	__Drive__"Media\\Video\\Others\\DIF-DVCPRO\\F-2006-0531-CHSKTEST.dv",
	__Drive__"Media\\Video\\Others\\DIF-DVCPRO\\NK-2005-1129-DVCPRO.dv",
};
FileGroupListTemplate(James,DV,1);

const char * const James_WMV_Filelist[] = 
{
	//FFmpeg WMV issues... audio does not seek
	__Drive__"Media\\Video\\WMV\\Picture 020.wmv",
	__Drive__"Media\\Video\\WMV\\Picture 001.wmv",
	//Audio only (for the wmv)
	__Drive__"Media\\Video\\WMV\\Run Dmc - Its Tricky.wma",
	__Drive__"Media\\Video\\WMV\\MVI_1346.wmv",
	__Drive__"Media\\Video\\WMV\\RevoDisco.wmv",
	__Drive__"Media\\Video\\WMV\\tartwar.wmv",
	__Drive__"Media\\Video\\WMV\\wc1hand-44_98.wmv",
	//Andrew created this... with window media encoder (this cannot seek)
	//It can only get duration... all other capabilities are not supported (e.g. cannot play after stop)
	__Drive__"Media\\Video\\WMV\\TCStudio Quick Tour.wmv",
	//These are Brian's clips:
	__Drive__"Media\\Video\\WMV\\mpeg4v3\\clip-cats.wmv",
	__Drive__"Media\\Video\\WMV\\vc-1\\FlightSimX_720p60.wmv",
	__Drive__"Media\\Video\\WMV\\vc-1\\TheNotebook-1080p.wmv",
	__Drive__"Media\\Video\\WMV\\wmv7\\clip-itsgonnarain.wmv",
	__Drive__"Media\\Video\\WMV\\wmv7\\clip-poona4.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-AdrenalineRush.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-CBS.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-CoralReefAdventure.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-FighterPilot.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-MysteryOfTheNile.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-T2.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-1080p-CoralReefAdventure.wmv"
};
FileGroupListTemplate(James,WMV,0);

const char * const James_MOV_Filelist[] = 
{
	//This is the default video format created on iPod, iPhone and iPad (h264 AAC) see case 37731
	__Drive__"Media\\Video\\MOV\\IMG_0755.MOV",  //180 rotation
	__Drive__"Media\\Video\\MOV\\IMG_0122.MOV",  //90 rotation
	__Drive__"Media\\Video\\MOV\\IMG_0124.MOV",  //270 rotation
	__Drive__"Media\\Video\\MOV\\wx13_stadium_promo.mov",
	__Drive__"Media\\Video\\MOV\\Wassup_SF[1].mov",
	__Drive__"Media\\Video\\MOV\\zapruder_stable.mov",
	__Drive__"Media\\Video\\MOV\\Dude.mov",	
	//Apple BMP video codec,  1920x1080... This tests the upper limit of the filereader-- AspectTest
	//Audio Codec: MACE 3:1
	__Drive__"Media\\Video\\MOV\\warthog.mov",
	//This file spreads the 4 audio streams out in a planar fashion forcing me to up my timeout threshold
	__Drive__"Media\\Video\\MOV\\HVXbamboo1080i.mov",
	__Drive__"Media\\Video\\MOV\\ep3.mov",
	__Drive__"Media\\Video\\MOV\\warthog_revisited.mov",
	//Assert in GetAudioStartTime for no PTS
	__Drive__"Media\\Video\\Others\\ShortFlash.swf"
};
FileGroupListTemplate(James,MOV,1);

const char * const James_FLV_Filelist[] = 
{
	__Drive__"Media\\Video\\FLV\\FRC_Texas\\videoplayback[B].flv",
	__Drive__"Media\\Video\\FLV\\20051210-w50s.flv",
	__Drive__"Media\\Video\\FLV\\20051210-w50s_56K.flv",
	__Drive__"Media\\Video\\FLV\\RenderTest004.flv",
	__Drive__"Media\\Video\\FLV\\youtube_640x480_2000kbps.flv",
	__Drive__"Media\\Video\\FLV\\zeldaHQ.flv",
	__Drive__"Media\\Video\\FLV\\ShortFlash.swf",
	//No audio seeking support yet
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\SpellBound_2.flv",
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\LikeItLikeIt.flv",
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\IWontTellYou.flv",
	//These require Flags Any on the video when seeking to 0
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\closer.flv",
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\heavens.flv",
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\ourtruth.flv",
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\swamped.flv",
	__Drive__"Media\\Video\\FLV\\LacunaCoil\\withinme.flv"
};
FileGroupListTemplate(James,FLV,0);

const char * const James_MXF_Filelist[] = 
{
	//Case 38023: Since customers are going to pay a million for this particular type, I should include it
	//960x720 16:9 59.94 progressive.  4:2:2 dv, 4 single channels of PCM 16 at 48Khz
	__Drive__"Media\\Video\\MXF\\ESPN_Quantel.mxf",
	//This has 4 channels of mono 16 bit
	__Drive__"Media\\Video\\MXF\\C0017.MXF",
	//This has 8 channels of mono 24 bit
	__Drive__"Media\\Video\\MXF\\hd422_1080i\\Clip\\C0002.MXF",
	//Clip Name		Meta2	Meta	Ted			Additional Info
	//0027JK.MXF	29.97	30		30pn	
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0027JK.MXF", 	//Audio is choppy (like missing buffers) 30p n
	//0028WA.MXF	23.97	24		No Audio	VFR Ratio 24/30
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0028WA.MXF", 	//No sound (video looks fast too)
	//0029HO.MXF	59.94	30		60p 24fps	2::3 Pulldown
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0029HO.MXF",  //You can see the 2-3 pull down when scrubbing through clip
	//0031TC.MXF	59.94	30		24p 24fps	
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0031TC.MXF",	//This one appears to be just like 29HO should be fine
	//0032OX.MXF	29.97	30		No Audio	VFR Ratio 30/24
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0032OX.MXF",	//No sound (video looks fast too)
	//0033PJ.MXF	23.97	24		24pn	
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0033PJ.MXF",	//The other "n"
	//0034D7.MXF	59.94	30		60p 60fps	
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0034D7.MXF",	//Should be 60/60 check field smoothness
	//0035DL.MXF	59.94	30		60p 12fps	
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\0035DL.MXF",	//12 frames imposed onto 60p.. does have some pull down in it
	//00249J.MXF	59.94	30		60p 30fps	2::2 Pulldown
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\00249J.MXF",
	//00306V.MXF	59.94	30		30p 24fps	2::3 Pulldown
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\00306V.MXF",
	//002530.MXF	59.94	30		30p 30fps	2::2 Pulldown
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\002530.MXF",
	//002662.MXF	59.94	30		24p 30fps	2::2 Pulldown
	__Drive__"Media\\Video\\MXF\\NewAllFlavors\\Sorted Clips\\720\\002662.MXF"
};
FileGroupListTemplate(James,MXF,0);

const char * const James_MP4_Filelist[] =
{
	//h264, AAC
	//The Intel codec plays this one fine; however, the ffmpeg codec will produce artifacts
	__Drive__"Media\\Video\\MPEG4\\BraschBence_A_szivem_nem_hatral.mp4",
	// MP4, AAC
	__Drive__"Media\\Video\\MPEG4\\crankygeeks.015.mp4",

	// MP2, PCM
	__Drive__"Media\\Video\\MPEG4\\EX-1.mp4",
	//h264, none
	__Drive__"Media\\Video\\MPEG4\\Friendly.mp4",
	//h264, AAC  (case 37614 this file needs extra pts tolerance)
	__Drive__"Media\\Video\\MPEG4\\KirkTest.mp4",

	//h264 these files fail miserably to render properly with Intel see case 37719
	__Drive__"Media\\Video\\MPEG4\\CAMN2RQX.mp4",
	__Drive__"Media\\Video\\MPEG4\\BadApple.mp4",

	//h264, AAC
	__Drive__"Media\\Video\\MPEG4\\Park_720p.mp4",
	//mpeg4, AAC
	__Drive__"Media\\Video\\MPEG4\\Smoking-Toasters2008-ipod.mp4",
	//h264, AAC
	__Drive__"Media\\Video\\MPEG4\\WhatWeDo.mp4",
	//h264 AAC and AC3 see case 37787
	__Drive__"Media\\Video\\MPEG4\\JerichoEpisode16_AAC_AC3.m4v"
};
FileGroupListTemplate(James,MP4,3);

const char * const James_WebM_Filelist[] =
{
	__Drive__"Media\\Video\\webm\\big_buck_bunny_480p.webm",
	__Drive__"Media\\Video\\webm\\hd_vp8.webm",
	__Drive__"Media\\Video\\webm\\redcliff450.webm",
	__Drive__"Media\\Video\\webm\\trailer_480p_180.webm",
	__Drive__"Media\\Video\\webm\\Amelia_720p.mkv"
};
FileGroupListTemplate(James,WebM,3);

const char * const Abheeshta_MXF_Filelist[] = 
{
	"C:\\Media\\Video\\Others\\MXF\\FALUNTEST2-16-9.MXF",
	"C:\\Media\\Video\\Others\\MXF\\0002H9.MXF",
	"C:\\Media\\Video\\Others\\MXF\\0001OA.MXF",
	"C:\\Media\\Video\\Others\\MXF\\freeMXF-mxf1.mxf",
	"C:\\Media\\Video\\Others\\MXF\\out.mxf",
	"C:\\Media\\Video\\Others\\MXF\\op1a-mpeg2k-wave_hd.mxf",
	"C:\\Media\\Video\\Others\\MXF\\jump.mxf",
	"C:\\Media\\Video\\Others\\MXF\\budlight.mxf",
	"C:\\Media\\Video\\Others\\MXF\\spears.mxf",
	"C:\\Media\\Video\\Others\\MXF\\mog.mxf"
};
FileGroupListTemplate(Abheeshta,MXF,1);

//Unfortunately my TriCaster C drive is small
const char * const James_TriCaster_Filelist[] = 
{
	"D:\\Media\\Video\\Others\\DIF DVCPRO50\\FALUNTEST2-16-9.dv",
	"D:\\Media\\Video\\m2t\\HD\\motoX_MainConcept MPEG-2_HDV 1080-60i[3].m2t"
};
FileGroupListTemplate(James,TriCaster,1);


//These are obsolete
#if 0
//nt25 no longer supported for 64bit
__Drive__"Media\\Video\\AVI\\nt25\\022_JB_HD_a.avi",
__Drive__"Media\\Video\\AVI\\nt25\\061_JB_HD_a.avi",
__Drive__"Media\\Video\\AVI\\nt25\\062_JB_HD_b.avi",
__Drive__"Media\\Video\\AVI\\nt25\\063_JB_HD_c.avi",
__Drive__"Media\\Video\\AVI\\nt25\\064_JB_HD_d.avi",
__Drive__"Media\\Video\\AVI\\nt25\\065_JB_HD_e.avi",
__Drive__"Media\\Video\\AVI\\nt25\\066_JB_HD_f.avi",
__Drive__"Media\\Video\\AVI\\nt25\\067_JB_HD_g.avi",
__Drive__"Media\\Video\\AVI\\nt25\\068_JB_HD_h.avi",
__Drive__"Media\\Video\\AVI\\nt25\\069_JB_HD_i.avi",
__Drive__"Media\\Video\\AVI\\nt25\\070_JB_HD_j.avi",
__Drive__"Media\\Video\\AVI\\nt25\\071_JB_HD_k.avi",
__Drive__"Media\\Video\\AVI\\nt25\\072_JB_HD_l.avi",
__Drive__"Media\\Video\\AVI\\nt25\\073_JB_HD_a.avi",
__Drive__"Media\\Video\\AVI\\nt25\\074_JB_HD_b.avi",
__Drive__"Media\\Video\\AVI\\nt25\\075_JB_HD_c.avi",
__Drive__"Media\\Video\\AVI\\nt25\\076_JB_HD_d.avi",
__Drive__"Media\\Video\\AVI\\nt25\\077_JB_HD_e.avi",
__Drive__"Media\\Video\\AVI\\nt25\\078_JB_HD_f.avi",
__Drive__"Media\\Video\\AVI\\nt25\\079_JB_HD_g.avi",
__Drive__"Media\\Video\\AVI\\nt25\\080_JB_HD_h.avi",
__Drive__"Media\\Video\\AVI\\nt25\\Heros_16x9_nt25.avi",
__Drive__"Media\\Video\\AVI\\nt25\\PAL1080NT25.avi",
__Drive__"Media\\Video\\AVI\\nt25\\RedBlueNT25.avi"
//Cannot find these
__Drive__"Media\\Video\\AVI\\Train.avi",
__Drive__"Media\\Video\\AVI\\Leaves1_LG.avi",
__Drive__"Media\\Video\\AVI\\capture0000_dv.avi",

#endif


FileStuff * const c_Groups[]=
{
	&James_Problems_Group,
	&James_MPG_Group,
	&James_MPGTS_Group,
	&James_AVI_DV_Group,
	&James_AVI_Group,
	&James_AVI_SpeedHQ_Group,
	&James_AVI_Uncompressed_Group,
	&James_WAV_Group,
	&James_DV_Group,
	&James_WMV_Group,
	&James_AVCHD_Group,
	&James_MOV_Group,
	&James_FLV_Group,
	&James_MXF_Group,
	&James_MP4_Group,
	&James_WebM_Group,
	&James_HTTP_Group
	//&Abheeshta_MXF_Group,
	//&James_TriCaster_Group
};
const size_t c_NoGroups=COUNTOF(c_Groups);

FileStuff *g_FileStuff=c_Groups[0];

const char *c_outpath = "C:\\Temp\\";

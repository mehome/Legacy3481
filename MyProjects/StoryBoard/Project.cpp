#include "Handlemain.h"
/*
char *filesource;
struct filternode *mediafilter;
UWORD totalframes;
short actualframes;
UWORD cropin;
UWORD cropout;
imageidentifier id;
mediatypes mediatype;

<IMAGE>			7
MediaFile=		?+10
TotalFrames=	12+5	=17
ActualFrames=	13+5	=18
CropIn=			7+5	=12
CropOut=			8+5	=13
Id=				3+1	=4
MediaType=		10+2	=12
*/
#define IMAGELISTSIZE 7+10+17+18+12+13+4+12+(8*2)
/*
char *filesource;
UBYTE *xytext;
UBYTE *thicktext;
short x,y;
UWORD in,out;
UBYTE thickness;

<FILTERCG>					10
FilterFile=					?+11
XYpoints=					?+9
TransparencyPoints=		?+19
X=								5+2	=7
Y=								5+2	=7
In=							5+3	=8
Out=							5+4	=9
Transparency=				3+13	=16
*/
#define FILTERCGSIZE 10+11+9+19+7+7+8+9+16+(9*2)
/*
	char *filesource;
	UWORD in,out;
	short frameoffset;

<AUDIO>						7
AudioFile= Dad :)			?+10
In=							5+3
Out=							5+4
Offset=						5+7
*/
#define AUDIOWAVSIZE 7+10+8+9+12+(5*2)
/*
<DVEPREFS>					10
AlphaBlendType=			15+1
Reverse=						8+1
*/
#define DVEALPHABLENDSIZE 10+16+9+(2*2)

enum structidtypes {sid_image,sid_filtercg,sid_audio,sid_dve};

const static char *structid[4]={"<IMAGE>","<FILTERCG>","<AUDIO>","<DVEPREFS>"};
const static int structidlength[4]={7,10,7,10};
#define IMAGEMAX 8
const static char *imagefieldname[IMAGEMAX]={"MediaFile=","TotalFrames=","ActualFrames=","CropIn=","CropOut=","Id=","MediaType=",0};
const static int imagefieldlength[IMAGEMAX]={10,12,13,7,8,3,10,0};
#define FILTERCGMAX 10
const static char *CGfieldname[FILTERCGMAX]={"FilterFile=","XYpoints=","TransparencyPoints=","X=","Y=","In=","Out=","Transparency=",0};
const static int CGfieldlength[FILTERCGMAX]={11,9,19,2,2,3,4,13,0};
#define AUDIOMAX 5
const static char *audiofieldname[AUDIOMAX]={"AudioFile=","In=","Out=","Offset="};
const static int audiofieldlength[AUDIOMAX]={10,3,4,7};
#define DVEALPHABLENDMAX 2
const static char *dveabfieldname[DVEALPHABLENDMAX]={"AlphaBlendType=","Reverse="};
const static int dveabfieldlength[DVEALPHABLENDMAX]={15,8};

char *projectclass::imagetotext(class storyboardclass *storyboard,int *actualsize) {
	int data[12];
	char *datastring[4];
	char *memindex,*mem=NULL;
	int size,t;
	struct imagelist *imagelisthead=storyboard->imagelisthead;
	struct imagelist *imageindex=imagelisthead;

	if (imagelisthead) {
		if (mem=memindex=(char *)mynew(&pmem,size=getsize(storyboard))) {
			do {
				strcpy(memindex,structid[sid_image]);
				memindex+=structidlength[sid_image];
				*memindex++=13;
				*memindex++='\n';
				//Lay down the image list
				t=0;
				strcpy(memindex,imagefieldname[t]);
				memindex+=imagefieldlength[t];
				strcpy(memindex,imageindex->filesource);
				memindex+=strlen(imageindex->filesource);
				*memindex++=13;
				*memindex++='\n';t++;

				data[1]=(int)imageindex->totalframes;
				data[2]=(int)imageindex->actualframes;
				data[3]=(int)imageindex->cropin;
				data[4]=(int)imageindex->cropout;
				data[5]=(int)imageindex->id;
				data[6]=(int)imageindex->mediatype;

				while (t<7) {
					strcpy(memindex,imagefieldname[t]);
					memindex+=imagefieldlength[t];
					wsprintf(string,"%d",data[t]);
					strcpy(memindex,string);
					memindex+=strlen(string);
					*memindex++=13;
					*memindex++='\n';
					t++;
					}
				//write in filterstructs if any
				if (imageindex->mediafilter) {
					allfilters *mediafilter=(allfilters *)imageindex->mediafilter;
					allfilters *filterindex=(allfilters *)mediafilter;
					filtertypes filtertype=mediafilter->node.filtertype;

					do {
						strcpy(memindex,structid[sid_filtercg]);
						memindex+=structidlength[sid_filtercg];
						*memindex++=13;
						*memindex++='\n';

						switch (filtertype) {
							case ft_CG: {
								datastring[0]=(char *)filterindex->CG.filesource;
								datastring[1]=(char *)filterindex->CG.xytext;
								datastring[2]=(char *)filterindex->CG.thicktext;
								t=0;
								while (t<3) {
									strcpy(memindex,CGfieldname[t]);
									memindex+=CGfieldlength[t];
									if (datastring[t]) {
										strcpy(memindex,datastring[t]);
										memindex+=strlen(datastring[t]);
										}
									*memindex++=13;
									*memindex++='\n';
									t++;
									}
								data[3]=(int)filterindex->CG.x>>2;
								data[4]=(int)filterindex->CG.y;
								data[5]=(int)filterindex->CG.in;
								data[6]=(int)filterindex->CG.out;
								data[7]=(int)filterindex->CG.thickness;

								while (t<8) {
									strcpy(memindex,CGfieldname[t]);
									memindex+=CGfieldlength[t];
									wsprintf(string,"%d",data[t]);
									strcpy(memindex,string);
									memindex+=strlen(string);
									*memindex++=13;
									*memindex++='\n';
									t++;
									}

								break;
								}
							case ft_alphasat:
								break;
							}// end switch filtertype
						} while(filterindex=(allfilters *)filterindex->node.next);
					} //end if media filters
				//Write Audio structs if any
				if (imageindex->audio) {
					//for now we'll do strictly wav
					struct wavinfo *audioindex=(struct wavinfo *)imageindex->audio;
					do {
						strcpy(memindex,structid[sid_audio]);
						memindex+=structidlength[sid_audio];
						*memindex++=13;
						*memindex++='\n';
						//TODO switch audio types here
						strcpy(memindex,audiofieldname[0]);
						memindex+=audiofieldlength[0];
						if (audioindex->filesource) {
							strcpy(memindex,audioindex->filesource);
							memindex+=strlen(audioindex->filesource);
							}
						*memindex++=13;
						*memindex++='\n';
						t=1;

						data[1]=(int)audioindex->in;
						data[2]=(int)audioindex->out;
						data[3]=(int)audioindex->frameoffset;

						while (t<4) {
							strcpy(memindex,audiofieldname[t]);
							memindex+=audiofieldlength[t];
							wsprintf(string,"%d",data[t]);
							strcpy(memindex,string);
							memindex+=strlen(string);
							*memindex++=13;
							*memindex++='\n';
							t++;
							}

						} while(audioindex=(struct wavinfo *)audioindex->node.next);
					} //end if audio
				//Write DVE prefs if any
				if (imageindex->DVEprefs) {
					strcpy(memindex,structid[sid_dve]);
					memindex+=structidlength[sid_dve];
					*memindex++=13;
					*memindex++='\n';
					switch (imageindex->mediatype) {
						case bmp:
						case pcx:
						case iff:
							//Softness
							strcpy(memindex,dveabfieldname[0]);
							memindex+=dveabfieldlength[0];
							wsprintf(string,"%d",((class alphaFX *)imageindex->DVEprefs)->alphablendtype);
							strcpy(memindex,string);
							memindex+=strlen(string);
							*memindex++=13;
							*memindex++='\n';
							//Reverse
							strcpy(memindex,dveabfieldname[1]);
							memindex+=dveabfieldlength[1];
							wsprintf(string,"%d",((class alphaFX *)imageindex->DVEprefs)->reverse);
							strcpy(memindex,string);
							memindex+=strlen(string);
							*memindex++=13;
							*memindex++='\n';
							break;
						case dveplugin:
							break;
						} // end switch
					} //end if DVE prefs
				} while (imageindex=imageindex->next);
			}
		}
	*actualsize=(int)(memindex-mem);
	return(mem);
	}


int projectclass::getsize(class storyboardclass *storyboard) {
	struct imagelist *imagelisthead=storyboard->imagelisthead;
	struct imagelist *imageindex=imagelisthead;
	int size=0;
	if (imagelisthead) {
		do {
			size+=IMAGELISTSIZE;
			size+=strlen(imageindex->filesource);
			//add filters
			if (imageindex->mediafilter) {
				allfilters *mediafilter=(allfilters *)imageindex->mediafilter;
				allfilters *filterindex=(allfilters *)mediafilter;
				filtertypes filtertype=mediafilter->node.filtertype;
				do {
					switch (filtertype) {
						case ft_CG:
							size+=FILTERCGSIZE;
							if (filterindex->CG.filesource) size+=strlen(filterindex->CG.filesource);
							if (filterindex->CG.xytext) size+=strlen((char *)filterindex->CG.xytext);
							if (filterindex->CG.thicktext) size+=strlen((char *)filterindex->CG.thicktext);
							break;
						case ft_alphasat:
							break;
						} // end switch filtertype
					} while(filterindex=(allfilters *)filterindex->node.next);
				} // end if filter
			//add audio
			if (imageindex->audio) {
				//for now just do uncompressed wav version
				//will not check for rtv audio here
				struct wavinfo *audioindex=(struct wavinfo *)imageindex->audio;
				do {
					size+=AUDIOWAVSIZE;
					if (audioindex->filesource) size+=strlen(audioindex->filesource);
					} while (audioindex=(struct wavinfo *)audioindex->node.next);
				} // if audio
			if (imageindex->DVEprefs) {
				switch (imageindex->mediatype) {
					case bmp:
						size+=DVEALPHABLENDSIZE;
						break;
					case dveplugin:
						break;
					}
				}
			} while (imageindex=imageindex->next);
		}
	return(size);
	}


int projectclass::projectsave(class storyboardclass *storyboard,char *filename) {
	int size;
	char *mem;
	mem=imagetotext(storyboard,&size);
	if (mem) {
		save(filename,size,mem);
		dispose((struct memlist *)mem,&pmem);
		return (TRUE);
		}
	return (0);
	}


int projectclass::openimage(class storyboardclass *storyboard,struct imagelist **imageprev,
	char **memindex,int *totalframes,float *linenumber) {
	HBITMAP hbm;
	struct imagelist *imageindex;
	int offset,t,temp,index,length;
	UWORD totalframescheck;
	BITMAPINFOHEADER *bitmapinfo;

	imageindex=(struct imagelist *)newnode(nodeobject,sizeof(imagelist));
	memset(imageindex,0,sizeof(imagelist));
	if (*imageprev) {
		//link previous to current
		(*imageprev)->next=imageindex;
		imageindex->prev=*imageprev;
		imageindex->next=NULL;
		}
	else storyboard->scrollpos=controls->streamptr=storyboard->imagelisthead=imageindex;
	*imageprev=imageindex;
	//fill in the rest of the node
	t=0;
	//since the members are constant we can skip parseing them
	//MediaFile=		MAX_PATH+10
	*memindex+=imagefieldlength[t];
	offset=linput(*memindex,string);
	*memindex+=offset;
	imageindex->filesource=(char *)newnode(nodeobject,length=(strlen(string)+1));
	strcpy(imageindex->filesource,string);
	//char *text; use string
	index=0;
	for (temp=0;temp<length;temp++) {
		if (string[temp]=='\\') index=temp;
		}
	if (temp) index++;
	imageindex->text=(char *)newnode(nodeobject,strlen(string+index)+1);
	strcpy(imageindex->text,string+index);
	t++;
	//TotalFrames=		12+6	=18
	*memindex+=imagefieldlength[t];
	offset=linput(*memindex,string);
	*memindex+=offset;
	imageindex->totalframes=(UWORD)atol(string);
	t++;
	//ActualFrames=	13+5	=18
	*memindex+=imagefieldlength[t];
	offset=linput(*memindex,string);
	*memindex+=offset;
	imageindex->actualframes=(short)atol(string);
	t++;
	//CropIn=			7+5	=12
	*memindex+=imagefieldlength[t];
	offset=linput(*memindex,string);
	*memindex+=offset;
	imageindex->cropin=(UWORD)atol(string);
	t++;
	//CropOut=			8+6	=14
	*memindex+=imagefieldlength[t];
	offset=linput(*memindex,string);
	*memindex+=offset;
	imageindex->cropout=(UWORD)atol(string);
	t++;
	//Id=					3+1	=4
	*memindex+=imagefieldlength[t];
	offset=linput(*memindex,string);
	*memindex+=offset;
	imageindex->id=(imageidentifier)atol(string);
	t++;
	//MediaType=		10+2	=12
	*memindex+=imagefieldlength[t];
	offset=linput(*memindex,string);
	*memindex+=offset;
	imageindex->mediatype=(mediatypes)atol(string);
	t++;
	//Now to fill in other manditory fields from the information given
	//float linenumber; //used to quickly know if this images is on the left side of glow
	imageindex->linenumber=*linenumber;
	*linenumber+=100;
	//struct filternode *mediafilter;
	imageindex->mediafilter=NULL;
	//HBITMAP image;
	bitmapinfo=NULL;

	if ((imageindex->id==id_media)||(imageindex->id==id_filter)) {
		hbm=medialoaders->openthumb(imageindex,&totalframescheck,&bitmapinfo);
		*totalframes+=imageindex->actualframes;
		}
	else  {
		//hbm=(HBITMAP)LoadImage(hInst,imageindex->filesource,IMAGE_BITMAP,VIDEOX,VIDEOY,LR_LOADFROMFILE);
		//hbm=medialoaders->bmpobj.getthumbbmp(imageindex,&bitmapinfo);
		hbm=medialoaders->openthumb(imageindex,&totalframescheck,&bitmapinfo);
		*totalframes-=imageindex->duration;
		}

	if (hbm==NULL) {
		printc("Warning unable to open %s",imageindex->text);
		imageindex->image=NULL;
		imageindex->id=id_error;
		}
	else copythumbtoimage(imageindex,hbm,bitmapinfo);

	//Close Media even if it was error
	//for those medias that are partially open
	/*
	if ((imageindex->id==id_media)||(imageindex->id==id_filter)) medialoaders->closethumb(imageindex,bitmapinfo);
	else if (hbm) DeleteObject(hbm);
	*/
	medialoaders->closethumb(imageindex,bitmapinfo);
	//Even though DVEprefs should be implemented, it may not so
	//well call the prefx for robustness
	if (hbm) storyboard->premediafx(imageindex);
	return(TRUE);
	}

void projectclass::copythumbtoimage(struct imagelist *imageindex,HBITMAP hbm,BITMAPINFOHEADER *bitmapinfo) {
	HDC hdc,shmem,dhmem;
	HPALETTE halftonepalette;

	hdc=GetDC(screen);
	SetBrushOrgEx(hdc,0,0,NULL);
	halftonepalette=CreateHalftonePalette(hdc);
	SelectPalette(hdc,halftonepalette,TRUE);
	RealizePalette(hdc);
	imageindex->image=CreateCompatibleBitmap(hdc,XBITMAP-2,YBITMAP-2);
	shmem=CreateCompatibleDC(NULL);
	SelectObject(shmem,hbm);
	dhmem=CreateCompatibleDC(NULL);
	SetStretchBltMode(dhmem,HALFTONE);
	SelectObject(dhmem,imageindex->image);
	StretchBlt(dhmem,0,0,XBITMAP-2,YBITMAP-2,shmem,0,0,bitmapinfo->biWidth,bitmapinfo->biHeight,SRCCOPY);
	DeleteDC(dhmem);
	DeleteDC(shmem);
	DeleteObject(halftonepalette);
	ReleaseDC(screen,hdc);
	}


int projectclass::projectopen(class storyboardclass *storyboard,char *filename) {
	long size;
	int offset,result=0;
	int count=0;
	int totalframes=0;
	char *mem=(char *)load(filename,&size,&pmem);
	char *memindex=mem;
	char *memmax=mem+size;
	struct imagelist *imageprev=NULL;
	float linenumber=100;
	structidtypes structidnum;

	isstartedbyproject=TRUE;
	//For now will not implement append
	if (storyboard->imagelisthead) {
		storyboard->closeproject();
		}
	if (memindex) {
		// There must be a struct id here
		do {
			offset=linput(memindex,string);
			memindex+=offset;
			if (strcmp(string,structid[sid_image])==0) structidnum=sid_image;
			else if (strcmp(string,structid[sid_filtercg])==0) structidnum=sid_filtercg;
			else if (strcmp(string,structid[sid_audio])==0) structidnum=sid_audio;
			else if (strcmp(string,structid[sid_dve])==0) structidnum=sid_dve;
			else {
				printc("Warning project file corrupt; Expecting a StructID");
				printc("Aborting Load");
				goto errorload;
				}
			switch (structidnum) {
				case sid_image:
					openimage(storyboard,&imageprev,&memindex,&totalframes,&linenumber);
					count++;
					break;
				case sid_filtercg: {
					char buffer[256];
					char *bufindex;
					struct filterCG  *mediafilterCG;
					int t,temp,index,length;
					//take advantage of initfilterCG by puting filesource and
					//text into a imagelist struct
					struct imagelist *dragimage=(struct imagelist *)newnode(nodeobject,sizeof(struct imagelist));

					//FilterFile=
					t=0;
					memindex+=CGfieldlength[t];
					offset=linput(memindex,string);
					memindex+=offset;
					dragimage->filesource=(char *)newnode(nodeobject,length=(strlen(string)+1));
					strcpy(dragimage->filesource,string);
					//char *text; use string
					index=0;
					for (temp=0;temp<length;temp++) {
						if (string[temp]=='\\') index=temp;
						}
					if (temp) index++;
					dragimage->text=(char *)newnode(nodeobject,strlen(string+index)+1);
					strcpy(dragimage->text,string+index);
					t++;
					mediafilterCG=filters->CGobj.initfilterCG(dragimage,imageprev);
					//Now dragimage has served its purpose so dispose
					if (dragimage) {
						if (dragimage->text) disposenode(nodeobject,(struct memlist *)dragimage->text);
						if (dragimage->filesource) disposenode(nodeobject,(struct memlist *)dragimage->filesource);
						disposenode(nodeobject,(struct memlist *)dragimage);
						}
					//Make our corrections to mediafilterCG get the advance to next pos
					//XYpoints=					?+9
					memindex+=CGfieldlength[t];
					length=0;
					bufindex=buffer;
					while (*memindex==13||*memindex==10) memindex++;
					//may want error checking here
					while (!(strncmp(memindex,CGfieldname[t+1],CGfieldlength[t+1])==0)) { 
						offset=linput(memindex,string);
						memindex+=offset;
						length=strlen(string);
						strcpy(bufindex,string);
						bufindex[length]=13;
						bufindex[length+1]=10;
						bufindex+=length+2;
						} 
					*bufindex=0;
					if (bufindex>buffer) {
						mediafilterCG->xytext=(char *)newnode(mediafilterCG->node.nodeobject,length=(strlen(buffer)+1));
						strcpy(mediafilterCG->xytext,buffer);
						filters->CGobj.xytext2points(mediafilterCG,imageprev);
						if (mediafilterCG->xypointshead) {
							mediafilterCG->xrange=(short *)newnode(mediafilterCG->node.nodeobject,imageprev->totalframes<<2);
							mediafilterCG->yrange=(short *)newnode(mediafilterCG->node.nodeobject,imageprev->totalframes<<2);
							filters->CGobj.updatexyrange(mediafilterCG,imageprev);
							}
						}
					t++;
					//TransparencyPoints=		?+19
					memindex+=CGfieldlength[t];
					length=0;
					bufindex=buffer;
					while (*memindex==13||*memindex==10) memindex++;
					//may want error checking here
					while (!(strncmp(memindex,CGfieldname[t+1],CGfieldlength[t+1])==0)) {
						offset=linput(memindex,string);
						memindex+=offset;
						length=strlen(string);
						strcpy(bufindex,string);
						bufindex[length]=13;
						bufindex[length+1]=10;
						bufindex+=length+2;
						}  
					*bufindex=0;
					if (bufindex>buffer) {
						mediafilterCG->thicktext=(char *)newnode(mediafilterCG->node.nodeobject,length=(strlen(buffer)+1));
						strcpy(mediafilterCG->thicktext,buffer);
						filters->CGobj.depthtext2points(mediafilterCG,imageprev);
						if (mediafilterCG->thickpointshead) {
							mediafilterCG->thickrange=(UBYTE *)newnode(mediafilterCG->node.nodeobject,imageprev->totalframes<<1);
							filters->CGobj.updatedepthrange(mediafilterCG,imageprev);
							}
						}
					t++;
					//X=							5+2	=7
					memindex+=CGfieldlength[t];
					offset=linput(memindex,string);
					mediafilterCG->x=((short)atol(string))<<2;
					memindex+=offset;
					t++;
					//Y=							5+2	=7
					memindex+=CGfieldlength[t];
					offset=linput(memindex,string);
					mediafilterCG->y=(short)atol(string);
					memindex+=offset;
					t++;
					//In=							5+3	=8
					memindex+=CGfieldlength[t];
					offset=linput(memindex,string);
					mediafilterCG->in=(UWORD)atol(string);
					memindex+=offset;
					t++;
					//Out=						5+4	=9
					memindex+=CGfieldlength[t];
					offset=linput(memindex,string);
					mediafilterCG->out=(UWORD)atol(string);
					memindex+=offset;
					t++;
					//Transparency=			3+13	=16
					memindex+=CGfieldlength[t];
					offset=linput(memindex,string);
					mediafilterCG->thickness=(UBYTE)atol(string);
					memindex+=offset;
					mediafilterCG->node.vfilterptr=&filters->CGobj;
					break;
					}

				case sid_audio: {
					struct wavinfo *audioindex;
					int t=0;
					memindex+=audiofieldlength[t];
					offset=linput(memindex,string);
					memindex+=offset;
					audioindex=audio->addwavtomedia(imageprev,string);
					t++;
					//now to put in control settings
					//In=							5+3	=8
					memindex+=audiofieldlength[t];
					offset=linput(memindex,string);
					audioindex->in=(UWORD)atol(string);
					memindex+=offset;
					t++;
					//Out=						5+4	=9
					memindex+=audiofieldlength[t];
					offset=linput(memindex,string);
					audioindex->out=(UWORD)atol(string);
					memindex+=offset;
					t++;
					//Offset=						5+7	=12
					memindex+=audiofieldlength[t];
					offset=linput(memindex,string);
					audioindex->frameoffset=(UWORD)atol(string);
					memindex+=offset;
					t++;

					break;
					} //end audio prefs

				case sid_dve: {
					/*
					switch(dve->mediatype) {
						}
					*/
					if (imageprev->id==id_dve) { //ensure we have a valid dve loaded
						imageprev->DVEprefs=(class generalFX *)(new alphaFX(imageprev));
						//softness
						memindex+=dveabfieldlength[0];
						offset=linput(memindex,string);
						((class alphaFX *)imageprev->DVEprefs)->alphablendtype=(alphablendtypes)atol(string);
						memindex+=offset;
						//reverse
						memindex+=dveabfieldlength[1];
						offset=linput(memindex,string);
						((class alphaFX *)imageprev->DVEprefs)->reverse=(BOOL)atol(string);
						memindex+=offset;
						}
					else { //skip to next position
						//softness
						memindex+=dveabfieldlength[0];
						offset=linput(memindex,string);
						memindex+=offset;
						//reverse
						memindex+=dveabfieldlength[1];
						offset=linput(memindex,string);
						memindex+=offset;
						}
					break;
					} //end dve prefs

				} //end switch
			} while(memindex<memmax);
		result=TRUE;
		controls->adjustframecounter(storyboard,linenumber,totalframes,TRUE);

		{
			//Set the maximum items in the scrollbar
			SCROLLINFO si;

			si.cbSize=sizeof(SCROLLINFO);
			si.fMask=SIF_RANGE|SIF_PAGE|SIF_POS;
			GetScrollInfo(storyboard->storywindow,SB_VERT,&si);
			si.nPage=(UINT)(storyboard->numrows);
			si.nMax=(count-1)/storyboard->numcolumns;
			si.nMin=0;
			si.nPos=0;
			SetScrollInfo(storyboard->storywindow,SB_VERT,&si,TRUE);
			}
		storyboard->imagecount=count;
		storyboard->scrolloffset=0;
		storyboard->updateimagelist();
		} //if memindex
errorload:
	if (mem) dispose((struct memlist *)mem,&pmem);
	isstartedbyproject=FALSE;
	return(result);
	}


void projectclass::projectclose(class storyboardclass *storyboard) {
	storyboard->closeproject();
	SetWindowText(screen,lpszTitle);
	}


void projectclass::projectnew(class storyboardclass *storyboard) {
	if (getopenfilename("Please enter a new for new project",FALSE))
		storyboard->closeproject();
	}


projectclass::projectclass() {
	GetCurrentDirectory(MAX_PATH,openstartpath);
	strcat(openstartpath,"\\projects");
	projectname[0]=0;
	projectfilename[0]=0;
	isstartedbyproject=FALSE;
	}


char *projectclass::getopenfilename(char *inputprompt,BOOL musthave) {
	OPENFILENAME ofn;
	BOOL bResult;

	//Clear out and fill in an OPENFILENAME structure in preparation
	//for creating a common dialog box to open a file.
	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize	= sizeof(OPENFILENAME);
	ofn.hwndOwner	= screen;
	ofn.hInstance	= hInst;
	ofn.lpstrFilter	= "Project Files\0*.txt\0\0";
	ofn.nFilterIndex	= 0;
	projectname[0]	= '\0';
	ofn.lpstrFile	= projectname;
	ofn.nMaxFile	= sizeof(projectname);
	ofn.lpstrFileTitle = projectfilename;
	ofn.nMaxFileTitle	= sizeof(projectfilename);
	ofn.lpstrInitialDir = openstartpath;
	ofn.lpstrDefExt	= "txt";
	ofn.lpstrTitle	= inputprompt;
	if (musthave) ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	else ofn.Flags=OFN_HIDEREADONLY;

	if (bResult=GetOpenFileName(&ofn)) {
		lstrcpy(openstartpath,projectname);
		projectfilename[strlen(projectfilename)-4]=0;
		wsprintf(string,"%s - %s",lpszTitle,projectfilename);
		SetWindowText(screen,string);
		return(projectname);
		}
	return(0);
	}


char *projectclass::getsavefilename(char *inputprompt) {
	OPENFILENAME ofn;
	BOOL bResult;

	//Clear out and fill in an OPENFILENAME structure in preparation
	//for creating a common dialog box to open a file.
	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize	= sizeof(OPENFILENAME);
	ofn.hwndOwner	= screen;
	ofn.hInstance	= hInst;
	ofn.lpstrFilter	= "Project Files\0*.txt\0\0";
	ofn.nFilterIndex	= 0;
	projectname[0]	= '\0';
	ofn.lpstrFile	= projectname;
	ofn.nMaxFile	= sizeof(projectname);
	ofn.lpstrFileTitle = projectfilename;
	ofn.nMaxFileTitle	= sizeof(projectfilename);
	ofn.lpstrInitialDir = openstartpath;
	ofn.lpstrDefExt	= "txt";
	ofn.lpstrTitle	= inputprompt;
	ofn.Flags		= OFN_HIDEREADONLY;

	if (bResult=GetSaveFileName(&ofn)) {
		lstrcpy(openstartpath,projectname);
		projectfilename[strlen(projectfilename)-4]=0;
		wsprintf(string,"%s - %s",lpszTitle,projectfilename);
		SetWindowText(screen,string);
		return(projectname);
		}
	return(0);
	}


#ifndef STRUCTSTAY_H
#define STRUCTSTAY_H
#include "structgo.h"

struct imagelist {
	struct imagelist *next;
	struct imagelist *prev;
	float linenumber; //used to quickly know if this images is on the left side of glow
	char *filesource;
	union { //All the type of media resources here
		APTR mediaother;
		ULONG *mediaiff;
		ULONG *mediatga;
		class avihandle *mediaavi;
		struct rtvhandle *mediartv;
		ULONG *mediabmp;
		};
	HBITMAP thumb;
	struct filternode *mediafilter;
	struct audionode *audio,*rtvaudio;
	class generalFX *DVEprefs;
	HBITMAP image;
	HBITMAP halfimage;
	char *text;
	long totalframes;
	union {
		long actualframes; //for media
		long duration; //for transition
		};
	long cropin;
	long cropout;
	class storysourcebase *idobj;
	imageidentifier id;
	mediatypes mediatype;
	UWORD selected;
	};

#endif /* STRUCTSTAY_H */

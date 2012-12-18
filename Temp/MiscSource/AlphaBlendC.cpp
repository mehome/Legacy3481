void fxclass::maketransition(struct imagelist *dve,ULONG *imagea,ULONG *imageb,ULONG *videobuf,ULONG dvetime) {
	UBYTE aY,aU,aV,aZ,bY,bU,bV,bZ,wY,wU,wV,wZ;
	ULONG transition;
	long total=dve->duration*2;
	long current=(long)dvetime*2;
	long gradient;
	long g,h;
	//long howfaralong;
	long t=86400;
	UBYTE *alpha=(UBYTE *)GlobalLock(dve->mediaalpha);

	while (t) {
		aV=(UBYTE)((*imagea&0xFF));
		aZ=(UBYTE)((*imagea&0xFF00)>>8);
		aU=(UBYTE)((*imagea&0xFF0000)>>16);
		aY=(UBYTE)((*imagea++&0xFF000000)>>24);

		bV=(UBYTE)((*imageb&0xFF));
		bZ=(UBYTE)((*imageb&0xFF00)>>8);
		bU=(UBYTE)((*imageb&0xFF0000)>>16);
		bY=(UBYTE)((*imageb++&0xFF000000)>>24);

		gradient=*(alpha+t*2);
		h=(long)(current*255/total);
		g=gradient+86;
		//gradient=(gradient*h)/(384-h);
		gradient=(255-h)*(255-g)-(h*g);
		if (gradient>255) gradient=0;
		gradient=(abs(gradient))>>7;
		if (gradient>255) gradient=255;

		wV=(UBYTE)(aV-(gradient*(aV-bV)/255));
		wZ=(UBYTE)(aZ-(gradient*(aZ-bZ)/255));
		wU=(UBYTE)(aU-(gradient*(aU-bU)/255));
		wY=(UBYTE)(aY-(gradient*(aY-bY)/255));

		transition=wV+(wZ<<8)+(wU<<16)+(wY<<24);
		*videobuf++=transition;
		t--;
		}

	t=86400;
	current++;
	while (t) {
		aV=(UBYTE)((*imagea&0xFF));
		aZ=(UBYTE)((*imagea&0xFF00)>>8);
		aU=(UBYTE)((*imagea&0xFF0000)>>16);
		aY=(UBYTE)((*imagea++&0xFF000000)>>24);

		bV=(UBYTE)((*imageb&0xFF));
		bZ=(UBYTE)((*imageb&0xFF00)>>8);
		bU=(UBYTE)((*imageb&0xFF0000)>>16);
		bY=(UBYTE)((*imageb++&0xFF000000)>>24);

		gradient=*(alpha+t*2);
		h=(long)(current*255/total);
		g=gradient+86;
		//gradient=(gradient*h)/(384-h);
		gradient=(255-h)*(255-g)-(h*g);
		if (gradient>255) gradient=0;
		gradient=(abs(gradient))>>7;
		if (gradient>255) gradient=255;

		wV=(UBYTE)(aV-(gradient*(aV-bV)/255));
		wZ=(UBYTE)(aZ-((aZ-bZ)/(float)255)*gradient);
		wU=(UBYTE)(aU-((aU-bU)/(float)255)*gradient);
		wY=(UBYTE)(aY-((aY-bY)/(float)255)*gradient);

		transition=wV+(wZ<<8)+(wU<<16)+(wY<<24);
		*videobuf++=transition;
		t--;
		}

	GlobalUnlock(dve->mediaalpha);
	} //end MakeTransition

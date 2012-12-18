		//aU=(UBYTE)((*imagea&0xFF));
__asm {
		mov	eax,imagea
		mov	edx,dword ptr [eax] ;lock edx we'll parse the bytes from register
		mov	eax,edx
		and	eax,0FFh
		mov	aU,a1
		}
		//aY=(UBYTE)((*imagea&0xFF00)>>8);
__asm {
		mov	eax,edx
		and	eax,0FF00h
		shr	eax,8
		mov	aY,a1
		}
		//aV=(UBYTE)((*imagea&0xFF0000)>>16);
__asm {
		mov	eax,edx
		and	eax,0FF0000h
		shr	eax,10h
		mov	aY,a1
		}
		//aZ=(UBYTE)((*imagea++&0xFF000000)>>24);
__asm {
		mov	eax,edx
		and	eax,0FF000000h
		shr	eax,18h
		mov	aY,a1
		add	edx,4
		mov	imagea,edx
		}

		//bU=(UBYTE)((*imageb&0xFF));
__asm {
		mov	eax,imageb
		mov	edx,dword ptr [eax] ;lock edx we'll parse the bytes from register
		mov	eax,edx
		and	eax,0FFh
		mov	bU,a1
		}
		//bY=(UBYTE)((*imageb&0xFF00)>>8);
__asm {
		mov	eax,edx
		and	eax,0FF00h
		shr	eax,8
		mov	bY,a1
		}
		//bV=(UBYTE)((*imageb&0xFF0000)>>16);
__asm {
		mov	eax,edx
		and	eax,0FF0000h
		shr	eax,10h
		mov	bY,a1
		}
		//bZ=(UBYTE)((*imagea++&0xFF000000)>>24);
__asm {
		mov	eax,edx
		and	eax,0FF000000h
		shr	eax,18h
		mov	aY,a1
		add	edx,4
		mov	imageb,edx
		}

/*Here's the intel version
void fxclass::maketransition(struct imagelist *dve,ULONG *imagea,ULONG *imageb,ULONG *videobuf,ULONG dvetime) {
	UBYTE aY,aU,aV,aZ,bY,bU,bV,bZ,wY,wU,wV,wZ;
	ULONG transition;
	long total=dve->duration*2;
	long current=(long)dvetime*2;
	long gradient;
	long h;
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

		__asm {
		//h=(long)((current<<8)/total);
			mov	eax,current
			shl	eax,8
			cdq
			idiv	total
			mov	h,eax

		//gradient=*(alpha+(t<<1));
			mov	edx,alpha
			mov	ecx,t
			shl	ecx,1
			add	edx,ecx
			xor	eax,eax
			mov	al,byte ptr[edx]
		//g=gradient+86;
			add	eax,86
		//gradient=(255-h)*(255-g)-(h*g);
			mov	ecx,eax
			mov	eax,255
			sub	eax,h
			mov	ebx,255
			sub	ebx,ecx
			imul	eax,ebx
			mov	ebx,h
			imul	ebx,ecx
			sub	eax,ebx
		//if (gradient>255) gradient=0;
			cmp	eax,255
			jle	dontsatblack
			mov	eax,0
dontsatblack:
		//gradient=(gradient=gradient>=0?gradient:-gradient)>>7;
			jge	gradisneg
			neg	eax
gradisneg:
			sar	eax,7

		//if (gradient>255) gradient=255;
			cmp	eax,255
			jle	dontsatwhite
			mov	eax,255
dontsatwhite:
			mov	gradient,eax
			}

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

		gradient=*(alpha+(t<<1));
		h=(long)((current<<8)/total);

		g=gradient+86;
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
*/

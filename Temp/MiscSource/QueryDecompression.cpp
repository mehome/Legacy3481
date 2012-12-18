/*************************************************************************/
#define FOURCC_MJPG     mmioFOURCC('M','J','P','G')
#define FOURCC_mjpg     mmioFOURCC('m','j','p','g')
#define FOURCC_dmb1     mmioFOURCC('d','m','b','1')

#define BI_MJPG         mmioFOURCC('M','J','P','G')
#define BI_mJPG         mmioFOURCC('m','J','P','G')
#define BI_mjpg         mmioFOURCC('m','j','p','g')
#define BI_dmb1         mmioFOURCC('d','m','b','1')
#define BI_dmb2         mmioFOURCC('d','m','b','2')

#define BI_UYVY         mmioFOURCC('U','Y','V','Y')
#define BI_VYUY         mmioFOURCC('V','Y','U','Y') //ATI-Alias for YUYV
#define BI_YUY2         mmioFOURCC('Y','U','Y','2') //MS-Alias for  YUYV
#define BI_YUV2         mmioFOURCC('Y','U','V','2') //MXB-Alias for UYVY

/*****************************************************************************
 *
 * DecompressQuery() implements ICM_DECOMPRESS_QUERY
 *

****************************************************************************/
LRESULT NEAR PASCAL DecompressQuery(INSTINFO * pinst,DWORD dwFlags,
	LPBITMAPINFOHEADER lpbiSrc,LPVOID pSrc,int xSrc,int ySrc,int dxSrc,int dySrc,
	LPBITMAPINFOHEADER lpbiDst,LPVOID pDst,int xDst,int yDst,int dxDst,int dyDst) {
	LRESULT l;

	//    DPF ("DecompressQuery()");
#ifdef DEBUG
	DPF("DecompQuery(Src:'%4.4ls', %ld, DSrc: %ix%i, Src: %ix%i, 0x%x)",
		fts(lpbiSrc->biCompression), lpbiSrc->biBitCount,
		dxSrc,dySrc,lpbiSrc->biWidth,lpbiSrc->biHeight,lpbiDst);
#endif

	if ((l=DecompressQueryFmt(pinst,lpbiSrc))) return (l);

	// allow (-1) as a default width/height
	if (dxSrc==-1) dxSrc=(int)lpbiSrc->biWidth;

	if (dySrc==-1) dySrc=(int)lpbiSrc->biHeight;
	//  we cant clip the source.

	if ((xSrc!=0)||(ySrc!=0)) return ICERR_BADPARAM;

	if ((dxSrc!=(int)lpbiSrc->biWidth)||(dySrc!=(int)lpbiSrc->biHeight))
		return ICERR_BADPARAM;

    //  are we being asked to query just the input format?

	if (lpbiDst==NULL) return ICERR_OK;

    // allow (-1) as a default width/height

	if (dxDst==-1) dxDst=(int)lpbiDst->biWidth;

	if (dyDst==-1) dyDst=abs((int)lpbiDst->biHeight);

	if (lpbiDst->biSize!=sizeof(BITMAPINFOHEADER)||dxSrc!=dxDst||dySrc!=dyDst||
		!((lpbiDst->biBitCount== 16&&(FALSE
#ifdef JCS__R555
		|| lpbiDst->biCompression == BI_RGB
#endif

#ifdef JCS__YUY2
		|| ((lpbiDst->biCompression == BI_YUY2) &&
     ((pinst->CodecConfigFlags & CF_DCOMP_YUY2_DISABLE)==0))
#endif

#ifdef JCS__UYVY
		|| ((lpbiDst->biCompression == BI_UYVY) &&
     ((pinst->CodecConfigFlags & CF_DCOMP_UYVY_DISABLE)==0))
#endif
		)) || (lpbiDst->biBitCount == 24 && ( FALSE
#ifdef JCS__BGR3
		|| lpbiDst->biCompression == BI_RGB
#endif
      )) || (lpbiDst->biBitCount == 32 && ( FALSE
#ifdef JCS__BGR4
		|| lpbiDst->biCompression == BI_RGB
#endif
		)) )) {

// BI_BITFIELDS Hack : some software uses BI_BITFIELD on place of BI_RGB

		if(lpbiDst->biCompression==BI_BITFIELDS) {
			DWORD * MaskPtr=((PBITMAPINFOMASK)lpbiDst)->dwMask;
#ifdef DEBUG
			DPF("BI_BITFIELDS: 0x%08X, 0x%08X, 0x%08X",MaskPtr[0],MaskPtr[1],MaskPtr[2]);
#endif

#ifdef JCS__BGR4
			if(lpbiDst->biBitCount==32)
				if(    MaskPtr[0]==0x00FF0000
				    && MaskPtr[1]==0x0000FF00
					 && MaskPtr[2]==0x000000FF) return ICERR_OK;
			}
#endif
	return ICERR_BADFORMAT;
		}
	return ICERR_OK;
	}


LRESULT NEAR PASCAL DecompressQueryFmt(INSTINFO *pinst,LPBITMAPINFOHEADER lpbiSrc) {
// determine if the input DIB data is in a format we like.

	if (lpbiSrc==NULL) {
		DPF ("DecomQueryFmt(NULL)");
		return ICERR_BADFORMAT;
		} 
	else {

#ifdef DEBUG
		DPF ("DecomQueryFmt('%4.4ls', %ld)",fts(lpbiSrc->biCompression),lpbiSrc->bi
#endif

	   if ((lpbiSrc->biBitCount != 24 && lpbiSrc->biBitCount != 16 && lpbiSrc->biBitCount != 8) ||
			(lpbiSrc->biCompression != BI_MJPG && lpbiSrc->biCompression != BI_mJPG &&
			lpbiSrc->biCompression != BI_mjpg && lpbiSrc->biCompression != BI_dmb2 &&
			lpbiSrc->biCompression != BI_dmb1)) 
			{return ICERR_BADFORMAT;}
		}
	return ICERR_OK;
	}


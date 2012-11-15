/*
  $Id: filter_functions.c,v 1.2 2000/01/05 15:51:34 markusm Exp $

  Functions for porting Windows and OS/2 filters to Unix
*/

#include "filter_functions.h"

#ifdef unix
#include "unistd.h"
#include <errno.h>
#else

#include <windows.h>
#include <io.h>

#endif
#include <stdio.h>


void DosSetFilePtr(FILE *hf,off_t offset,int whence,ULONG* bytes_read) {
	fseek(hf, offset, whence);
	*bytes_read=ftell(hf);
	}

int DosRead(FILE *hf,void *buf,size_t count,ULONG *bytes_read) {
	*bytes_read=fread(buf,1,count,hf);
#ifdef unix
	return errno;
#endif
#ifdef WIN32
	if (*bytes_read == count) return 0; else return 1;
#endif
	}

int DosWrite(FILE *hf,void *buf,size_t count,ULONG *bytes_written) {
	*bytes_written=fwrite( buf, 1, count, hf );
#ifdef unix
	return errno;
#endif
#ifdef WIN32
	if ( *bytes_written == count ) return 0; else return 1;
#endif
	}

int DosClose(FILE *hf) {
	return fclose(hf);
	}


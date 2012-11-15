#ifndef FILTERFUNC_H
#define FILTERFUNC_H

#include <windows.h>
#include <stdio.h>
#include <io.h>

typedef long off_t;

void DosSetFilePtr(FILE *hf,off_t offset,int whence,ULONG *bytes_read);
int DosRead(FILE *hf,void *buf,size_t count,ULONG *bytes_read);
int DosWrite(FILE *hf,void *buf,size_t count,ULONG* bytes_written);
int DosClose(FILE *hf);

#endif //end FILTERFUNC_H

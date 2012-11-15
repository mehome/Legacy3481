#ifndef RGBRENDER_H
#define RGBRENDER_H

void renderyuvscale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,BOOL upsidedown);
void renderyuv(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown);
void renderrgb32(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown);
void renderrgb32scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,BOOL upsidedown);
void renderrgb24(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown);
void renderrgb24scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits);
void renderrgb16(ULONG *videobuf,int x,int y,char *decodedbits,BOOL upsidedown);
void renderrgb16scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,BOOL upsidedown);
void renderrgb8(ULONG *videobuf,int x,int y,char *decodedbits,LPBITMAPINFOHEADER bitmapinfo,BOOL upsidedown);
void renderrgb8scale(ULONG *videobuf,int x,int y,int scalex,int scaley,char *decodedbits,LPBITMAPINFOHEADER bitmapinfo);

#endif /* RGBRENDER_H */

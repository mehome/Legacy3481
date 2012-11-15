#ifndef UNLZW_H
#define UNLZW_H

// Copyright 2000 by Dan Stockelman, Dan's Dealers and Computers (DDC), James Killian, and Exodus.

class UnLZWclass {
	public:
	struct lengthstring {
		ULONG length;
		char *string;
		} out;
	ULONG predict;
	ULONG width, rowsperstrip;
	UINT bpp;
	UWORD tablemax;
	struct lengthstring table[4098];
	struct nodevars *nodetable;
	char *tablestart;
	int sizer;
	UINT striplength;

	UnLZWclass(ULONG predictparm,UINT bppparm,ULONG widthparm,ULONG rpsparm);
	~UnLZWclass();
	void UnLZWstrip(char *currstrip, char *newstripindex);

	private:
	void Inittable();
	void Blanktable();
	void Unpredictgrey(char *strip, int bcount, int bpp);
	void Unpredictrgb(char *strip, int bcount, int bpp);
	void Unpredictrgba(char *strip, int bcount, int bpp);
	bool Isintable(UWORD code);
	void Addstringtotable();
	UWORD Getnextcode(char *currstrip, int *bitoffset);
	};

#endif /* UNLZW_H */

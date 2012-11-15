#include <Windows.h>
#include <stdio.h>

void DebugOutput(char *format, ... )
{	char Temp[4096];
	va_list marker;
	va_start(marker,format);
		vsprintf(Temp,format,marker);
		OutputDebugString(Temp);
		//APIDebugOutput(Temp);
	va_end(marker); 
}

HANDLE OpenWriteSeq(char *filename,bool preview=false) {
	if (!preview) {
		HANDLE hf;
		hf=CreateFile(filename,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (hf==INVALID_HANDLE_VALUE) hf=(void *)-1;
		return (hf);
		}
	else {
		DebugOutput("\nOutPut:\n\n");
		return NULL;
		}
	}


int myWrite(HANDLE hf,byte *buf,unsigned long count,bool preview=false) {
	if (!preview) {
		DWORD bytes_read;
		if (!(WriteFile(hf,buf,count,&bytes_read,NULL))) bytes_read=-1;
		return ((int)bytes_read);
		}
	else {
		static unsigned long CarriageReturnCount=0;
		unsigned index=0;
		if (index<count) {
			do {
				DebugOutput("%.2x ",buf[index]);
				CarriageReturnCount++;
				if (CarriageReturnCount==8) {
					DebugOutput("	");
					}
				if (CarriageReturnCount>=16) {
					DebugOutput("\n");
					CarriageReturnCount=0;
					}
				} while(++index<count);
			}
		return count;
		}
	}

int myClose(HANDLE hf,bool preview=false) {
	if (!preview) {
		int value=-1;
		if (CloseHandle(hf)) value=0;
		return (value);
		}
	else {
		DebugOutput("\n");
		return 0;
		}
	}


const char MedHeader[32]={
	0x4d,0x53,0x48,0x00, 0xff,0xff,0x00,0x00,	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x02,
	0x00,0x02,0x03,0x03, 0x00,0x01,0x40,0xFF,	0x00,0xFF,0x00,0x00, 0x00,0x1E,0x00,0x40 //last byte =128 samples
	};

double GetTrianglePatternSample(double x) {
	double y=0;
	if (x<0.5) {
		x*=2; //stretch it
		y=x;
		}
	else {
		x=x-0.5;
		x*=2; //stretch it
		x=1-x; //grabbing the distance
		y=x;
		}
	return y;
	}

double GetWavePatternSample(double x) {
	double sample;
	if (x<0.5) {
		x*=2; //stretch it
		sample=GetTrianglePatternSample(x);
		}
	else {
		x=x-0.5;
		x*=2; //stretch it
		sample=GetTrianglePatternSample(x);
		sample*=-1;  //this is the negative counterpart
		}
	return sample;
	}

bool MakeWaves(unsigned NumOfWaves,byte *destbuffer,unsigned long wavesize=128) {
	bool success=true;
	double subwavesize=(double)wavesize/(double)NumOfWaves;
	for (unsigned i=0;i<wavesize;i++) {
		//solving for current distance of a subwave size
		double subwavedistance=(((double)i+0.5)/subwavesize);
		unsigned currentwave=(unsigned)subwavedistance;
		subwavedistance-=currentwave;
		double sample=GetWavePatternSample(subwavedistance);
		
		//now to convert the sample to a (128 signed byte)
		byte Bytesample=(byte)(sample*128.0); //but cast it to an unsigned byte
		//DebugOutput("%d %f %f %d\n",currentwave,subwavedistance,sample,Bytesample);
		destbuffer[i]=Bytesample;
		}
	return success;
	}

void main() {
	bool preview=false;
	char *name="Synth";
	char *ext=".dat";
	char filename[MAX_PATH];

	for (unsigned i=0;i<32;i++) {
		sprintf(filename,"%s%.2x%s",name,i+1,ext);
		HANDLE hf=OpenWriteSeq(filename,preview);
		if (hf!=(void *)-1) {
			//we'll start with the header
			myWrite(hf,(byte *)MedHeader,32,preview);
			byte buffer[128];
			MakeWaves(i+1,buffer,128);
			myWrite(hf,buffer,128,preview);
			myClose(hf,preview);
			}
		}
	}

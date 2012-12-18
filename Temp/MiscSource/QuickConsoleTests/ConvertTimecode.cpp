#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h> //added to compile for NT

bool ConvertSecondstoTimecodeString(char *Timecode,double seconds,double num=30000.0,double den=1001.0) {
	unsigned hrs=0,mins=0,sec;
	double dec;
	unsigned frames;

	if (seconds<0) {
		Timecode[0]='-';
		Timecode++;
		seconds=-seconds;
		}

	sec=(unsigned int)seconds;
	dec=seconds-sec;
	if (seconds>=60) {
		mins=sec/60;
		sec-=mins*60;
		if (seconds>=3600) {
			hrs=mins/60;
			mins-=hrs*60;
			}
		}
	frames=(unsigned int)(dec*num/den);
	//TODO decrease the frames by 1 for out point
	sprintf(Timecode,"%2d:%2d:%2d;%2d",hrs,mins,sec,frames);
	if (Timecode[0]==' ') Timecode[0]='0';
	if (Timecode[3]==' ') Timecode[3]='0';
	if (Timecode[6]==' ') Timecode[6]='0';
	if (Timecode[9]==' ') Timecode[9]='0';
	//Success
	return true;
	}
/*
void main() {
	char string[16];
	float value;
	bool again;
	while (again) {
		again=false;
		printf("Enter seconds value (a float)... \n");
		scanf("%f",&value);
		ConvertSecondstoTimecodeString(string,(double)value);
		printf("time=%s\n",string);
		printf("Continue (Y/N) ?\n");
		scanf("%s",&string);
		if ((string[0]=='y')||(string[0]=='Y')) again=true;
		}
	}
*/

bool ConvertTimecodeStringtoSeconds(char *Timecode,double &seconds,double num=30000.0,double den=1001.0) {
	unsigned TimeSlot[4][2];
	unsigned field=0;
	int i,j=1;

	memset(TimeSlot,0,8*sizeof(int));
	//check for negative
	bool negative=false;
	if (Timecode[0]=='-') {
		negative=true;
		Timecode++;
		}

	for (i=strlen(Timecode)-1;i>=0;i--) {
		if (field>3) break;
		if ((Timecode[i]>'9')||(Timecode[i]<'0')) { //Delimiter detected
			if (j==0) j--;
			}
		else {
			TimeSlot[field][j]=Timecode[i]-'0';
			j--;
			}
		if (j<0) {
			field++;
			j=1;
			}
		}

	seconds=(TimeSlot[3][0]*10+TimeSlot[3][1])*3600;
	seconds+=(TimeSlot[2][0]*10+TimeSlot[2][1])*60;
	seconds+=TimeSlot[1][0]*10+TimeSlot[1][1];
	//TODO for end point advance this value by 1
	seconds+=((TimeSlot[0][0]*10+TimeSlot[0][1])*den)/num;
	if (negative) seconds=-seconds;
	//Success
	return true;
	}

void main() {
	char string[16];
	double value;
	bool again;
	while (again) {
		again=false;
		printf("Enter Timecode [-]00:00:00;00... \n");
		scanf("%s",&string);
		ConvertTimecodeStringtoSeconds(string,value);
		printf("time=%f\n",value);
		printf("Continue (Y/N) ?\n");
		scanf("%s",&string);
		if ((string[0]=='y')||(string[0]=='Y')) again=true;
		}
	}


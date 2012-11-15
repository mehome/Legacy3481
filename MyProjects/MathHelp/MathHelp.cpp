#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

void  NewTek_free(void* Ptr)
{	free(Ptr);
}


char *NewTek_malloc(const char *Mem) 
{	if (!Mem) return NULL;
	unsigned CharLength=strlen(Mem)+1;
	char *Temp=(char*)malloc(CharLength);
	if (!Temp) return NULL;
	memcpy(Temp,Mem,CharLength);
	return Temp;
}

void *NewTek_malloc(unsigned int Size)
{	return malloc(Size);
}

void *NewTek_realloc(void *Ptr,unsigned int Size)
{	return realloc(Ptr,Size);
}

#include "ListTemplate.h"


void reverse(register char *s) {
	long c,i,j;
	for (i=0,j=strlen(s)-1;i<j;i++,j--)  {
		c=s[i];
		s[i]=s[j];
		s[j]=(char)c;
		}
	}

long Hex2Long (char *string) {
	long value;
	//convert string to a long value
	char *hexstring=strrchr(string,'x');
	//todo check for zero before x
	if (hexstring) {
		value=0;
		unsigned index=1;
		char Digit;
		while (Digit=hexstring[index++]) {
			value=value<<4;
			Digit-='0';
			if (Digit>16) Digit-=7;
			if (Digit>16) Digit-=32;
			value+=Digit;
			}
		}
	else
		value=atol(string);
	return value;
	}

void Hex2Ascii (char *string) {
	long value=Hex2Long(string);

	//convert long value to ascii
	char buffer[4];
	memset(buffer,' ',4);
	unsigned index=0;
	char asciichar;
	while (value) {
		asciichar=value&0xff;
		value=value>>8;
		buffer[index++]=asciichar;
		}
	printf("%s\n",buffer);
	reverse(buffer);
	printf("%s\n",buffer);
	}



void reduceby2(int *num,int *den) {
		__asm {
			mov		ecx,num
			mov		eax,[ecx]
			mov		ecx,den
			mov		edx,[ecx]
			mov		ecx,32
loop2:
			test		eax,1
			jne			bitset
			test		edx,1
			jne			bitset
			shr		eax,1
			shr		edx,1
			loopnz	loop2
			//overflow exit
			jmp		error
bitset:
			//Here the either of the values has bit one set
			//save them back
			mov		ecx,eax
			mov		eax,num
			mov		[eax],ecx
			mov		ecx,edx
			mov		edx,den
			mov		[edx],ecx
			jmp		done
error:
			mov		eax,num
			mov		[eax],0
			mov		edx,den
			mov		[edx],0
done:
			}

	}

void reduce(int *num,int *den) {
doitagain:
	reduceby2(num,den);
	//check for 3
	if (*num%3==0) {
		if (*den%3==0) {
			*den/=3;*num/=3;
			goto doitagain;
			}
		}
	//check for 5
	if (*num%5==0) {
		if (*den%5==0) {
			*den/=5;*num/=5;
			goto doitagain;
			}
		}
	}

bool SortCondition(int compvar,int scanvar) {
	return (compvar<scanvar);
	}

void InsertionSort(tList<int> &List) {
	int n=List.NoItems;
	int i,j;
	int a;

	for (j=2;j<=n;j++) {
		a=List[j-1];
		i=j-1;
		while (i>0 && SortCondition(a,List[i-1])) {
			List[i]=List[i-1];
			i--;
			}
		List[i]=a;
		}
	}



void PrimeFactorization(tList<int> &Factors,int val) {
	int Prime[10] = { 2,3,5,7,11,13,17,19,23,29};  //this should be plenty
	while (val>1) {
		//a slight speed increase will be to check for bit 0
		if ((val & 1)==0) {
			Factors.Add(2);
			val>>=1;
			continue;
			}
		for (unsigned i=1;i<10;i++) {
			if ((val % Prime[i])==0) {
				Factors.Add(Prime[i]);
				val/=Prime[i];
				break;
				}
			}
		if (i==10) {
			//If it has made it this far, then chances are it is a prime number (or a possible multiple of a higher prime)
			Factors.Add(val);
			break;
			}
		}
	InsertionSort(Factors);
	}

bool Sqrt(int val,int &result,int &radicalrem) {
	tList<int> Factors;
	PrimeFactorization(Factors,val);

	//consolidate matching pairs against ones which are not.
	tList<int> ResultList,RadicalList;

	for (unsigned i=0;i<Factors.NoItems;i++) {
		if (i < Factors.NoItems-1) {
			if (Factors[i]==Factors[i+1]) {
				ResultList.Add(Factors[i]);
				i+=1; 
				}
			else {
				RadicalList.Add(Factors[i]);
				}
			}
		else
			RadicalList.Add(Factors[i]);
		}

	//Now to get the get the products
	result=1;
	radicalrem=1;

	for (i=0;i<ResultList.NoItems;i++) 
		result*=ResultList[i];

	for (i=0;i<RadicalList.NoItems;i++) 
		radicalrem*=RadicalList[i];

	return true;
	}

int NumberOfTimes(tList<int> &Factors,int val) {
	int result=0;
	for (unsigned i=0;i<Factors.NoItems;i++) {
		if (Factors[i]==val)
			result++;
		}
	return result;
	}

int LeastCommonMultiple(int val1,int val2) {
	int result=1;
	tList<int> Factors1,Factors2,usedlist;
	PrimeFactorization(Factors1,val1);
	PrimeFactorization(Factors2,val2);
	for (unsigned i=0;i<Factors1.NoItems;i++) {
		int Value=Factors1[i];
		if (usedlist.Exists(Value))
			continue;
		//assert this number has not been used yet
		usedlist.Add(Value);
		int A,B;
		A=NumberOfTimes(Factors1,Value);
		B=NumberOfTimes(Factors2,Value);
		A=max(A,B); //A=the greatest number of times it has been used
		for(int j=0;j<A;j++)
			result*=Value;
		}
	//Now to find any factor which was not found from factors1 list
	for (i=0;i<Factors2.NoItems;i++) {
		int Value=Factors2[i];
		if (usedlist.Exists(Value))
			continue;
		usedlist.Add(Value);
		//since there is no match from factors 1 we can automatically factor these in
		int NumTimes=NumberOfTimes(Factors2,Value);
		for(int j=0;j<NumTimes;j++)
			result*=Value;
		}

	return result;
	}


void quad(int a,int b,int c,double &no1,double &no2) {
	no1=((-1*b)+sqrt((b*b)-(4*a*c)))/(2*a);
	no2=((-1*b)-sqrt((b*b)-(4*a*c)))/(2*a);
	}


bool main (int argc,char **argv) {
	static char input_line[128];

	while (gets(input_line)) {
		static char		command[32];
		static char		str_1[64];
		static char		str_2[64];
		static char		str_3[64];
		static char		str_4[64];

		static char		g_char = 0;

		command[0] = '\0';
		str_1[0] = '\0';
		str_2[0] = '\0';
		str_3[0] = '\0';
		str_4[0] = '\0';

		if (sscanf( input_line,"%s %s %s %s %s",command,str_1,str_2,str_3,str_4)>=1) {

			if (!strnicmp( input_line, "Hex2Ascii", 9 )) {
				//file_test( rtc->rtc_contig_smp, str_1, param_2, param_3, _BOING_CREATE );
				Hex2Ascii(str_1);
				}
			else if (!strnicmp( input_line, "Reduce", 6)) {
				int num=Hex2Long(str_1);
				int den=Hex2Long(str_2);
				reduce(&num,&den);
				printf("%d/%d\n",num,den);
				}
			else if (!strnicmp( input_line, "Sqrt", 4)) {
				int result=0;
				int radical=0;
				Sqrt(Hex2Long(str_1),result,radical);
				printf("%d v~ %d\n",result,radical);
				}
			else if (!strnicmp( input_line, "LCM", 3)) {
				int result=LeastCommonMultiple(Hex2Long(str_1),Hex2Long(str_2));
				printf("%d\n",result);
				}
			else if (!strnicmp( input_line, "QuadHelp", 8)) {
				printf("                                2\n");
				printf("Used to define x in the form  Ax + Bx + C=0\n");
				printf("using the quadratic equation:\n\n");
				printf("          _________ \n");
				printf("     +   /  2 +     \n");
				printf("  -b - \\/  b  - 4ac\n");
				printf("--------------------------\n"); 
				printf("         2a\n\n\n");
				printf("        2\n");
				printf("(ex = 4x - 16x + 4 = 0)\n\n");
				printf("SYNTAX:\n");
				printf("\tquadform [a] [b] [c]\n");
				printf("where a,b,c are integers, and a!=0\n");
				}
			else if (!strnicmp( input_line, "Quad", 4)) {
				int a=Hex2Long(str_1);
				int b=Hex2Long(str_2);
				int c=Hex2Long(str_3);

				double answer1,answer2;
				quad(a,b,c,answer1,answer2);
				printf("\n In the equation:\n");
				printf("\t%dx^2 + %dx + %d = 0\n",a,b,c);
				printf("x = %f , %f\n\n",answer1,answer2);
				}
			else if (!strnicmp(input_line,"help",4)) {
				//"mkfile <filename> [filesize_MB] [bufsize_kB] - create file\n"
				printf(
					"Reduce Num,Den \n"
					"Sqrt Int \n"
					"LCM Int Int  (Least common multiple) \n"
					"QuadHelp \n"
					"Quad a b c \n"
					"Hex2Ascii <hexnumber|decimal> \n"
					"quit \n"
					);
					}
			else if (!strnicmp( input_line, "quit", 4)) {
				break;
				}
			else {
				printf("huh? - try \"help\"\n");
				}
			}
		}
	return(0);
	}

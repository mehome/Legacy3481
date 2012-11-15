  /***************************************************************/
 /*								Security										 */
/***************************************************************/
#include "Handlemain.h"

BOOL securityclass::pass() {
	static char encodedkey[sizeof(struct keycontents)];
	char *tempname="Terminator";
	char *tempencodekey=";=EJ%7rFt,NDs"; //Terminator
	//char *tempname="Exodus Multimedia";
	//char *tempencodekey="E@9-$<@'/3@P5"; //Exodus Multimedia my machine
	//char *tempencodekey="E99-$<@'/#4P5"; //Exodus Multimedia Stan's machine

	struct keycontents *key=(struct keycontents *)newnode(nodeobject,sizeof(struct keycontents));
	BOOL result;
	//Name and encodekey will be hard coded later
	strcpy((char *)encodedkey,tempencodekey);
	strcpy((char *)username,tempname);

	strcpy((char *)key,(char *)encodedkey);
	encrypt(username,(char *)key,TRUE);
	encrypt(username,(char *)key,TRUE);
	result=(strcmp(key->piiiserial,getpiiiserialnumber())==0);
	clrmem((char *)key,sizeof(struct keycontents));
	disposenode(nodeobject,(struct memlist *)key);
	return (result);
	}

char *securityclass::getpiiiserialnumber() {
	char *number=(char *)newnode(nodeobject,24);
	ULONG ispiii,first,middle,last;
	__asm{
		mov eax,1
		CPUID
		mov [first],eax
		mov [ispiii],edx
		mov eax,3
		CPUID
		mov [middle],edx
		mov [last],ecx
		}
	if (ispiii&(18<<2)) {
		wsprintf(number,"%lx,%lx,%lx",first,middle,last);
		}
	else number[0]=0;
	return (number);
	}


void securityclass::encrypt(char *passwordkey,char *mem,UBYTE recover) {
	struct nodevars *cryptnode;
	char *pww[50];
	char keycheck[100];
	char code[200];
	char decode[100];
	ULONG size=strlen(mem);
	ULONG index;
	UWORD b;
	UBYTE bulk,remain,pwlen,x,y,t;
	char a;

	cryptnode=createnode(&pmem,2048,0);
	/*-----------------------Make Code--------------------*/
	for (t=0;t<100;t++) keycheck[t]=0;
	//printf("Encrypting %lx size %ld\n",mem,size);
	pwlen=strlen(passwordkey);
	for (t=0;t<50;t++) pww[t]=(char *)newnode(cryptnode,(ULONG)pwlen);
	strcpy(pww[0],passwordkey);
	bulk=95/pwlen;
	remain=95 % bulk;
	//printf("pww %lx,size %d,bulk %d,remain %d\n",pww[0],pwlen,bulk,remain);
	//printf("keycheck[]->%lx\n",&keycheck);
	for (y=1;!(y>bulk);y++) {
		strcpy(pww[y],pww[y-1]);
		//printf("line %lx->",pww[y]);
		for (x=0;x<pwlen;x++) {
			a=*(pww[y]+x);
			while(keycheck[a-32]) {
				if (x % 2) a++;
				else a--;
				if (a>=126) a=32;
				else if (a<=32) a=126;
				}
			keycheck[a-32]=1;
			//printf("%c",a);
			*(pww[y]+x)=a;
			}
		//printf(",\n");
		}
	//printf("last %lx->",pww[y]);
	for(x=0;x<pwlen;x++) *(pww[y]+x)=0; /*initialize remainder*/
	for(x=0,t=0;t<95;t++) { /* search for remaining letters */
		if (!(keycheck[t])) {
			*(pww[y]+x)=t+32;
			//printf("%c",t+32);
			x++;
			}
		}
	//if (x==remain) printf("\nChecksum Good %d %d\n",x,remain);
	//else printf("\nChecksum Bad %d %d\n",x,remain);
	for (x=0,y=1;y<(bulk+2);y++) {
		transmembyte(pww[y],(char *)&code[x],pwlen);
		x+=pwlen;
		}
	//printf("Code->%lx,decode->%lx\n",&code[0],&decode[0]);
	for(t=0;t<95;t++) {
		decode[(code[t]-32)]=t+32;
		}
	/*------------------Convert Text---------------------------*/
	if (!(recover)) {  // precrypt
		//printf("Precrypt\n");
		for(t=1,index=0;index<size;t++,index++) {
			b=(UWORD)*(mem+index);
			if (b==10) {
				t=0;
				}
			else {
				b+=t;
				if (b>126) b-=95;
				a=(UBYTE)b;
				*(mem+index)=a;
				}
			}
		}
	//printf("Key code conversion\n");
	for(index=0;index<size;index++) {  // Key code conversion
		a=*(mem+index);
		if (!(a==10)) {
			if (!(recover)) a=code[a-32];
			else a=decode[a-32];
			//printf("%c",a);
			*(mem+index)=a;
			}
		//else printf("\n");
		}
	if (recover) {  // postcrypt
		//printf("Postcrypt\n");
		for(t=1,index=0;index<size;t++,index++) {
			a=*(mem+index);
			if (a==10) {
				t=0;
				}
			else {
				a-=t;
				if (a<32) a+=95;
				*(mem+index)=a;
				}
			}
		}		
	killnode(cryptnode,&pmem);
	} //end encrypt

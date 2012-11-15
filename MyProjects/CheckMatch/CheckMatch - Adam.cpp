// CheckMatch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

struct patternNode
	{
		char string[64];
		bool match;
		patternNode *nxt;
	};

bool notMatch;
patternNode *startPtr= NULL;

void addNode(char newString[64])
{
	patternNode *temp, *temp2;
    temp= new patternNode;
	temp2= new patternNode;
    strcpy(temp->string, newString);
    temp->nxt= NULL;
    if (startPtr== NULL)
		startPtr= temp;
    else
	{ 
		temp2= startPtr;
		while (temp2->nxt!= NULL)
			temp2= temp2->nxt;
    }
        temp2->nxt= temp;
}

void clearList()
{
	while(startPtr)
	{
		patternNode *Current=new patternNode;
		*Current= *startPtr;
		startPtr= startPtr->nxt;
		delete Current;
	}
}

int check_match(char*string,char*pattern)
{
	char fs,fp;
    size_t i,j,k;
loop:
	if(!*pattern && !*string)
		return(1);
	fp=*pattern++;                                  // fp = "*" *pattern = "l*l*o"  *string = "ello"
	if(fp=='?')
	{
		string++;
		goto loop;
	}
	if(fp=='!')
		fp=*pattern++;
	if(!*pattern)
		return(1);
	while(*pattern =='?')
	{
		pattern++;
		string++;
	}
	j=strlen(pattern);
	k=strlen(string);
	if(!*(pattern+j))
	{
		if((strnicmp(string+k-j,pattern,j))==0)
			return(0);
		return(1);
	}
	if(j)
	{
		i=-1;
		while(strnicmp(string++,pattern,j))
		{
			i++;
			if(i==(k-1))
				return(0);
		}
	pattern=pattern+j;
	string=string+j-1;
	}
	goto loop;
	fs=*string++;
	if(fs && !fp)
		return(0);
	if(strnicmp(&fs,&fp,1))
		return(0);
	goto loop;
}

int buildList(char*string, char*pattern)
{
	char newPattern[64]="";
	patternNode *temp= new patternNode;
	for(int i=0;*pattern;)
	{
		if(*pattern==';')
		{
			addNode(newPattern);
			i=0;
			for(int j=0;j<7;j++)
				newPattern[j]=0;
			*pattern++;
		}
		else
		{
            newPattern[i]= *pattern;
			*pattern++;
			i++;
		}
	}
	if(newPattern[0]!= 0)
		addNode(newPattern);
	temp= startPtr;
	do
	{
		if(check_match(string, temp->string)==0)
			temp->match=true;
		else
			temp->match=false;
		temp= temp->nxt;
	}
	while(temp!= NULL);
	temp= startPtr;
	do
	{
		if((temp->match==true)&&(temp->string[0]=='!'))
			return 1;
		else if(temp->match==true)
			return 0;
		temp= temp->nxt;
	}
	while(temp!= NULL);
	return 1;
}

bool main (int argc,char **argv) 
{
		static char input_line[128];

	while (gets(input_line)) 
	{
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

		if (sscanf( input_line,"%s %s %s %s %s",command,str_1,str_2,str_3,str_4)>=1) 
		{
			if (!strnicmp( input_line, "Search", 6 )) 
			{
				if ((str_1)&&(str_2))
				{
					printf(" result is %d\n",buildList(str_1,str_2));
					clearList();
					}
				else
					printf("Need <string> and <pattern>\n");

			}
			else if (!strnicmp(input_line,"help",4)) 
			{
				//"mkfile <filename> [filesize_MB] [bufsize_kB] - create file\n"
				printf(
					"Search <string> <pattern>  \n"
					"quit    - quit this program\n"
					);
			}
			else if (!strnicmp( input_line, "quit", 4)) 
				break;
			else 
				printf("huh? - try \"help\"\n");
		}
	}
	return(0);
}
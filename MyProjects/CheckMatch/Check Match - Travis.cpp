// Check Match - Linked.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "list.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct patternNode	{
	char extension[64];		// name of file extension
	bool include;			//for exclusions: true==inclusion false==exclusion
	patternNode *nxt;	// pointer to next patternNode
};

patternNode *start_ptr = NULL;

void add_patternNode (char ext[64], bool in_ex)
  {  
     // initialize new patternNode
     patternNode *temp;		// Temporary pointer
	 patternNode *temp2;	//"					"

	 temp= new patternNode;
	 temp2= new patternNode;
     strcpy(temp->extension,ext);
	 temp->include=in_ex;
	 temp->nxt = NULL;

     // link new & previous patternNodes
     if (start_ptr == NULL)
         start_ptr = temp;
     else
       { temp2 = start_ptr;
         // We know this is not NULL - list not empty!
         while (temp2->nxt != NULL)
           {  temp2 = temp2->nxt;
         // Move to next link in chain
           }
         temp2->nxt = temp;
       }
  }

void clearList()	{
	while( start_ptr )
	{
		patternNode *Current=new patternNode;
		*Current = *start_ptr;
		start_ptr = start_ptr->nxt;
		delete Current;
	}
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

 void separate_extensions(char *source)	{
	int i=0,		// beginning index
		j=0;		// ending index
	int difference; // from beginnig index to present

	while(source[i+1])	{
		char dest[64]="";
		difference=i;

		for(j = i+1 ; *(source+j)!=';' && *(source+j) ; j++){}
	
		for( int index=0; index < strlen(dest); index++)
			dest[index]=0;

		for( int k; i < j ; i++)	{
			k = i - difference;
			dest[k]=source[i];
		}

	if(dest[0] == '!')	{
		 for(int a=0; a < strlen(dest) ; a++)	{
			 dest[a]=dest[a+1];
			 /*if(a+1==strlen(dest))
				 dest[a]=0;*/
		 }
		 add_patternNode(dest,false);
	 }
	else
		add_patternNode(dest,true);



		i++;		//increments i to procede past ';' in source


	}
}
int check_match(char*string,char*pattern)
{
	bool allExcluded=true, oneExtension=true;

		patternNode *temp = new patternNode;
		patternNode *node;

		separate_extensions(pattern);

		temp = start_ptr;

		char *check;

		/*while(temp)
			{
			printf("%s", temp->extension);
			temp=temp->nxt;
		}
			printf("\n");
			temp = start_ptr;*/

		check=&(*string);

		if((*string == 0) || (*string) == '.')
			return(7);

		/*if()
			return(5);*/

		if(*pattern == 0)	{
			for(;*string != 0;string++)
				if(*string == ';')
					return(7);
			return(6);
		}

        for( int q=0 ; (q < strlen(string)) ; q++, check++)
			if(*(check+1) == '.')
				break;
			else if(string[q+1]==0)
				return (4);

        int i;
		char *ext, *str;

loop:
		node=temp->nxt;

		if(node != NULL)
			oneExtension=false;

		if(temp->include==true)
			allExcluded=false;

		ext=&(*temp->extension);

		check=&(*temp->extension);

		if( (*check != '.') && (*(check+1) !='.'))
			return (5);

		if(!temp && !*string) return(1);		//Checks existence

		while(*ext != '.')
			ext++;

		str=&(*string);

		while (*str != '.')
			str++;

		str++;ext++;
		while ( *str && (*ext==*str))	{
			ext++;str++;
		}

		if( (*str == 0) && (*ext == 0) && temp->include==false)
			return(3);
		else if( (*str == 0) && (*ext == 0))
			return(0);
		else if( (allExcluded==true) && node==NULL )
			return(2);
		else if( temp->include==false && oneExtension==true )
			return (2);
		else if( temp->nxt )	{
			temp=temp->nxt;
			goto loop;
		}
		else
			return(1);

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
			if(str_2 == 0)
				printf("Must enter extension(s) to compare.\n\n");
			else if (!strnicmp( input_line, "Search", 6 )) 
			{
				if ((str_1)&&(str_2))	{
					switch(check_match(str_1,str_2))	{
						case 0:	printf("File matches.\n\n");break;
						case 1: printf("File does NOT match.\n\n");break;
						case 2: printf("File was INcluded.\n\n");break;
						case 3: printf("File was EXcluded.\n\n");break;
						case 4: printf("File must include extension.\n"
											" (ex: movie.avi  bitmap.bmp)\n\n");break;
						case 5: printf("Extensions must begin with a period(.) or asterisk(*).\n"
											"\t\t\t     (ex: .mpeg *.mpeg  !.jpg !*.jpg)\n\n");
								break;
						case 6: printf("Must enter extension(s) to compare.\n\n");break;
						case 7: printf("Must enter file name to compare.\n\n");break;
					}
				}

				else
					printf("Need <string> and <pattern>\n");

			}
			else if (!strnicmp(input_line,"help",4)) 
			{
				//"mkfile <filename> [filesize_MB] [bufsize_kB] - create file\n"
				system("CLS");
				printf(
					"Search <filename> <extension(s)>  \n"
					"---------------------------\n\n"
					"Files\t      - <name>.<ext> for files\t\t( ex: image.jpg )\n\n"
					"Extension(s)  -       .<ext> for extension(s)   ( ex: !.mpg   .zip )\n\n"
					";\t      - Use to separate extensions\n\n"
					"!\t      - Use before extensions to mark for exclusion\n\n"
					"---------------------------\n"
					"quit\t- Quit this program\n\n"
					);
			}
			else if (!strnicmp( input_line, "quit", 4)) 
				break;
			else 
				printf("huh? - try \"help\"\n\n");
		}

		clearList();

	}
	return(0);
}


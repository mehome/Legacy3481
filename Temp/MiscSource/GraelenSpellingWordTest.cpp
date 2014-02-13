
#include "stdafx.h"

using namespace std;

void main()
{
	char input_line[128];
	char		command[32];

	while (cout << ">",cin.getline(input_line,128))
	{
		command[0]=0;

		if (sscanf( input_line,"%s",command)>=1)
		{
			if (!_strnicmp( input_line, "Quit", 4))
				break;
			else
			{
				size_t Table[26]={
					1,5,3,4,1,7,5,2,1,8,
					3,2,4,2,1,6,9,3,2,2,
					1,8,6,9,7,9};
				size_t result=0;
				for (size_t i=0;i<strlen(command);i++)
					result+=Table[command[i]-'a'];
				printf("%d\n",result);
			}

		}
	}
}

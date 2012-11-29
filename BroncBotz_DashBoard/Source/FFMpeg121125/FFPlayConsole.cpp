#include "stdafx.h"
#include "../FrameWork/FrameWork.h"
#include "FrameGrabber.h"

#include "../FileSelectionLib/Common.h"

class DebugOut_Update : public FrameWork::Outstream_Interface
{
protected:
	virtual void process_frame(const FrameWork::Bitmaps::bitmap_bgra_u8 *pBuffer)
	{
		static size_t counter=0;
		printf ("\r %d received           ",counter++);
	}
};


class FFPlayTest
{
public:
	FFPlayTest();
	virtual ~FFPlayTest();
	//returns true to quit
	bool CommandLineInterface();
private:
	void DisplayHelp();
	void CloseResources();
	void OpenResources();

	DebugOut_Update m_TestOutDebug;
	FFPlay_Controller *m_Streamer;
};


using namespace std;

  /*******************************************************************************************************************************************/
 /*																	FFPlayTest																*/
/*******************************************************************************************************************************************/


FFPlayTest::FFPlayTest() : m_Streamer(NULL)
{
}

FFPlayTest::~FFPlayTest()
{
	CloseResources();
}

void FFPlayTest::DisplayHelp()
{
	DisplayFileSelectionHelp();
	printf(
		"Reset\n"
		"Start\n"
		"Flush\n"
		"End\n"
		"Transport Controls:\n"
		"SetRate <100=unity>\n"
		"Play (or Run) [rate=0 no change]\n"
		"Pause, Stop\n"
		"Seek <position (float seconds)> "
		"Help  (displays this)\n"
		"\nType \"Exit\" at anytime to exit to main\n"
		"Type \"Quit\" at anytime to exit this application\n"
		);
}

bool FFPlayTest::CommandLineInterface()
{
	size_t FileSelection=InitFileSelection();
	cout << endl;
	ShowCurrentSelection();
	cout << endl;
	DisplayHelp();
	cout << "Ready." << endl;

	char input_line[128];
	while (cout << "FFPlay >",cin.getline(input_line,128))
	{
		char		command[32];
		char		str_1[64];
		char		str_2[64];
		char		str_3[64];
		char		str_4[64];

		command[0]=0;
		str_1[0]=0;
		str_2[0]=0;
		str_3[0]=0;
		str_4[0]=0;

		if (FillArguments(input_line,command,str_1,str_2,str_3,str_4)>=1) 
		{
			if (HandleFileSelectionCommands(FileSelection,input_line,str_1))
				continue;
			else if (!_strnicmp( input_line, "Reset", 5))
			{
				CloseResources();
				OpenResources();
			}
			else if (!_strnicmp( input_line, "Black", 5))
			{
				#if 0
				StreamFormat::ResolutionInfo info=m_Format.GetResolutionInfo();
				int XRes=(int)info.XRes;
				int YRes=(int)info.YRes;
				bool IsProgressive=info.IsProgressive;
				int YResToUse=IsProgressive?YRes:YRes/2;

				using namespace FC3::video;
				message msg(message::data_format_ycbcr_422_u8,XRes,YResToUse);
				const size_t PixelByteSize=2;
				const int FieldSize_ =(int) (XRes*YResToUse*PixelByteSize);
				BlackField((PBYTE)msg.ycbcr()(),FieldSize_);
				msg.send(cwsz_VideoName);
				#endif
			}
			else if (!_strnicmp( input_line, "MemTest", 7))
			{
				if (m_Streamer)
					CloseResources();
				for (size_t i=0;i<100;i++)
				{
					OpenResources();
					CloseResources();
					printf("0x%lx       \r",i);
				}
				printf("Finished!\n");
			}
			else if (!_strnicmp( input_line, "Start", 5))
			{
				if (m_Streamer)
					CloseResources();
				OpenResources();
			}
			else if (!_strnicmp( input_line, "Flush", 5))
			{
				m_Streamer->Flush();
			}

			else if (!_strnicmp( input_line, "End", 4))
			{
				CloseResources();
			}
			else if (!_strnicmp( input_line, "SetRate", 4))
			{
				OpenResources();
				int rate=atoi(str_1);
				if (rate==0)
					rate=100;
				m_Streamer->SetRate(rate);
			}
			else if ((!_strnicmp( input_line, "Run", 3))||(!_strnicmp( input_line, "Play", 4)))
			{
				::Profile Test("Startup Time");
				OpenResources();
				if (m_Streamer)
				{
					//May want to look at master clock option
					//m_MasterClock->play();
					int rate=atoi(str_1);
					if (rate!=0)
						m_Streamer->SetRate(rate);
					int result=m_Streamer->Run();
					printf("result=%d\n",result);
					if (result==1)
						CloseResources();
				}
			}
			else if (!_strnicmp( input_line, "Pause", 5))
			{
				OpenResources();
				//m_MasterClock->pause();
				m_Streamer->Pause();
			}
			else if (!_strnicmp( input_line, "Stop", 4))
			{
				OpenResources();
				//m_MasterClock->pause();
				m_Streamer->Stop();
			}
			else if (!_strnicmp( input_line, "TestInit", 5))
			{
				//TODO 
			}

			else if (!_strnicmp( input_line, "Seek", 4))
			{
				OpenResources();
				double time=atof(str_1);
				if (m_Streamer)
				{
					//m_MasterClock->pause();
					//m_Streamer->Pause();
					m_Streamer->Seek(time,0,true);
				}
			}
			else if (!_strnicmp( input_line, "test", 4))
			{
				//TODO
			}
			else if (!_strnicmp( input_line, "Help", 4))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Exit", 4))
				break;
			else if (!_strnicmp( input_line, "Quit", 4))
				return true;
			else
				cout << "huh? - try \"help\"" << endl;
		}
	}
	return false;  //just exit
}

void FFPlayTest::CloseResources()
{
	delete m_Streamer;
	m_Streamer=NULL;
}
void FFPlayTest::OpenResources()
{
	if (m_Streamer) return;
	char2wchar(InputFileName);
	m_Streamer=new FFPlay_Controller(&m_TestOutDebug,char2wchar_pwchar);	
}

enum ConsoleApp
{
	eConsoleMain,
}	g_DefaultConsole=eConsoleMain;

int main(int argc, char **argv)
{
	char *DefaultFileName="FFPlayTest.dat";
	if (argc==2)
		DefaultFileName=argv[1];

	size_t l_BuildEnvironment;
	//Set up any global init here
	g_DefaultConsole=(ConsoleApp)LoadDefaults(DefaultFileName,&l_BuildEnvironment);

	InitFileSelection();

	if (g_DefaultConsole==eConsoleMain)
	{
		FFPlayTest streamtester; //a portal to a new command line interface
		if (streamtester.CommandLineInterface())
			goto SaveOnExit;
		else
			g_DefaultConsole=eConsoleMain;
	}

	#if 0
	DisplayHelp();
	ShowFileSelections();
	CommandLineInterface();
	#endif
SaveOnExit:
	SaveDefaults(DefaultFileName,(size_t)g_DefaultConsole,0);

}

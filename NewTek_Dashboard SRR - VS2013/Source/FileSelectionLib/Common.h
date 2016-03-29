#pragma once

size_t InitFileSelection();
bool HandleFileSelectionCommands(size_t &FileSelection,const char *input_line,const char *str_1);
void cls(HANDLE hConsole=NULL);
long Hex2Long (const char *string);
void ShowCurrentSelection();
void ShowFileSelections();
void DisplayFileSelectionHelp();
void SetGroupSelection(std::string GroupID,size_t Selection);
size_t LoadDefaults(const char *DefaultFileName,size_t *BuildEnvironment=NULL);
void SaveDefaults(const char *DefaultFileName,size_t DefaultConsole,size_t BuildEnvironment=0);

size_t split_arguments(const std::string& str, std::vector<std::string>& arguments); //this one is ideal
size_t FillArguments(const char *input_line,char *command,char *str_1,char *str_2,char *str_3,char *str_4); //this one supports legacy way

extern const char *InputFileName;
extern const char *c_outpath;

class Profile
{
	public:
		Profile()
		{
			init();
		}
		Profile(const char *label)
		{
			init();
			m_Label=label;
		}
		virtual ~Profile()
		{
			stop();
		}
		void start()
		{
			QueryPerformanceCounter((LARGE_INTEGER *)&m_bf);
		}
		void stop()
		{
			QueryPerformanceCounter((LARGE_INTEGER *)&m_at);
			printf("%sTime %d\n",m_Label?m_Label:"",((m_at-m_bf)*1000)/m_freq);
		}
		void chop()
		{
			stop();
			start();
		}
	private:
		void init()
		{
			m_Label=NULL;
			QueryPerformanceFrequency((LARGE_INTEGER *)&m_freq);
			start();
		}
		__int64 m_bf,m_at,m_freq;
		const char *m_Label;
};

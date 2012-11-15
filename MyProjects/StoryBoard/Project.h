class projectclass {
	public:
	char projectname[MAX_PATH];
	BOOL isstartedbyproject;

	projectclass();
	//Project stuff
	int getsize(class storyboardclass *storyboard);
	char *getopenfilename(char *inputprompt,BOOL musthave);
	char *getsavefilename(char *inputprompt);
	char *imagetotext(class storyboardclass *storyboard,int *actualsize);
	int projectopen(class storyboardclass *storyboard,char *filename);
	int projectsave(class storyboardclass *storyboard,char *filename);
	void projectclose(class storyboardclass *storyboard);
	void projectnew(class storyboardclass *storyboard);
	void copythumbtoimage(struct imagelist *imageindex,HBITMAP hbm,BITMAPINFOHEADER *bitmapinfo);

	private:
	char openstartpath[MAX_PATH];
	char projectfilename[32];

	int openimage(class storyboardclass *storyboard,struct imagelist **imageprev,char **memindex,int *totalframes,float *linenumber);
	};

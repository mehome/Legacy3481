
class securityclass {
	public:
	//Security stuff
	BOOL pass();
	void encrypt(char *passwordkey,char *mem,UBYTE recover);

	private:
	char *getpiiiserialnumber();
	};

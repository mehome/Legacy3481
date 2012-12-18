#ifndef TESTCLASS_H
#define TESTCLASS_H

class compclass {
	public:
	short testthis;
	class baseclass {
		public:
		baseclass();
		virtual ~baseclass();
		baseclass *next;
		baseclass *prev;
		virtual long getspecs()=0;
	};

	class firstclass : public baseclass{
		public:
		firstclass();
		~firstclass();
		void test();
		void test2();
		long getspecs();
	} obj1;

	class secondclass : public baseclass{
		public:
		secondclass();
		~secondclass();
		void test();
		long getspecs();
	} obj2;

	compclass();
	~compclass();
	void funcptrtest(int parm);
	void test();

	};

#endif /*Test Class.h*/

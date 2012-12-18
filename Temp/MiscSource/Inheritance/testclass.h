#ifndef TESTCLASS_H
#define TESTCLASS_H

struct uniontest {
	long image;
	union {
		short actualframes;
		short duration;
		};
	long totalframes;
	};

class baseclass {
	public:
		baseclass();
		void funcptrtest(int parm);
		void test();
		~baseclass();

	protected:
		short testthis;
	};

class firstclass : public baseclass {
	public:
		firstclass();
		void test();
		void test2();
		~firstclass();
	};

class secondclass : public baseclass {
	public:
		secondclass();
		void test();
		~secondclass();
	};

extern secondclass test2;

#endif /*Test Class.h*/

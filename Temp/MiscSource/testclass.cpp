/*Test Class.cpp Here is a class */
#include <stdafx.h>
#include "testclass.h"

/*
long compclass::baseclass::getspecs() {
	return (0);
	}
*/
compclass::baseclass::baseclass() {
	}

compclass::baseclass::~baseclass() {
	}

long compclass::firstclass::getspecs() {
	return (1);
	}

long compclass::secondclass::getspecs() {
	return (2);
	}

compclass::firstclass::firstclass() : baseclass() {
	}


void compclass::firstclass::test() {
	printf("firstclass.test\n");
	//testthis=4;
	}

void compclass::firstclass::test2() {
	//printf("test2 = %d\n",testthis);
	printf("FirstClass.Test2\n");
	}

compclass::firstclass::~firstclass() {
	}

/* NEXT CLASS here! */

compclass::secondclass::secondclass() : baseclass() {
	}


void compclass::secondclass::test() {
	//testthis=9;
	//printf("secondclass.test %d\n",testthis);
	}


compclass::secondclass::~secondclass() {
	}

compclass::compclass() {
	}

void compclass::funcptrtest(int parm) {
	printf("funcptr worked! %d\n",parm);
	}

void compclass::test() {
	/*
	void (compclass::*funcptr)(int);
	funcptr=funcptrtest;
	(this->*funcptr)(6);
	obj1.test2();
	*/
	baseclass *general;
	firstclass first,*test;
	secondclass second;

	general=&first;
	printf("first %ld\n",general->getspecs());
	general=&second;
	printf("second %ld\n",general->getspecs());
	test=new firstclass();
	printf("test %ld\n",test->getspecs());
	delete test;
	}

compclass::~compclass() {
	}

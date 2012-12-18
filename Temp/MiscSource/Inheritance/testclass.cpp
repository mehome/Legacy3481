/*Test Class.cpp Here is a class */
#include <stdafx.h>
#include "testclass.h"

firstclass::firstclass() {
	}


void firstclass::test() {
	printf("firstclass.test\n");
	testthis=4;
	}

void firstclass::test2() {
	printf("test2 = %d\n",testthis);
	}

firstclass::~firstclass() {
	}

/* NEXT CLASS here! */

secondclass::secondclass() {
	}


void secondclass::test() {
	testthis=9;
	printf("secondclass.test %d\n",testthis);
	}


secondclass::~secondclass() {
	}

baseclass::baseclass() {
	}

void baseclass::funcptrtest(int parm) {
	printf("funcptr worked! %d\n",parm);
	}

void baseclass::test() {
	void (baseclass::*funcptr)(int);
	funcptr=funcptrtest;
	(this->*funcptr)(6);
	}

baseclass::~baseclass() {
	}

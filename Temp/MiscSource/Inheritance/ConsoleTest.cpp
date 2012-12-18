// Consoletest.cpp : Defines the entry point for the console application.
//

#include <stdafx.h>
#include <math.h>
#include "testclass.h"

int main(
	int argc, 
	char **argv)
	{
	baseclass test;
	secondclass test2;
	struct uniontest imagelisthead;

	test.test();
	//test2.test();
	//test.test2();
	imagelisthead.totalframes=9;
	imagelisthead.actualframes=5;
	imagelisthead.duration=6;
	return(0);
	}


#include <windows.h>

#ifndef hr_timer
#include "hr_time.h"
#define hr_timer
#endif

void startTimer(stopWatch *timer) {
	QueryPerformanceCounter(&timer->start);
}

void stopTimer(stopWatch *timer) {
	QueryPerformanceCounter(&timer->stop);
}

double LIToMSecs(LARGE_INTEGER * L) {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return (((double)L->QuadPart / (double)frequency.QuadPart)) * 1000.0;
}

double getElapsedTime(stopWatch *timer) {
	LARGE_INTEGER time;
	time.QuadPart = timer->stop.QuadPart - timer->start.QuadPart;
	return LIToMSecs(&time);
}


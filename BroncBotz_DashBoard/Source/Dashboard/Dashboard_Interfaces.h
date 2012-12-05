#pragma once

class Dashboard_Controller_Interface
{
public:
	virtual void Flush()=0;
	virtual int Run (void)=0;											// run the filter graph
	virtual int Stop (void)=0;										// stop filter graph
	virtual int Pause (void)=0;										// pause filter graph
	virtual int Seek (double, double,  bool scrubbing =false)=0;		// seek to start/stop positions (in seconds)
	virtual int SetRate (int)=0;										// set the play speed  (as percentage of normal)
};

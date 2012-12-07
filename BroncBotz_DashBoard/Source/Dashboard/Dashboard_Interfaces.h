#pragma once
//Here are the ranges for each
//brightness	-1.0 - 1.0
//contrast		 0.0 - 1.0
//hue		  -180.0 - 180.0
//saturation     0.0 - 1.0
//u_offset		-1.0 - 1.0
//v_offset		-1.0 - 1.0
//u_gain		 0.0 - 1.0
//v_gain	 	 0.0 - 1.0


enum ProcAmp_enum
{
	e_procamp_brightness,
	e_procamp_contrast,
	e_procamp_hue,
	e_procamp_saturation,
	e_procamp_u_offset,
	e_procamp_v_offset,
	e_procamp_u_gain,
	e_procamp_v_gain,
	e_procamp_pedestal,
	e_no_procamp_items
};

class Dashboard_Controller_Interface
{
public:
	virtual void Flush()=0;
	virtual int Run (void)=0;											// run the filter graph
	virtual int Stop (void)=0;										// stop filter graph
	virtual int Pause (void)=0;										// pause filter graph
	virtual int Seek (double, double,  bool scrubbing =false)=0;		// seek to start/stop positions (in seconds)
	virtual int SetRate (int)=0;										// set the play speed  (as percentage of normal)

	virtual bool Set_ProcAmp(ProcAmp_enum ProcSetting,double value)=0;
};

#pragma once

#undef __2011_TestCamera__

class FrameProcessing
{
	public:
		typedef unsigned char BYTE;
		enum TestChorminance
		{
			eTestPb,
			eTestPr
		}; 
	
		struct XZ_Offset
		{
			double X,Z;
			bool IsError;  //true if error detected
		};
	
		void operator()(HSLImage &hsl_image,TestChorminance color=eTestPr,double AspectRatio=4.0/3.0);
	protected:
		void TestMinMax(MonoImage &image,BYTE &TestMin,BYTE &TestMax);

	private:
		//TestChorminance m_TestChorminance;
		HSLImage *m_TempMsg; //for temp testing;
		TestChorminance m_ColorToTest;
		size_t m_Threshold;
		KalmanFilter m_Dx,m_Dz,m_Ax,m_Az;
		double m_AspectRatio;
		int m_XRes,m_YRes;
};

class FRC_2012_CameraProcessing
{
	public:
		FRC_2012_CameraProcessing();
		~FRC_2012_CameraProcessing();
		void CameraProcessing_TimeChange(double dTime_s);

	private:
		AxisCamera *m_Camera;  //This is a singleton, but treated as a member that is optional
		FrameProcessing m_FrameProcessor;
		double m_LastTime;  //Keep track of frame rate
		int m_Xres,m_Yres;
};

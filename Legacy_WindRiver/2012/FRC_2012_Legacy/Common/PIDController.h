#pragma once

class KalmanFilter
{
	public:
		KalmanFilter();
		/// \return the filtered value
		double operator()(double input);
		void Reset();
	private:
	    //initial values for the kalman filter
	    double m_x_est_last;
	    double m_last;
	    //the noise in the system
	    const double m_Q;
	    const double m_R;
	    bool m_FirstRun; //This avoids a stall when first starting
};

/// This manages a PID control loop.  This was originally written for First WPI library, but refactored to be non-threaded, where both input and output
/// can be called on within this class.  It manages the integral calculations, and provides the PIDOutput
class PIDController2
{
	public:
		PIDController2(
			double p,					///< proportional coefficient
			double i,					///< integral coefficient
			double d,					///< derivative coefficient
			double maximumOutput=1.0,	///< maximum output
			double minimumOutput=-1.0,	///< minimum output|
			double maximumInput=1.0,	///< maximum input - limit setpoint to this
			double minimumInput=-1.0,	///< minimum input - limit setpoint to this
			double m_tolerance=.05,		///<the percentage error that is considered on target
			bool continuous=false,		///< do the endpoints wrap around? eg. Absolute encoder
			bool enabled=false	 		///< If client knows all the above, set to true; otherwise enable use Enable() for late binding
			);
		~PIDController2();

		///This is the main method which performs the computations, and must be called for each time slice
		/// \return the final output result
		/// \param setpoint is the actual desired position.  \note The range of this must be managed by client code!
		/// \param input is the actual position where it is
		/// \param dTime_s is the slice of time for this call
		double operator()(double setpoint,double input,double dTime_s);

		///	This is always centered on zero and constrained the the max and min outs
		///	\return the latest calculated output
		double Get();

		///Set the PID controller to consider the input to be continuous, rather than using the max and min in as constraints, it considers them to
		///be the same point and automatically calculates the shortest route to the setpoint.
		///	\param continuous Set to true turns on continuous, false turns off continuous
		void SetContinuous(bool continuous = true);

		void SetInputRange(double minimumInput, double maximumInput);
		void SetOutputRange(double mimimumOutput, double maximumOutput);
		void SetPID(double p, double i, double d);
		double GetP();
		double GetI();
		double GetD();

		double GetError();  ///< Returns the current difference of the input from the setpoint

		///Set the percentage error which is considered tolerable for use with OnTarget.
		///	\param percent percentage of error which is tolerable
		void SetTolerance(double percent);

		/// \return true if the error is within the percentage of the total input range,
		/// determined by SetTolerance. 
		/// \note this uses the input range therefore ensure it has been set properly
		bool OnTarget();
		
		void Enable();
		void Disable();

		///Resets the previous error, the integral term  (This does not disable as it does in the WPI lib)
		void Reset();
		/// A quick call often function that zero's the I only
		void ResetI();
		void ResetI(double totalError);  //allow client to set to specific value
	private:
		double m_P;			// factor for "proportional" control
		double m_I;			// factor for "integral" control
		double m_D;			// factor for "derivative" control
		double m_maximumOutput;	// |maximum output|
		double m_minimumOutput;	// |minimum output|
		double m_maximumInput;		// maximum input - limit setpoint to this
		double m_minimumInput;		// minimum input - limit setpoint to this
		double m_prevError;	// the prior sensor input (used to compute velocity)
		double m_totalError; //the sum of the errors for use in the integral calculation
		double m_tolerance;	//the percentage error that is considered on target
		double m_error;
		double m_result;
		bool m_continuous;	// do the endpoints wrap around? eg. Absolute encoder
		bool m_enabled;		//is the pid controller enabled
};

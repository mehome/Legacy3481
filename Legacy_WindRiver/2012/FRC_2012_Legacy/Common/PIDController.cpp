#include <math.h>
#include "PIDController.h"

  /***********************************************************************************************************/
 /*												KalmanFilter												*/
/***********************************************************************************************************/

void KalmanFilter::Reset()
{
	m_FirstRun=true;
    //initial values for the kalman filter
    m_x_est_last = 0.0;
    m_last = 0.0;
}

KalmanFilter::KalmanFilter(): m_Q(0.022),m_R(0.617)  //setup Q and R as the noise in the system
{
}

double KalmanFilter::operator()(double input)
{
	//For first run set the last value to the measured value
	if (m_FirstRun)
	{
		m_x_est_last=input;
		m_FirstRun=false;
	}
    //do a prediction
    double x_temp_est = m_x_est_last;
    double P_temp = m_last + m_Q;
    //calculate the Kalman gain
    double K = P_temp * (1.0/(P_temp + m_R));
    //the 'noisy' value we measured
    double z_measured = input;
    //correct
    double x_est = x_temp_est + K * (z_measured - x_temp_est); 
    double P = (1- K) * P_temp;
    
    //update our last's
    m_last = P;
    m_x_est_last = x_est;
    
	//Test for NAN
	if ((!(m_x_est_last>0.0)) || (!(m_x_est_last<0.0)))
		m_x_est_last=0;

    return x_est;
}

  /***********************************************************************************************************/
 /*												PIDController2												*/
/***********************************************************************************************************/


PIDController2::PIDController2(double p, double i, double d,double maximumOutput,double minimumOutput,double maximumInput,
	double minimumInput,double m_tolerance,bool continuous,bool enabled) :
	m_P(p),m_I(i),m_D(d),m_maximumOutput(maximumOutput),m_minimumOutput(minimumOutput),m_maximumInput(maximumInput),m_minimumInput(minimumInput),
	m_continuous(continuous),m_enabled(enabled)
{
	m_prevError = 0;
	m_totalError = 0;
	m_tolerance = .05;

	m_result = 0;
}

PIDController2::~PIDController2()
{
}

double PIDController2::operator()(double setpoint,double input,double dTime_s)
{
	if (m_enabled)
	{
		//m_error = (setpoint - input) * dTime_s;  //Using dTime_s will keep the errors consistent if time is erratic
		//I have found that multiplying by time is not correct as it forces client to use large values
		m_error = (setpoint - input);
		if (m_continuous)
		{
			if (fabs(m_error) > 
				(m_maximumInput - m_minimumInput) / 2)
			{
				if (m_error > 0)
					m_error = m_error  - m_maximumInput + m_minimumInput;
				else
					m_error = m_error  +
					m_maximumInput - m_minimumInput;
			}
		}

		//Note: here is the original code, which is correct but incomplete; 
		//this check really needs an else case, where if the error grows too large it would stop accumulating error and become stuck
		//on an undesirable value
		//if (((m_totalError + m_error) * m_I < m_maximumOutput) && ((m_totalError + m_error) * m_I > m_minimumOutput))
		//	m_totalError += m_error;

		double TotalErrorCheck=(m_totalError + m_error) * m_I;
		if (TotalErrorCheck < m_maximumOutput)
		{
			if (TotalErrorCheck > m_minimumOutput)
				m_totalError += m_error;
			else //less than the minimum output
			{
				//accumulate by an error which would equal the minimum output
				double MinError=(m_minimumOutput - ( m_I * m_totalError)) / m_I;
				m_totalError += MinError;
			}
		}
		else //greater than the maximum output
		{
			//accumulate by an error which would equal the maximum output
			double MaxError=(m_maximumOutput - ( m_I * m_totalError)) / m_I;
			m_totalError += MaxError;  
		}
				
		m_result = m_P * m_error + m_I * m_totalError + m_D * (m_error - m_prevError);
		m_prevError = m_error;
		
		if (m_result > m_maximumOutput)
			m_result = m_maximumOutput;
		else if (m_result < m_minimumOutput)
			m_result = m_minimumOutput;
	}
	return m_result;
}

void PIDController2::SetPID(double p, double i, double d)
{
	m_P = p;
	m_I = i;
	m_D = d;
}

double PIDController2::GetP()
{
	return m_P;
}

double PIDController2::GetI()
{
	return m_I;
}

double PIDController2::GetD()
{
	return m_D;
}

double PIDController2::Get()
{
	return m_result;
}

void PIDController2::SetContinuous(bool continuous)
{
	m_continuous = continuous;
}

void PIDController2::SetInputRange(double minimumInput, double maximumInput)
{
	m_minimumInput = minimumInput;
	m_maximumInput = maximumInput;	
}

void PIDController2::SetOutputRange(double minimumOutput, double maximumOutput)
{
	m_minimumOutput = minimumOutput;
	m_maximumOutput = maximumOutput;
}

double PIDController2::GetError()
{
	return m_error;
}

void PIDController2::SetTolerance(double percent)
{
	m_tolerance = percent;
}

bool PIDController2::OnTarget()
{
	return (fabs(m_error)<m_tolerance / 100 * (m_maximumInput - m_minimumInput));
}

void PIDController2::Enable()
{
	m_enabled = true;
}

void PIDController2::Disable()
{
	m_result=0.0;  //I cannot see a case where we would want to retain the last result during a disabled state
	m_enabled = false;
}

void PIDController2::Reset()
{
	m_prevError = 0;
	m_totalError = 0;
	m_result = 0;
}

void PIDController2::ResetI()
{
	m_totalError = 0;
}

void PIDController2::ResetI(double totalError)
{
	m_totalError = totalError;
}

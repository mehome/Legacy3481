#include <math.h>
#include "PIDController.h"

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
		m_error = (setpoint - input) * dTime_s;  //Using dTime_s will keep the errors consistent if time is erratic
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

		if (((m_totalError + m_error) * m_I < m_maximumOutput) && ((m_totalError + m_error) * m_I > m_minimumOutput))
			m_totalError += m_error;

				
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

#pragma once

struct filter;

struct FRAMEWORKAUDIO2_API low_pass
{			// Constructor
			low_pass( const double fc, const double fs, const int n ) : m_Fc( fc ), m_Fs( fs ), m_N( n ) {}

private:	double m_Fc;      // Cutoff frequency
			double m_Fs;      // Sampling frequency
			int m_N;          // number of taps (number of non-repeated coefficients)                

			// A friend
			friend filter;
};

struct FRAMEWORKAUDIO2_API high_pass
{			// Constructor
			high_pass( const double fc, const double fs, const int n ) : m_Fc( fc ), m_Fs( fs ), m_N( n ) {}

private:	double m_Fc;      // Cutoff frequency
			double m_Fs;      // Sampling frequency
			int m_N;          // number of taps (number of non-repeated coefficients)                

			// A friend
			friend filter;
};

struct FRAMEWORKAUDIO2_API band_pass
{			// Constructor
			band_pass( const double fl, const double fh, const double fs, const int n ) : m_Fl( fl ), m_Fh( fh ), m_Fs( fs ), m_N( n ) {}

private:	double m_Fl;      // Low-frequency cutoff
			double m_Fh;      // High-frequency cutoff
			double m_Fs;      // Sampling frequency
			int m_N;          // number of taps (number of non-repeated coefficients)                

			// A friend
			friend filter;
};

struct FRAMEWORKAUDIO2_API custom_pass
{			// Constructor
			custom_pass( const double fs, const int n ) : m_Fs( fs ), m_N( n ) {}

			// Implement this !
			virtual const double gain( const double freq ) const { return 1.0; }

private:	double m_Fs;      // Sampling frequency
			int m_N;          // number of taps (number of non-repeated coefficients)                

			// A friend
			friend filter;
};
#include "stdafx.h"
#include "FrameWork.h"

void FrameWork::DebugOutput(const char *format, ... )
{	char Temp[2048];
	va_list marker;
	va_start(marker,format);
		vsprintf(Temp,format,marker);
		OutputDebugStringA(Temp);
	va_end(marker);		
}

void FrameWork::DrawField( PBYTE pField, const int FrameWidth, const int FieldHeight, const int FieldNumber )
{
	{ // aka Black Field section
		const int FrameSize = FrameWidth * FieldHeight * sizeof(USHORT);
		PWORD pField_ = (PWORD) pField, pEnd_ = pField_ + (FrameSize/sizeof(WORD));

		int FieldHeight_ = FrameSize / (FrameWidth * sizeof(USHORT));
		int OneThird_ = ( FieldHeight_ * 1 ) / 3;
		int TwoThird_ = ( FieldHeight_ * 2 ) / 3;

		while(pField_ != pEnd_)
		{	*pField_++ = 0x1080;
		}
		// Draw gradient
		for (int Idx_ = 0; Idx_ < OneThird_; Idx_++)
		{	PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

		for (int X_ = 0; X_ < FrameWidth; X_++)
		{	WORD w_ = ((X_&0xFF)<<8)+ 0x80;
		*pw_++ = w_;
		}
		}
		// Draw bars
		for (int Idx_ = TwoThird_; Idx_ < FieldHeight_; Idx_++)
		{	PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

		for (int X_ = 0; X_ < FrameWidth; X_++)
		{	WORD w_ = 0xC000 | ((X_*256)/FrameWidth);
		*pw_++ = w_;
		}
		}

	}
	int ThreeNine_ = ( FieldHeight * 3 ) / 9;
	int FourNine_  = ( FieldHeight * 4 ) / 9;
	int FiveNine_  = ( FieldHeight * 5 ) / 9;
	int SixNine_   = ( FieldHeight * 6 ) / 9;

	// Draw stationary vertical lines
	for (int Idx_ = ThreeNine_; Idx_ < FourNine_; Idx_++)
	{	
		PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

		for (int X_ = 0; X_ < FrameWidth; X_++)
		{	
			int Of16_ = (X_ % 16);
			if (Of16_ == 0 || Of16_ == 1)
			{	
				*pw_++ = 0xE080;
			}
			else
			{	
				*pw_++ = 0x1080;
			}
		}
	}

	// Draw moving horizontal lines
	for (int Idx_ = FourNine_; Idx_ < FiveNine_; Idx_++)
	{	PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

	int Of16_ = ((Idx_ + FieldNumber) % 16);
	//int Of16_ = (Idx_  % 16);

	if (Of16_ == 0)
	{	
		for (int X_ = 0; X_ < FrameWidth; X_++)
			*pw_++ = 0xE080;
	}
	else
	{	
		for (int X_ = 0; X_ < FrameWidth; X_++)
			*pw_++ = 0x1080;
	}
	}
	//Draw moving diagonal lines
	for (int Idx_ = FiveNine_; Idx_ < SixNine_; Idx_++)
	{	
		PWORD pw_ = ((PWORD) pField + (Idx_ * FrameWidth));

		for (int X_ = 0; X_ < FrameWidth; X_++)
		{	int Of16_ = ((X_ + Idx_ + FieldNumber) % 16);
		if (Of16_ == 0 || Of16_ == 1)
		{	*pw_++ = 0xE080;
		}
		else
		{	*pw_++ = 0x1080;
		}
		}
	}
	//--------------------------------------------------------------------------------Field alignment test
	// Draw black on field line 480, F0 Left, F1 Right.
	// These would be frame lines 960 and 961.
	// Effect should be Left Line above Right Line.
	{
		int Line480_ = ( FieldHeight * 8 ) / 9;
		PWORD pwLine_ = ((PWORD) pField + (Line480_ * FrameWidth));

		// If F1, advance to right half.
		if (FieldNumber & 1)
		{
			pwLine_ += (FrameWidth/2);
		}
		// Draw half a line.
		for (int X_ = 0; X_ < (FrameWidth/2); X_++)
		{	*pwLine_++ = 0x1080;
		}
	}
	// Line 0: F0 Black / White, F1 White Black.
	// These would be frame lines 0 and 1.
	// Effect should be Frame Line 0 Black / White,
	// and Frame Line 1 White Black, ie Left is
	// Black ontop of White, and Right is White
	// ontop of Black. These are so we can verify
	// output with a digital scope.
	{
		PWORD pwLine_ = (PWORD) pField;
		WORD wLeft_, wRight_;

		if (FieldNumber & 1)
			wLeft_ = 0xF080, wRight_ = 0x1080;
		else
			wLeft_ = 0x1080, wRight_ = 0xF080;

		for (int X_ = 0; X_ < FrameWidth; X_++)
		{
			if (X_ < (FrameWidth/2))
				*pwLine_++ = wLeft_;
			else
				*pwLine_++ = wRight_;
		}
	}
}

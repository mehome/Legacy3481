#ifndef __BurnInTimeCodeH__
#define __BurnInTimeCodeH__

class BurnInTimeCode {
	public:
		bool BlitTimeCode(
			char *TimeCodeString,		//##:##:##:##
			byte *FieldEvenZero,		//Top Row Even Field... or interleaved (if FieldOddone is NULL)
			byte *FieldOddOne=NULL,			//2nd Row Odd Field
			unsigned XOffset=200,
			unsigned YOffset=200,	//Position to place text
			unsigned long BuffXres=720, //Size of the VideoBuffer
			unsigned long BuffYres=480
			);
	private:
		void FillInRow(byte *Buffer,unsigned long TimeCode,unsigned row);
	};

#endif	__BurnInTimeCodeH__
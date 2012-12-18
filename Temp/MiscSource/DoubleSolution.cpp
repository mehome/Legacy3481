
class VTTime {
	private:

		double val;
		#define _VTTime_Epsilon (1e-5)

	public:

		inline VTTime(double t) {val=t;}
		inline VTTime(VTTime & copyme) {val=copyme.val;}
		inline VTTime() {val=0.0;}

		inline VTTime& operator= (double t) 
			{val=t; return (*this);}

		inline VTTime& operator= (VTTime t)	
			{val=t.val; return (*this);}

		//==
		inline bool operator== (VTTime t) 
			{return (fabs(val-t.val) < _VTTime_Epsilon);}
		inline bool operator== (double t) 
			{return (fabs(val-t) < _VTTime_Epsilon);}
		// >
		inline bool operator> (VTTime t) 
			{return ((val-t.val) > _VTTime_Epsilon);}
		inline bool operator> (double t) 
			{return ((val-t) > _VTTime_Epsilon);}
		// <
		inline bool operator< (VTTime t) 
			{return ((t.val-val) > _VTTime_Epsilon);}
		inline bool operator< (double t) 
			{return ((t-val) > _VTTime_Epsilon);}
		//>=
		inline bool operator>= (VTTime t) 
			{return (val > (t.val-_VTTime_Epsilon));}
		inline bool operator>= (double t) 
			{return (val > (t-_VTTime_Epsilon));}
		//<=
		inline bool operator<= (VTTime t) 
			{return (val < (t.val+_VTTime_Epsilon));}
		inline bool operator<= (double t) 
			{return (val < (t+_VTTime_Epsilon));}
		// +
		inline VTTime operator+ (VTTime & t) const
		{return (val+t.val);}
		inline VTTime operator+ (double t) const
		{return (val+t);}
		// -
		inline VTTime operator- (VTTime & t) const
		{return (val-t.val);}
		inline VTTime operator- (double t) const
		{return (val-t);}
	};

void main() {
	double dtest=0.0345,dtest2=2.0,dtest3=0.0;
	VTTime vttest=0.234;
	VTTime vttest2=0.234;

	vttest=(vttest2-vttest);
	dtest3=dtest2-dtest;
	if (vttest<=vttest2) printf("Test\n");
	printf("Hello\n");
	}


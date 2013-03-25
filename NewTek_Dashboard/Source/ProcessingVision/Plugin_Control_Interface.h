

enum VisionSetting_enum
{
	eTrackerType,
	eDisplayType,
	eSolidMask,
	eOverlays,
	eAimingText,
	eBoundsText,
	e3PtGoal,
	eThresholdStart,
	eThresholdPlane1Min = eThresholdStart,
	eThresholdPlane2Min,
	eThresholdPlane3Min,
	eThresholdPlane1Max,
	eThresholdPlane2Max,
	eThresholdPlane3Max,
	eThresholdEnd,
	eThresholdMode = eThresholdEnd,	
	eNumTrackerSettings,
	eNumThresholdSettings = eThresholdEnd - eThresholdStart
};

enum TrackerType 
{
	eGoalTracker,
	eFrisbeTracker,
	eNumTrackers
};

enum DisplayType
{
	eNormal,
	eThreshold,
	eMasked,
	eNumDisplayTypes
};

enum ThresholdColorSpace
{
	eThreshRGB,
	eThreshHSV,
	eThreshLuma,
	eNumThreshTypes
};

typedef bool (*function_set) (VisionSetting_enum Setting, double value);
typedef double (*function_get) (VisionSetting_enum Setting);
typedef void (*function_reset) (void);

class Plugin_Controller_Interface
{
public:
	Plugin_Controller_Interface(function_get pGetFunc, function_set pSetFunc, function_reset pResetFunc )
	{
		m_fpGetSettings = pGetFunc;
		m_fpSetSettings = pSetFunc;
		m_fpResetThreshholds = pResetFunc;
	}

	~Plugin_Controller_Interface() {}

	bool Set_Vision_Settings(VisionSetting_enum Setting, double value) 
	{ 
		if (m_fpSetSettings) 
			return (*m_fpSetSettings)(Setting, value); 
		return false;
	}

	double Get_Vision_Settings(VisionSetting_enum Setting) 
	{ 
		if (m_fpGetSettings) 
			return (*m_fpGetSettings)(Setting); 
		return false;
	}

	void ResetThresholds( void )
	{
		if (m_fpResetThreshholds)
			m_fpResetThreshholds();
	}
private:
	function_get m_fpGetSettings;

	function_set m_fpSetSettings;

	function_reset m_fpResetThreshholds;
};
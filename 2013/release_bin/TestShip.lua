TestShip = {
	Mass = 25, -- Weight kg
	MaxAccelLeft = 5, MaxAccelRight = 5, MaxAccelForward = 5, MaxAccelReverse = 5, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = 2.916, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 2.916, -- Engagement Speed
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 2, -- Take this to the limit
	BRAKE = 10,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 1200,
	
	Dimensions =
	{ Length=0.5, Width=0.5 }, --These should be "roughly" correct in meters
	
	UI =
	{
		Length=2, Width=7,
		TextImage="/^\\\n-||X||-"
	}
}

Ship = TestShip

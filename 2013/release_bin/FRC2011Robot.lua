
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787
OunceInchToNewton=0.00706155183333

FRC2011_wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=22.3125	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelTurningDiameter_In= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Width_In * WheelBase_Width_In) ) ^ 0.5
GearSpeed = (372.63 / 60.0) * Pi * FRC2011_wheel_diameter_in * Inches2Meters

Drive_MaxAccel=4
skid=math.cos(math.atan2(WheelBase_Width_In,WheelBase_Width_In))

TestShip = {
	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	--Note we scale down the yaw to ease velocity rate for turning
	MaxTorqueYaw =  (2 * Drive_MaxAccel * Meters2Inches / WheelTurningDiameter_In) * skid * 0.9,
	rotate_to_scale = 1.0, rotate_to_scale_high = 1.0,

	MAX_SPEED = GearSpeed, -- Maximum Speed (m/s) use to be 2.916 but computed to 2.974848
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (2 * GearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,  --was originally computed to 514
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (will matter for turning radius!

	tank_drive =
	{
		is_closed=1,						--This should always be false for high gear
		show_pid_dump='yes',
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=WheelBase_Width_In, width_in=WheelBase_Width_In},	--The length is measure for 4 wheels (so it is half of the wheel base)
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = FRC2011_wheel_diameter_in,
		left_pid=
		{p=200, i=0, d=50},
		right_pid=
		{p=200, i=0, d=50},					--These should always match, but able to be made different
		latency=0.0,
		heading_latency=0.6,
		drive_to_scale=0.80,
		left_max_offset=0.5 , right_max_offset=-0.01,   --Ensure both tread top speeds are aligned
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		curve_voltage=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		reverse_steering='yes',
		left_encoder_reversed='no',
		right_encoder_reversed='no',
		inv_max_accel = 1.0/15.0,  --solved empiracally
		forward_deadzone_left  = 0.02,
		forward_deadzone_right = 0.110,
		reverse_deadzone_left  = 0.115,
		reverse_deadzone_right = 0.04
	},

	controls =
	{
		Joystick_1 =
		{
			control = "logitech dual action",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
		}
	},
		
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

Robot2011 = TestShip

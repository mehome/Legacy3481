
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787

FRC2012_wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
--Parker claimed 20.38, but I've measured 22 5/16
WheelBase_Width_In=22.3125	  --The wheel base will determine the turn rate, must be as accurate as possible!
HighGearSpeed = (427.68 / 60.0) * Pi * FRC2012_wheel_diameter_in * Inches2Meters  --RPM's from Parker
LowGearSpeed  = (167.06 / 60.0) * Pi * FRC2012_wheel_diameter_in * Inches2Meters

KeyDistance_in=144;
KeyWidth_in=101;
KeyDepth_in=48;
HalfKeyWidth_in=KeyWidth_in/2.0;

MainRobot = {
	Mass = 25, -- Weight kg
	MaxAccelLeft = 10, MaxAccelRight = 10, MaxAccelForward = 10, MaxAccelReverse = 10, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (HighGearSpeed / (Pi * WheelBase_Width_In * Inches2Meters)) * Pi2,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (This is not used except for UI ignore)
	
	tank_drive =
	{
		is_closed=0,						--This should always be false for high gear
		show_pid_dump='n',
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=27.5, width_in=WheelBase_Width_In},	--The length is not used but here for completion MATTTTTTTTTTTTTTTTTTTTTTT farther then 180 make smaller, not far enough make larger
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = FRC2012_wheel_diameter_in, --MATTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT - less then 5ft, +  greater then 5ft
		left_pid=
		{p=0, i=0, d=0},					--In FRC 2011 pid was 1,1,0 but lets keep i to zero if we can
		right_pid=
		{p=0, i=0, d=0},					--These should always match, but able to be made different
		
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		reverse_steering='no'
	},
	
	robot_settings =
	{
		key_1 = { x_in=0				, y_in=KeyDistance_in },
		key_2 = { x_in=-HalfKeyWidth_in	, y_in=KeyDistance_in },
		key_3 = { x_in= HalfKeyWidth_in	, y_in=KeyDistance_in },
		ds_display_row=1,					--This will display the coordinates and heading (may want to leave on)
		ds_target_vars_row=-1,				--Only used during keying the grid
		ds_power_velocity_row=3,			--Only used during keying the grid
		
		grid_corrections =
		{
			c11={p=2.4, x=1.0}, c12={p=2.4, x=1.0}, c13={p=2.4, x=1.0},
			c21={p=2.4, x=1.0}, c22={p=2.4, x=1.0}, c23={p=2.4, x=1.0},  --MATTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT change P
			c31={p=2.4, x=1.0}, c32={p=2.4, x=1.0}, c33={p=2.4, x=1.0},
		},
		
		auton =
		{
			ramp_left  ={x_in=0, y_in=0 },
			ramp_right ={x_in=0, y_in=0 },
			ramp_center={x_in=0, y_in=0 }
		},
		
		turret =
		{
			is_closed='yes',				--It is closed loop when feedback has been properly calibrated
			show_pid_dump='n',				--Only turn on if you want to analyze the PID dump (only one at a time, and it must be closed loop)
			ds_display_row=0,				--Assign to a row (e.g. 0-4) when trying to calibrate the potentiometer
			pid=
			{p=25, i=3, d=2},
			tolerance=0.001,				--we need high precision
			encoder_to_wheel_ratio=1.064,     --Just use the gearing ratios here			
			voltage_multiply=1.0,			--May be reversed using -1.0
			max_speed=10,
			accel=10.0,						--These are only needed if we bind keys for turret
			brake=10.0,
			max_accel_forward=20,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=20,
			using_range=0,
			min_range_deg=-180,				--These are probably good to go, but may need to be smaller
			max_range_deg= 180
		},
		pitch =
		{
			is_closed='no',
			show_pid_dump='no',
			ds_display_row=-1,
			pid=
			{p=1, i=0, d=0},
			tolerance=0.001,				--we need high precision

			max_speed=10,
			max_accel_forward=10,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=10,
			min_range_deg=45-3,				--These should be good to go
			max_range_deg=70+3
		},
		power =
		{
			is_closed='yes',
			show_pid_dump='n',
			ds_display_row=4,				--Use this display to determine max speed (try to get a good match)
			pid=
			--{p=0, i=0, d=0},
			--{p=0.1, i=0.5, d=0},
			--{p=400.0, i=5.0, d=200.0},
			{p=400.0, i=75.0, d=200.0},
			tolerance=200.0,					--we need decent precision (this will depend on ramp up time too)
			encoder_to_wheel_ratio=0.85,     --Just use the gearing ratios here
			voltage_multiply=-1,
			square_voltage='no',

			length_in=6,					--6 inch diameter (we shouldn't worry about tweaking this just measure it and be done)
			max_speed=(5000.0/50.0) * Pi2,	--(This is clocked at 5000 rpm) in radians
			accel=200.0,						--These are only needed if we bind keys for power in meters per second
			brake=200.0,
			max_accel_forward=200,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=200,			--The wheel may some time to ramp up
			min_range=21 * Pi2				--We borrow the min range to represent the min speed
		},
		flippers =
		{
			is_closed=0,				--Not sure yet about this
			show_pid_dump='no',
			ds_display_row=-1,
			pid=
			{p=1, i=0, d=0},
			tolerance=0.01,					--should not matter much
			
			max_speed=1.4 * Pi2,			--(Parker gave this one, should be good)
			accel=10.0,						--We may indeed have a two button solution (match with max accel)
			brake=10.0,
			max_accel_forward=10,			--These are in radians, just go with what feels right
			max_accel_reverse=10,
			using_range=0,					--Warning Only use range if we have a potentiometer!
			min_range_deg=-90,				--TODO find out what these are
			max_range_deg= 90
		},
		conveyor =
		{
			--Note: there are no encoders here so is_closed is ignored
			tolerance=0.01,					--we need good precision
			voltage_multiply=-1.0,			--May be reversed
			
			max_speed=28,
			accel=112,						--These are needed and should be high enough to grip without slip
			brake=112,
			max_accel_forward=112,
			max_accel_reverse=112
		},
		low_gear = 
		{
			--While it is true we have more torque for low gear, we have to be careful that we do not make this too powerful as it could
			--cause slipping if driver "high sticks" to start or stop quickly.
			MaxAccelLeft = 10, MaxAccelRight = 10, MaxAccelForward = 10 * 2, MaxAccelReverse = 10 * 2, 
			MaxTorqueYaw = 25 * 2, 
			
			MAX_SPEED = LowGearSpeed,
			ACCEL = 10*2,    -- Thruster Acceleration m/s2 (1g = 9.8)
			BRAKE = ACCEL, 
			-- Turn Rates (deg/sec) This is always correct do not change
			heading_rad = (LowGearSpeed / (Pi * WheelBase_Width_In * Inches2Meters)) * Pi2,
			
			tank_drive =
			{
				is_closed=0,						--By default false, and can be turned on dynamically
				show_pid_dump='no',
				ds_display_row=-1,
				--We must NOT use I or D for low gear, we must keep it very responsive
				--We are always going to use the encoders in low gear to help assist to fight quickly changing gravity shifts
				left_pid=
				{p=50, i=0, d=0},
				right_pid=
				{p=50, i=0, d=0},					--These should always match, but able to be made different
				
				--I'm explicitly keeping this here to show that we have the same ratio (it is conceivable that this would not always be true)
				--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
				encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
				voltage_multiply=1.0,				--May be reversed using -1.0
				reverse_steering='no'
			}
		},
		controls =
		{
			Joystick_1 =
			{
				control = "CH FLIGHTSTICK PRO",
				Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.1, is_squared=true},
				Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=0.8, filter=0.1, is_squared=false},
				Robot_SetLowGearValue = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.0, is_squared=false},
				Robot_Flippers_Solenoid = {type="joystick_button", key=3, on_off=true},
				Robot_SetCreepMode = {type="joystick_button", key=1, on_off=true}
			},

			Joystick_2 =
			{
				control = "Logitech Dual Action",
				--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
				Turret_SetCurrentVelocity = {type="joystick_analog", key=0, is_flipped=false, multiplier=0.5, filter=0.1, is_squared=false},
				--Ball_Grip = {type="joystick_button", key=2, on_off=true},
				Ball_Squirt = {type="joystick_button", key=1, on_off=true},
				--Ball_Fire = {type="joystick_button", key=4, on_off=true},
				--PowerWheels_IsRunning = {type="joystick_button", key=3, on_off=true},
				Robot_TurretSetTargetingOff = {type="joystick_button", key=6, on_off=true},
				Robot_SetPreset1 = {type="joystick_button", key=5, on_off=false},
				Robot_SetPreset2 = {type="joystick_button", key=9, on_off=false},
				Robot_SetPreset3 = {type="joystick_button", key=10, on_off=false},
				
				--Until we have the ball sensors working we'll need to re-assign the aim and fire buttons below to use all three button for the grip
				Ball_Fire = {type="joystick_button", key=8, on_off=true},
				PowerWheels_IsRunning = {type="joystick_button", key=7, on_off=true},
				Ball_GripL = {type="joystick_button", key=2, on_off=true},
				Ball_GripM = {type="joystick_button", key=3, on_off=true},
				Ball_GripH = {type="joystick_button", key=4, on_off=true}
			},

			Joystick_3 =
			{	
				control = "CH THROTTLE QUADRANT",
				PitchRamp_SetIntendedPosition = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.142000, filter=0.0, is_squared=false},
				Robot_SetTargetingValue = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.142000, filter=0.0, is_squared=false},
				PowerWheels_SetCurrentVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0000, filter=0.0, is_squared=false},
				--This top one is only for open loop mode, and works like the game pad
				--Turret_SetIntendedPosition = {type="joystick_analog", key=2, is_flipped=true, multiplier=0.5, filter=0.1, is_squared=true},
				--Turret_SetIntendedPosition = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, is_squared=false},
				Robot_SetDefensiveKeyValue = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.0, is_squared=false},
				
				--Ball_Grip = {type="joystick_button", key=2, on_off=true},
				Ball_Squirt = {type="joystick_button", key=1, on_off=true},
				--Ball_Fire = {type="joystick_button", key=6, on_off=true},
				--PowerWheels_IsRunning = {type="joystick_button", key=4, on_off=true},
				Robot_SetDefensiveKeyOn = {type="joystick_button", key=11, on_off=false},
				Robot_SetDefensiveKeyOff = {type="joystick_button", key=12, on_off=false},
				
				Ball_GripL = {type="joystick_button", key=2, on_off=true},
				Ball_GripM = {type="joystick_button", key=4, on_off=true},
				Ball_GripH = {type="joystick_button", key=6, on_off=true},
				PowerWheels_IsRunning = {type="joystick_button", key=8, on_off=true},
				Ball_Fire = {type="joystick_button", key=10, on_off=true}
			}
		}
	},
	--This is only used in the AI tester, can be ignored
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(   )\n|   |\n(   )"
	}
}

Robot2012 = MainRobot

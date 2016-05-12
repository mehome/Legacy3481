
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787
OunceInchToNewton=0.00706155183333

FRC2012_wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
--Parker claimed 20.38, but I've measured 22 5/16
WheelBase_Width_In=22.3125	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelTurningDiameter_In= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Width_In * WheelBase_Width_In) ) ^ 0.5
HighGearSpeed = (427.68 / 60.0) * Pi * FRC2012_wheel_diameter_in * Inches2Meters  --RPM's from Parker
LowGearSpeed  = (167.06 / 60.0) * Pi * FRC2012_wheel_diameter_in * Inches2Meters
Drive_MaxAccel=4
skid=math.cos(math.atan2(WheelBase_Width_In,WheelBase_Width_In))

--CIM Motor torque
CIM_StallTorque_OzIn=343.3
CIM_StallTorque_Nm=CIM_StallTorque_OzIn*OunceInchToNewton
CIM_MaxRPM=5310
CIM_Vel_To_Torque_oz=(1.0/(CIM_MaxRPM/60.0)) * CIM_StallTorque_OzIn
CIM_Vel_To_Torque_nm=CIM_Vel_To_Torque_oz*OunceInchToNewton
CIM_MotorTorque=(CIM_MaxRPM / 60) * CIM_Vel_To_Torque_nm  --long math to show its equal to CIM_StallTorque_Nm

DriveTrain_GearReduction = 12.4158
DriveTrain_Efficiency=1.0
DriveTrain_MaxTorque = 2.0 * CIM_MotorTorque * DriveTrain_GearReduction * DriveTrain_Efficiency

--Now to compute the linear equivalent
DriveTrain_PayloadMass = 3.0   --This combines mass with each moment used to compute acceleration
DriveTrain_MaxAccel_rad= DriveTrain_MaxTorque / DriveTrain_PayloadMass
DriveTrain_MaxAccel_rps= DriveTrain_MaxAccel_rad / (2.0 * Pi)
DriveTrain_WheelDiameter = Inches2Meters * FRC2012_wheel_diameter_in
DriveTrain_MaxAccel_linear = DriveTrain_MaxAccel_rps * (Pi * DriveTrain_WheelDiameter)
DriveTrain_MaxForce = DriveTrain_MaxAccel_linear * DriveTrain_PayloadMass

--extra computations
DriveTrain_MaxWheelRPS_High = HighGearSpeed / (Pi * DriveTrain_WheelDiameter)  --The fastest RPS of the wheels
DriveTrain_MaxWheelRPS_Low  = LowGearSpeed  / (Pi * DriveTrain_WheelDiameter)
DriveTrain_MotorRPS = DriveTrain_MaxWheelRPS_High * DriveTrain_GearReduction
DriveTrain_MaxAngularVelocity_High = DriveTrain_MaxWheelRPS_High * 2.0 * Pi --In radians

KeyDistance_in=144
--KeyDistance_in=0
KeyWidth_in=101
KeyDepth_in=48
HalfKeyWidth_in=KeyWidth_in/2.0


MainRobot = {
	--Version helps to identify a positive update to lua
	version = 3.6,

	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	--MaxAccelForward_High = 10, MaxAccelReverse_High = 10, 
	--Note we scale down the yaw to ease velocity rate for turning
	MaxTorqueYaw =  (2 * Drive_MaxAccel * Meters2Inches / WheelTurningDiameter_In) * skid * 0.7,
	rotate_to_scale = 1.0, rotate_to_scale_high = 1.0,
	
	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (2 * HighGearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (This is not used except for UI ignore)
	
	tank_drive =
	{
		is_closed=1,						--This should always be false for high gear
		show_pid_dump='no',
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=WheelBase_Width_In, width_in=WheelBase_Width_In},	--The length is measure for 4 wheels (so it is half of the wheel base)
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = FRC2012_wheel_diameter_in,
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
		inv_max_accel = 1.0/15.0  --solved empiracally
	},
	
	robot_settings =
	{
		key_1 = { x_in=0				, y_in=KeyDistance_in },
		key_2 = { x_in=-HalfKeyWidth_in	, y_in=KeyDistance_in },
		key_3 = { x_in= HalfKeyWidth_in	, y_in=KeyDistance_in },
		ds_display_row=1,					--This will display the coordinates and heading (may want to leave on)
		ds_target_vars_row=-1,				--Only used during keying the grid
		ds_power_velocity_row=3,			--Only used during keying the grid
		fire_trigger_delay=0.100,			--Used to wait for a stable rate before engaging the conveyor
		fire_stay_on_time=0.200,			--Used to prevent ball from get stuck during a fire operation (keep small)
		
		grid_corrections =
		{
			c11={p=1.7, x=1.0}, c12={p=1.7, x=1.0}, c13={p=1.7, x=1.0},
			c21={p=1.8, x=1.0}, c22={p=1.8, x=1.0}, c23={p=1.8, x=1.0},
			c31={p=1.7, x=1.0}, c32={p=1.7, x=1.0}, c33={p=1.7, x=1.0},
		},
		
		auton =
		{
			move_forward_ft =0.0,
			two_shot_scaler =1.4,
			ramp_left  ={x_in=0, y_in=0 },
			ramp_right ={x_in=0, y_in=0 },
			ramp_center={x_in=0, y_in=0 },
			x_left_arc=1.9,
			x_right_arc=1.9,
			--If you put -1.0 for the timeout wait it will wait infinitely (good for initial testing or if we are not tipping ramps)
			--ball 1 initial wait should be long enough for a good ramp up from zero speed
			--ball_1 ={initial_wait=  2.0, tolerance=75.0, timeout_wait=4.0},
			--ball 2 initial wait should be long enough to recover from dip and short enough to be active during second ball's deployment
			--ball_2 ={initial_wait=0.500, tolerance=75.0, timeout_wait=4.0}
			--panic mode incase the wait ball doesn't work... using zero makes it work like before just pure time
			ball_1 ={initial_wait=  3.5, tolerance=0.0, timeout_wait=-1.0},
			ball_2 ={initial_wait=  3.5, tolerance=0.0, timeout_wait=-1.0}
		},
		
		turret =
		{
			is_closed='yes',				--It is closed loop when feedback has been properly calibrated
			show_pid_dump='no',				--Only turn on if you want to analyze the PID dump (only one at a time, and it must be closed loop)
			ds_display_row=2,				--Assign to a row (e.g. 0-4) when trying to calibrate the potentiometer
			pid=
			{p=200, i=5, d=50},
			tolerance=0.1,				--we need high precision
			encoder_to_wheel_ratio=1.0,     --Just use the gearing ratios here			
			voltage_multiply=1.0,			--May be reversed using -1.0
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
			inv_max_decel = 1/60.0,
			inv_max_accel = 1/30.0,

			max_speed=14.42,
			accel=10.0,						--These are only needed if we bind keys for turret
			brake=10.0,
			max_accel_forward=10,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=10,
			using_range=0,
			min_range_deg=-180,				--These are probably good to go, but may need to be smaller
			max_range_deg= 180
		},
		pitch =
		{
			is_closed='yes',
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
			is_closed='no',
			show_pid_dump='no',
			ds_display_row=4,				--Use this display to determine max speed (try to get a good match)
			pid=
			--{p=0, i=0, d=0},
			--{p=0.1, i=0.5, d=0},
			--{p=400.0, i=5.0, d=200.0},
			{p=400.0, i=75.0, d=200.0},
			latency=0.170,
			tolerance=10.0,					--we need decent precision (this will depend on ramp up time too)
			encoder_to_wheel_ratio=0.85,     --Just use the gearing ratios here
			voltage_multiply=-1.0,
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},

			length_in=6,					--6 inch diameter (we shouldn't worry about tweaking this just measure it and be done)
			max_speed=(5000.0/60.0) * Pi2,	--(This is clocked at 5000 rpm) in radians
			accel=60.0,						--These are only needed if we bind keys for power in meters per second
			brake=60.0,
			max_accel_forward=60,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=60,			--The wheel may some time to ramp up
			min_range=28 * Pi2				--We borrow the min range to represent the min speed
		},
		flippers =
		{
			is_closed=1,				--Not sure yet about this
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
			using_range=1,					--Warning Only use range if we have a potentiometer!
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
			MaxAccelLeft = 5, MaxAccelRight = 5, MaxAccelForward = 5 * 2, MaxAccelReverse = 5 * 2, 
			MaxTorqueYaw = (2 * 10 * Meters2Inches / WheelTurningDiameter_In) * skid * 0.7, 
			
			MAX_SPEED = LowGearSpeed,
			ACCEL = 10*2,    -- Thruster Acceleration m/s2 (1g = 9.8)
			BRAKE = ACCEL, 
			-- Turn Rates (deg/sec) This is always correct do not change
			heading_rad = (2 * LowGearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,
			
			tank_drive =
			{
				is_closed=0,						--By default false, and can be turned on dynamically
				show_pid_dump='no',
				ds_display_row=-1,
				--We must NOT use I or D for low gear, we must keep it very responsive
				--We are always going to use the encoders in low gear to help assist to fight quickly changing gravity shifts
				left_pid=
				{p=25, i=0, d=5},
				right_pid=
				{p=25, i=0, d=5},					--These should always match, but able to be made different
				latency=0.300,
				--I'm explicitly keeping this here to show that we have the same ratio (it is conceivable that this would not always be true)
				--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
				encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
				voltage_multiply=1.0,				--May be reversed using -1.0
				curve_voltage=
				{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
				reverse_steering='yes',
				left_encoder_reversed='no',
				right_encoder_reversed='no',
				inv_max_accel = 1.0/15.0  --solved empiracally
			}
		}
	},
	
	controls =
	{
		Joystick_1 =
		{
			control = "logitech dual action",
			--control = "logitech attack 3",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Robot_SetLowGearValue = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Turret_SetIntendedPosition = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			Robot_Flippers_Solenoid = {type="joystick_button", key=2, on_off=true},
			Robot_SetLowGearOff = {type="joystick_button", key=6, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=5, on_off=false},
			Robot_SetCreepMode = {type="joystick_button", key=1, on_off=true},
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			Turn_180 = {type="joystick_button", key=7, on_off=false}
		},

		Joystick_2 =
		{
			control = "logitech attack 3",
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			Turret_SetCurrentVelocity = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
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
			control = "ch throttle quadrant",
			PitchRamp_SetIntendedPosition = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.142000, filter=0.0, curve_intensity=0.0},
			Robot_SetTargetingValue = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.142000, filter=0.0, curve_intensity=0.0},
			PowerWheels_SetCurrentVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0000, filter=0.0, curve_intensity=0.0},
			--This top one is only for open loop mode, and works like the game pad
			--Turret_SetIntendedPosition = {type="joystick_analog", key=2, is_flipped=true, multiplier=0.5, filter=0.1, curve_intensity=1.0},
			--Turret_SetIntendedPosition = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			Robot_SetDefensiveKeyValue = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			
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

	},
	
	--This is only used in the AI tester, can be ignored
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(   )\n|   |\n(   )"
	}
}

Robot2012 = MainRobot

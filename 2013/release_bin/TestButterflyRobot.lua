Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787

wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=19.5	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=27.5
WheelTurningDiameter_In= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Length_In * WheelBase_Length_In) ) ^ 0.5
HighGearSpeed = (492.83 / 60.0) * Pi * wheel_diameter_in * Inches2Meters  --RPM's from Parker
LowGearSpeed  = (184.81 / 60.0) * Pi * wheel_diameter_in * Inches2Meters
MaxCentripetalTraverse=20 --The maximum amount of centripetal force that can be allowed (may want to be higher for better drive)
skid=math.cos(math.atan2(WheelBase_Length_In,WheelBase_Width_In))

TestShip = {

	Mass = 25, -- Weight kg
	MaxAccelLeft = MaxCentripetalTraverse, MaxAccelRight = MaxCentripetalTraverse, 
	MaxAccelForward = 4, MaxAccelReverse = 4, 
	MaxAccelForward_High = 10, MaxAccelReverse_High = 10, 
	MaxTorqueYaw = (2 * 4 * Meters2Inches / WheelTurningDiameter_In) * skid, 
	rotate_to_scale = 0.5, rotate_to_scale_high = 1.0,
	
	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (2 * HighGearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (will matter for turning radius!
	
	swerve_drive =
	{
		is_closed=1,
		is_closed_swivel=0,
		
		show_pid_dump_wheel=
		{fl=0, fr=0, rl=0, rr=0},
		show_pid_dump_swivel=
		{fl=0, fr=0, rl=0, rr=0},
		
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=WheelBase_Length_In, width_in=WheelBase_Width_In},	--where length is in 5 inches in, and width is 3 on each side (can only go 390 degrees a second)
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = 6,
		wheel_pid=
		{p=200, i=0, d=50},
		swivel_pid=
		{p=100, i=0, d=50},
		latency=0.0,
		heading_latency=0.0,
		drive_to_scale=0.50,				--For 4 to 10 50% gives a 5 inch tolerance
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		curve_voltage_wheel=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		--curve_voltage_swivel=
		--{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		reverse_steering='no',
		inv_max_accel = 1/12,  --solved empiracally
		motor_specs =
		{
			gear_reduction=5310.0/492.83,
		}

	},

	low_gear = 
	{
		--While it is true we have more torque for low gear, we have to be careful that we do not make this too powerful as it could
		--cause slipping if driver "high sticks" to start or stop quickly.
		MaxAccelLeft = MaxCentripetalTraverse, MaxAccelRight = MaxCentripetalTraverse, 
		MaxAccelForward = 10, MaxAccelReverse = 10, 
		MaxAccelForward_High = 20, MaxAccelReverse_High = 20, 
		MaxTorqueYaw = 25 * 2, 
		
		MAX_SPEED = LowGearSpeed,
		ACCEL = 10*2,    -- Thruster Acceleration m/s2 (1g = 9.8)
		BRAKE = ACCEL, 
		-- Turn Rates (deg/sec) This is always correct do not change
		--heading_rad = (LowGearSpeed / (Pi * WheelBase_Width_In * Inches2Meters)) * Pi2,
		heading_rad = 0,  --No turning for traction mode
		
		swerve_drive =
		{
			is_closed=1,
			pid={p=50, i=0, d=0},
			inv_max_accel = 1/38.0,  --solved empiracally
			motor_specs =
			{
				gear_reduction=5310.0/184.81,
			}
		},
	},

	controls =
	{
		tank_steering_tolerance=0.07,
		Joystick_1 =
		{
			control = "any",
			--Use Arcade/FPS enable
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Analog_Turn = {type="joystick_analog", key=5, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Use tank steering enable
			Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			Joystick_SetRightVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			
			Butterfly_SetLowGearOn = {type="joystick_button", key=8, on_off=false},
			Butterfly_SetLowGearOff = {type="joystick_button", key=6, on_off=false},
			Turn_180 = {type="joystick_button", key=7, on_off=false}
		},
		Joystick_2 =
		{
			control = "logitech dual action",
			Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			Joystick_SetRightVelocity = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			None = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0}
		}

	},

	UI =
	{
		Length=5, Width=5,
		TextImage="     \n,   ,\n(-+-)\n'   '\n     "
	}
}

RobotButterfly = TestShip

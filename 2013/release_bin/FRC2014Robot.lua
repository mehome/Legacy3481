
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787
OunceInchToNewton=0.00706155183333
Pounds2Kilograms=0.453592
Deg2Rad=(1/180) * Pi

Catapult_ArmToMotorRatio=5 * 8.3
Catapult_MotorToArmRatio=1.0/Catapult_ArmToMotorRatio
Catapult_PotentiometerToArmRatio=1/3
Catapult_PotentiometerToMotorRatio=Catapult_PotentiometerToArmRatio * Catapult_ArmToMotorRatio
--TODO get max speed of bag motor under load
--Catapult_MaxSpeed=(8000.0/60.0) * Pi2
Catapult_MaxSpeed=(8000.0/60.0) * Pi2 * 0.125

Intake_ArmToMotorRatio=1.0
Intake_MotorToArmRatio=1.0/Intake_ArmToMotorRatio
Intake_PotentiometerToArmRatio=1.0
Intake_PotentiometerToMotorRatio=Intake_PotentiometerToArmRatio * Intake_ArmToMotorRatio

g_wheel_diameter_in=4   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=26.5	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=10  --was 9.625
WheelTurningDiameter_In= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Length_In * WheelBase_Length_In) ) ^ 0.5
HighGearSpeed = (733.14 / 60.0) * Pi * g_wheel_diameter_in * Inches2Meters  --RPM's from Parker
LowGearSpeed  = (167.06 / 60.0) * Pi * g_wheel_diameter_in * Inches2Meters
Drive_MaxAccel=5
skid=math.cos(math.atan2(WheelBase_Length_In,WheelBase_Width_In))
gMaxTorqueYaw = (2 * Drive_MaxAccel * Meters2Inches / WheelTurningDiameter_In) * skid

MainRobot = {
	control_assignments =
	{
		--by default module is 1, so only really need it for 2
		victor =
		{
			id_1 = { name= "left_drive_1", channel=1, module=1}, 
			id_2 = { name= "left_drive_2", channel=2}, 
			id_3 = { name="right_drive_1", channel=3},
			id_4 = { name="right_drive_2", channel=4},
			id_5 = { name= "left_drive_3", channel=5}, 
			id_6 = { name="right_drive_3", channel=6},
			id_7 = { name="winch",         channel=7}, 
			id_8 = { name="intake_arm_1",  channel=8},
			id_9 = { name="intake_arm_2",  channel=9},
			id_10= { name="rollers",       channel=10},
		},
		double_solenoid =
		{
			id_1 = { name="use_low_gear",    forward_channel=1, reverse_channel=2},
			id_2 = { name="release_clutch",  forward_channel=3, reverse_channel=4},
			id_3 = { name="catcher_shooter", forward_channel=5, reverse_channel=6},
			id_4 = { name="catcher_intake",	 forward_channel=7, reverse_channel=8}
		},
		digital_input =
		{
			--These channels must be unique to digital input encoder channels as well
			--Also ensure you do not use the slot for the compressor ;)
			id_1 = { name="intake_min_1",  channel=7},
			id_2 = { name="intake_max_1",  channel=8}, 
			id_3 = { name="intake_min_2",  channel=9},
			id_4 = { name="intake_max_2",  channel=10}, 
			id_5 = { name="catapult_limit",channel=11} 
		},
		digital_input_encoder =
		{	
			--encoder names must be the same name list from the victor (or other speed controls)
			--These channels must be unique to digital input channels as well
			id_1 = { name= "left_drive_1",  a_channel=1, b_channel=2},
			id_2 = { name="right_drive_1",  a_channel=3, b_channel=4},
			id_3 = { name="winch",  a_channel=5, b_channel=6}
		},
		compressor	=	{ relay=8, limit=14 }
	},
	--Version helps to identify a positive update to lua
	--version = 1;
	
	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	MaxAccelForward_High = Drive_MaxAccel * 2, MaxAccelReverse_High = Drive_MaxAccel * 2, 
	MaxTorqueYaw =  gMaxTorqueYaw,
	MaxTorqueYaw_High = gMaxTorqueYaw * 5,
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
		is_closed=1,
		show_pid_dump='no',
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=WheelBase_Length_In, width_in=WheelBase_Width_In},
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = g_wheel_diameter_in,
		left_pid=
		{p=200, i=0, d=50},
		right_pid=
		{p=200, i=0, d=50},					--These should always match, but able to be made different
		latency=0.0,
		heading_latency=0.0,
		drive_to_scale=0.50,				--For 4 to 10 50% gives a 5 inch tolerance
		left_max_offset=0.0 , right_max_offset=0.0,   --Ensure both tread top speeds are aligned
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		curve_voltage=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		force_voltage=
		{t4=0, t3=0, t2=0, t1=0, c=1},
		reverse_steering='no',
		 left_encoder_reversed='no',
		right_encoder_reversed='no',
		inv_max_accel = 1/15.0,  --solved empiracally
		forward_deadzone_left  = 0.02,
		forward_deadzone_right = 0.02,
		reverse_deadzone_left  = 0.02,
		reverse_deadzone_right = 0.02,
		motor_specs =
		{
			wheel_mass=1.5,
			cof_efficiency=1.0,
			gear_reduction=5310.0/733.14,
			torque_on_wheel_radius=Inches2Meters * 1,
			drive_wheel_radius=Inches2Meters * 2,
			number_of_motors=1,
			
			free_speed_rpm=5310.0,
			stall_torque=6.561,
			stall_current_amp=399,
			free_current_amp=8.1
		}
	},
	
	robot_settings =
	{
		ds_display_row=-1,					--This will display the coordinates and heading (may want to leave on)
		ball_camera_scalar=0.20,
		ball_latency_count=0.100,
		
		catapult=
		{
			arm_to_motor=Catapult_ArmToMotorRatio,
			pot_to_arm=Catapult_PotentiometerToArmRatio,
			--The winch is set up to force the numbers to go up from 0 - 90 where 0 is pointing up
			--This allows gain assist to apply max voltage to its descent
			chipshot_angle_deg=45,
			goalshot_angle_deg=90,
		},
		intake=
		{
			arm_to_motor=Intake_ArmToMotorRatio,
			pot_to_arm=Intake_PotentiometerToArmRatio,
			--The intake uses a starting point of 90 to force numbers down from 90 - 0 where zero is pointing straight out
			--This allows the gain assist to apply max force when it goes from deployed to stowed
			stowed_angle=90,
			deployed_angle=61,
			squirt_angle=90
		},

		auton =
		{
			ball_target_distance_ft=4,
		},
		
		winch =
		{
			is_closed=1,
			show_pid_dump='n',
			ds_display_row=-1,
			use_pid_up_only='y',
			pid_up=
			{p=100, i=0, d=0},
			tolerance=0.15,
			tolerance_count=20,
			voltage_multiply=1.0,			--May be reversed
			encoder_to_wheel_ratio=Catapult_PotentiometerToMotorRatio,
			--curve_voltage=
			--{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
			
			max_speed=Catapult_MaxSpeed,
			accel=100.0,						--We may indeed have a two button solution (match with max accel)
			brake=100.0,
			--This will be about a second and then some for entire retraction should be fast... the second scalar is 1/x of a second to
			--reach full speed which should be very quick
			max_accel_forward=Catapult_MaxSpeed * 10,
			max_accel_reverse=Catapult_MaxSpeed * 10,
			using_range=1,					--Warning Only use range if we have a potentiometer!
			--These are arm converted to gear ratio
			--The winch is set up to force the numbers to go up from 0 - 90 where 0 is pointing up
			max_range_deg= 92 * Catapult_ArmToMotorRatio,
			max_limit_deg=88 * Catapult_ArmToMotorRatio,  --The angle the limit switch is placed (this is offset from maxrange to determine final velocity when hit)
			min_range_deg=(-10) * Catapult_ArmToMotorRatio,
			use_aggressive_stop = 'no',
			inv_max_accel_up = 0.05,
			inv_max_decel_up = 0.0,
			inv_max_accel_down = 0.05,
			inv_max_decel_down = 0.01,
			--slow_velocity_voltage = 4.0,
			--slow_velocity = 2.0,
			--predict_up=.400,
			--predict_down=.400,
			--pulse_burst_time=0.06,
			--pulse_burst_range=0.5,
			--reverse_deadzone=0.10,
			slow_angle_scalar = Catapult_MotorToArmRatio,
			--distance_scale = 0.5,
			motor_specs =
			{
				wheel_mass=Pounds2Kilograms * 16.27,
				cof_efficiency=0.2,
				gear_reduction=1.0,
				torque_on_wheel_radius=Inches2Meters * 1.0,
				drive_wheel_radius=Inches2Meters * 2.0,
				number_of_motors=2,
				
				free_speed_rpm=233.0,
				stall_torque=0.4,
				stall_current_amp=41,
				free_current_amp=1.8
			}
		},
		
		intake_arm =
		{
			starting_position_deg=90,
			show_pid_dump='n',
			ds_display_row=-1,
			use_pid_up_only='n',
			pid_up=
			{p=100, i=0, d=0},
			pid_down=
			{p=100, i=0, d=0},
			tolerance=0.15,
			tolerance_count=1,
			voltage_multiply=1.0,			--May be reversed
			encoder_to_wheel_ratio=1.0,
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
			
			--max_speed=(19300/64/60) * Pi2,	--This is about 5 rps (a little slower than hiking viking drive)
			max_speed=8.8,	--loaded max speed (see sheet) which is 2.69 rps
			accel=0.5,						--We may indeed have a two button solution (match with max accel)
			brake=0.5,
			max_accel_forward=1,			--These are in radians, just go with what feels right
			max_accel_reverse=1,
			using_range=1,					--Warning Only use range if we have a potentiometer! or limit switch
			--These are arm converted to gear ratio
			--The intake uses a starting point of 90 to force numbers down from 90 - 0 where zero is pointing straight out
			max_range_deg= (95) * Intake_ArmToMotorRatio,
			max_limit_deg=90 * Intake_ArmToMotorRatio,  --The angle the limit switch is placed when intake is stowed
			min_range_deg= 45 * Intake_ArmToMotorRatio,
			min_limit_deg=60 * Intake_ArmToMotorRatio,  --The angle the limit switch is placed when intake is deployed
			use_aggressive_stop = 'yes',
			inv_max_accel_up = 0.05,
			inv_max_decel_up = 0.0,
			inv_max_accel_down = 0.05,
			inv_max_decel_down = 0.01,
			slow_velocity_voltage = 4.0,
			slow_velocity = 2.0,
			predict_up=.400,
			predict_down=.400,
			--pulse_burst_time=0.06,
			--pulse_burst_range=0.5,
			reverse_deadzone=0.10,
			slow_angle_scalar = Intake_GearToArmRatio,
			distance_scale = 0.5,
			motor_specs =
			{
				wheel_mass=Pounds2Kilograms * 16.27,
				cof_efficiency=0.2,
				gear_reduction=1.0,
				torque_on_wheel_radius=Inches2Meters * 1.0,
				drive_wheel_radius=Inches2Meters * 2.0,
				number_of_motors=2,
				
				free_speed_rpm=84.0,
				stall_torque=10.6,
				stall_current_amp=18.6,
				free_current_amp=1.8
			}
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
			heading_rad = (2 * LowGearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,
			
			tank_drive =
			{
				is_closed=1,
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
				reverse_steering='no',
				 left_encoder_reversed='no',
				right_encoder_reversed='no',
				inv_max_accel = 0.0  --solved empiracally
			}
		}
	},

	controls =
	{
		Joystick_1 =
		{
			control = "airflo",
			--Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetLeft_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Joystick_SetRightVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetRight_XAxis = {type="joystick_analog", key=5, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Analog_Turn = {type="joystick_culver", key_x=5, key_y=2, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			Robot_SetDriverOverride = {type="joystick_button", key=5, on_off=true},
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			--Turret_SetCurrentVelocity = {type="joystick_analog", key=5, is_flipped=false, multiplier=0.75, filter=0.3, curve_intensity=3.0},
			--PitchRamp_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=2.0},
			Robot_SetLowGearOff = {type="joystick_button", key=8, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=6, on_off=false},
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Turn_180 = {type="joystick_button", key=7, on_off=false},
			Turn_180_Hold = {type="joystick_button", key=7, on_off=true},
			FlipY_Hold = {type="joystick_button", key=7, on_off=true},
			SlideHold = {type="joystick_button", key=7, on_off=true},
			Robot_TestWaypoint={type="joystick_button", key=3, on_off=true},
			
			Winch_SetChipShot = {type="joystick_button", key=4, on_off=false},
			Winch_SetGoalShot = {type="joystick_button", key=2, on_off=false},
			--Winch_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Winch_Fire={type="joystick_button", key=1, keyboard='j', on_off=true},
			Winch_Advance={type="keyboard", key='k', on_off=true},
			IntakeArm_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			IntakeArm_SetDeployed={type="keyboard", key='l', on_off=false},
			IntakeArm_SetStowed={type="keyboard", key=';', on_off=false},
			Robot_BallTargeting_On={type="keyboard", key='t', on_off=false},
			Robot_BallTargeting_Off={type="keyboard", key='y', on_off=false},
			Winch_Advance={type="keyboard", key='k', on_off=true},
			Robot_CatcherShooter={type="keyboard", key='u', on_off=true},
			Robot_CatcherIntake={type="keyboard", key='i', on_off=true},
			TestWaypoint={type="keyboard", key='q', on_off=true},
			TestAuton={type="keyboard", key='g', on_off=false},
			--Slide={type="keyboard", key='g', on_off=false},
		},
		
		Joystick_2 =
		{
			control = "logitech dual action",
			--Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Joystick_SetLeft_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Joystick_SetRightVelocity = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetRight_XAxis = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Analog_Turn = {type="joystick_culver", key_x=2, key_y=5, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Turret_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=false, multiplier=0.5, filter=0.1, curve_intensity=0.0},
			Robot_SetLowGearOff = {type="joystick_button", key=6, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=5, on_off=false},
			
			Winch_SetChipShot = {type="joystick_button", key=4, on_off=false},
			Winch_SetGoalShot = {type="joystick_button", key=3, on_off=false},
			Winch_SetCurrentVelocity = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Winch_Fire = {type="joystick_button", key=2, on_off=true},
			
			--Ball_Squirt = {type="joystick_button", key=1, on_off=true},
			--PowerWheels_IsRunning = {type="joystick_button", key=7, on_off=true},
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			Turn_180_Hold = {type="joystick_button", key=7, on_off=true},
			FlipY_Hold = {type="joystick_button", key=7, on_off=true},
			SlideHold = {type="joystick_button", key=7, on_off=true}
		},
		Joystick_3 =
		{
			control = "gamepad f310 (controller)",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetRightVelocity = {type="joystick_analog", key=4, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Turret_SetCurrentVelocity = {type="joystick_analog", key=3, is_flipped=false, multiplier=0.75, filter=0.3, curve_intensity=3.0},
			--PitchRamp_SetCurrentVelocity = {type="joystick_analog", key=4, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=2.0},
			
			--IntakeRollers_Grip = {type="joystick_button", key=?, on_off=true},
			--IntakeRollers_Squirt = {type="joystick_button", key=?, on_off=true},
			IntakeRollers_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=1.0},

			Robot_SetLowGearOff = {type="joystick_button", key=6, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=5, on_off=false},
			Robot_TestWaypoint={type="joystick_button", key=3, on_off=true},
			
			Winch_SetChipShot = {type="joystick_button", key=4, on_off=false},
			Winch_SetGoalShot = {type="joystick_button", key=2, on_off=false},
			Winch_SetCurrentVelocity = {type="joystick_analog", key=4, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Winch_Fire={type="joystick_button", key=1, keyboard='j', on_off=true},
		}

	},
	
	--This is only used in the AI tester, can be ignored
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

Robot2014 = MainRobot

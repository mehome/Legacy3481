
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787
OunceInchToNewton=0.00706155183333
Pounds2Kilograms=0.453592
Deg2Rad=(1/180) * Pi

ArmLength_m=48 * Inches2Meters  --4 feet
ArmToGearRatio=72.0/28.0
GearToArmRatio=1.0/ArmToGearRatio
PotentiometerToArmRatio=36.0/54.0
PotentiometerToGearRatio=PotentiometerToArmRatio * ArmToGearRatio
PotentiometerMaxRotation_r=270.0 * Deg2Rad
GearHeightOffset_m=38.43 * Inches2Meters
MotorToWheelGearRatio=12.0/36.0


g_wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=24.52198975	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=28.7422  
WheelTurningDiameter_In= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Length_In * WheelBase_Length_In) ) ^ 0.5
HighGearSpeed = (749.3472 / 60.0) * Pi * g_wheel_diameter_in * Inches2Meters  * 0.9  --RPMs from BHS2015 Chassis.SLDASM
LowGearSpeed  = (346.6368 / 60.0) * Pi * g_wheel_diameter_in * Inches2Meters  * 0.9
Drive_MaxAccel=5
--Omni wheels means no skid
--skid=math.cos(math.atan2(WheelBase_Length_In,WheelBase_Width_In))
skid=1
gMaxTorqueYaw = (2 * Drive_MaxAccel * Meters2Inches / WheelTurningDiameter_In) * skid

MainRobot = {
	version = 1.0;
	control_assignments =
	{
		--by default module is 1, so only really need it for 2
		victor =
		{
			id_1 = { name= "right_drive_1", channel=5, module=1}, 
			id_2 = { name= "right_drive_2", channel=6}, 
			id_3 = { name="left_drive_1", channel=7},
			id_4 = { name="left_drive_2", channel=4},
			id_5= { name="turret", channel=1},
			id_6= { name="arm", channel=2}
			--If we decide we need more power we can assign these
			--id_3 = { name= "right_drive_3", channel=3}, 
			--id_6 = { name="left_drive_3", channel=6},
		},
		relay =
		{
			id_1 = { name= "CameraLED", channel=1}
		},
		digital_input =
		{
			--These channels must be unique to digital input encoder channels as well
			--Also ensure you do not use the slot for the compressor ;)
			id_1 = { name="dart_upper_limit",  channel=5},
			id_2 = { name="dart_lower_limit",  channel=6}
		},
		analog_input =
		{
			id_1 = { name="arm_potentiometer",  channel=2},
		},
		digital_input_encoder =
		{	
			--encoder names must be the same name list from the victor (or other speed controls)
			--These channels must be unique to digital input channels as well
			id_1 = { name= "left_drive_1",  a_channel=3, b_channel=4},
			id_2 = { name="right_drive_1",  a_channel=1, b_channel=2},
		},
		compressor	=	{ relay=8, limit=14 }
	},
	--Version helps to identify a positive update to lua
	--version = 1;
	
	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	MaxAccelForward_High = Drive_MaxAccel * 2, MaxAccelReverse_High = Drive_MaxAccel * 2, 
	MaxTorqueYaw =  gMaxTorqueYaw,  --Note Bradley had 0.78 reduction to get the feel
	MaxTorqueYaw_High = gMaxTorqueYaw * 5,
	MaxTorqueYaw_SetPoint = gMaxTorqueYaw * 2,
	MaxTorqueYaw_SetPoint_High = gMaxTorqueYaw * 10,
	rotation_tolerance=Deg2Rad * 2,
	rotation_distance_scalar=1.0,

	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (2 * HighGearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (This is not used except for UI ignore)
	
	tank_drive =
	{
		is_closed=0,
		show_pid_dump='no',
		--we should turn this off in bench mark testing
		use_aggressive_stop=1,  --we are in small area want to have responsive stop
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
		encoder_to_wheel_ratio=0.5,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		--Note: this is only used in simulation as 884 victors were phased out, but encoder simulators still use it
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
			gear_reduction=5310.0/749.3472,
			torque_on_wheel_radius=Inches2Meters * 1,
			drive_wheel_radius=Inches2Meters * 2,
			number_of_motors=2,
			
			free_speed_rpm=5310.0,
			stall_torque=2.43,
			stall_current_amp=133,
			free_current_amp=2.7
		}
	},
	
	robot_settings =
	{
		ds_display_row=-1,					--This will display the coordinates and heading (may want to leave on)

		height_presets =
		--Heights are in inches
		{rest=0.0, tote_3=11.75*2 + 2 },
		auton =
		{
			first_move_ft=2,
			side_move_rad=10,
			arm_height_in=12,
			support_hotspot='n',
			show_auton_variables='y'
		},

		turret =
		{
			is_closed=0,
			show_pid_dump='n',
			ds_display_row=-1,
			use_pid_up_only='y',
			pid_up={p=100, i=0, d=25},
			pid_down={p=100, i=0, d=25},
			tolerance=0.3,
			tolerance_count=1,
			voltage_multiply=1.0,			--May be reversed
			--this may be 184: 84 * 36 : 20... using 180 as the ring is 3.8571428571428571428571428571429
			encoder_to_wheel_ratio=1.0,
			Arm_SetPotentiometerSafety=true,	
			--max_speed=(19300/64/60) * Pi2,	--This is about 5 rps (a little slower than hiking viking drive)
			max_speed=0.7,	--100 rpm... with a 15x reduction in radians
			accel=1.0,						--We may indeed have a two button solution (match with max accel)
			brake=1.0,
			max_accel_forward=1,			--These are in radians, just go with what feels right
			max_accel_reverse=1,
			using_range=0,	--Warning Only use range if we have a potentiometer!

			max_range_deg= 52.36 * ArmToGearRatio,
			min_range_deg=(-46.12) * ArmToGearRatio,
			pot_offset=-46.12 * Deg2Rad,
			use_aggressive_stop = 'yes',
			inv_max_accel_up = 0.3,
			inv_max_decel_up = 0.3,
			inv_max_accel_down = 0.3,
			inv_max_decel_down = 0.3,
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
		arm =
		{
			is_closed=0,
			show_pid_dump='n',
			ds_display_row=-1,
			use_pid_up_only='y',
			pid_up=
			{p=100, i=0, d=25},
			pid_down=
			{p=100, i=0, d=25},
			tolerance=0.15,
			tolerance_count=20,
			voltage_multiply=1.0,			--May be reversed
			encoder_to_wheel_ratio=1.0,
			
			--max_speed=(19300/64/60) * Pi2,	--This is about 5 rps (a little slower than hiking viking drive)
			max_speed=8.8,	--loaded max speed (see sheet) which is 2.69 rps
			accel=1.0,						--We may indeed have a two button solution (match with max accel)
			brake=1.0,
			max_accel_forward=10,			--These are in radians, just go with what feels right
			max_accel_reverse=10,
			using_range=0,					--Warning Only use range if we have a potentiometer!
			--These min/max are arm converted to gear ratio (TODO reseach this more)
			max_range_deg= 52.36 * ArmToGearRatio,
			min_range_deg=(-46.12) * ArmToGearRatio,
			starting_position_deg=-46.12,
			use_aggressive_stop = 'yes',
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
	},

	controls =
	{
		slotlist = {slot_1="airflo"},
		--field_centric_x_axis_threshold=0.40,
		--tank_steering_tolerance=0.05,
		Joystick_1 =
		{
			control = "airflo",
			axis_count = 4,
			--Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Analog_Turn = {type="joystick_culver", key_x=5, key_y=2, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Joystick_FieldCentric_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_FieldCentric_YAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--FieldCentric_Enable = {type="joystick_button", key=4, on_off=false},
			--Robot_SetDriverOverride = {type="joystick_button", key=5, on_off=true},
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			--Turret_SetCurrentVelocity = {type="joystick_analog", key=5, is_flipped=false, multiplier=0.75, filter=0.3, curve_intensity=3.0},
			--PitchRamp_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=2.0},
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Turn_180 = {type="joystick_button", key=7, on_off=false},
			--Turn_180_Hold = {type="joystick_button", key=7, on_off=true},
			--FlipY_Hold = {type="joystick_button", key=7, on_off=true},
			--SlideHold = {type="joystick_button", key=7, on_off=true},
			--TestWaypoint={type="joystick_button", key=3, keyboard='q', on_off=true},
			
			Robot_BallTargeting_On={type="keyboard", key='t', on_off=false},
			Robot_BallTargeting_Off={type="keyboard", key='y', on_off=false},
			TestAuton={type="keyboard", key='g', on_off=false},
			--Slide={type="keyboard", key='g', on_off=false},
			
			turret_SetCurrentVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			arm_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			turret_Advance={type="keyboard", key='k', on_off=true},
			turret_Retract={type="keyboard", key='j', on_off=true},
	
		},
		
		Joystick_2 =
		{
			control = "logitech dual action",
			--Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetLeft_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Joystick_SetRightVelocity = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetRight_XAxis = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Analog_Turn = {type="joystick_culver", key_x=2, key_y=5, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			Joystick_FieldCentric_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_FieldCentric_YAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Turret_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=false, multiplier=0.5, filter=0.1, curve_intensity=0.0},
			Robot_SetLowGearOff = {type="joystick_button", key=6, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=5, on_off=false},
			
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
			--Analog_Turn = {type="joystick_culver", key_x=3, key_y=4, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetRightVelocity = {type="joystick_analog", key=4, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_FieldCentric_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_FieldCentric_YAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Turret_SetCurrentVelocity = {type="joystick_analog", key=3, is_flipped=false, multiplier=0.75, filter=0.3, curve_intensity=3.0},
			--PitchRamp_SetCurrentVelocity = {type="joystick_analog", key=4, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=2.0},
			
			arm_SetCurrentVelocity = {type="joystick_analog", key=4, is_flipped=true, multiplier=0.6, filter=0.1, curve_intensity=3.0},
			--FieldCentric_EnableValue = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},

			Robot_SetLowGearOff = {type="joystick_button", key=6, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=5, on_off=false},
			TestWaypoint={type="joystick_button", key=3, on_off=true},
		},
		Joystick_4 =
		{
			control = "controller (xbox 360 for windows)",
			--Joystick_SetLeft_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Joystick_SetRight_XAxis = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_FieldCentric_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_FieldCentric_YAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			Analog_Turn = {type="joystick_culver", key_x=3, key_y=4, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			Robot_SetLowGearOff = {type="joystick_button", key=2, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=1, on_off=false},
						
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			Robot_SetDriverOverride = {type="joystick_button", key=5, on_off=true},
			Turn_180_Hold = {type="joystick_button", key=6, on_off=true},
			FlipY_Hold = {type="joystick_button", key=6, on_off=true},
			SlideHold = {type="joystick_button", key=6, on_off=true}
		},
		Joystick_5 =
		{	
			control = "ch throttle quadrant",
			PitchRamp_SetIntendedPosition = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.142000, filter=0.0, curve_intensity=0.0},
			Robot_SetTargetingValue = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.142000, filter=0.0, curve_intensity=0.0},
			PowerWheels_SetCurrentVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0000, filter=0.0, curve_intensity=0.0},
			Turret_SetIntendedPosition = {type="joystick_analog", key=2, is_flipped=true, multiplier=0.5, filter=0.1, curve_intensity=1.0},
			Robot_SetDefensiveKeyValue = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.0, curve_intensity=0.0},

			Arm_SetPosRest     = {type="joystick_button", key=2, on_off=false},
			Arm_SetTote2Height = {type="joystick_button", key=4, on_off=false},
			Arm_SetTote3Height = {type="joystick_button", key=6, on_off=false},
			Arm_SetTote4Height = {type="joystick_button", key=8, on_off=false},
			Arm_SetTote5Height = {type="joystick_button", key=10, on_off=false},
			Arm_SetTote6Height = {type="joystick_button", key=12, on_off=false}
		},

	},
	
	--This is only used in the AI tester, can be ignored
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

Curivator = MainRobot

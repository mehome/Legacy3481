--This lua has orginal values

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
	--Version helps to identify a positive update to lua
	version = 1.2;
	control_assignments =
	{
		--by default module is 1, so only really need it for 2
		victor =
		{
			id_1 = { name= "right_drive_1", channel=1, module=1}, 
			id_2 = { name= "right_drive_2", channel=2}, 
			id_3 = { name="left_drive_1", channel=3},
			id_4 = { name="left_drive_2", channel=4},
			id_5= { name="kicker_wheel", channel=5},
			id_6= { name="arm", channel=6}
			--If we decide we need more power we can assign these
			--id_3 = { name= "right_drive_3", channel=3}, 
			--id_6 = { name="left_drive_3", channel=6},
		},
		relay =
		{
			id_1 = { name= "CameraLED", channel=1}
		},
		double_solenoid =
		{
			id_1 = { name="use_low_gear",    forward_channel=2, reverse_channel=1},
			id_2 = { name="fork_left",    forward_channel=3, reverse_channel=4},
			id_3 = { name="fork_right",    forward_channel=5, reverse_channel=6},
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
	
	Mass = 25, -- Weight kg
	MaxAccelLeft =10, MaxAccelRight = 10, 
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	MaxAccelForward_High = Drive_MaxAccel, MaxAccelReverse_High = Drive_MaxAccel, 
	MaxTorqueYaw =  gMaxTorqueYaw * 0.75, --Bradly's reduction feel
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
		--left_max_offset=-0.7 , right_max_offset=0.0,   --Ensure both tread top speeds are aligned
		left_max_offset=0.0 , right_max_offset=0.0,
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.25,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,
		--May be reversed using -1.0
		--Note: this is only used in simulation as 884 victors were phased out, but encoder simulators still use it
		--curve_voltage=
		--{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		force_voltage=
		{t4=0, t3=0, t2=0, t1=1, c=0},
		reverse_steering='no',
		 left_encoder_reversed='no',
		right_encoder_reversed='yes',
		inv_max_accel = 1/15.0,  --solved empiracally
		linear_gain_assist = 0.03,
		--forward_deadzone_left  = 0.02,
		--forward_deadzone_right = 0.02,
		--reverse_deadzone_left  = 0.02,
		--reverse_deadzone_right = 0.02,
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
			arm_height_in=12,
			support_hotspot='n',
			show_auton_variables='y'
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
			Arm_SetPotentiometerSafety=true,	
			--max_speed=(19300/64/60) * Pi2,	--This is about 5 rps (a little slowr than hiking viking drive)
			max_speed=8.8,	--loaded max speed (see sheet) which is 2.69 rps
			accel=1.0,						--We may indeed have a two button solution (match with max accel)
			brake=1.0,
			max_accel_forward=100,			--These are in radians, just go with what feels right
			max_accel_reverse=200,
			using_range=0,	--Warning Only use range if we have a potentiometer!

			--slow_velocity_voltage = 2.0, Problematic

			--These min/max are arm converted to gear ratio (TODO reseach this more)
			max_range_deg= 52.36 * ArmToGearRatio,
			--Note the sketch used -43.33, but tests on actual assembly show -46.12
			min_range_deg=(-46.12) * ArmToGearRatio,
			starting_position_deg=-46.12,
			use_aggressive_stop = 'yes',
			--inv_max_accel_up = 0.05,
			--inv_max_decel_up = 0.0,
			--inv_max_accel_down = 0.05,
			--inv_max_decel_down = 0.01,
			--slow_velocity_voltage = 4.0,
			--slow_velocity = 2.0,
			--predict_up=.400,
			--predict_down=.400,
			--pulse_burst_time=0.06,
			--pulse_burst_range=0.5,
			--reverse_deadzone=0.10,
			--slow_angle_scalar = GearToArmRatio,
			--distance_scale = 0.5,
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

		kicker =
		{
			is_closed=0,
			show_pid_dump='no',
			ds_display_row=-1,				--Use this display to determine max speed (try to get a good match)
			pid=
			{p=100, i=0, d=50 },
			latency=0.0,
			voltage_multiply=1.0,

			length_in=4,					--6 inch diameter (we shouldn't worry about tweaking this just measure it and be done)
			max_speed=42,					--with 13.2 gear reduction in radians (default is 42)					
			accel=10.0,						--These are only needed if we bind keys for power in meters per second
			brake=10.0,
			--These are low because of traction
			max_accel_forward=85,
			max_accel_reverse=85,
			--inv_max_accel = 1/23,  --solved empiracally
		},

		low_gear = 
		{
			--While it is true we have more torque for low gear, we have to be careful that we do not make this too powerful as it could
			--cause slipping if driver "high sticks" to start or stop quickly.
			--for this year... there is no high gear... so we'll inherit these from high gear
			--MaxAccelLeft = 10, MaxAccelRight = 10, MaxAccelForward = 10 * 2, MaxAccelReverse = 10 * 2, 
			--MaxTorqueYaw = 25 * 2,
			--MaxTorqueYaw_High = 25 * 2,

			MAX_SPEED = LowGearSpeed,
			ACCEL = 10*2,    -- Thruster Acceleration m/s2 (1g = 9.8)
			BRAKE = ACCEL, 
			-- Turn Rates (deg/sec) This is always correct do not change
			heading_rad = (2 * LowGearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,
			
			tank_drive =
			{
				is_closed=0,
				show_pid_dump='n',
				ds_display_row=-1,
				--We must NOT use I or D for low gear, we must keep it very responsive
				--We are always going to use the encoders in low gear to help assist to fight quickly changing gravity shifts
				left_pid=
				{p=25, i=0, d=5},
				right_pid=
				{p=25, i=0, d=5},					--These should always match, but able to be made different
				voltage_multiply=1.0,				--May be reversed using -1.0
			}
		}
	},

	controls =
	{
		--Competition Settings--
		slotlist = {slot_1="controller (xbox 360 for windows)", slot_2="gamepad f310 (controller)"},
		
	--	slotlist = {slot_1="controller (xbox 360 for windows)", slot_2="gamepad f310 (controller)", slot_3="logitech dual action"},
		--Testing settings--
		--slotlist = {slot_1="controller (xbox 360 for windows)", slot_2="gamepad f310 (controller)", slot_3="ch throttle quadrant", slot_4="logitech attack 3"},
			
		--field_centric_x_axis_threshold=0.40,
		--tank_steering_tolerance=0.05,

		Joystick_1 =
		{
			--Driver
			control = "controller (xbox 360 for windows)",
			--Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetLeft_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},
			--Joystick_SetRightVelocity = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetRight_XAxis = {type="joystick_analog", key=5, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=1.0},			             		--Analog_Turn = {type="joystick_culver", key_x=5, key_y=2, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			
			Analog_Turn = {type="joystick_analog", key=4, is_flipped=false, multiplier=1.0, filter=0.05, curve_intensity=1.0},
			KickerWheel_SetCurrentVelocity = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.5, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.2, curve_intensity=0.0},
	t
			--Robot_SetLowGearOff = {type="joystick_button", key=5, keyboard=';', on_off=false},
			--Robot_SetLowGearOn = {type="joystick_button", key=6, keyboard='l', on_off=false},
			
			--Previously assigned to 8
			--POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},		
		
		},

		Joystick_2 =
		{
			--Operator
			control = "gamepad f310 (controller)",
			
			Arm_SetCurrentVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=2.4, filter=0.1, curve_intensity=3.0},
		
			--Arm_ForkBoth = {type="joystick_button", key=6, on_off=true},
			--Arm_ForkRight = {type="joystick_button", key=4, on_off=true},
			--Arm_ForkLeft = {type="joystick_button", key=5, on_off=true},
				
			Robot_SetLowGearOff = {type="joystick_button", key=5, keyboard=';', on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=6, keyboard='l', on_off=false},
			
			--Claw_Close =	 {type="joystick_button", key=11, on_off=true},
		},
	

		Joystick_3 =
		{
			--Operator
			control = "ch throttle quadrant",
						
			Arm_SetCurrentVelocity = {type="joystick_analog", key=0, is_flipped=true, multiplier=2, filter=0.1, curve_intensity=3.0},

			Arm_SetPosRest     = {type="joystick_button", key='0', on_off=false},
			Arm_SetTote2Height = {type="joystick_button", key='2', on_off=false},
			Arm_SetTote3Height = {type="joystick_button", key='4', on_off=false},
			Arm_SetTote4Height = {type="joystick_button", key='6', on_off=false},
			Arm_SetTote5Height = {type="joystick_button", key='8', on_off=false},
			Arm_SetTote6Height = {type="joystick_button", key='10', on_off=false},
						
		},

		Joystick_4 =
		{
		
			control = "logitech dual action",
			--Note for the Y right axis... it is 5 on the simulation and 3 for the driver station (i.e. using real robot)
			Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Joystick_SetRightVelocity = {type="joystick_analog", key=3, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--Joystick_FieldCentric_XAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			--Joystick_FieldCentric_YAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			
			Robot_SetDriverOverride = {type="joystick_button", key=8, on_off=true},
			Robot_SetLowGearOff = {type="joystick_button", key=6, on_off=false},
			Robot_SetLowGearOn = {type="joystick_button", key=5, on_off=false},
			
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--optionally comment out once robot is calibrated (it may be offered from POV
			TestWaypoint={type="joystick_button", key=3, keyboard='q', on_off=true},
			--Turn_90R = {type="joystick_button", key=3, on_off=false},
			--Turn_90L = {type="joystick_button", key=1, on_off=false},
			Turn_180 = {type="joystick_button", key=2, on_off=false},
			--comment out once robot is calibrated
			Robot_TestWaypoint={type="joystick_button", key=4, on_off=true},
				
		}

			
	},
	
	--This is only used in the AI tester, can be ignored
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

Robot2015 = MainRobot

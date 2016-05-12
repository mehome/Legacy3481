
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787
Inches2Meters=0.0254
OunceInchToNewton=0.00706155183333
Pounds2Kilograms=0.453592
Deg2Rad=(1/180) * Pi

ArmLength_m=1.8288  --6 feet
ArmToGearRatio=72.0/28.0
GearToArmRatio=1.0/ArmToGearRatio
PotentiometerToArmRatio=36.0/54.0
PotentiometerToGearRatio=PotentiometerToArmRatio * ArmToGearRatio
PotentiometerMaxRotation_r=270.0 * Deg2Rad
GearHeightOffset_m=55 * Inches2Meters
MotorToWheelGearRatio=12.0/36.0


FRC2011_wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=22.3125	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=9.625
WheelTurningDiameter_In= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Length_In * WheelBase_Length_In) ) ^ 0.5
GearSpeed = (372.63 / 60.0) * Pi * FRC2011_wheel_diameter_in * Inches2Meters

Drive_MaxAccel=4
skid=math.cos(math.atan2(WheelBase_Length_In,WheelBase_Width_In))
gMaxTorqueYaw = (2 * Drive_MaxAccel * Meters2Inches / WheelTurningDiameter_In) * skid

MainRobot = {
--Version helps to identify a positive update to lua
	version = 1.2;
	
	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	MaxAccelForward_High = Drive_MaxAccel * 2, MaxAccelReverse_High = Drive_MaxAccel * 2, 
	--Note we scale down the yaw to ease velocity rate for turning
	MaxTorqueYaw =  gMaxTorqueYaw,
	MaxTorqueYaw_High = gMaxTorqueYaw * 5,
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
		is_closed=0,						--This should always be false for high gear
		show_pid_dump='no',
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
		heading_latency=0.0,
		drive_to_scale=0.50,
		left_max_offset=0.0 , right_max_offset=0.0,   --Ensure both tread top speeds are aligned
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
	robot_settings =
	{
		arm =
		{
			is_closed=0,
			show_pid_dump='n',
			ds_display_row=-1,
			pid=
			{p=100, i=0, d=0},
			tolerance=0.1,	
			voltage_multiply=1.0,			--May be reversed
			encoder_to_wheel_ratio=1.0,
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
			
			--max_speed=(19300/64/60) * Pi2,	--This is about 5 rps (a little slower than hiking viking drive)
			max_speed=8.8,	--loaded max speed (see sheet) which is 2.69 rps
			accel=10,						--We may indeed have a two button solution (match with max accel)
			brake=10,
			max_accel_forward=10,			--These are in radians, just go with what feels right
			max_accel_reverse=10,
			using_range=1,					--Warning Only use range if we have a potentiometer!
			--These are arm converted to gear ratio
			max_range_deg= 70 * ArmToGearRatio,
			min_range_deg=(-50) * ArmToGearRatio,
			use_aggressive_stop = 'yes',
			inv_max_accel = 1.0/17.0,
			--inv_max_accel = 1.0/15.0,
			--inv_max_decel = 1.0/24.0,
			distance_scale = 0.5,
			motor_specs =
			{
				wheel_mass=Pounds2Kilograms * 16.27,
				cof_efficiency=1.0,
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
		claw =
		{
			--Note: there are no encoders here so is_closed is ignored
			tolerance=0.01,					--we need good precision
			voltage_multiply=1.0,			--May be reversed
			
			max_speed=28,
			accel=112,						--These are needed and should be high enough to grip without slip
			brake=112,
			max_accel_forward=112,
			max_accel_reverse=112
		}
	},
	controls =
	{
		--slotlist = {slot_1="gamepad f310 (controller)"},
		--slotlist = {slot_1="logitech dual action"},
		slotlist = {slot_1="airflo"},
	
		Joystick_1 =
		{
			control = "gamepad f310 (controller)",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Turn_180 = {type="joystick_button", key=7, on_off=false},
			
			Arm_SetPos0feet = {type="joystick_button", key=1, on_off=false},
			Arm_SetPos3feet = {type="joystick_button", key=3, on_off=false},
			Arm_SetPos6feet = {type="joystick_button", key=2, on_off=false},
			Arm_SetPos9feet = {type="joystick_button", key=4, on_off=false},
			Arm_SetCurrentVelocity = {type="joystick_analog", key=4, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Arm_Rist={type="joystick_button", key=5, on_off=true},
			
			Claw_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			Claw_Close =	 {type="joystick_button", key=6, on_off=true},
			Claw_Grip =		 {type="joystick_button", key=9, on_off=true},
			Claw_Squirt =	 {type="joystick_button", key=7, on_off=true},
			Robot_CloseDoor= {type="joystick_button", key=8, on_off=true}
		},
		Joystick_2 =
		{
			control = "logitech dual action",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			--POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Turn_180 = {type="joystick_button", key=7, on_off=false},
			
			Arm_SetPos0feet = {type="joystick_button", key=2, on_off=false},
			Arm_SetPos3feet = {type="joystick_button", key=1, on_off=false},
			Arm_SetPos6feet = {type="joystick_button", key=3, on_off=false},
			Arm_SetPos9feet = {type="joystick_button", key=4, on_off=false},
			Arm_SetCurrentVelocity = {type="joystick_analog", key=3, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Arm_Rist={type="joystick_button", key=5, on_off=true},
			
			--Claw_SetCurrentVelocity  --not used
			Claw_Close =	 {type="joystick_button", key=6, on_off=true},
			Claw_Grip =		 {type="joystick_button", key=8, on_off=true},
			Claw_Squirt =	 {type="joystick_button", key=7, on_off=true},
			Robot_CloseDoor= {type="joystick_button", key=9, on_off=true}
		},
		Joystick_3 =
		{
			control = "airflo",
			--Joystick_SetLeftVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			--Joystick_SetRightVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Turn_180 = {type="joystick_button", key=7, on_off=false},
			
			Arm_SetPos0feet = {type="joystick_button", key=1, keyboard='y', on_off=false},
			Arm_SetPos3feet = {type="joystick_button", key=3, keyboard='u', on_off=false},
			Arm_SetPos6feet = {type="joystick_button", key=2, keyboard='l', on_off=false},
			Arm_SetPos9feet = {type="joystick_button", key=4, keyboard=';', on_off=false},
			Arm_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=3.0},
			Arm_Rist={type="joystick_button", key=5, keyboard='r', on_off=true},
			Arm_Advance={type="keyboard", key='k', on_off=true},
			Arm_Retract={type="keyboard", key='j', on_off=true},
			
			--Claw_SetCurrentVelocity  --not used
			Claw_Close =	 {type="joystick_button", key=6, keyboard='c', on_off=true},
			Claw_Grip =		 {type="joystick_button", key=8, keyboard='i', on_off=true},
			Claw_Squirt =	 {type="joystick_button", key=7, keyboard='h', on_off=true},
			Robot_CloseDoor= {type="joystick_button", key=9, keyboard='o', on_off=true}
		}
	},
		
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

RobotHikingViking = MainRobot

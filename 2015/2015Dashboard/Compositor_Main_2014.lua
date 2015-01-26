Pi=3.14159265358979323846

--This is used load persistence of the coordinates last saved etc
function GetSettings()
	local loaded_file = loadfile("CompositorSave_Main.lua")
	if (loaded_file==nil) then
		return nil
	else
		loaded_file()  --We can now make the call and it will succeed
		return sequence_load
	end
end

CompositorProps = {
	bypass_plugin="ProcessingVision.dll",
	settings =
	{
		x_scalar=0.05,
		y_scalar=0.05,
		square_reticle_props =
		{
			square_reticle_1 =
			{
				thickness_x=6,thickness_y=6,opacity=0.7,
				r=0,g=255,b=100,

				use_shadow='y', exclude_region='y',
				shadow =
				{
					thickness_x=6,thickness_y=6,opacity=0.9,
					r=0,g=0,b=0
				},
				x_offset=2,y_offset=1
			},
			square_reticle_2 =
			{
				thickness_x=10,thickness_y=10,opacity=0.7,
				r=255,g=100,b=0,

				use_shadow='y', exclude_region='y',
				shadow =
				{
					thickness_x=10,thickness_y=10,opacity=0.9,
					r=0,g=0,b=0
				},
				x_offset=2,y_offset=1
			},
			square_reticle_3 =
			{
				thickness_x=40,thickness_y=3,
				r=0,g=255,b=255
			}
		},
		--Shape reticles depend on the path align to setup the camera confiruation even if the path align is never used
		shape_reticle_props =
		{
			-- draw_shape -- may be "square", "circle", or "cube".
			shape_reticle_1 =
			{
				--remote_name="apex_reticle",
				size_in=25,
				r=0,g=255,b=100,
				plane_selection="xy_and_xz",
				draw_shape="circle"
			},
			shape_reticle_2 =
			{
				--remote_name="land_reticle",
				size_in=25,
				r=250,g=250,b=0,
				plane_selection="xy",
				draw_shape="circle"
			}
		},
		
		path_align =
		{
			width_in = 25, length_ft = 27,
			pivot_in = 2,
			camera_position =	{ x_in=0, y_in=12.5, z_in=-5.0 },
			camera_rotation =	{ x_deg=0, y_deg=7.30457, z=0 },
			fov=45,
			r=0,g=255,b=100,
			num_segments=27,
			disable_turns="y", --Using path align to gauge distance to target landing
			draw_shape = "path",
			-- draw_shape -- may be "path", "square", "circle", or "cube".
		},
		
		--Types	"none","square","composite","bypass","pathalign"
		sequence =
		{
			sequence_1 = 
			{
				type="composite",
				composite=
				{
					composite_1 = {	type="bypass" },
					composite_2 = {	type="square", selection=1 },
					composite_3 = { type="shape", selection=1, x=0, y=2.29786 , z=3.94196 },
					composite_4 = { type="shape", selection=2,  y=0.310857 , z=8.0574 },
					composite_5 = {	type="pathalign"}
				}
			},
			sequence_2 = {	type="bypass" },
			--This must stay here to load the settings
			load_settings = GetSettings(),
		}
	},
	controls =
	{
		Joystick_1 =
		{
			control = "logitech dual action",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			SetZAxis = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			SequencePOV =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			ToggleLinePlot     = {type="joystick_button", key=6, on_off=false},
			ResetPos     = {type="joystick_button", key=2, keyboard=' ', on_off=false},
		},
		Joystick_2 =
		{
			control = "gamepad f310 (controller)",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			SetZAxis = {type="joystick_analog", key=4, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			SequencePOV =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			ToggleLinePlot     = {type="joystick_button", key=6, on_off=false},
			ResetPos     = {type="joystick_button", key=1, keyboard=' ', on_off=false},
		},		
		Joystick_3 =
		{
			control = "airflo",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			SetZAxis = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			--These sequence events happen whether or not you are in edit so choose what you want to use during the game
			--The POV is not supported by the driver station so it is the perfect chose... I've added buttons here to show if you need to use them instead
			--NextSequence     = {type="joystick_button", key=2, keyboard='y', on_off=false},
			--PreviousSequence = {type="joystick_button", key=3, keyboard='y', on_off=false},
			ResetPos     = {type="joystick_button", key=1, on_off=false},
			ToggleLinePlot     = {type="joystick_button", key=6, on_off=false},
			SequencePOV =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
		},

	},
}

Compositor = CompositorProps

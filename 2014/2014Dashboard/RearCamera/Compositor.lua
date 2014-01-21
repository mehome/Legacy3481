Pi=3.14159265358979323846

--This is used load persistence of the coordinates last saved etc
function GetSettings()
	local loaded_file = loadfile("CompositorSave_RearView.lua")
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
		},
		path_align =
		{
			width_in = 8.5, length_in = 11,
			pivot_in = 2,
			camera_position =	{ x_in=0, y_in=3.5, z_in=-1.0 },
			camera_rotation =	{ x=0, y_deg=0, z=0 },
			fov_x=67,fov_y=67,
			draw_cube = 0,
			-- draw_cube will render a 1x1 meter cube centered at point 0,0,0.
			-- for practical reasons, you'll need to move the camera position back to at least 0.5 meters.
			-- if you place the camera at -1 meter, and set the orientation to 45 degress in one direction, 
			-- you should see the corresponding front face edge at the center of the screen.
			-- note that if you change 2 angles (say, toward the top right vertex), the corner will of the cube will NOT be centered.
			--    this is normal due to perspective because the corner points are further away.
		},
				
		--Types	"none","square","composite","bypass"
		sequence =
		{
			sequence_1 = 
			{	
				type="composite",
				composite=
				{
					composite_1 = {	type="square", selection=1	},
					composite_2 = {	type="pathalign" },
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
			SequencePOV =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			ToggleLinePlot     = {type="joystick_button", key=6, on_off=false},
		},
		Joystick_2 =
		{
			control = "gamepad f310 (controller)",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			SequencePOV =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			ToggleLinePlot     = {type="joystick_button", key=6, on_off=false},
		},		
		Joystick_3 =
		{
			control = "airflo",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
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

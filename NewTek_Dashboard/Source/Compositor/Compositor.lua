Pi=3.14159265358979323846

--This is used load persistence of the coordinates last saved etc
function GetSettings()
	local loaded_file = loadfile("CompositorSave.lua")
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
				thickness_x=3,thickness_y=40,
				r=255,g=255,b=100
			},
			square_reticle_3 =
			{
				thickness_x=40,thickness_y=3,
				r=0,g=255,b=255
			}
		},
		
		line_plot_props =
		{
			--plot arm
			line_plot_list_1 = 
			{
				line_1 = { name="voltage", r= 16,g=240,b= 16},
				line_2 = { name="actual y",   r=10,g=128,b=240, scalar=1/(Pi/2) }, 
				line_3 = { name="desired y",   r=240,g=10,b=10, scalar=1/(Pi/2) }, 
				line_4 = { name="actual velocity",   r=10,g=240,b=240, scalar=1/5 }, 
				line_5 = { name="desired velocity",   r=240,g=10,b=240, scalar=1/5 }, 
				line_6 = { name="pid error offset",   r=240,g=240,b=128, scalar=1/3 }, 
			},
			line_plot_list_2 = 
			{
				line_1 = { name="actual y",					r=128,g=128,b=128, scalar=1/10, offset=   0}, 
				line_2 = { name="LeftVoltage",				r= 16,g=240,b= 16, scalar= 0.5, offset=-1/2},
				line_3 = { name="LeftEncoder",				r= 10,g=240,b=240, scalar= 1/8, offset=-1/2}, 
				line_4 = { name="desired velocity-left",	r=240,g= 10,b=240, scalar= 1/8, offset=-1/2}, 
				line_5 = { name="pid error offset-left",	r=240,g=240,b=128, scalar= 1/6, offset=-1/2}, 
				line_6 = { name="pid cs-left",				r=240,g=153,b=16 , scalar= 1/6, offset=-1/2},
				line_7 = { name="RightVoltage",				r= 16,g=240,b= 16, scalar= 0.5, offset= 1/2},				
				line_8 = { name="RightEncoder",				r= 10,g=240,b=240, scalar= 1/8, offset= 1/2}, 
				line_9 = { name="desired velocity-right",   r=240,g= 10,b=240, scalar= 1/8, offset= 1/2}, 
				line_10 = { name="pid error offset-right",  r=240,g=240,b=128, scalar= 1/6, offset= 1/2}, 
				line_11 = { name="pid cs-right",			r=240,g=153,b=16 , scalar= 1/6, offset= 1/2}, 
			},
			line_plot_list_3 = 
			{
				line_1 = { name="actual y",					r=128,g=128,b=128, scalar=1/10}, 
				line_2 = { name="RightVoltage",				r= 16,g=140,b= 16, scalar= 0.5},
				line_3 = { name="RightEncoder",				r= 10,g=128,b=240, scalar= 1/4}, 
				line_4 = { name="desired velocity-right",   r=240,g= 10,b= 10, scalar= 1/4}, 
				line_5 = { name="pid error offset-right",	r=140,g=140,b=128, scalar= 1/3}, 
				line_6 = { name="pid cs-right",				r=140,g=153,b=16 , scalar= 1/3}, 
				line_7 = { name="LeftVoltage",				r= 16,g=240,b= 16, scalar= 1.0},
				line_8 = { name="LeftEncoder",				r= 10,g=240,b=240, scalar= 1/4}, 
				line_9 = { name="desired velocity-left",	r=240,g= 10,b=240, scalar= 1/4}, 
				line_10 = { name="pid error offset-left",	r=240,g=240,b=128, scalar= 1/3}, 
				line_11 = { name="pid cs-left",				r=240,g=153,b=16 , scalar= 1/3},
			}
		},
		
		--Types	"none","square","composite","bypass"
		sequence =
		{
			sequence_1 = {	type="square", selection=1	},
			sequence_2 = 
			{
				type="composite",
				composite=
				{
					composite_1 = {	type="square", selection=2	},
					composite_2 = {	type="square", selection=3	},
				}
			},
			sequence_3 = {	type="bypass" },
			sequence_4 = {	type="line_plot", selection=1	},
			sequence_5 = {	type="line_plot", selection=2	},
			sequence_6 = {	type="line_plot", selection=3	},
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
			ToggleLinePlot     = {type="joystick_button", key=6, on_off=false},
			SequencePOV =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
		},

	},
}

Compositor = CompositorProps

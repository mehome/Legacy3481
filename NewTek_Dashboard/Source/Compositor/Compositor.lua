
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
				thickness_x=5,thickness_y=5,opacity=0.6,
				r=0,g=255,b=100,

				use_shadow='y',
				shadow =
				{
					thickness_x=5,thickness_y=5,opacity=0.2,
					r=0,g=0,b=0
				},
				x_offset=1,y_offset=1
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
		
		--Types	"none","square","composite","bypass"
		--TODO composite, and perhaps alpha bitmap and vector from NIVision
		sequence =
		{
			sequence_1 = {	type="square", selection=1	},
			sequence_2 = {	type="square", selection=2	},
			sequence_3 = {	type="square", selection=3	},
			sequence_4 = {	type="bypass" },
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
		},
		
		Joystick_2 =
		{
			control = "airflo",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
			--These sequence events happen whether or not you are in edit so choose what you want to use during the game
			--The POV is not supported by the driver station so it is the perfect chose... I've added buttons here to show if you need to use them instead
			NextSequence     = {type="joystick_button", key=2, keyboard='y', on_off=false},
			PreviousSequence = {type="joystick_button", key=3, keyboard='y', on_off=false},
			SequencePOV =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
		},

	},
}

Compositor = CompositorProps

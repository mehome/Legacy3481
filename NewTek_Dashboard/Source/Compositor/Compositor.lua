
CompositorProps = {
	settings =
	{
		x_scalar=0.05,
		y_scalar=0.05,
		square_reticle =
		{
			thickness=5,
			r=0,g=255,b=100
		}
	},
	controls =
	{
		Joystick_1 =
		{
			control = "logitech dual action",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
		},
		
		Joystick_2 =
		{
			control = "airflo",
			SetXAxis = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			SetYAxis = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.3, curve_intensity=0.0},
		},

	},
}

Compositor = CompositorProps

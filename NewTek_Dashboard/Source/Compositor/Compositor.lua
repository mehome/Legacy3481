
CompositorProps = {
	settings =
	{
		x_scalar=0.05,
		y_scalar=0.05,
		square_reticle_props =
		{
			square_reticle_1 =
			{
				thickness_x=5,thickness_y=5,
				r=0,g=255,b=100
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

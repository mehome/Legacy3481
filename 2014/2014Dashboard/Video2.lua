--This is used load persistence of the coordinates last saved etc
function GetSettings()
	local loaded_file = loadfile("Video2Save.lua")
	if (loaded_file==nil) then
		return nil
	else
		loaded_file()  --We can now make the call and it will succeed
		return settings_load
	end
end

DashboardProps = 
{
	settings =
	{
		title= "RearView",
		url= "black_33_640x480",
		robot_ip_address= "localhost",
		--robot_ip_address= "10.34.82.1",
		--robot_ip_address= "10.34.82.2",
		stream_profile= "default",
		left= 1244,
		top= 189,
		right= 1884,
		bottom= 669,
		--smart_dashboard= "java -jar C:\\WindRiver\\WPILib\\SmartDashboard.jar ip localhost",
		smart_dashboard="none",
		window_name= "SmartDashboard",
		is_popup= 0,
		plug_in= "Compositor.dll",
		aux_startup_file="",
		aux_startup_file_args="",
		record_frames= 0,
		record_path= "D:/media/Robot_Capture/RearView",
		
		--We can control which of these we want to have persistence
		--ignore_position=true,
		--ignore_url=true,
		--ignore_popup=true,
		--ignore_record=true,
		
		--This must stay here to load the settings
		load_settings = GetSettings(),
	},
}

Dashboard = DashboardProps

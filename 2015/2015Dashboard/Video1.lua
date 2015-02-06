--This is used load persistence of the coordinates last saved etc
function GetSettings()
	local loaded_file = loadfile("Video1Save.lua")
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
		--Pick which overlay you wish to use by giving it a title name
		--title= "Main",
		--title= "Main_2014",
		title= "FOV_Test",
		--No camera!  use this
		url= "black_33_600x800",
		--url= "10.34.81.11",
		--url="10.28.1.20",
		--by default RTSP uses 554 and is now allowed for FMS
		port=554,
		--port=1180,  --This has worked in past years... good backup if there are any issues
		robot_ip_address= "localhost",
		--robot_ip_address= "10.34.81.2",
		--robot_ip_address= "10.34.82.2",
		--robot_ip_address= "10.28.1.2",
		stream_profile= "default",
		left= 596,
		top= 8,
		right= 1196,
		bottom= 808,
		--smart_dashboard= "java -jar C:\\WindRiver\\WPILib\\SmartDashboard.jar ip localhost",
		--smart_dashboard= "C:\\WindRiver\\WPILib\\SmartDashboard.jar",
		smart_dashboard="none",
		--window_name= "SmartDashboard",
		is_popup= 1,
		plug_in= "Compositor.dll",
		--aux_startup_file="Dashboard.exe",
		--aux_startup_file_args="Video2.lua",
		--record_frames= 1,
		record_path= "D:/media/Robot_Capture/",
		
		--We can control which of these we want to have persistence
		--ignore_position=true,
		ignore_url=true,
		--ignore_popup=true,
		--ignore_record=true,
		
		--This must stay here to load the settings
		load_settings = GetSettings(),
	},
}

Dashboard = DashboardProps

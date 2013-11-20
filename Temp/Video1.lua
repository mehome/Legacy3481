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
		title= "Preview",
		url= "E:/Media/Video/WMV/ReboundRedTest.wmv",
		robot_ip_address= "localhost",
		stream_profile= "default",
		--TODO offload to the save file
		left= 537,
		top= 144,
		right= 1216,
		bottom= 653,
		--smart_dashboard= "java -jar C:\\WindRiver\\WPILib\\SmartDashboard.jar ip localhost",
		smart_dashboard="none",
		window_name= "SmartDashboard",
		is_popup= 0,
		plug_in= "Compositor.dll",
		aux_startup_file="",
		aux_startup_file_args="",
		record_frames= 0,
		record_path= "D:\\media\\Robot_Capture\\",
		
		--This must stay here to load the settings
		load_settings = GetSettings(),
	},
}

Dashboard = DashboardProps

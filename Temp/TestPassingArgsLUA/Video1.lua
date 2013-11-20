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

Configuration = 0
URL_ToUse = "10.28.1.20"
function SetupConfiguration(config)
	Configuration=config
	if (Configuration=="1") then
		URL_ToUse="black"
	end
end


function Test()
	return URL_ToUse
end

DashboardProps = 
{
	settings =
	{
		title= "Preview",
		url= Test(),
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

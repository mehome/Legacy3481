I've put the binaries in this link:
https://www.dropbox.com/sh/ev32dvxz42s0yao/AACHi6MxoZZlHKxTjN0Nu3loa?dl=0

When using a web browser you'll see release, and TestServer.exe (skip to line below on TestServer)... and in there you have 3 Dll's and one exe
These files end up in this folder... where this folder (where this ReadMe) is located here:

.\Code\2016\2016Dashboard

This folder needs Video1.lua (included in this folder) which is the first thing Dashboard.exe reads
It controls the overall settings of the vision.
I have this file set to run the compositer which in turn runs the processing vision... we can change
Compositor_Main.lua in regards to what overlay we want to see with the camera.

I currently have this setup to display a black window with a square


---------------


TestServer.exe
This file makes it easy to test the vision without a robot... it will establish itself as the robot in regards to NetworkTables, which in turn allows our Dashboard to communicate to the smart dashboard one test you can do is this:

1. Launch TestServer.exe
2. Launch Dashboard.exe
3. Launch SmartDashboard using this:
java -jar C:\Users\YOURPATH\wpilib\tools\SmartDashboard.jar ip localhost

Note: Video1.lua needs to also be configured to localhost if not already.
When runing this setup... blank out the smart dashboard by using the new... this will help avoid the clutter.

Right away the smartdashboard with show
Edit Position Main
Rotation Velocity
Sequence
Velocity
Main_Is_Targeting

From this... our dashboard with show the black with the square... use the pov hat to advance the sequence or you can type it in the sequence (sorry that this is floating... you can put 2 here).  The square disappears and now vision processing sequence is on... now look at Main_Is_Targeting... you can either change this to true or right-click on our dashboard and choose targeting...  This should reflect what is written in the smart dashboard... when true the X Position and Y Position should be shown... this is what vision sends back to the robot  (look for distance soon).  There is also a vision directory that allows to set the thresholds... For now ignore as this will slightly change as it is still set for 2014's game.  We'll keep the option for targeting ball but most-likely will not have this for alamo.


The robot can programatically write to determine the sequence and if it is targeting... so auton can use sequence 2, while teleop can use sequence 1.  The POV can be disabled for competition if necessary, by the settings in Compositor_Main.lua.  Note: some of the other joystick controls in there only are used when in edit mode, only the POV is always active.

One other thing worth mentioning... I'd recommend reading Main_Is_Targeting to control when to turn on the ring lights... in testing at home these get hot and drain the battery.  It probably will be fine in competition, but during practice it may really help make a difference... it also can help you determine if it is in the right sequence and that it is turned on during auton.


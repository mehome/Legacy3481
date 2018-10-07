
GCode Tools
In its current state is a commnd line driven program that currently supports adding tabs to tool jobs of GCode files (e.g. .nc extention), and producing GCode that can make your CNC machine sing.  I may have other things in here as I work with parts on the machine.

Since adding tabs is very simple to use... I'll include them here.  NotePlayer has its own readme.

As it currently stands adding tabs has been designed to work well with HsmExpress editor with backplot support.  This is free and there are probably other backplot editors out there.  You'll really need a backplot editor to use this as you'll need to know what line number you wish to insert the tab.  

I currently support 2 scenarios... adding a tab in between a straight line, and adding a tab in between an arc.  If you need to add a tab in between a span of points it may be easy enough to do this manually in the GCode, since it requires no insertion of new gcode.  I will plan to add this case eventually.

Commands:------------------------------------------------------------------------------------------

load_tj <filename>
Loads the tool job GCode (e.g. .nc extention)

SetWorkingFile [filename blank=console dump]
Specifies working folder, it may be the same as the source, and if it is you'll only be able to perform one tab at a time and need to reload after each tab is added (this clears the tab list and factors in the inserted code)

SetTabSize <height> <width> use zero for default
In inches... note the height needs to factor in the extra depth that goes below the stock, so if you go .02" below and want a .06" height use .08.

AddTab_hw <line number> [offset=0] <height> <width>
AddTab <line number> [offset=0]
This does all the tab work and will modify the working file once it is added, you may be able to leave the file viewed in the backplot editor and it will update with the changes (Hsm Editor does this).

RemoveTab <line number>
This is ideal for tweaking a tab's offset as well as its properties (i.e. height width).  In this scenario you can load the source, use a different working file name, and continuously remove and add it again, or just remove it and add it to a new line number

Examples:------------------------------------------------------------------------------------------
CasterContourTest.nc
This can demonstrated the arc tab abilites try
AddTab 292 0.5
Then load that file (with the modified tab)
AddTab 297 0.5

Loading from a modified file allows for consecutive tabs to be added in a tight space.

TabTest.nc
AddTab 41 1.0
This is a simpler tab along side a straight cut
In this example keep the source file use a destination file (different name)
RemoveTab 41
AddTab 41 0.5
Now the tab is moved to a different place


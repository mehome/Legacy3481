
GCode Tools
In its current state is a commnd line driven program that currently supports adding tabs to tool jobs of GCode files (e.g. .nc extention), and producing GCode that can make your CNC machine sing.  I may have other things in here as I work with parts on the machine.

Since adding tabs is very simple to use... I'll include them here.  NotePlayer has its own readme.

As it currently stands adding tabs has been designed to work well with HsmExpress editor with backplot support.  This is free and there are probably other backplot editors out there.  You'll really need a backplot editor to use this as you'll need to know what line number you wish to insert the tab.  

I currently support 2 scenarios... adding a tab in between a straight line, and adding a tab in between an arc.  If you need to add a tab in between a span of points it may be easy enough to do this manually in the GCode, since it requires no insertion of new gcode.  I will plan to add this case eventually.

One other thing to mention on the backplotter.  Ideally it would be nice to run 2 instances of it, but HsmExpress does not.  So working with one.  You can initially view the source file to work out the line numbers and then switch over to viewing the modified folder to see the tabs.  When viewing the modified file beware that the line numbers may not match what is currently loaded in memory.  It depends on where the patch code is written... but they will match if an apply command is used.  See below for details on the apply command.

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

LoadProject <filename>
Ability to save all tabs added in one setting as well as the source, and outfile names, and the default tab size used.  This workflow makes it possible to handle most of the tabs under different sessions while keeping ability to edit and remove tabs as needed.  Ideally this is the work flow to use... along with the Apply Command

SaveProject [filename blank=console dump]
Saves the project settings including the source and oufile names and default tab size

Apply [disable update=0]
This makes it possible to update a batch of tabs to a modified file that is specified in SetWorkingFile, and it works in a way to keep the original source file left intact for future sessions.  The disable update argument is for completeness and has the same effect of adding or removing a tab, leaving this blank, the modified GCode gets updated and the internal state of the GCode also has the modification, and this history is recorded when saving the project, so that it can be applied later in the same order.  Ideally, using apply solves the problem when needing tabs on consecutive segmented lines, or having multiple tabs in a long segment.  Ideally reserving the use of this for these cases will give the best performance during loading and saving of the project.

SetMultiPass <enabled> [pass length] [number of passes]
This makes it possible to apply one tab to a contour cut that makes multiple passes.  This works under the assumption that each pass is exactly the same and the round trip is the same length per pass.  It will verify the lines are the same before applying the tab, and warn you if they are not.  This works backwards only, so it needs to start with the last pass, and the user must detemine the length of the passes.  This is as simple as subtracting any matching line numbers of GCode, and it would be good to see if it is consistent.  This feature becomes more necessary the shallower cuts that need to be made an example of this is a 0.0165" pass on 1/8" aluminum.  So to detemine the number of passes 0.125/0.0165 = 7.57 passes, but that is for the total height... if we want to use tab hieght say 0.08, it would be 0.08/0.0165 = 4.8.  So what this does is it will simply add tabs on the corresponding linenumbers before for each number of passes specified.  The result is one tab being made on one addtab call, instead of needing to make 5 calls for each pass.  See example below for more details on a workflow of this.

Examples:------------------------------------------------------------------------------------------
A completed example is 
CasterContour.ini
Type 
>loadproject CasterContour.ini
It will say successful and generate CasterContour2.nc with all the tabs in it
If you want... type 
>SaveProject
(and leave the argument blank)
It will show a list of all the tabs added on the console dump

Skip over this section if you wish to use multi pass--------------
It didn't take me too long to make this, and it does shallow passes needed for M3.  What I've found that works best is to work backwards only making apply updates for consecutive tabs.  I also found to actually edit the .ini file directly when doing the other passes as each pass line is a constant distance from the next.  I implement the tabs on the last 3 passes.  So when I had a huge batch of tabs that can be on one update I was able to copy them all and subtract the line count difference.  This is a bit of a pain and I may add something to automate that in the near future.  In looking at the .ini file I did put some Apply = NoUpdate breaks in between each group run.  This helped me to keep track of where I was... ideally I should allow for comments if the work flow really becomes ideal to edit the project file itself, but for now this will suffice.  When I add new tabs I used the command line to do it... and then looked at the back plot to fine tune the offset, once I like it... I then edited the project file for the other passes as mentioned previously.  Now let's say I need to add yet more height to the tab... I would have to edit the previous pass as well, so yes that is a pain... as for the width each tab add can have its own width.  On a one pass all of this is trivial.
---------------------------------------------------------------------------------------------------

SetMultiPass---
Type
>loadproject MultiPass_test.ini
It will say successful and generate MultiPass_test.nc, which is really CasterContour.ini in a state where a long list of tabs are ready to be made, while working with this load up CasterContour.ini on a separate text editor to observe how this works.

Like before we work backwards, and to avoid having to switch the backplot start with line 1533 and keep multipass off.  First step between 1533 and 1534 to determine this is where you want to add a tab... now scroll through the code and take note that the same pass of this happens on line 1308.  Subtract 1533-1308 to get 225... you'll need this number when you active the multipass.  Go ahead and add the tab, and keep moving backwards 1491... step between 1491 and 1492 to know you want a tab here... add it with 0.5" offset... let the backplot reload... see the inserted patch of code... step through it and see if the tab looks like it is in the right place.  Note: since we are on the last pass we can't see any depth of the tab, but we can see where it will be and this will have to suffice, to avoid needing to switch files to view.  Keep moving backwards, and see how these numbers are laid out in CasterContour.ini adding them as you go, and once you've made the round trip of the pass with all the tabs you want.  In this case I chose 14 tabs... type saveproject to list them all out.  Now save this list on a notepad, and activate the multipass.  If we use the default height of 0.08 we'll need 0.08/0.02=4 passes, and we already know about the 225 earlier.  Type:
>SetMultiPass 1 225 4

Now with that list of 14 tabs, enter them again, once entered all the tabs will be in place.  If you save project you can see each tab was added per pass and this compares to the manual method in CasterContour.ini (which only does 3 passes).  As done there... I apply it here then move to the next section.


In this iteration from a command point of view using the multi pass with the addtab restores the meaning of what a tab is and intended to be a tab, but the project data does not yet reflect this... instead it reflects a tab slice within a multipass.  I may change this in the future to allow multipass to be a part of the project, but for now it works out fine as is, and it may not be necessary to change since it is not directly used to be read in the workflow.

---------------------------------------------------------------------------------------------------

Try loading the CasterContourProject.ini (included in archive)
This will add tabs to a file named CasterContour_Modified.nc

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


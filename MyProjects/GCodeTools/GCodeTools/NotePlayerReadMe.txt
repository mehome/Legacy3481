
First thing about Note Player is that the compact text method (currently the only supported method) of note entry is a similar model from the note support on the commodore 128, and the concept of using play blocks is borrowed from the work flow of OctaMed (for the Amiga).  I use the term block, but this represents a line of text.  Typically on OctaMed a block by default was 64 elements.  A text line can be as long as you would like, in my example I ported over a song from OctaMed and divided the block in half for a good size to work with and test.

Note: Currently the player has a clicking sound once a second... I'm still trying to fix this.

Commands:------------------------------------------------------------------------------------------
load_ct [filename]
Load a compact text song

playb <block number=0>
Play a particular line of text... this number is ordinal (starting with 0), and most text editors start with line 1, so be aware when picking a block number. 

plays <position=0.0>
Currently the position is not yet supported.  This will play all the blocks, and currently there is no support for a playing sequence.

stop
Stops playing, and stops streaming the task in the player.

pause (toggles state)
Pauses the play, able to resume in same place

reversech <0 normal 1 reversed>
Able to reverse the channel assignments... this comes in handy when testing tuning methods against themselves, by running two instances of the note player... reversing one, and using the pause to align them both to play at the same time.

export [filename]
Exports the GCode to filename.  This should be able to be played on the GCode sender as-is

setbounds <x,y,z>
By default this is 4 , 4 and 1.   Where in inches specifies how far out it moves in each direction from center before flipping back.  When reviewing the GCode the absolute coordinates reflect the max.  You have to factor in padding in my tests it's about .2 more as it plays a note and then checks the bounds (it was easier logic to implement this way).


Examples:------------------------------------------------------------------------------------------

Here are a couple of examples (These are included in archive)

TestSong.txt   (only using 2 voices, so you can keep router in spindle mount)
tg80	v1o4sreao5co4bebo5diceo4#go5e	v2o2iao3a#grsaeao4co3bebo4d
        v1o4saeao5co4bebo5dico4aqr	    v2o4ico3a#gesaeao4co3bebo4d
v1o5s rece o4a o5c o4e g i fa o5d f	s fd o4b o5d o4gbdf	v2i o4c o3a o4c o3a s o4d o3afadf o2b o3d i o2b o3dgb

FugaVIII_tc.txt  (using 3 voices)
k$e u1  tg90 v2o4 q#d .#a ib #a #g #f #g q#a #d .#g v1 wr r ir
	v2o4i#f qf .#d if q#f #g .#a i#g  v1 .qr qo4#a o5.#d i#f f #d #c #d qf
v1o4 q#a o5.#d i#c qc io4#a o5c q.#c q#c io4b q#a  v2o4 i#f f #f sf #d if q#a ia .q#a i#g g #d .q#g i#f
	v1o5 .q#d id #d f q#f if #f #g f #f #g	    v2o4 if  #d qf i#f #g #a o5c d #d f d #d f  v3 .hr o3q#d .#a ib #a #g

Where:
tg is time global in beats per minute
k = *key signature (optional used for mean tone tuning)
u = tuning method (0=equal temperament default, 1=mean tone)
v - voice  (this uses 2 voices, but can support up to 8... will have demo using the z axis soon)
o - octave
w, q, h, i, s, 3  <-- note duration (and use of . as well)-  whole, half, quarter, eighth, sixteenth, 32nd
# sharps,   $ flats
a, b, c, d, e, f ,g  - notes

*For key signature... lowercase indicates minor, and capital indicates major, the accidental must proceed the key if it has one.  It is possible to use any key with mean-tone and it still sounds very much in tune, and the key can change on a per-block basis, so it is possible to modulate the key signature as the melody modulates for the best tuning.


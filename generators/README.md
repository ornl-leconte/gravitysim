# generators


Generators are ways to desribe how to spawn particles.

They're basically just storing commands which map to C functions in `gravitysim.c`

Keep in mind the world space is [-100, 100] for x, y, z values, and these are in meters, `m`

Mass should be above 0, and below 10000 for most normal values. The values are in kilograms, `kg`

x is left/right (by default, the camera view can changes this)
z is backward/forward (again, camera view can change this)
y is always down/up (camera view does **not** change `y`)


## Functions

You use functions by doing this:

`FUNCNAME arg0 arg1 ...`

# func 'p'

The most basic function is `p` which spawns a single particle:

`p POSITIONX,POSITIONY,POSITIONZ [MASS] [VELOCITYX,VELOCITYY,VELOCITYZ]`

where `[MASS]` and `[VELOCITYX,VELOCITYY,VELOCITYZ]` are optional

`[MASS]` defaults to 1.0, and all velocities default to 0.0 as well

So, all these calls do the same thing:

`p 10.0,50.0,40.0`
`p 10.0,50.0,40.0 1.0`
`p 10 50,40 1 0,0,0`


# func 'g'

The `g` func spawns a grid of particles

`g STARTX,STARTY,STARTZ ENDX,ENDY,ENDZ NX,NY,NZ [MASS]`

Creates a lattice of total num of `NX*NY*NZ` particles evenly spaced between the start and end

So,

`g 0,0,0 10,10,10 10,10,10`
`g 0,0,0 10,10,10 10,10,10 1.0`

spawns 1000 particles spaced with 1 block between each each having a mass 1







-- The server running bsod-server.
network_host	= "bsod-server:34567"

-- Screen resolution
width		= 1024
height		= 768
bpp		= 32

-- Display in fullscreen or windowed mode
fullscreen	= "yes"

-- Initial location and facing
start_location	= {3.876768,0.004369,22.06319}
pitch	= 1.0
heading = 12.3

-- Initial Particle speed and size
-- Increasing the speed can improve framerate on slower graphics cards
-- and make it easier to visualise heavy traffic
speed				= 1.0

-- Decreasing teh size of a particle can improve framerate, and make
-- visualisation of heavy traffic more readable.
size				= 1.0

-- Brightness of each particle
particle_opacity		= 0.5

-- "jitter" particles around, makes bulk transfers look
-- thicker
jitter				= "yes"

-- Always have particles facing the screen.  This requires
-- your graphics card to support "point sprites"
billboard			= "yes"

-- The image of an individual packet
particle			= "data/particle.png"

-- Release unused memory to the Operating System
do_gcc				= "yes"

-- Hides the menu (for use in public displays)
no_gui				= "no"

-- Hides the mouse cursor (for use in public displays)
no_cursor			= "no" 

-- Start and end positions (for the planes only):
start_x				= -15.0
end_x				= 15.0

-- OpenGL is the only display mode supported currently
display		=  "opengl" 

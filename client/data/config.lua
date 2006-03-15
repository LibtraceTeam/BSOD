-- BSOD client configuration file
-- comments are prefixed with a double hyphen
-- see the README file for more information on configuration options


--------------------------------------------------------
-- NETWORK OPTIONS
-- host running the bsod server, and the port the server is listening on
-- in "host:port" notatio
network_host	= "host:32500"



--------------------------------------------------------
-- CAMERA OPTIONS
-- start location for the camera {X, Y, Z} coordinates
start_location	= {4.23,0.00,21.73}

-- pitch (angle upwards/downwards) the camera should start with
pitch = 1.0

-- heading (angle left/right) the camera should start with
heading = 12.3



--------------------------------------------------------
-- WINDOW OPTIONS
-- leave this as opengl
display			=  "opengl"

-- If "yes", will run in fullscreen mode, "no" means run in a window.
fullscreen		= "no"

-- Resolution to run the bsod client in. If fullscreen mode is on make
-- sure you use values supported by your monitor and videocard.
width			= 1024
height			= 768
bpp			= 32



--------------------------------------------------------
-- MISC OPTIONS
-- Multiplier to the speed particles travel across the display.
speed = 1.0

-- Size the particles should be drawn.
size = 2.0

-- Add jitter between particles based on the time since the last frame
-- was drawn. This helps to smooth out the display and prevent banding
-- when the frame rate is low.
jitter = "yes"

-- Do billboarding using point sprites. This is faster and looks nicer
-- on hardware that supports this feature. If BSOD crashes on startup
-- try disabling this to fall back to the textured quads method of
-- drawing particles.
billboard = "yes"

-- Path to the image that should be used for the particles.
particle = "data/particle.png"

-- Do extra garbage collection on unused flows to remove them from the freelist.
-- If you have a lot of memory and large volumes of traffic turning this off
-- may be a good option as there may be a slight pause when the garbage
-- collection cycle runs.
do_gcc = "yes"

-- When this is set to "yes" the GUI menu is hidden.
no_gui = "no"


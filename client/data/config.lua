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

-- if "yes", will run in fullscreen mode, "no" means run in a window
fullscreen		= "no"

-- resolution to run the bsod client in
width			= 1024
height			= 768
bpp			= 32



--------------------------------------------------------
-- MISC OPTIONS
-- multiplier to the speed particles travel across the display
speed = 1.0

-- size the particles should be drawn
size = 2.0

-- add jitter between particles based on the time since the last frame
-- was drawn. This helps to smooth out the display and prevent banding
-- when the frame rate is low
jitter = "yes"

-- billboarding is a work in progress, leave this as "no" for now
billboard = "no"

-- path to the image that should be used for the particles
particle = "data/particle.png"

-- do extra garbage collection on unused flows to remove them from the freelist
do_gcc = "yes"


BSOD Client 2.0.2 for Windows

-------------------------------------------------------------------------------
Copyright (c) 2005-2011 The University of Waikato, Hamilton, New Zealand.
All rights reserved.

This code has been developed by the University of Waikato WAND
research group. For further information please see http://www.wand.net.nz/.
-------------------------------------------------------------------------------

This package contains a precompiled Win32 executable of BSOD client, a 
network traffic visualisation tool which displays the flow of network data 
between hosts in real time. BSOD client is only one half of the visualisation
process; it connects to a BSOD server which tells the client what to render
on screen.

Further information about both the BSOD client and server can be found at
http://research.wand.net.nz/software/visualisation.php

This particular build was created using Visual Studio 2010 and tested on a 64 
bit Windows 7 install. We expect that it will work fine on Vista machines, but
has not been explicitly tested as yet. Windows XP may or may not work, but feel
free to try your luck. If you absolutely must have a working version of BSOD
client for Windows XP, get in touch and we'll see what we can do.

Running BSOD Client
===================
IMPORTANT for Windows users: If you have not installed Visual Studio on your
machine, you may not have all the development libraries required to run BSOD
client. The file vcredist_x86.exe (included in the zip file) will install 
these libraries for you. 

Edit the configuration file "bsod2.cfg" to suit your requirements (see 
Configuration below) and then simply double-click on the bsodclient executable.
It will automatically attempt to connect to the server specified in your
config file, if you have specified one.

Configuration
=============
The "bsod2.cfg" file provides all the configuration for the BSOD client. Each 
configuration option is documented within the bsod2.cfg file itself.

The config file can be used to set the BSOD server to connect to, the 
screen resolution and the method to use to render the packet particles.

Configuration options are pairs of the form:

<key> = <value>

The options supported by bsodclient are described below:

option: Server
	The bsodserver to connect to upon launch. This may be a hostname or an
	IP address. If you do not specify a server, you can manually connect
	using the server browser within bsodclient itself.

option: Port
	The port on the bsodserver to connect to.

option: LeftTex
	Specifies a texture to use on the left plane instead of the texture
	sent by the server.

option: RightTex
	Specifies a texture to use on the right plane instead of the texture
	sent by the server.

option: ResX
	Defines the screen resolution along the X axis. If 0 or not defined,
	the current desktop resolution will be used.

option: ResY
	Defines the screen resolution along the X axis. If 0 or not defined,
	the current desktop resolution will be used.

option: Fullscreen
	If 1, will run bsodclient in full screen mode. If 0, will attempt to
	run in windowed mode. Defaults to 1.

option: ParticleMethod
	Selects the method to render the particles. Possible options are:
	
		0 - autodetect, will try to guess the best option for you.
		1 - triangles, will work on just about anything but is slow.
		2 - pointsprites
		3 - shaders
		4 - texture (experimental)

	Note: autodetection of the particle method is not always reliable. If 
	the BSOD client is performing poorly, try manually setting the particle 
	method to pointsprites or triangles.

option: ParticleSizeScale
	Specifies a multiplier to apply to the size of the particles.

option: ParticleSpeedScale
	Specifies a multiplier to apply to the speed of the particles.

option: MaxFrameRate
	Specifies an upper limit on the frame rate. If 0, the frame rate is
	unlimited.

option: DropPacketThresh
	Specifies the minimum frame rate before bsodclient starts dropping
	packets.

option: DropFlowThresh
	Specifies the minimum frame rate before bsodclient starts dropping
	entire flows.

option: ShowDarknet
	If 1, darknet traffic will be shown. If 0, darknet traffic will not be
	displayed. Can also be toggled at runtime via the Options window.

option: ShowNonDarknet
	If 1, non-darknet traffic will be shown. If 0, non-darknet traffic will 
	not be displayed. Can also be toggled at runtime via the Options window.

option: FlipTextures
	If the textures are loaded upside down, set this option to 1 to flip
	them up the right way.

Navigation

==========


The camera can be moved using both the keyboard and mouse. Clicking and dragging with 
the 
left mouse button will adjust the direction that the camera is pointing. The camera

orientation can also be changed using the arrow keys on the keyboard. 



The location of the camera can be changed using the W, S, A and D keys on the keyboard. 
W 
moves the camera forward, S will reverse, A will strafe left and D will strafe right. 



All camera movements made while holding down the left shift key will be 10x faster.



If all else fails, pressing Space will return the camera to its original position and
 
orientation.



Acknowledgements
================

The original BSOD client was developed by Sam Jansen and Brendon Jones and was
based on the BuNg 3D engine written by Sam Jansen and Jesse Baker 
(http://www.wand.net.nz/~stj2/bung/).

The current BSOD client was developed by Paul Hunkin and uses OpenGL.

The Windows executable was created by Shane Alcock.

For further information or to report bugs, please email contact@wand.net.nz.

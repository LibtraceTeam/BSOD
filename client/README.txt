*******************************************************************************
                             BSOD2 client README
*******************************************************************************


Dependencies:
--------------

You will need these development libraries installed:
- SDL
- SDL_net
- SDL_ttf
- DevIL 
- GLEW
- libconfuse
- CEGUI (v0.5)


Linux build instructions: 
-------------------------

On a typical Ubuntu system, the libs can be installed by issuing the following:

	sudo apt-get install libsdl-dev libsdl-net1.2-dev libsdl-ttf2.0-dev \
                         libdevil-dev libglew-dev libconfuse-dev \
                         libcegui-mk2-dev
                         
Then:
	make
	
The binary will be created in the 'runtime' folder. Run it from that location. 
                         

Windows build instructions: 
---------------------------

A Visual Studio 2008 solution is in the 'windows' folder. It may need adjusting
to set paths for your computer. 





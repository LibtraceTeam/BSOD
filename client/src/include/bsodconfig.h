/* 
 * This file is part of BSOD client
 *
 * Copyright (c) 2011 The University of Waikato, Hamilton, New Zealand.
 *
 * Author: Paul Hunkin
 *
 * Contributors: Shane Alcock
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND research
 * group. For further information please see http://www.wand.net.nz/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */


/*******************************************************************************
							BSOD2 Client - config.h
							
 This provides some #defines for compile-time options and defaults for config
 file options. 
*******************************************************************************/

//#define ENABLE_CGL_COMPAT //for running under clustergl
//#define ENABLE_VTF //vertex texture fetch particle system. Doesn't compile :P
#define ENABLE_GUI //comment out to not require CEGUI
#define DEFAULT_PORT 34567
#define CONFIG_FILE "bsod2.cfg"
#define PARTICLE_FPS fParticleFPS //hack!
#define MAX_FLOW_DESCRIPTORS 255
#define MAX_PARTICLES 1000000 //global cap of 1m particles for particle systems
#define SLAB_SIZE 40
#define GUI_HIDE_DELAY 5.0f //seconds
#define DEFAULT_MAX_FRAME_RATE 60

//Toggles between std::map and unordered_map
#ifndef _WINDOWS 
	#define USE_TR1 //On Windows we don't have TR1, at least not without pain
	#define PACKED __attribute__((packed));
#else
	#define PACKED //MSVC doesn't have __attribute__
	#define vsnprintf _vsnprintf //Windows puts a _ on the start of this...
#endif

//UDP ports
#define UDP_SERVER_PORT 2080 //the base port the server listens to broadcast on
#define UDP_PORT_RANGE 5

//Displayed in the options window
#define CLIENT_VERSION "2.0.1"

//The time between particle system cleanups
#define CLEANUP_TIMER (60.0f * 60.0f); //an hour

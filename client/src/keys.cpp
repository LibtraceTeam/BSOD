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


#include "main.h"

#define MAX_KEYS 512
bool keyArray[MAX_KEYS];

/*********************************************
		Called on key-down
**********************************************/
void App::handleKeyEvent( SDL_keysym *keysym , int type )
{
	switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
		//ESC key was pressed 
		notifyShutdown();
		break;
	case SDLK_F1:
		//Toggle fullscreen
		SDL_WM_ToggleFullScreen( surface );
		break;
	/*
	case SDLK_F2:
		fTimeScaleScale = 1.0f/1000.0f;
		break;
		
	case SDLK_F3:
		fTimeScaleScale = 10.0f;
		break;
	*/
	default:
		break;
	}
		
	keySetState(keysym->sym, type == SDL_KEYDOWN);		
	
	return;
}


/*********************************************
		Set initial key state
**********************************************/
void App::keyInit(){
	for(int i=0;i<MAX_KEYS;i++){
		keyArray[i] = false;
	}
}


/*********************************************
		Other util functions
**********************************************/
void App::keySetState(int code, bool pressed){
	keyArray[code] = pressed;
}

bool App::keyDown(int code){
	return keyArray[code];
}

void App::onMouseEvent(int code, int eventType){
	bMouse[code] = (eventType == SDL_MOUSEBUTTONDOWN);	
}

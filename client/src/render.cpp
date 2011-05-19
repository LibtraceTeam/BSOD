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

float fTimeScale = 0.0f;

/*********************************************
  Update the FPS counter, and wait if needed
**********************************************/
void App::calcFps(){
	iFrameCounter++;
	
	int thisFrame = SDL_GetTicks();	
	int diff = thisFrame - iLastFrameTicks;
	if(diff >= 1000){
		iLastFrameTicks = thisFrame;
		iFPS = iFrameCounter;
		iFrameCounter = 0;	
	
		const char *t = ctime(&iCurrentTime);
		if(t){
			mStatusString = "Current time: " + string(t);
		}else{
			mStatusString = "";
		}
		
	}
	
}



/*********************************************
		Main render method
**********************************************/
void App::renderMain(){
	calcFps();

	utilBeginRender();
		
	camLook();
	
	if(isConnected()){
		mFlowMgr->render();	
		calculateMousePoint();
	}
		
	mParticleSystem->render();
	
	if(isConnected())	
		mFlowMgr->renderSelection();	
				
	utilEndRender();
}


/*********************************************
			Set the camera transform
**********************************************/
void App::camLook(){
	gluLookAt(	fCameraX, 		fCameraY, 		fCameraZ, 
			fLookX,			fLookY,			fLookZ, 
			0, 			1, 			0		);
				
	glTranslatef(0, 0, fZoom);
				
	glRotatef(fRot[0], 1, 0, 0);
	glRotatef(fRot[1], 0, 1, 0);
	glRotatef(fRot[2], 0, 0, 1);
}


/*********************************************
			Status bar
**********************************************/
void App::drawStatusBar(){
	

	//Set stuff to make fonts work
	glEnable(GL_TEXTURE_2D);
	glColor3f(1,1,1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	//Text
	glColor4f(1,1,1,1);
	
	if(fGUITimeout <= 0.0f){
		/*
		if(!isConnected()){
			writeTextCentered(iScreenX / 2, iScreenY - 30, 
						"Press any key to display the GUI, \
						then use the Servers button to connect to a server");
		}		
		*/
	}else{
		writeText(10, 7, "%d fps, %d particles, %0.1f uptime", 
					iFPS, ps()->getActive(), fUptime);	
		
		if(isConnected()){
	
			//Update the text at the bottom of the screen
			float w = getTextWidth(mStatusString.c_str()) + 10;
			writeText(iScreenX - w, iScreenY - 20, mStatusString.c_str());			
						
			string s = "Connected to " + mServerAddr + 
						":" + toString(iServerPort);						
			w = getTextWidth(s.c_str()) + 10;			
			writeText(iScreenX - w, iScreenY - 40, s.c_str());	
		
		}

	}
	
}


/*********************************************
		Does all the 2D rendering
			(fonts, HUD, etc)
**********************************************/
void App::render2D(){
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	drawStatusBar();
	
	if(isConnected()){
		mFlowMgr->render2d();
	}
	
	
	ps()->render2D();
	
	//Render the GUI

#ifdef ENABLE_GUI
	renderGUI();	
#endif

#ifdef ENABLE_CGL_COMPAT

	//Get rid of the system mouse cursor, it's useless
	SDL_ShowCursor(SDL_DISABLE);

	//render a mouse cursor on our end
	int x = iMouseX;
	int y = iMouseY;
	int s = iScreenX / 32;
		
	Texture *tex = texGet("mouse.png");
	
	if(!tex){
		LOG("No mouse cursor texture!\n");
	}else{
		tex->bind();
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1,1,1,0.0f);
		
		glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex2f(x, y);
			glTexCoord2f(1, 0); glVertex2f(x + s, y);
			glTexCoord2f(1, 1); glVertex2f(x + s, y + s);
			glTexCoord2f(0, 1); glVertex2f(x, y + s);
		glEnd();
		
		glDisable(GL_BLEND);
	}
#endif
	
		
}

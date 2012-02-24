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


/*********************************************
		per-frame logic goes here
**********************************************/
void App::updateMain(){

	//Get the mouse pointer pos in screen space
	SDL_GetMouseState(&iMouseX, &iMouseY); 
	
	//Throw away any bad frames
	if(fTimeScale == infinity || fTimeScale == 0.0f){
		return;
	}
	
	float fCamSpeed = fTimeScale * 10.0f;
	float fDragScale = 0.05f;

	//Right button means we reset rotation and such
	//NOTE: I did have this as 'both left and right at once', which seemed
	//to make more sense, but it doesn't work great on systems that map this
	//to middle-mouse		
	if(mouseDown(3)){
		resetCam();
	}

	//Rotation for the wall
	/*
	float camTime = fUptime * 0.1f;

	float fRotateSpeed = fCamSpeed;
	fRot[1] += fRotateSpeed;
	fRot[0] = (sinf(camTime) * 5);
	fCameraY = cosf(camTime) * 5;
	
	fZoom = (sinf(camTime) * 15) + 27;
	*/
		
	//LOG("%f, %f\n", fRot[1], fZoom);
	
	//fZoom = -10;
	
	//Keyboard rotation
	if(keyDown(SDLK_RSHIFT) || keyDown(SDLK_LSHIFT)){
		fCamSpeed *= 10;
	}
	
	if(keyDown(SDLK_LEFT))	fRot[1] -= fCamSpeed;
	if(keyDown(SDLK_RIGHT))	fRot[1] += fCamSpeed;
	if(keyDown(SDLK_UP))	fRot[0] -= fCamSpeed;
	if(keyDown(SDLK_DOWN))	fRot[0] += fCamSpeed;

	if(keyDown(SDLK_w)) {
		fCameraZ -= fCamSpeed; 
		//fRot[0] = 0.0; fRot[1] = 0.0; fRot[2] = 0.0;
		fLookZ -= fCamSpeed;
	}
	
	if(keyDown(SDLK_s)) {
		fCameraZ += fCamSpeed; 
		//fRot[0] = 0.0; fRot[1] = 0.0; fRot[2] = 0.0;
		fLookZ += fCamSpeed;
	}
	if(keyDown(SDLK_a)) {
		fCameraX -= fCamSpeed;
		fLookX -= fCamSpeed;
	}
	if(keyDown(SDLK_d)) {
		fCameraX += fCamSpeed;
		fLookX += fCamSpeed;
	}
	
	if(keyDown(SDLK_SPACE))	resetCam();
	
	//If we're actively dragging with the mouse
	if(bDrag){
		
		//Figure out the drag vectors and stuff
		Vector2 drag = getMouse();
		Vector2 diff = (dragStart - drag) * fDragScale;
				
		//Left mouse button means we modify the rotation
		if(mouseDown(1)){
			fRot[1] -= diff.x;
			fRot[0] -= diff.y;			
		}
		
		//Middle mouse means we modify the zoom
		else if(mouseDown(2)){
			fZoom += diff.y;
		}
		
		dragVel = diff;
		dragStart = getMouse();
		
		
		
	}else{
		//fRot[1] -= dragVel.x;
		//fRot[0] -= dragVel.y;
	}
	

	updateSocket();
	
	//Hack! The shader system doesn't really play nice with the banner, so
	//we disable it here.
	if(!isConnected()){
		if(ps()->getType() <= PARTICLE_SYSTEM_POINTSPRITES)
			generateTestData();
		else
			fGUITimeout = 1.0f;
	}
	
	mParticleSystem->update();	
}

/*********************************************
	Reset cam orientation, zoom, rotation
**********************************************/
void App::resetCam(){
	for(int i=0;i<3;i++){
		fRot[i] = 0.0f;
	}
	fRot[0] = 20;
	fZoom = 0.0f;
	dragVel = Vector2(0,0);
	
	endDrag();
	camSetPos(0, -SLAB_SIZE/3, SLAB_SIZE);
        camLookAt(0, 0, 0);
}

/*********************************************
	Generate particles that are shown
	while not connected to a server
**********************************************/
float fTestTime = 0.0f;
int iTestRow = 0;
bool isRotating = false;

void App::generateTestData(){
	
	fTestTime += fTimeScale;
	
	Texture *tex = texGet("banner.png");
	byte *data = tex->mData;
	
	if(fTestTime > 0.02f / fParticleSpeedScale){
		fTestTime = 0.0f;
		iTestRow++;
		
		if(iTestRow >= tex->iSizeX - 5){
			iTestRow = 1;
			isRotating = true;
		}
		
		int x = iTestRow;
		
		/* When we loaded this texture, it will have been flipped to
		 * compensate for OpenGL using a bottom up coordinate
		 * system, so we have to be careful how we render each row */
		for(int y=0;y<tex->iSizeY;y++){
			
			int index = (x * 4) +  ((tex->iSizeY - 1 - y) * tex->iSizeX * 4);
			
			byte r = data[index + 0];
			byte g = data[index + 1];
			byte b = data[index + 2];
			
			if(r > 240 && g > 240 && b > 240){
				r = g = b = 255;
			}
						
			if(r > 0 || g > 0 || b > 0){
				float v = 0.05f;
				
				Vector3 vel = Vector3(-10, 0, 0);// + Vector3(randFloat(-v,v),
								//randFloat(-v,v),randFloat(-v,v));
				//Vector3 pos = Vector3(randFloat(-v,v),randFloat(-v,v),
				//				randFloat(-v,v) + randFloat(-1,1));
				Vector3 pos = Vector3(0,0,0);
	
				ps()->add(pos + Vector3(60, (y / 2.0f) - 20, 0), vel, 
							Color(r,g,b), 1.0f, 15.0f);
			}
			
		}
	}
	
	if(isRotating)
		fRot[2] += fTimeScale * 10; //rotate
	
}	


/*********************************************
		SDL event loop
**********************************************/
void App::utilEventLoop(){

	SDL_Event event;

    while ( !done ){
	   
	    while ( SDL_PollEvent( &event ) ){
	    
	    	//Hand the event off to the GUI first. If the GUI handles it, it's
	    	//done. 
#ifdef ENABLE_GUI
	    	if(processGUIEvent(event)){
	    		continue;
	    	}
#endif
	    
		    switch( event.type ){
						      
			case SDL_VIDEORESIZE:
			    //handle resize event
			    surface = SDL_SetVideoMode( event.resize.w, event.resize.h, 
			    							16, videoFlags );
			    if ( !surface ){
				    ERR( "Could not get a surface after resize: %s\n", 
				    	SDL_GetError( ) );
				    notifyShutdown();
				}
			    resizeWindow( event.resize.w, event.resize.h );
#ifdef ENABLE_GUI
			    resizeGUI( event.resize.w, event.resize.h );
#endif
			    break;
			
			case SDL_QUIT:
			    //handle quit requests
			    notifyShutdown();
			    break;
			
			case SDL_MOUSEBUTTONDOWN:
								
				if(mFlowMgr->onClick(event.button.button, 
									fMouseX, fMouseY, fMouseZ)){
					break;
				}			
				
				onMouseEvent(event.button.button, SDL_MOUSEBUTTONDOWN); 
				beginDrag();
												
				break;	
				
			case SDL_MOUSEBUTTONUP:
				onMouseEvent(event.button.button, SDL_MOUSEBUTTONUP); 
				endDrag();
				break;	
				
			case SDL_KEYDOWN:
				handleKeyEvent(&event.key.keysym, event.type);
				break;
			
			case SDL_KEYUP:
				handleKeyEvent(&event.key.keysym, event.type);
				break;
			
			default:
			    break;
			}
		}
		
	    if (!done){
	    
	    	//Do one frames worth of work and figure out the length of time
	    	uint32_t startTime = SDL_GetTicks();
			renderMain();
			updateMain();
			uint32_t endTime = SDL_GetTicks();
			
			//Figure out the scaling factor for FPS-independent movement
			uint32_t diff = endTime - startTime;
			
			if (iMaxFrameRate > 0 && diff < 1000 / iMaxFrameRate) {
				SDL_Delay((1000 / iMaxFrameRate) - diff);
				diff = 1000 / iMaxFrameRate;
			}
			
			fTimeScale = (float)diff * fTimeScaleScale;
			
			//Every hour, do a cleanup
			if(fCleanupTimer < 0.0f){
				ps()->doPeriodicCleanup();				
				fCleanupTimer = CLEANUP_TIMER;
			}
			
			//Update our various timers
			fCleanupTimer -= fTimeScale;
			fUptime += fTimeScale;
			fParticleFPS = fTimeScale;
		}
	}
	
}

/*********************************************
		Camera rotate
**********************************************/
void App::beginDrag(){
	dragStart = getMouse();
	bDrag = true;
}

void App::endDrag(){
	dragStart = Vector2(0, 0);
	bDrag = false;
}



/*********************************************
		Camera movement
**********************************************/
void App::camMove(float x, float y, float z){
	fCameraX += x; 	fCameraY += y; 	fCameraZ += z;	
}
void App::camSetPos(float x, float y, float z){
	fCameraX = x; 	fCameraY = y; 	fCameraZ = z;	
}
void App::camLookAt(float x, float y, float z){
	fLookX = x; fLookY = y; fLookZ = z;
}


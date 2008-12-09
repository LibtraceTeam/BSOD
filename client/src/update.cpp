#include "main.h"


#define CAMERA_SPEED 0.5f

float infinity = std::numeric_limits<float>::infinity();

int flowCount = 0; //for testing

//Camera movement
void App::camMove(float x, float y, float z){
	fCameraX += x; 	fCameraY += y; 	fCameraZ += z;	
}
void App::camSetPos(float x, float y, float z){
	fCameraX = x; 	fCameraY = y; 	fCameraZ = z;	
}
void App::camLookAt(float x, float y, float z){
	fLookX = x; fLookY = y; fLookZ = z;
}

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
	
	float fCamSpeed = fTimeScale * 4.0f;
		
	//Update particle timer	
	fNextParticleUpdate -= fTimeScale;
	
	//Update particle system if necessary
	if(fNextParticleUpdate < 0.0f){
		mParticleSystem->update();
		fNextParticleUpdate = PARTICLE_FPS;
	
		if(isConnected()){
			updateSocket();
		}	
	}			
	
	if(bDrag){
	
		Vector2 drag = Vector2(iMouseX, iMouseY);
		Vector2 diff = dragStart - drag;
		
		if(mouseDown(1)){//left
			fRot[1] -= diff.x / 10.0f;
			fRot[0] -= diff.y / 10.0f;		
		}
		else if(mouseDown(2)){//middle
			fZoom += diff.y / 10.0f;
		}
		
		dragStart = Vector2(iMouseX, iMouseY);
		
	}
}


void App::beginDrag(){
	dragStart = Vector2(iMouseX, iMouseY);
	bDrag = true;
}

void App::endDrag(){
	dragStart = Vector2(0, 0);
	bDrag = false;
}

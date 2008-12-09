#include "main.h"



/*********************************************
		Updates the particle logic
 This is in a seperate thread so we can use
 more CPUs :)
**********************************************/
int ParticleUpdateWrapper(void *unused){

	while(App::S()->particleUpdateThread()){ }
	return 0;
}

bool App::particleUpdateThread(){

	//Throw away any bad frames
	if(fTimeScale == infinity || fTimeScale == 0.0f){
		return true;
	}
	
	//TODO: This will wait the same amount of time irregardless of how long
	//the actual logic update takes. We should calculate this properly...
	usleep(PARTICLE_FPS * 1024 * 1024);
	
	//Get the mutex and update the particle logic
	SDL_mutexP(mPartLock);		
		mParticleSystem->update();			
	SDL_mutexV(mPartLock);		
			
	return true;	
	
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
	

	if(isConnected()){
		updateSocket();
	}
}


/*********************************************
		Camera rotate
**********************************************/
void App::beginDrag(){
	dragStart = Vector2(iMouseX, iMouseY);
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


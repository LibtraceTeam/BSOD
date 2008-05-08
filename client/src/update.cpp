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

void App::generateTestData(){

	static int flowCount = 0;

	//Update network here in future
	//For now we do random stuff
	int flow = randInt(1, (flowCount + 1) * 100);
	if(flowCount < 30){
		
		IPaddress src;
		IPaddress dest;
				
		Vector3 start = Vector3(randFloat(-10, 10), randFloat(-10, 10), randFloat(-10, 10));
		Vector3 end = Vector3(randFloat(-10, 10), randFloat(-10, 10), randFloat(-10, 10));
				
		mCurrentModule->newFlow(flowCount++, src, dest, start, end);
		
		//LOG("New flow %d\n", flowCount);
		
	}
	
	//if(randInt(1, 10) > 7){	
	
		if(flowCount <= 0){
			return;
		}
			
		for(int i=0;i<randInt(0, flowCount);i++){		
			mCurrentModule->newPacket(i % flowCount, randInt(100, 1500), 50, getFD(i));
		}
	//}


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
	
	
	//Options
	if(bOptions[OPTION_ROTATE_X])	fRot[0] += fCamSpeed;
	if(bOptions[OPTION_ROTATE_Y])	fRot[1] += fCamSpeed;
	if(bOptions[OPTION_ROTATE_Z])	fRot[2] += fCamSpeed;
	
	//Update particle timer	
	fNextParticleUpdate -= fTimeScale;
	
	//Update particle system if necessary
	if(fNextParticleUpdate < 0.0f){
		mParticleSystem->update();
		fNextParticleUpdate = PARTICLE_FPS;
		
		//Update the module if necessary
		if(mCurrentModule){
			mCurrentModule->update(0, PARTICLE_FPS);
		
			if(isConnected()){
				updateSocket();
			}else{
				generateTestData();
			}
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

void App::setOption(int index, bool enabled){

	if(index == OPTION_RESET_ROTATION){
		for(int i=0;i<3;i++){
			fRot[i] = 0.0f;
			fZoom = 0.0f;
			bDrag = false;
		}
		return;
	}
	bOptions[index] = enabled;
}

void App::beginDrag(){
	dragStart = Vector2(iMouseX, iMouseY);
	bDrag = true;
}

void App::endDrag(){
	dragStart = Vector2(0, 0);
	bDrag = false;
}

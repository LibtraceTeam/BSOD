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
	
	//Keyboard rotation
	if(keyDown(SDLK_RSHIFT) || keyDown(SDLK_LSHIFT)){
		fCamSpeed *= 10;
	}
	
	if(keyDown(SDLK_LEFT))	fRot[1] += fCamSpeed;
	if(keyDown(SDLK_RIGHT))	fRot[1] -= fCamSpeed;
	if(keyDown(SDLK_UP))	fRot[0] += fCamSpeed;
	if(keyDown(SDLK_DOWN))	fRot[0] -= fCamSpeed;
	
	
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
		fRot[1] -= dragVel.x;
		fRot[0] -= dragVel.y;
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
		
		for(int y=1;y<tex->iSizeY - 1;y++){
			
			int index = (x * 4) + ((tex->iSizeY - y) * tex->iSizeX * 4);
			
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
				Vector3 pos = Vector3(randFloat(-v,v),randFloat(-v,v),
								randFloat(-v,v) + randFloat(-1,1));
	
				ps()->add(pos + Vector3(60, (y / 3.0f) - 20, 0), vel, 
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
	    	if(processGUIEvent(event)){
	    		continue;
	    	}
	    
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
			    resizeGUI( event.resize.w, event.resize.h );
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

	    /* draw the scene */
	    if (!done){
	    	uint32_t startTime = SDL_GetTicks();
			renderMain();
			updateMain();
			uint32_t endTime = SDL_GetTicks();
			
			uint32_t diff = endTime - startTime;
			fTimeScale = (float)diff / 1000.0f;
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


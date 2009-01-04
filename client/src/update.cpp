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
	
	fNextParticleUpdate -= fTimeScale;
	
	if(fNextParticleUpdate <= 0.0f){
		mParticleSystem->update();		
		mFlowMgr->update(0.0f, fTimeScale);
		fNextParticleUpdate = PARTICLE_FPS;
	}
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
			    surface = SDL_SetVideoMode( event.resize.w, event.resize.h, 16, videoFlags );
			    if ( !surface ){
				    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
				    utilShutdown( 1 );
				}
			    resizeWindow( event.resize.w, event.resize.h );
			    break;
			
			case SDL_QUIT:
			    /* handle quit requests */
			    done = true;
			    break;
			
			case SDL_MOUSEBUTTONDOWN:
				onMouseEvent(event.button.button, SDL_MOUSEBUTTONDOWN); 
				beginDrag();
				break;	
				
			case SDL_MOUSEBUTTONUP:
				onMouseEvent(event.button.button, SDL_MOUSEBUTTONUP); 
				endDrag();
				break;	
			
			default:
			    break;
			}
		}

	    /* draw the scene */
	    if (!done){
			renderMain();
			updateMain();
		}
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


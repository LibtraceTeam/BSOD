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
		utilShutdown( 0 );
		break;
	case SDLK_F1:
		//Toggle fullscreen
		SDL_WM_ToggleFullScreen( surface );
		break;
		
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

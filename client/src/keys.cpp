#include "main.h"

#define MAX_KEYS 512

bool keyArray[MAX_KEYS];

void App::keyInit(){
	for(int i=0;i<MAX_KEYS;i++){
		keyArray[i] = false;
	}
}

void App::keySetState(int code, bool pressed){
	keyArray[code] = pressed;
}

bool App::keyDown(int code){
	return keyArray[code];
}

void App::onKeyEvent(int code, int eventType){

	if(eventType == SDL_KEYDOWN){
				
	}
		
}
void App::onMouseEvent(int code, int eventType){
	bMouse[code] = (eventType == EVENT_MOUSE_DOWN);	
}

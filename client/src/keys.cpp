#include "main.h"


#include <sys/select.h>

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

int inputThread(void *unused){

	while(true){

		char c = std::cin.get();
		
		App::S()->onKey(c);
	
	}
}

void App::onKey(char c){

	if(c == 10){
		return;
	}

	//LOG("Hit %d\n", c);

	if(c == 'x')	bOptions[OPTION_ROTATE_X] = !bOptions[OPTION_ROTATE_X];			
	if(c == 'y')	bOptions[OPTION_ROTATE_Y] = !bOptions[OPTION_ROTATE_Y];			
	if(c == 'z')	bOptions[OPTION_ROTATE_Z] = !bOptions[OPTION_ROTATE_Z];		

	if(c == 'w')	fRot[0] += 10;
	if(c == 's')	fRot[0] -= 10;
	if(c == 'a')	fRot[1] += 10;
	if(c == 'd')	fRot[1] -= 10;		
	if(c == 'q')	fRot[2] += 10;
	if(c == 'e')	fRot[2] -= 10;


	if(c == '-')	fZoom += 1;
	if(c == '=')	fZoom -= 1;


	if(c == 'r'){
		for(int i=0;i<3;i++){
			fRot[i] = 0.0f;
			fZoom = 0.0f;
			bDrag = false;
		}	
		fRot[0] = 20;
	
	
		bOptions[OPTION_ROTATE_X] = false;
		bOptions[OPTION_ROTATE_Y] = false;
		bOptions[OPTION_ROTATE_Z] = false;		
	}
}


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

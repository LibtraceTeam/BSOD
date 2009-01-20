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
	
		mStatusString = "Current time: " + string(ctime(&iCurrentTime));
		
	}
	
}



/*********************************************
		Main render method
**********************************************/
void App::renderMain(){
	calcFps();

	utilBeginRender();
		
	camLook();
			
	if(isConnected())	
		mFlowMgr->render();	
		
	calculateMousePoint();
	
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
				0, 				1, 				0			);
				
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
	writeText(10, 7, "%d fps, %d particles", iFPS, ps()->getActive());	
	
	if(fGUITimeout <= 0.0f){
		if(!isConnected()){
			writeTextCentered(iScreenX / 2, iScreenY - 30, 
						"Press any key to display the GUI, \
						then use the Servers button to connect to a server");
		}		
	}else{
	
		if(isConnected()){
	
			float w = getTextWidth(mStatusString.c_str()) + 10;
			writeText(iScreenX - w, iScreenY - 20, mStatusString.c_str());			
						
			string s = "Connected to " + mServerAddr + ":" + toString(iServerPort);
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
	renderGUI();	
			
}

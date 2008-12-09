#include "main.h"

float fTimeScale = 0.0f;
extern float infinity;

// GL_ARB_point_parameters
PFNGLPOINTPARAMETERFARBPROC  glPointParameterfARB  = NULL;
PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB = NULL;

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
	
		fTimeScale = 1.0f / (float)iFPS;
		LOG("%d fps\n", iFPS);
	}
	
}



/*********************************************
		Main render method
**********************************************/
void App::renderMain(){
	calcFps();

	utilBeginRender();
	
	//position the GL camera
	gluLookAt(	fCameraX, 		fCameraY, 		fCameraZ, 
				fLookX,			fLookY,			fLookZ, 
				0, 				1, 				0			);
				
	glTranslatef(0, 0, fZoom);
				
	glRotatef(fRot[0], 1, 0, 0);
	glRotatef(fRot[1], 0, 1, 0);
	glRotatef(fRot[2], 0, 0, 1);
			
	calculateMousePoint();
	
	mFlowMgr->render();	
	mParticleSystem->render();
							
	utilEndRender();
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
	writeText(10, 7, "%d fps, time %d", iFPS, iTime);	

	if(fTimeScale == infinity || fTimeScale == 0.0f){
		writeText(5, 25, "(waiting)");	   
	}else{
		//writeText(5, 25, "%f/%f/%f", fMouseX, fMouseY, fMouseZ);	   
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
		
	//Render the GUI
	
}

#include "../stdafx.h"
#include "../system_driver.h"
#include "../world.h"
#include "../exception.h"
#include "../reporter.h"
#include "../action.h"
#include "../entity_manager.h"
#include "../camera.h"
#include "../player.h"
#include "linux_system_driver.h"

#include "../gl/gl_display_manager.h"
#include "../misc.h"

#include "SDL/SDL.h"

int main(int argc, char *argv[])
{
	/* Initialize the SDL library (starts the event loop) */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr,
                "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
	
	/* Set the title bar in environments that support it */
	SDL_WM_SetCaption("BSOD", NULL);

	world.sys = new CLinuxSystemDriver();

	return BungMain(argc, argv);
}


CDisplayManager *CLinuxSystemDriver::InitDisplay(
		int width, 
		int height, 
		int bpp, 
		bool fullScreen, 
		DisplayType type)
{
	if(type != DISPLAY_OPENGL)
		throw CException("Unknown display type requested!");
	
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );	
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	
	/* The mouse isn't much use unless we have a display for reference */
    if ( SDL_SetVideoMode(width, height, 0, SDL_OPENGL) == NULL ) {
		throw CException("Couldn't set video mode!");
//        SDL_GetError()
    }

	CGLDisplayManager *gl = new CGLDisplayManager;
	gl->WindowResized(width, height);

	return gl;
}

int CLinuxSystemDriver::RunMessageLoop()
{
	float start_time, carry_time;

	start_time = TimerGetTime();

	SDL_ShowCursor(0);
	SDL_WarpMouse(world.display->GetWidth() / 2, 
			      world.display->GetHeight() / 2);
	
	while ( ! done ) {
		world.Draw();

		{ 
			SDL_Event event;
			while ( SDL_PollEvent(&event) ) 
			{
				if ( event.type == SDL_QUIT ) 
				{
					done = true;
				}
				else if ( event.type == SDL_KEYDOWN ) 
				{
					if ( event.key.keysym.sym == SDLK_ESCAPE ) {
						done = true;
					}
					world.actionHandler->KeyDown( 
							(CActionHandler::Keycode)event.key.keysym.sym );
				}
				else if ( event.type == SDL_KEYUP )
				{
					world.actionHandler->KeyUp( 
							(CActionHandler::Keycode)event.key.keysym.sym );
				}
				else if ( event.type == SDL_MOUSEMOTION )
				{
					int w = world.display->GetWidth() / 2,
					    h = world.display->GetHeight() / 2;
						
					if((event.motion.x == w) && (event.motion.y == h))
						continue;
					
					int xrel, yrel;

					xrel = -(event.motion.x - w);
					yrel = -(event.motion.y - h);

					world.entities->GetPlayer()->mpos.x = yrel; //event.motion.xrel; 
					world.entities->GetPlayer()->mpos.y = xrel; //event.motion.yrel;

					SDL_WarpMouse(world.display->GetWidth() / 2, 
							      world.display->GetHeight() / 2);
				}
				else if ( event.type == SDL_MOUSEBUTTONDOWN )
				{
					world.actionHandler->KeyDown(CActionHandler::BKC_LEFTMOUSEBUT);
				}
				else if ( event.type == SDL_MOUSEBUTTONUP )
				{
					world.actionHandler->KeyUp(CActionHandler::BKC_LEFTMOUSEBUT);
				}
			}
		}

		carry_time = TimerGetTime();
		world.Update( (carry_time - start_time) / 1000.0f );
		start_time = carry_time;

		world.entities->GetPlayer()->mpos.x = 0;
		world.entities->GetPlayer()->mpos.y = 0;
	}

	SDL_Quit();

	return 0;
}

void CLinuxSystemDriver::ResizeWindow(int width, int height)
{
	fprintf(stderr, "Warning: Resizing window not supported yet! (%d,%d)\n",
			width, height);
}

void CLinuxSystemDriver::Quit()
{
	fprintf(stdout, "Quit() called. BuNg exiting.\n\n");
	done = true;
}

void CLinuxSystemDriver::TimerInit(void)
{
}

#ifdef LINUX
#include <sys/time.h>
#endif

float CLinuxSystemDriver::TimerGetTime()
{
	// TODO: make a higher resolution timer here: apparently SDL_GetTicks() isn't the greatest (?)
#if 0
	struct timeval tv;
	
	if( gettimeofday(&tv, NULL) == 0 ) {
		return tv.tv_sec * 1000 + (tv.tv_usec / 1000.0f);
	}
#else
	return SDL_GetTicks();
#endif
}

void CLinuxSystemDriver::ForceWindowDraw()
{
	SDL_GL_SwapBuffers();
}

void CLinuxSystemDriver::ErrorMessageBox(string title, string message)
{
//	fprintf(stderr, "%s : %s\n", title.c_str(), message.c_str());
	string tcl = bsprintf("tk_messageBox -title \"%s\" -message \"%s\" "
			"-type ok -icon error\nexit\n", title.c_str(), message.c_str());
	string command = bsprintf("echo '%s' | wish", tcl.c_str());

	fprintf(stderr, "%s\n", command.c_str());
	system( command.c_str() );
}



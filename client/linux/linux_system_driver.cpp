/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#include "stdafx.h"
#include "system_driver.h"
#include "world.h"
#include "exception.h"
#include "reporter.h"
#include "action.h"
#include "entity_manager.h"
#include "camera.h"
#include "player.h"
#include "linux_system_driver.h"

#include "gl/gl_display_manager.h"
#include "misc.h"

#include "SDL/SDL.h"

#include "SDL/SDL_syswm.h"//added to force window position - Brendon
#include <getopt.h>

int Xpos = 0;
int Ypos = 0; //again a lazy hack - Brendon

// Duplicated from PartVis.h
// Simpler than including the header and then figuring out all the dependencies but perhaps there needs to be a util.h or something
// for global helper functions like this?
inline bool PointInRect( int px, int py, int rx, int ry, int rwidth, int rheight )
{
	if( px < rx )
		return( false );
	if( py < ry )
		return( false );
	if( px > (rx+rwidth) )
		return( false );
	if( py > (ry+rheight) )
		return( false );

	return( true );
}

int main(int argc, char *argv[])
{
	/* Initialize the SDL library (starts the event loop) */
    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) {
        fprintf(stderr,
                "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    /* Do getopt stuff for window position - Brendon */
    int opt;
    while( (opt = getopt(argc, argv, "x:y:")) != -1)
    {
	switch(opt)
	{
	    case 'x': Xpos = atoi(optarg); break;
	    case 'y': Ypos = atoi(optarg); break;
	    default: fprintf(stderr, "Usage: %s [-x pos] [-y pos]\n", argv[0]);
		     exit(1);
	};
    }
	
	/* Set the title bar in environments that support it */
	SDL_WM_SetCaption("BSOD", NULL);

	world.sys = new CLinuxSystemDriver();

	return BungMain(argc, argv);
}

void CLinuxSystemDriver::GetMousePos( int *x, int *y )
{
	SDL_GetMouseState( x, y );
}

CDisplayManager *CLinuxSystemDriver::InitDisplay(
		int width, 
		int height, 
		int bpp, 
		bool fullScreen, 
		DisplayType type,
		char *title/*not used*/)
{
    int flags = SDL_OPENGL;

    if(type != DISPLAY_OPENGL)
	throw CException("Unknown display type requested!");

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );	
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );

    if(fullScreen)
	flags |= SDL_FULLSCREEN;

    /* The mouse isn't much use unless we have a display for reference */
    if ( SDL_SetVideoMode(width, height, 0, flags) == NULL ) {
	throw CException("Couldn't set video mode!");
	//        SDL_GetError()
    }

    /* Begin Brendon hack for window positioning */
    if(!fullScreen)
    {
	SDL_SysWMinfo info;
	// ignore the checking that we should be doing here...
	SDL_VERSION(&info.version);
	SDL_GetWMInfo(&info);

	info.info.x11.lock_func();
	XMoveWindow(info.info.x11.display, info.info.x11.wmwindow, Xpos, Ypos);
	info.info.x11.unlock_func();
    }
    /* End Brendon hack */

    CGLDisplayManager *gl = new CGLDisplayManager;
    gl->WindowResized(width, height);

    return gl;
}

int CLinuxSystemDriver::RunMessageLoop()
{
    float start_time, carry_time;
    //bool first_time = true;

    start_time = TimerGetTime();

	if( world.actionHandler->no_cursor )
		SDL_ShowCursor( SDL_DISABLE ); // HACK
    //SDL_WarpMouse(world.display->GetWidth() / 2, 
    //       world.display->GetHeight() / 2);

    world.entities->GetPlayer()->mpos.x = 0;
    world.entities->GetPlayer()->mpos.y = 0;

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
			// Do some NASTY translation of keycodes that differ to win32. Is there a nicer way to do this?
		    if( event.key.keysym.sym == 273 )
			    world.actionHandler->KeyDown( CActionHandler::BKC_UP );
		    else if( event.key.keysym.sym == 275 )
			    world.actionHandler->KeyDown( CActionHandler::BKC_RIGHT );
		    else if( event.key.keysym.sym == 274 )
			    world.actionHandler->KeyDown( CActionHandler::BKC_DOWN );
		    else if( event.key.keysym.sym == 276 )
			    world.actionHandler->KeyDown( CActionHandler::BKC_LEFT );
		    else if( event.key.keysym.sym == 13 )
			    world.actionHandler->KeyDown( CActionHandler::BKC_RETURN );

		    else
                    	world.actionHandler->KeyDown( 
                            	(CActionHandler::Keycode)event.key.keysym.sym );
                }
                else if ( event.type == SDL_KEYUP )
                {
                    if( event.key.keysym.sym == 273 )
			    world.actionHandler->KeyUp( CActionHandler::BKC_UP );
		    else if( event.key.keysym.sym == 275 )
			    world.actionHandler->KeyUp( CActionHandler::BKC_RIGHT );
		    else if( event.key.keysym.sym == 274 )
			    world.actionHandler->KeyUp( CActionHandler::BKC_DOWN );
		    else if( event.key.keysym.sym == 276 )
			    world.actionHandler->KeyUp( CActionHandler::BKC_LEFT );
		    else if( event.key.keysym.sym == 13 )
			    world.actionHandler->KeyUp( CActionHandler::BKC_RETURN );

		    else
                    	world.actionHandler->KeyUp( 
                            	(CActionHandler::Keycode)event.key.keysym.sym );
                }
                else if ( event.type == SDL_MOUSEMOTION )
                {
                    /*int w = world.display->GetWidth() / 2;
                    int h = world.display->GetHeight() / 2;

                    if((event.motion.x == w) && (event.motion.y == h))
                        continue;

                    int xrel, yrel;

                    if(first_time) 
					{
                        xrel = 0;
                        yrel = 0;
                        first_time = false;
                    } else 
					{
                        xrel = -(event.motion.x - w);
                        yrel = -(event.motion.y - h);
                    }

                    world.entities->GetPlayer()->mpos.x = yrel; //event.motion.xrel; 
                    world.entities->GetPlayer()->mpos.y = xrel; //event.motion.yrel;

                    SDL_WarpMouse(world.display->GetWidth() / 2, 
                            world.display->GetHeight() / 2);*/
					
					
					
					/*if( first_time )
					{
						lastPoint.x = event.motion.x;
						lastPoint.y = event.motion.y;
						first_time = false;
					}
					else if( world.actionHandler->lmb_down && !world.actionHandler->gui_open )
					{
						// Do the movement:
						world.entities->GetPlayer()->mpos.x = ( lastPoint.x - event.motion.x);
						world.entities->GetPlayer()->mpos.y = ( lastPoint.y - event.motion.y);
						SDL_WarpMouse( (int)(lastPoint.x), (int)(lastPoint.y) );
					}
					else
					{
						lastPoint.x = event.motion.x;
						lastPoint.y = event.motion.y;
					}*/


					if( world.actionHandler->lmb_down && !world.actionHandler->gui_open )
					{
						float yd = (float)(lastPoint.x - event.motion.x);
						float xd = (float)(lastPoint.y - event.motion.y);
						//if( (xd > 1) && (yd > 1) )
						//{
							//SDL_WarpMouse( (Uint16)(lastPoint.x), (Uint16)(lastPoint.y) );
							world.entities->GetPlayer()->mpos.y = yd * 2.0f;
							world.entities->GetPlayer()->mpos.x = xd * 2.0f;
							lastPoint.x = event.motion.x;
							lastPoint.y = event.motion.y;

						//}
					}
					else
					{
						// TODO: Check that mouse is in window.
						lastPoint.x = event.motion.x;
						lastPoint.y = event.motion.y;
					}
                }
                else if ( event.type == SDL_MOUSEBUTTONDOWN )
                {
					if( event.button.button == SDL_BUTTON_LEFT )
               			world.actionHandler->KeyDown(CActionHandler::BKC_LEFTMOUSEBUT);
					else if( event.button.button == SDL_BUTTON_RIGHT )
						world.actionHandler->KeyDown(CActionHandler::BKC_RIGHTMOUSEBUT);
					else if( event.button.button == 4 )
						world.actionHandler->KeyDown(CActionHandler::BKC_MOUSESCROLLUP);
					else if( event.button.button == 5 )
						world.actionHandler->KeyDown(CActionHandler::BKC_MOUSESCROLLDOWN);
                }
                else if ( event.type == SDL_MOUSEBUTTONUP )
                {
					if( event.button.button == SDL_BUTTON_LEFT )
                    	world.actionHandler->KeyUp(CActionHandler::BKC_LEFTMOUSEBUT);
					else if( event.button.button == SDL_BUTTON_RIGHT )
						world.actionHandler->KeyUp(CActionHandler::BKC_RIGHTMOUSEBUT);
					else if( event.button.button == 4 )
						world.actionHandler->KeyUp(CActionHandler::BKC_MOUSESCROLLUP);
					else if( event.button.button == 5 )
						world.actionHandler->KeyUp(CActionHandler::BKC_MOUSESCROLLDOWN);
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
    done = true;
}

void CLinuxSystemDriver::TimerInit(void)
{
}

float CLinuxSystemDriver::TimerGetTime()
{
	// TODO: make a higher resolution timer here: apparently SDL_GetTicks() isn't the greatest (?)
	return SDL_GetTicks();
}

void CLinuxSystemDriver::ForceWindowDraw()
{
    SDL_GL_SwapBuffers();
}

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Code from FreeBSD which(1) program. */
static int
g_is_there(char *candidate)
{
    struct stat fin;

    /* XXX work around access(2) false positives for superuser */
    if (access(candidate, X_OK) == 0 &&
            stat(candidate, &fin) == 0 &&
            S_ISREG(fin.st_mode) &&
            (getuid() != 0 ||
             (fin.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0)) {
        return (1);
    }
    return (0);
}

static int
g_which(char *path, char *filename)
{
    char candidate[PATH_MAX];
    const char *d;

    if (strchr(filename, '/') != NULL) 
        return (g_is_there(filename) ? 0 : -1);
    while ((d = strsep(&path, ":")) != NULL) {
        if (*d == '\0')
            d = ".";
        if (snprintf(candidate, sizeof(candidate), "%s/%s", d,
                    filename) >= (int)sizeof(candidate))
            continue;
        if (g_is_there(candidate)) {
            return 1;
        }
    }
    return 0;
}

string g_escape(string str)
{
    for(string::iterator i = str.begin(); i != str.end(); ++i) {
        if(*i == '\'') {
            i = str.insert(i, '\\');
            i++;
        }
    }
    return str;
}

void CLinuxSystemDriver::ErrorMessageBox(string title, string message)
{
    char *p = getenv("PATH");
    char *path;
    string p_msg = message;
    string p_title = title;

    // Go back to desktop/clean up SDL
    SDL_Quit();

    // zenity is used to create gtk apps; prefer this. It is quick and
    // nice.
    path = strdup(p);
    if(g_which(path, "zenity")) {
        system(
            bsprintf("zenity --error --title=\"%s\" --text=\"%s\"", 
                p_title.c_str(),
                p_msg.c_str()).c_str()
            );
        free(path);
        return;
    }
    free(path);
    // kdialog can potentially take some time to load if it exists and
    // kde is not already running. Fall back to this if zenity doesn't
    // exist.
    path = strdup(p);
    if(g_which(path, "kdialog")) {
        system(
            bsprintf("kdialog --title \"%s\" --error \"%s\"",
                p_title.c_str(),
                p_msg.c_str()).c_str()
            );
        free(path);
        return;
    }
    free(path);
    // xdialog is not as nice as zenity or kdialog, but if it exists it
    // is the only remaining reasonable option.
    path = strdup(p);
    if(g_which(path, "Xdialog")) {
        system(
            bsprintf("Xdialog --title \"%s\" --msgbox \"%s\" 10 50",
                p_title.c_str(),
                p_msg.c_str()).c_str()
            );
        free(path);
        return;
    }
    free(path);
    // Fall back to attempting to create a tk dialog with the wish
    // interpreter. Not very nice, but it works.
    path = strdup(p);
    if(g_which(path, "wish")) {
        system(
            bsprintf("echo 'tk_messageBox -title \"%s\" -message \"%s\" "
                "-type ok -icon error\nexit\n' | wish", 
                g_escape(p_title).c_str(), 
                g_escape(p_msg).c_str()).c_str()
            );
        free(path);
        return;
    }
    free(path);
    
    // If we get to here there is nothing nice installed to make
    // dialogs for us, we can only print out to stderr.
    fprintf(stderr, "%s: %s\n", title.c_str(), message.c_str());
}



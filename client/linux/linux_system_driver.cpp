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

int main(int argc, char *argv[])
{
	/* Initialize the SDL library (starts the event loop) */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
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


CDisplayManager *CLinuxSystemDriver::InitDisplay(
		int width, 
		int height, 
		int bpp, 
		bool fullScreen, 
		DisplayType type)
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
    bool first_time = true;

    start_time = TimerGetTime();

    SDL_ShowCursor(0);
    SDL_WarpMouse(world.display->GetWidth() / 2, 
            world.display->GetHeight() / 2);

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

                    if(first_time) {
                        xrel = 0;
                        yrel = 0;
                        first_time = false;
                    } else {
                        xrel = -(event.motion.x - w);
                        yrel = -(event.motion.y - h);
                    }

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



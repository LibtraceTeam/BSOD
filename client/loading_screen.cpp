#include "stdafx.h"
#include "misc.h"
#include "display_manager.h"
#include "texture_manager.h"
#include "world.h"
#include "system_driver.h"
#include "reporter.h"
#include "loading_screen.h"

//#include <GL/gl.h>

#ifdef _WIN32
#include <io.h>
#endif

void CLoadingScreen::Redraw()
{
    world.display->BeginFrame2();
    world.display->Begin2D();
    world.display->SetBlend(false);
    world.display->BindTexture(background);

    float nw, nh;
    if(background->orig_width > width || background->orig_height > height)
    {
        // Find out optimal scaling for the image so it is as large as possible but
        // the aspect is still the same.

        float aspect = (float)background->orig_width / (float)background->orig_height;

        // 1.333 is the normal aspect ratio of a monitor

        // Maximise to screen size
        if(aspect > 1.3333f) // width is the prevailing dimension
        {
            nw = (float)width;
            nh = nw / aspect;
        } else {
            nh = (float)height;
            nw = aspect * nh;
        }

    } else {
        nw = background->orig_width;
        nh = background->orig_height;
    }

    world.display->Draw2DQuad( 
            (int)((width - nw) / 2), 
            (int)((height - nh) / 2), 
            (int)(width - ((width - nw) / 2)), 
            (int)(height - ((height - nh) / 2)) 
            );


    world.display->SetBlend(true);	
    world.display->BindTexture(NULL);


    /*	world.display->SetColour(0.2f, 0.1f, 0.3f, 0.7f);

        world.display->Draw2DQuad(8, 43, maxlen * 8 + 12, messages.size() * 20 + 47);*/

    world.display->SetColour(1.0f, 1.0f, 1.0f);

    list<string>::iterator i;
    int counter = 0;
    for(i = messages.begin(); i != messages.end(); ++i)
    {
        world.display->DrawString2(10, counter++ * 20 + 45, *i);

    }

    world.display->End2D();

    world.display->SetColour(1.0f, 1.0f, 1.0f);

    world.display->EndFrame2();
}

CLoadingScreen::CLoadingScreen(int w, int h) {
	width = w;
	height = h;
	string texName;
	maxlen = 0;

	background = CTextureManager::tm.LoadTexture("data/loading.png");

	Redraw();
}

CLoadingScreen::~CLoadingScreen()
{
	CTextureManager::tm.UnloadTexture(background);
}

void CLoadingScreen::AddMessage(string message) {
	messages.push_back(message);

	if(message.length() > (unsigned int)maxlen)
		maxlen = message.length();

	if(messages.size() > 10)
		messages.pop_front();

	CReporter::Report(CReporter::R_MESSAGE, message + "\n");

	Redraw();
}

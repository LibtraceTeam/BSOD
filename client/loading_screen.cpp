/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:55  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
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

	Log("Redrawing...\n");

	{
		// Find out optimal scaling for the image so it is as large as possible but
		// the aspect is still the same.

		float aspect = (float)background->orig_width / (float)background->orig_height;
		float nw, nh;

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

		Log("1.1...\n");
		world.display->Draw2DQuad( 
			(int)((width - nw) / 2), 
			(int)((height - nh) / 2), 
			(int)(width - ((width - nw) / 2)), 
			(int)(height - ((height - nh) / 2)) 
			);

	}

	Log("1.2...\n");
	
	world.display->SetBlend(true);	
	world.display->BindTexture(NULL);

	Log("1.3...\n");

	world.display->SetColour(0.2f, 0.1f, 0.3f, 0.7f);

	world.display->Draw2DQuad(8, 43, maxlen * 8 + 12, messages.size() * 20 + 47);

	world.display->SetColour(1.0f, 1.0f, 1.0f);

	Log("2...\n");

	list<string>::iterator i;
	int counter = 0;
	for(i = messages.begin(); i != messages.end(); ++i)
	{
		world.display->DrawString2(10, counter++ * 20 + 45, *i);

	}

	world.display->End2D();

	world.display->SetColour(1.0f, 1.0f, 1.0f);

	Log("3...\n");

	world.display->EndFrame2();

	Log("Finished redrawing...\n");
}

CLoadingScreen::CLoadingScreen(int w, int h) {
	width = w;
	height = h;
	string texName;
	maxlen = 0;

	Log("Loading texture 'data/loading.png' ... ");
	background = CTextureManager::tm.LoadTexture("data/loading.png");
	Log("Loaded.\n");

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

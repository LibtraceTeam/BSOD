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
#else
#include <unistd.h>
#endif


void CLoadingScreen::Redraw()
{
    world.display->BeginFrame2();
    world.display->Begin2D();
    world.display->SetBlend(false);
    world.display->BindTexture(background);

    float nw, nh;
    if(background->orig_width > width)
    {
        // Find out optimal scaling for the image so it is as large as possible but
        // the aspect is still the same.

        float aspect = (float)background->orig_width / (float)background->orig_height;

        // 1.333 is the normal aspect ratio of a monitor

        // Maximise to screen size
        nw = (float)width;
        nh = nw / aspect;
    } else {
        nw = background->orig_width;
        nh = background->orig_height;
    }

    world.display->Draw2DQuad(
	0, 0,
	nw, nh);

    world.display->SetBlend(true);	
    world.display->BindTexture(NULL);


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

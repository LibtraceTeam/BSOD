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
#include "../stdafx.h"
#include "../display_manager.h"
#include "../system_driver.h"
#include "../gl/gl_display_manager.h"
#include "../exception.h"
#include "../world.h"
#include "../entity_manager.h"
#include "../camera.h"
#include "../action.h"
#include "../font.h"
#include "../player.h"

#include <windows.h>		// Header File For The Windows Library
#include <gl/gl.h>			// Header File For The OpenGL32 Library
#include <gl/glu.h>			// Header File For The GLu32 Library

#define DIRECTINPUT_VERSION	0x800
#include <dinput.h>			// Direct Input 8 header file
#include <hash_map>			// STL hash map for DI keys -> BuNg keys
#include "win32_system_driver.h"

#include <d3dx8.h>
#include "../d3d/d3d_display_manager.h"

const int KEYBOARD_BUFFER_SIZE = 32; // Buffer size.  32 should easily be big enough (I think) - Sam

int CWin32SystemDriver::RunMessageLoop()
{
	MSG		msg;									// Windows Message Structure
	BOOL	done = FALSE;							// Bool Variable To Exit Loop
	float	start_time, carry_time;

	DI_Initialise();

	start_time = TimerGetTime();
	active = true;
	minimised = false;
	SetCursorPos(world.display->GetWidth() / 2, world.display->GetHeight() / 2);

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active)								// Program Active?
			{
				//POINT temp_mouse;
				/*GetCursorPos(&temp_mouse);
				SetCursorPos(world.display->GetWidth() / 2, world.display->GetHeight() / 2);*/
				//world.entities->GetPlayer()->mpos.y = (float)(world.display->GetWidth() / 2 - temp_mouse.x);
				//world.entities->GetPlayer()->mpos.x = (float)(world.display->GetHeight() / 2 - temp_mouse.y);
				
				carry_time = TimerGetTime ();			// Get The Tick Count
				world.Update ((carry_time - start_time)/1000);	// Update The Counter
				start_time = carry_time;				// Set Last Count To Current Count

				DI_ProcessKeyboard();
			} 
			else
			{
				start_time = TimerGetTime(); // ?????
			}

			// Minimisation needs to be worked on: this doesn't seem to fix everything
			if(!minimised) {
				world.Draw();					// Draw the scene
			}
		}
	}
	
	DI_Cleanup();

	if(openGL)
		KillGLWindow();

	return 0;
}


CDisplayManager *CWin32SystemDriver::InitDisplay(int width, int height, int bpp, 
												 bool full_screen, CSystemDriver::DisplayType type)
{
	
	fullScreen = full_screen;

	TimerInit();
	
	if(type == DISPLAY_OPENGL)
	{
		openGL = true;

		// Create Our OpenGL Window
		if (!CreateGLWindow("BuNg", width, height, bpp))
		{
			throw CException("Unable to create display manager.");
		}

		CGLDisplayManager *disp = new CGLDisplayManager();
		disp->WindowResized(width, height);

		return disp;
	}
	else if(type == DISPLAY_DIRECT3D)
	{
/*		openGL = false;

		CreateWindowD3D("BuNg", width, height, bpp);

		CD3DDisplayManager *disp = new CD3DDisplayManager();
		disp->CreateD3D(width, height, full_screen, hWnd);

		return disp;*/
	}

	throw CException("Unsupported display type in InitDisplay()");
}

void CWin32SystemDriver::ResizeWindow(int width, int height)
{
	if(world.display == NULL)
		return;

	world.display->WindowResized(width, height);
}

void CWin32SystemDriver::Quit()
{
	PostMessage(hWnd, WM_QUIT, 0, 0);
}

void CWin32SystemDriver::TimerInit(void)
{
	memset(&timer, 0, sizeof(timer));   
	// Clear Our timer Structure
    // Check To See If A Performance Counter Is Available
    // If One Is Available The timer Frequency Will Be Updated
    if (!QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency))
    {
        // No Performace Counter Available
        timer.performance_timer = FALSE;  
		// Set Performance timer To FALSE
        timer.mm_timer_start = timeGetTime();			// Use timeGetTime() To Get Current Time
        timer.resolution  = 1.0f/1000.0f;				// Set Our timer Resolution To .001f
        timer.frequency   = 1000;   
		// Set Our timer Frequency To 1000
        timer.mm_timer_elapsed = timer.mm_timer_start;	// Set The Elapsed Time To The Current Time
    }
    else
    {
        // Performance Counter Is Available, Use It Instead Of The Multimedia timer
        // Get The Current Time And Store It In performance_timer_start
        QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
        timer.performance_timer   = TRUE;    // Set Performance timer To TRUE
        // Calculate The timer Resolution Using The timer Frequency
        timer.resolution    = (float) (((double)1.0f)/((double)timer.frequency));
        // Set The Elapsed Time To The Current Time
        timer.performance_timer_elapsed = timer.performance_timer_start;
    }
}

float CWin32SystemDriver::TimerGetTime()
{
	__int64 time;        
	// 'time' Will Hold A 64 Bit Integer
    if (timer.performance_timer)						// Are We Using The Performance timer?
    {
        QueryPerformanceCounter((LARGE_INTEGER *) &time);	// Grab The Current Performance Time
        // Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
        return ( (float) ( time - timer.performance_timer_start) * timer.resolution)*1000.0f;
    }
    else
    {
        // Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
        return( (float) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
    }
}

void CWin32SystemDriver::ForceWindowDraw()
{
	// TODO: this assumes OpenGL!
	SwapBuffers(hDC);
}

void CWin32SystemDriver::ErrorMessageBox(string title, string message)
{
	MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
}

CWin32SystemDriver::~CWin32SystemDriver()
{
	// TODO: this assumes OpenGL!
	KillGLWindow();
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	int		ret = 0;
	//int		cmdLineCount;
	//LPTSTR	lptCmdLine = GetCommandLine();
	//LPWSTR	lpwCmdLine = CommandLineToArgvW(lptCmdLine, &cmdLineCount);

	world.sys = new CWin32SystemDriver();
	ret = BungMain(0, NULL);

	//GlobalFree(lpwCmdLine);

	return ret;										// Exit The Program
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	CWin32SystemDriver *sys = (CWin32SystemDriver *)world.sys;

	switch (uMsg)									// Check For Windows Messages
	{
		
		case WM_ACTIVATE:								// Watch For Window Activate Message
		{
			//if ((LOWORD(wParam)==WA_ACTIVE)||(LOWORD(wParam)==WA_CLICKACTIVE))
			// ... and so on

			if(LOWORD(wParam) == WA_INACTIVE)
			{
				sys->SetActive(false);
			}
			else
			{
				if(HIWORD(wParam))
				{
					// Minimised
					sys->SetActive(false);
					sys->SetMinimised(true);
				}
				else
				{
					// Not minimised
					if(world.display) // a little hacked in, but oh well
						SetCursorPos(world.display->GetWidth() / 2, world.display->GetHeight() / 2);
					sys->SetActive(true);
					sys->SetMinimised(false);
				}
			}

			return 0;
		}

		case WM_KILLFOCUS:
		{
			sys->SetActive(FALSE);
			return 0;
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			world.sys->ResizeWindow(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}

		case WM_LBUTTONDOWN:
		{
			world.actionHandler->KeyDown(CActionHandler::BKC_LEFTMOUSEBUT);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			world.actionHandler->KeyUp(CActionHandler::BKC_LEFTMOUSEBUT);
			return 0;
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DirectInput specific stuff below:

void CWin32SystemDriver::DI_KeymapInit()
{
	// This MUST be kept up to date with is what is in CActionHandler.h
	// Yes, this IS a definciency in design to some degree.  However, that is what you get when you strive
	// for platform independence.
	keyMap[ DIK_ESCAPE ] = CActionHandler::BKC_ESCAPE;
	keyMap[ DIK_BACK ] = CActionHandler::BKC_BACKSPACE;
	keyMap[ DIK_RETURN ] = CActionHandler::BKC_RETURN;
	keyMap[ DIK_TAB ] = CActionHandler::BKC_TAB;
	keyMap[ DIK_A ] = CActionHandler::BKC_A;
	keyMap[ DIK_B ] = CActionHandler::BKC_B;
	keyMap[ DIK_C ] = CActionHandler::BKC_C; 
	keyMap[ DIK_D ] = CActionHandler::BKC_D; 
	keyMap[ DIK_E ] = CActionHandler::BKC_E; 
	keyMap[ DIK_F ] = CActionHandler::BKC_F; 
	keyMap[ DIK_G ] = CActionHandler::BKC_G; 
	keyMap[ DIK_H ] = CActionHandler::BKC_H; 
	keyMap[ DIK_I ] = CActionHandler::BKC_I; 
	keyMap[ DIK_J ] = CActionHandler::BKC_J; 
	keyMap[ DIK_K ] = CActionHandler::BKC_K; 
	keyMap[ DIK_L ] = CActionHandler::BKC_L; 
	keyMap[ DIK_M ] = CActionHandler::BKC_M; 
	keyMap[ DIK_N ] = CActionHandler::BKC_N; 
	keyMap[ DIK_O ] = CActionHandler::BKC_O; 
	keyMap[ DIK_P ] = CActionHandler::BKC_P; 
	keyMap[ DIK_Q ] = CActionHandler::BKC_Q; 
	keyMap[ DIK_R ] = CActionHandler::BKC_R; 
	keyMap[ DIK_S ] = CActionHandler::BKC_S; 
	keyMap[ DIK_T ] = CActionHandler::BKC_T; 
	keyMap[ DIK_U ] = CActionHandler::BKC_U; 
	keyMap[ DIK_V ] = CActionHandler::BKC_V; 
	keyMap[ DIK_W ] = CActionHandler::BKC_W; 
	keyMap[ DIK_X ] = CActionHandler::BKC_X;
	keyMap[ DIK_Y ] = CActionHandler::BKC_Y; 
	keyMap[ DIK_Z ] = CActionHandler::BKC_Z;
	keyMap[ DIK_1 ] = CActionHandler::BKC_1; 
	keyMap[ DIK_2 ] = CActionHandler::BKC_2; 
	keyMap[ DIK_3 ] = CActionHandler::BKC_3; 
	keyMap[ DIK_4 ] = CActionHandler::BKC_4; 
	keyMap[ DIK_5 ] = CActionHandler::BKC_5; 
	keyMap[ DIK_6 ] = CActionHandler::BKC_6;
	keyMap[ DIK_7 ] = CActionHandler::BKC_7; 
	keyMap[ DIK_8 ] = CActionHandler::BKC_8; 
	keyMap[ DIK_9 ] = CActionHandler::BKC_9; 
	keyMap[ DIK_0 ] = CActionHandler::BKC_0;
	//keyMap[  ] = BKC_EXCLAMATION;
	keyMap[ DIK_AT ] = CActionHandler::BKC_AT;
	//keyMap[  ] = BKC_HASH;
	//keyMap[  ] = BKC_DOLLAR;
	//keyMap[  ] = BKC_PERCENT;
	//keyMap[  ] = BKC_HAT;
	//keyMap[  ] = BKC_AMPERSAND;
	//keyMap[  ] = BKC_ASTERISK;
	keyMap[ DIK_LBRACKET ] = CActionHandler::BKC_LBRACKET;
	keyMap[ DIK_RBRACKET ] = CActionHandler::BKC_RBRACKET;
	keyMap[ DIK_MINUS ] = CActionHandler::BKC_MINUS;
	keyMap[ DIK_UNDERLINE ] = CActionHandler::BKC_UNDERSCORE;
	//keyMap[  ] = BKC_PLUS;
	keyMap[ DIK_EQUALS ] = CActionHandler::BKC_EQUALS;
	//keyMap[  ] = BKC_PIPE;
	keyMap[ DIK_BACKSLASH ] = CActionHandler::BKC_BACKSLASH;
	keyMap[ DIK_SLASH ] = CActionHandler::BKC_SLASH;
	keyMap[ DIK_COLON ] = CActionHandler::BKC_COLON;
	keyMap[ DIK_SEMICOLON ] = CActionHandler::BKC_SEMICOLON;
	keyMap[ DIK_APOSTROPHE ] = CActionHandler::BKC_APOSTRAPHE;
	//keyMap[  ] = BKC_QUOTE;
	keyMap[ DIK_SPACE ] = CActionHandler::BKC_SPACE;
	keyMap[ DIK_PERIOD ] = CActionHandler::BKC_PERIOD;
	keyMap[ DIK_COMMA ] = CActionHandler::BKC_COMMA;
	//keyMap[  ] = BKC_QUESTION_MARK;

	keyMap[ DIK_LEFT ] = CActionHandler::BKC_LEFT;
	keyMap[ DIK_RIGHT ] = CActionHandler::BKC_RIGHT;
	keyMap[ DIK_UP ] = CActionHandler::BKC_UP;
	keyMap[ DIK_DOWN ] = CActionHandler::BKC_DOWN;
}

void CWin32SystemDriver::DI_Initialise()
{
	if(FAILED(DirectInput8Create(hInstance, 
								 DIRECTINPUT_VERSION, 
								 IID_IDirectInput8, 
								 (void **)&directInput, 
								 NULL)
	  ))
		throw CException("Unable to create DirectInput8 device.");

	// Keyboard initialisation:
	if(FAILED(directInput->CreateDevice(GUID_SysKeyboard, 
								 &keyboard, 
								 NULL)
	  ))
		throw CException("Unable to create DirectInput8 keyboard device.");

	if(FAILED(keyboard->SetDataFormat(&c_dfDIKeyboard)))
		throw CException("DirectInput8: keyboard->SetDataFormat failed.");
	
	if(FAILED(keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND 
												| DISCL_NONEXCLUSIVE 
												| DISCL_NOWINKEY // Disable windows key! Yay for me! - Sam
										    )))
		throw CException("DirectInput8: keyboard->SetCooperativeLevel failed.");

	DIPROPDWORD prop;
	prop.diph.dwHeaderSize	= sizeof(DIPROPHEADER);
	prop.diph.dwHow			= DIPH_DEVICE;
	prop.diph.dwObj			= 0;
	prop.diph.dwSize		= sizeof(DIPROPDWORD);
	prop.dwData				= KEYBOARD_BUFFER_SIZE;
	if(FAILED(keyboard->SetProperty(DIPROP_BUFFERSIZE, &prop.diph)))
		throw CException("DirectInput8: keyboard->SetProperty (buffer size) failed.");
	
	// TODO: look into whether this should be a fatal error. If the window is not focused when this function
	// is called, the app effectively crashes. This is quite recoverable from, it is probably handled above
	// when we try to re-aquire it in the message loop or something.
	//if(FAILED(keyboard->Acquire()))
	//	throw CException("DirectInput8: keyboard->Acquire failed.");

	DI_KeymapInit();
}

void CWin32SystemDriver::DI_ProcessKeyboard()
{
    DIDEVICEOBJECTDATA didod[ KEYBOARD_BUFFER_SIZE ];  // Receives buffered data 
    DWORD              dwElements;
    DWORD			   i;
    HRESULT            hr;

    dwElements = KEYBOARD_BUFFER_SIZE;
    hr = keyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                     didod, &dwElements, 0 );

	if( hr != DI_OK ) 
    {
		hr = keyboard->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = keyboard->Acquire();

		return; // Everything should be processed in the next call
	}

	if(FAILED(hr))
		throw CException("GetDeviceData failed and stuff. I don't even know if "
			"this is a particularly bad thing.  Huh.  Maybe I should look into "
			"this or something.  Maybe.  Or maybe Jesse should just fix everything, "
			"and like, add cool stuff to BuNg.  Like character models that are as "
			"detailed as the WolfMan dude on the NVidia demo, but render at 200FPS "
			"on a TNT2 M64.  And other cool stuff.  Yeah.  Anyway, apparently "
			"Direct Input failed with the buffered keyboard stuff.  It probably "
			"needs to be fixed.  Well, I guess if you are seeing this it NEEDS to "
			"be fixed.  Yep.");

	// Study each of the buffer elements and process them.
    for( i = 0; i < dwElements; i++ ) 
    {
        DWORD scanCode = didod[ i ].dwOfs;
		bool down = (didod[ i ].dwData & 0x80) ? true : false;
		hash_map<int, CActionHandler::Keycode>::iterator key;

		key = keyMap.find(scanCode);

		if(key == keyMap.end())
			return; // No key defined.
        
		if(down)
			world.actionHandler->KeyDown( (*key).second );
		else
			world.actionHandler->KeyUp( (*key).second );
    }
}

void CWin32SystemDriver::DI_Cleanup()
{
	keyboard->Unacquire();
	keyboard->Release();
	directInput->Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GL Specific stuff below:
void CWin32SystemDriver::KillGLWindow()
{
	if (fullScreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

bool CWin32SystemDriver::CreateGLWindow(char *title, int width, int height, int bits)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	
	WindowRect.left		= (long)0;			
	WindowRect.right	= (long)width;		
	WindowRect.top		= (long)0;				
	WindowRect.bottom	= (long)height;		

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;											// Return false
	}
	
	if (fullScreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullScreen=false;		// Windowed Mode Selected.  Fullscreen = false
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return false;									// Return false
			}
		}
	}

	if (fullScreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;							// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(false);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, false, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return false
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 24Bit Z-Buffer (Depth Buffer)  (16 bit induced errors - Sam)
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC = GetDC(hWnd)))						// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return false
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return false
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return false
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return false
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return false
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ShowCursor(FALSE);								// We don't want no stinking cursor! (Sam)

	return true;									// Success
}

//////////////////////////////////////////////////////////////////////////////////
// D3D Specific stuff!
void CWin32SystemDriver::CreateWindowD3D(const char *title, int width, int height, int bpp)
{
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	
	WindowRect.left		= (long)0;			
	WindowRect.right	= (long)width;		
	WindowRect.top		= (long)0;				
	WindowRect.bottom	= (long)height;		

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "D3DBuNg";							// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;											// Return false
	}
	
	if (fullScreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;							// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(false);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, false, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"D3DBuNg",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;								// Return false
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ShowCursor(FALSE);								// We don't want no stinking cursor! (Sam)
}

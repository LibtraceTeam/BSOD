#ifndef __WIN32_SYSTEM_DRIVER_H__
#define __WIN32_SYSTEM_DRIVER_H__

class CWorld;
class CDisplayManager;

class CWin32SystemDriver : public CSystemDriver
{
private:
	HDC			hDC;			// Private GDI Device Context
	HWND		hWnd;			// Holds Our Window Handle
	HINSTANCE	hInstance;		// Holds The Instance Of The Application
	bool		fullScreen;
	bool		active;
	bool		minimised;
	bool		openGL;

	// D3D Specifc...
	void CreateWindowD3D(const char *name, int width, int height, int bpp);

	// DirectInput Specific
	LPDIRECTINPUT		directInput;
	LPDIRECTINPUTDEVICE keyboard;
	hash_map<int, CActionHandler::Keycode> keyMap;
	
	void		DI_Initialise();
	void		DI_ProcessKeyboard();
	void		DI_Cleanup();
	void		DI_KeymapInit();
	
	// GL Specific:
	void		KillGLWindow();
	bool		CreateGLWindow(char *title, int width, int height, int bits);
	HGLRC		hRC;			// Permanent Rendering Context

	// Timer stuff:
	struct Win32Timer {
		__int64       frequency;					// Timer Frequency
		float         resolution;					// Timer Resolution
		unsigned long mm_timer_start;     
		  
		// Multimedia Timer Start Value
		unsigned long mm_timer_elapsed;				// Multimedia Timer Elapsed Time
		bool		  performance_timer;    
		  
		// Using The Performance Timer?
		__int64       performance_timer_start;		// Performance Timer Start Value
		__int64       performance_timer_elapsed;	// Performance Timer Elapsed Time
	} timer;

public:
	void SetActive(bool a) { active = a; }
	void SetMinimised(bool m) { minimised = m; }
	virtual ~CWin32SystemDriver();

	enum DisplayType { DISPLAY_OPENGL, DISPLAY_DIRECT3D };

	virtual CDisplayManager *InitDisplay(int width, int height, int bpp, 
										 bool full_screen, CSystemDriver::DisplayType type);
	virtual int RunMessageLoop();
	virtual void ResizeWindow(int width, int height);
	virtual void Quit();
	virtual void TimerInit(void);
	virtual float TimerGetTime();
	virtual void ForceWindowDraw();
	virtual void ErrorMessageBox(string title, string message);
};

int BungMain(int argc, char *argv[]);

#endif
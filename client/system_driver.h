/* $CVSID$ */ 
#ifndef __SYSTEM_DRIVER_H__
#define __SYSTEM_DRIVER_H__

class CWorld;
class CDisplayManager;

class CSystemDriver
{
protected:

public:
	enum DisplayType { DISPLAY_OPENGL, DISPLAY_DIRECT3D };

	virtual CDisplayManager *InitDisplay(int width, int height, int bpp, bool fullScreen, 
		DisplayType type) = 0;
	virtual int RunMessageLoop() = 0;
	virtual void ResizeWindow(int width, int height) = 0;
	virtual void Quit() = 0;
	virtual void TimerInit(void) = 0;
	virtual float TimerGetTime() = 0;
	virtual void ForceWindowDraw() = 0;
	virtual void ErrorMessageBox(string title, string message) = 0;
};

int BungMain(int argc, char *argv[]);

#endif


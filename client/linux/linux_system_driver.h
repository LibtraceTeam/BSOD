#ifndef __LINUX_SYSTEM_DRIVER_H__
#define __LINUX_SYSTEM_DRIVER_H__

class CLinuxSystemDriver : public CSystemDriver
{
private:
	bool done;

public:
	CLinuxSystemDriver() : done(false) {  }

	virtual CDisplayManager *InitDisplay(int width, int height, int bpp, bool fullScreen, 
		DisplayType type);
	virtual int RunMessageLoop();
	virtual void ResizeWindow(int width, int height);
	virtual void Quit();
	virtual void TimerInit(void);
	virtual float TimerGetTime();
	virtual void ForceWindowDraw();
	virtual void ErrorMessageBox(string title, string message);
};


#endif // __LINUX_SYSTEM_DRIVER_H__

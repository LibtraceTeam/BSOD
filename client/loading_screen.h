#ifndef _LOADING_SCREEN_H_
#define _LOADING_SCREEN_H_

struct CTexture;

class IMessageCallback
{
public:
	virtual void AddMessage(string message) = 0;
};

class CLoadingScreen : public IMessageCallback
{
private:
	list<string> messages;
	int width, height;
	CTexture *background;
	int maxlen;

	void Redraw();
public:
	CLoadingScreen(int w, int h);
	virtual ~CLoadingScreen();

	virtual void AddMessage(string message);
};

#endif // _LOADING_SCREEN_H_


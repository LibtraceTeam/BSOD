/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
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


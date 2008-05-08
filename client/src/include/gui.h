#include "main.h"

#ifndef _GUI_H
#define _GUI_H

#define WINDOW_TITLE_HEIGHT 24


/*********************************************
			Event handling
**********************************************/
enum UIEvent{

	//Mouse events
	EVENT_MOUSE_DOWN,
	EVENT_MOUSE_UP,
	EVENT_MOUSE_CLICK,
	EVENT_MOUSE_DOUBLECLICK,
	EVENT_MOUSE_MOVED,
	EVENT_MOUSE_ENTER,
	EVENT_MOUSE_EXIT,
	
	//Key events
	EVENT_KEY_DOWN,
	EVENT_KEY_UP,
	EVENT_KEY_TYPED,
	
	//Misc events
	EVENT_TIMER,
	EVENT_PAINT
};

typedef void (*evtHandler_t)(UIEvent evt, int eventData, void *userData);

typedef struct{
	evtHandler_t callback;
	void *userData;
} EventCallback;

/*********************************************
			Base class for all UI
**********************************************/
class UIElement{
protected:

	Vector2 mPos;
	Vector2 mSize;
	string mName;
	
	bool bIsHidden;
	
	//Color 	
	Color mColor;
	float fColourMultiplier;	
	float fChildColorMultiplier;
	
	UIElement *mParent;
		
	map< string, byte * > mProperties; 
	map< UIEvent, map<int, EventCallback *> > mEventHandlers;	
	vector< UIElement * > mChildren;
	
	string mData; //generic 'data'
	
public:
	virtual ~UIElement(){}
	
	void initGeneric(const char *name, Vector2 pos, Vector2 size);
	
	//Parent
	UIElement *parent(){return mParent;}
	void setParent(UIElement *e){mParent = e;}
	
	//Can (and probably should) be overridden
	virtual void init(){}
	virtual void render()=0;
	virtual void addChild(UIElement *);
	virtual void renderChildren();
	
	//Various getters and setters
	Vector2 getSize(){return mSize;}
	Vector2 getPos(){return mPos;}
	Color getColor(){return mColor;}
	void setPos(Vector2 v){mPos = v;}
	void setSize(Vector2 s){mSize = s;}
	void setColor(Color c){mColor = c;}
	string getData(){return mData;}
	void setData(string s){mData = s;}
	void setData(const char *c){mData = c;}
	UIElement *getChild(int i){ return mChildren[i]; }
		
	//Event handling
	void addEventHandler(UIEvent evt, void *userData, evtHandler_t callback);
	void addEventHandler(UIEvent evt, int eventData, void *userData, evtHandler_t callback);

	//Called to signify an event has happened, and will invoke any callbacks
	//Returns TRUE if it was handled
	bool event(UIEvent evt, int eventData, int eventX, int eventY); 
	bool event(UIEvent evt, int eventX, int eventY);
	bool event(UIEvent evt);
	
	EventCallback *getEventHandler(UIEvent evt, int eventData);
	EventCallback *getEventHandler(UIEvent evt);
	
	//Util functions
	bool intersects(int x, int y); 
	Vector2 getWorldPos(); //returns the actual screen pos, not relative
	
	//Hidden
	bool isHidden(){return bIsHidden;}
	void hide(bool b){bIsHidden = b;}
	
	//Color management
	//(for fancy fading windows, etc)
	void setColorMultiplier(float f){fColourMultiplier = f;}
	void bindColor(float r, float g, float b); //Takes color multiplier into account
	void bindColor(Color c);	
	
	//Called when a child wants to move from its current pos to newPos
	//in Z-order
	void modifyChildZOrder(UIElement *e, int newPos);
};

/*********************************************
			Clickable button
**********************************************/
class UIButton : public UIElement {
	void renderBorderless();
	void renderNormal();
	
	int mStyle;
	
	Texture *mTexture;
public:
	void render();
	void init();
	
	void setStyle(int s){ mStyle = s; }
	
	//Style consts
	static const int STYLE_BORDERLESS = 1;
};


/*********************************************
			Toggle-able checkbox
**********************************************/
class UICheckbox : public UIElement {
	bool bChecked;
	
	Texture *mTexture[2];
public:
	void render();
	void init();
};

/*********************************************
		Simple window
**********************************************/
enum WindowState { NORMAL, CLOSING, OPENING, MINIMISED, CLOSED, MOVING };
	
class UIWindow : public UIElement{

	//Only used for the titlebar
	Texture *mTexture;
	
	//Title bar buttons
	UIButton *mCloseButton;
	UIButton *mMinButton;	
	UIButton *mTitleButton;
	
	//Transperancy 
	float fAlpha;
		
	//Current state
	WindowState mState;
	
	int mStyle;
	
public:

	static const int STYLE_BORDERLESS = 1;

	void render();
	void init();	
	void setState(WindowState s){mState = s;}		
	void setStyle(int i);
};


/*********************************************
			A top-level element
**********************************************/
class UIScreen : public UIElement{
public:
	void render();
};


/*********************************************
			Simple text label
**********************************************/
class UIText : public UIElement {
public:
	void render();
};





#endif

#include "gui.h"


/*********************************************
				UIElement
**********************************************/
void UIElement::initGeneric(const char *name, Vector2 pos, Vector2 size){
	
	mPos = pos;
	mSize = size;
	mColor = Color(1,1,1);
	mName = name;
	mData = "";
	
	mName = name; //hack!
	mEventHandlers.clear();	
	setParent(NULL);
	hide(false);	
	setColorMultiplier(1.0f);
	fChildColorMultiplier = 1.0f;
}

void UIElement::bindColor(float r, float g, float b){
	glColor4f(r * fColourMultiplier, g * fColourMultiplier, b * fColourMultiplier, fColourMultiplier);
}

void UIElement::bindColor(Color c){
	bindColor(c.r, c.g, c.b);
}

//Event handling
void UIElement::addEventHandler(UIEvent evt, void *userData, evtHandler_t callback){
	addEventHandler(evt, 0, userData, callback);	
}

void UIElement::addEventHandler(UIEvent evt, int eventData, void *userData, evtHandler_t callback){

	EventCallback *c = new EventCallback;
	c->callback = callback;
	c->userData = userData;

	mEventHandlers[evt][eventData] = c;
}


//Called to signify an event has happened, and will invoke any callbacks
bool UIElement::event(UIEvent evt, int eventData, int eventX, int eventY){

	//Check to see if it overlaps any of our children...
	for(int i=(int)mChildren.size() - 1;i>=0;i--){
	
		if(mChildren[i]->isHidden()){	
			continue;
		}
	
		if(mChildren[i]->intersects(eventX - (int)mPos.x, eventY - (int)mPos.y)){
			return mChildren[i]->event(evt, eventData, eventX - (int)mPos.x, eventY - (int)mPos.y);
		}
	}
		
	//If we get here, no child is intersecting the event.
	//We can handle it ourselves if we have a handler	
	EventCallback *handler = getEventHandler(evt, eventData);	
	bool handled = false;
	
	if(handler){
		handler->callback(evt, eventData, handler->userData);
		handled = true;
	}
		
	//We check with eventData==0 here as well
	//FIXME: this is a bit clunky...
	handler = getEventHandler(evt, 0);
	
	if(handler){
		handler->callback(evt, eventData, handler->userData);
		handled = true;
	}
	
	//LOG("%s - %shandled event %d(%d) at %d/%d\n", 
	//	mName, !handled ? "Un" : "", evt, 
	//	eventData, eventX, eventY);
				
	return handled;
}

inline bool UIElement::event(UIEvent evt){
	return event(evt, 0, 0, 0);
}

inline bool UIElement::event(UIEvent evt, int eventX, int eventY){
	return event(evt, 0, eventX, eventY);
}

//Event handler getters
EventCallback *UIElement::getEventHandler(UIEvent evt, int eventData){
	
	if(mEventHandlers.find(evt) == mEventHandlers.end()){
		return NULL;
	}
		
	EventCallback *handler = mEventHandlers[evt][eventData];
	
	if(!handler){
		return NULL;
	}
	
	return handler;
}

inline EventCallback *UIElement::getEventHandler(UIEvent evt){
	return getEventHandler(evt, 0);
}

//Util
bool UIElement::intersects(int x, int y){
	return 	x > mPos.x && 
			y > mPos.y && 
			x < mPos.x + mSize.x && 
			y < mPos.y + mSize.y;
}

void UIElement::addChild(UIElement *e){
	mChildren.push_back(e);	
	e->setParent(this);
	
	Vector2 pos = e->getPos();
	Vector2 size = e->getSize();
	
	//Center in X?
	if(pos.x == -1){
		pos.x = mSize.x/2 - size.x/2;
	}
	
	//Offset from the right?
	else if(pos.x < -1){
		pos.x = mSize.x - size.x + pos.x;		
	}
	
	
	if(pos.y == -1){
		pos.y = mSize.y/2 - size.y/2;
	}
	
	//Offset from the bottom?
	else if(pos.y < -1){
		pos.y = mSize.y - size.y + pos.y;		
	}
	
	e->setPos(pos);
	e->setSize(size);
	
}

void UIElement::renderChildren(){

	for(int i=0;i<(int)mChildren.size();i++){
	
		if(mChildren[i]->isHidden()){				
			continue;
		}
	
		Vector2 pos = mChildren[i]->getPos();
		Vector2 size = mChildren[i]->getSize();
		
		//LOG("%s - %f/%f, %f/%f\n", mName.c_str(), pos.x, pos.y, size.x, size.y);
		
		//Vector2 scr = mChildren[i]->getWorldPos();		
		//glScissor((int)scr.x, (int)scr.y, (int)size->x, (int)size->y);
		
		glTranslatef(pos.x, pos.y, 0);	
			mChildren[i]->setColorMultiplier(fChildColorMultiplier);		
			mChildren[i]->render();
		glTranslatef(-pos.x, -pos.y, 0);
	}
}

//This is a bit nasty. We go back through the tree
//till we get to the root node, incrementing position as we go
Vector2 UIElement::getWorldPos(){
	Vector2 pos = Vector2(0,0);
	
	UIElement *current = this;
	
	while(current){
		pos = pos + current->getPos();		
		current = current->parent();
	}
	
	return pos;
}

void UIElement::modifyChildZOrder(UIElement *e, int newPos){

	if(newPos < 0){
		newPos = 0;
	}else if(newPos >= (int)mChildren.size()){
		newPos = mChildren.size() - 1;
	}

	for(int i=0;i<(int)mChildren.size();i++){
		if(mChildren[i] == e){
			mChildren[i] = mChildren[newPos];
			mChildren[newPos] = e;
			return;
		}
	}
}

/*********************************************
				UIWindow
**********************************************/
void onWindowClose(UIEvent evt, int eventData, void *userData){
	UIWindow *wnd = (UIWindow *)userData;	
	wnd->setState(CLOSING);
}

void onWindowTitleClick(UIEvent evt, int eventData, void *userData){
	UIWindow *wnd = (UIWindow *)userData;
	
	if(evt == EVENT_MOUSE_DOWN){
		wnd->setState(MOVING);
		wnd->parent()->modifyChildZOrder(wnd, 9999); //bring it to the top
	}else{
		wnd->setState(NORMAL);
	}
}

void UIWindow::init(){	
	setData("Test Window");
	
	mTexture = App::S()->texGet("wm.png");
	
	//Close
	mCloseButton = App::S()->guiCreate<UIButton>("btnClose", 
							Vector2(mSize.x - 20, 2), 
							Vector2(WINDOW_TITLE_HEIGHT, WINDOW_TITLE_HEIGHT),
							this
				);
				
	mCloseButton->setStyle(UIButton::STYLE_BORDERLESS);
	mCloseButton->setData("x");
	mCloseButton->addEventHandler(EVENT_MOUSE_UP, (void *)this, &onWindowClose);
	
	//Minimize
	/*
	mMinButton = App::S()->guiCreate<UIButton>("btnMin", 
							Vector2(mSize.x - 40, 2), 
							Vector2(WINDOW_TITLE_HEIGHT, WINDOW_TITLE_HEIGHT),
							this
				);
				
	mMinButton->setStyle(UIButton::STYLE_BORDERLESS);
	mMinButton->setData("-");
	//mMinButton->addEventHandler(EVENT_MOUSE_UP, (void *)this, &onWindowClose);
	*/
	
	//Title
	mTitleButton = App::S()->guiCreate<UIButton>("btnClose", 
							Vector2(0, 0), 
							Vector2(mSize.x - 40, WINDOW_TITLE_HEIGHT),
							this
				);
				
	mTitleButton->setStyle(UIButton::STYLE_BORDERLESS);
	mTitleButton->setData("");
	mTitleButton->addEventHandler(EVENT_MOUSE_UP, (void *)this, &onWindowTitleClick);
	mTitleButton->addEventHandler(EVENT_MOUSE_DOWN, (void *)this, &onWindowTitleClick);
			 
	fAlpha = 0.0f; //so it fades in
	fChildColorMultiplier = 0.0f;
	
	mState = OPENING;
	
	hide(false);
		
	LOG("Made window\n");
}

void UIWindow::render(){
	

	if(mStyle == STYLE_BORDERLESS){
	
		glDisable(GL_TEXTURE_2D);
		float c = fAlpha * 0.05f;
		glColor4f(c, c, c, fAlpha * 0.5f);
	
		glBegin(GL_QUADS);
			glVertex2f(0, 0);
			glVertex2f(mSize.x, 0);
			glVertex2f(mSize.x,  mSize.y);
			glVertex2f(0,  mSize.y);
		glEnd();
	}
	
	
	else{
	
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glColor4f(fAlpha, fAlpha, fAlpha, fAlpha);
	
		mTexture->bind();
	
		//Draw the title bar
		glBegin(GL_QUADS);
			glTexCoord2f(0,0); glVertex2f(0, 0);
			glTexCoord2f(1,0); glVertex2f(mSize.x, 0);
			glTexCoord2f(1,1); glVertex2f(mSize.x,  WINDOW_TITLE_HEIGHT);
			glTexCoord2f(0,1); glVertex2f(0,  WINDOW_TITLE_HEIGHT);
		glEnd();
	
		//Draw the title text	
		App::S()->writeText(5, 2, "%s", mData.c_str());
		
		//Draw the rest of the window
		glDisable(GL_TEXTURE_2D);
		float c = fAlpha * 0.25f;
		glColor4f(c, c, c, fAlpha * 0.5f);
	
		glBegin(GL_QUADS);
			glVertex2f(0, WINDOW_TITLE_HEIGHT);
			glVertex2f(mSize.x, WINDOW_TITLE_HEIGHT);
			glVertex2f(mSize.x,  mSize.y - WINDOW_TITLE_HEIGHT);
			glVertex2f(0,  mSize.y - WINDOW_TITLE_HEIGHT);
		glEnd();
	
	}
		
	renderChildren();
	
	//Update stuff based on state
	if(mState == OPENING){	
		fAlpha += fTimeScale * 2;
		
		if(fAlpha >= 1.0f){
			fAlpha = 1.0f;
			mState = NORMAL;
		}
	}
	else if(mState == CLOSING){
		fAlpha -= fTimeScale * 2;
		
		if(fAlpha <= 0.0f){
			fAlpha = 0.0f;
			mState = CLOSED;
			hide(true);			
		}
	}
	else if(mState == MOVING){
		Vector2 m = App::S()->getMouse();
		mPos.x = m.x - mSize.x/2;
		mPos.y = m.y - WINDOW_TITLE_HEIGHT/2;
	}
	
	fChildColorMultiplier = fAlpha;
}

void UIWindow::setStyle(int i){

	mStyle = i;
	
	mCloseButton->hide(mStyle == STYLE_BORDERLESS);
	mMinButton->hide(mStyle == STYLE_BORDERLESS);
	mTitleButton->hide(mStyle == STYLE_BORDERLESS);
}
	

/*********************************************
				UIScreen
**********************************************/
void UIScreen::render(){
	renderChildren();
}



/*********************************************
				UIButton
**********************************************/
void onButtonMouseDown(UIEvent evt, int eventData, void *userData){
	UIButton *btn = (UIButton *)userData;	
	btn->setColor(Color(1,0.5f,0.5f));
}

void onButtonMouseUp(UIEvent evt, int eventData, void *userData){
	UIButton *btn = (UIButton *)userData;	
	btn->setColor(Color(1,1,1));
}

void UIButton::init(){
	setData("button");
	setStyle(0);
	setColor(Color(1,1,1));
	
	mTexture = App::S()->texGet("wm.png");
	
	if(!mTexture){
		mTexture = App::S()->texLoad("wm.png", 0);
		mTexture = App::S()->texGet("wm.png");
	}
	
	addEventHandler(EVENT_MOUSE_DOWN, 1, (void *)this, &onButtonMouseDown);
	addEventHandler(EVENT_MOUSE_UP, 1, (void *)this, &onButtonMouseUp);
}

void UIButton::render(){
		
	if(mStyle == 0){
		renderNormal();
	}else if(mStyle == STYLE_BORDERLESS){
		renderBorderless();
	}else{
		LOG("Button: bad style %d\n", mStyle);
	}
	
}

void UIButton::renderBorderless(){
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	
	bindColor(mColor.r, mColor.g, mColor.b);

	App::S()->writeText(3, 0, "%s", mData.c_str());
}

void UIButton::renderNormal(){

	glEnable(GL_TEXTURE_2D);
	
	mTexture->bind();
	
	bindColor(mColor.r, mColor.g, mColor.b);
	
	glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex2f(0, 0);
		glTexCoord2f(1,0); glVertex2f(mSize.x, 0);
		glTexCoord2f(1,1); glVertex2f(mSize.x,  mSize.y);
		glTexCoord2f(0,1); glVertex2f(0,  mSize.y);
	glEnd();	
	
	bindColor(mColor.r * 0.5f, mColor.g * 0.5f, mColor.b * 0.5f);
	
	glBegin(GL_LINES);
		glVertex2f(0, 0);
		glVertex2f(mSize.x, 0);
		glVertex2f(mSize.x, 0);
		glVertex2f(mSize.x,  mSize.y);
		glVertex2f(mSize.x,  mSize.y);
		glVertex2f(0,  mSize.y);
		glVertex2f(0,  mSize.y);
		glVertex2f(0, 0);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	App::S()->writeText(3, 0, "%s", mData.c_str());	
}


/*********************************************
				UIText
**********************************************/
void UIText::render(){
	glEnable(GL_TEXTURE_2D);
	bindColor(mColor.r, mColor.g, mColor.b);
	App::S()->writeText(0, 0, "%s", mData.c_str());
}


/*********************************************
				UICheckbox
**********************************************/
void UICheckbox::render(){

	bChecked = (mData == "true");

	glEnable(GL_TEXTURE_2D);
	
	mTexture[bChecked ? 1 : 0]->bind();
	
	bindColor(mColor.r, mColor.g, mColor.b);
	
	glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex2f(0, 0);
		glTexCoord2f(1,0); glVertex2f(mSize.x, 0);
		glTexCoord2f(1,1); glVertex2f(mSize.x,  mSize.y);
		glTexCoord2f(0,1); glVertex2f(0,  mSize.y);
	glEnd();	

	
}

void UICheckbox::init(){
	bChecked = false;
	
	mTexture[0] = App::S()->texGet("unticked.png");
	mTexture[1] = App::S()->texGet("ticked.png");
}


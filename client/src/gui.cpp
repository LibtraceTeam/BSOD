#include "main.h"

//There are some nasty issues with this file pulling in gl.h and conflicting
//with glew.h, so we have it #included here instead of in libs.h
#include "RendererModules/OpenGLGUIRenderer/openglrenderer.h"

//These are alo apparently not included (properly) by CEGUI.h
#include "CEGUIDefaultResourceProvider.h"
#include "XMLParserModules/XercesParser/CEGUIXercesParser.h"

//Only for this file...
using namespace CEGUI;


/*********************************************
	CEGUI components**********************************************/
OpenGLRenderer*mGUI = NULL;
DefaultWindow *root = NULL;
WindowManager *winMgr = NULL;
FrameWindow *mProtoWindow = NULL;
FrameWindow *mServerWindow = NULL;
FrameWindow *mOptionWindow = NULL;

//Globals. Stolen from:
//http://www.cegui.org.uk/wiki/index.php/Using_CEGUI_with_SDL_and_OpenGL
bool handle_mouse_down(Uint8 button);
bool handle_mouse_up(Uint8 button);
CEGUI::uint SDLKeyToCEGUIKey(SDLKey key);

float fGUITimeout = GUI_HIDE_DELAY;


/*********************************************
	Called by the SDL event loop to pass
	events off to CEGUI**********************************************/
bool App::processGUIEvent(SDL_Event e){

	//Whenever any even happens, make sure the GUI is shown
	fGUITimeout = GUI_HIDE_DELAY;

	bool handled = false;
	CEGUI::uint kc = 0;
	    
    switch( e.type ){
				      
	case SDL_MOUSEMOTION:
		// we inject the mouse position directly.
		CEGUI::System::getSingleton().injectMousePosition(
										e.motion.x,e.motion.y );
		break;

	case SDL_MOUSEBUTTONDOWN:
		handled = handle_mouse_down(e.button.button);
		break;

	case SDL_MOUSEBUTTONUP:
		handled = handle_mouse_up(e.button.button);
		break;
		
	case SDL_KEYDOWN:
		// to tell CEGUI that a key was pressed, we inject the scancode.
		kc = SDLKeyToCEGUIKey(e.key.keysym.sym);
		CEGUI::System::getSingleton().injectKeyDown(kc);

		// as for the character it's a litte more complicated. we'll use for translated unicode value.
		// this is described in more detail below.
		if (e.key.keysym.unicode != 0){
			CEGUI::System::getSingleton().injectChar(e.key.keysym.unicode);
		}
				
		break;

	case SDL_KEYUP:
		// like before we inject the scancode directly.
		kc = SDLKeyToCEGUIKey(e.key.keysym.sym);
		CEGUI::System::getSingleton().injectKeyUp(kc);
		break;
		
	case SDL_VIDEORESIZE:
		mGUI->grabTextures();
		break;
		
	}
			
	return handled;
}

/*********************************************
	Called when we click a menu toggle**********************************************/
bool App::onMenuButtonClicked(const EventArgs &args){
	
	//Get the control that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	String senderID = we->window->getName();
	FrameWindow *target = NULL;
	
	//From that we can figure out which window we want to toggle
	if(senderID == "btnServers") target = mServerWindow;
	else if(senderID == "btnProtocols") target = mProtoWindow;
	else if(senderID == "btnOptions") target = mOptionWindow;
	
	//Toggle the window if we found it. TODO: We could do fancy fades with 
	//setAlpha() and such...
	if(target){
		if(target->isVisible()){
			target->hide();
		}else{
			target->show();
			target->moveToFront();
		}
	}else{
		LOG("Bad target from button '%s' in onMenuButtonClicked\n", 
			senderID.c_str());	
	}
	return true;
}


/*********************************************
	Called when we click a protocol toggle**********************************************/
bool App::onProtocolClicked(const EventArgs &args){
	//Get the control that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	String senderID = we->window->getName();
	Checkbox *cb = (Checkbox *)we->window;
		
	//Kinda nasty: The name of the checkbox is the protocol ID it's associated 
	//with. Perhaps this should be made less hacky. 
	int id = stringTo<int>(senderID.c_str());
	
	//LOG("Protocol ID %d %s\n", id, cb->isSelected() ? "on":"off");
	
	//Make sure we have a valid flow descriptor ID
	if(getFD(id)){
		//Toggling bShown makes sure that no new particles of this type get
		//created.
		getFD(id)->bShown = cb->isSelected();
		ps()->showColor(getFD(id)->mColor, cb->isSelected());
	}
	
	return true;	
}

/*********************************************
	Called when we click a protocol button**********************************************/
bool App::onProtocolButtonClicked(const EventArgs &args){
	
	//Get the object that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	String senderID = we->window->getName();
	
	bool show = false;
	
	if(senderID == "btnShowAll"){
		show = true;
	}else if(senderID == "btnHideAll"){
		show = false;
	}else{	
		//Rogue button?
		return true;
	}
	
	//Set all the protocol checkboxes
	for(int i=0;i<MAX_FLOW_DESCRIPTORS;i++){
	
		//Make sure this one is valid
		if(!getFD(i)) continue;		
	
		Checkbox *cb = (Checkbox *)winMgr->getWindow(toString(i));
				
		//Toggle the checkbox
		cb->setSelected(show);
		
		//And set the internal state too
		getFD(i)->bShown = show;
	}
		
	return true;
}

/*********************************************
	Called when we click a server button**********************************************/
bool App::onServerButtonClicked(const EventArgs &args){
	
	//Get the object that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	String senderID = we->window->getName();
	
	bool show = false;
	
	if(senderID == "btnShowAll"){
		show = true;
	}else if(senderID == "btnHideAll"){
		show = false;
	}else{	
		//Rogue button?
		return true;
	}
	
	return true;
}

bool App::onServerListClicked(const EventArgs &args){

	Editbox *eb = (Editbox *)winMgr->getWindow("txtCustomServer");
	Listbox *lb = (Listbox *)((WindowEventArgs *)&args)->window;
	
	eb->setText(lb->getFirstSelectedItem()->getText());
		
	return true;
}

/*********************************************
	Called when we click a close button**********************************************/
bool App::onWndClose(const CEGUI::EventArgs &args){
	//Get the object that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	we->window->hide();
	
	return true;
}

bool App::onMouseCursorChanged(const CEGUI::EventArgs&){

	//If CEGUI has changed to a valid image, hide the SDL cursor. This ensures
	//that we do sensible things when we mouse over edit boxes and such, 
	//otherwise we have duplicate cursors. 
	if(CEGUI::MouseCursor::getSingleton().getImage()){
		SDL_ShowCursor(SDL_DISABLE);
	}else{
		SDL_ShowCursor(SDL_ENABLE);
	}
}

bool App::onOptionSliderMoved(const CEGUI::EventArgs& args){
	WindowEventArgs *we = (WindowEventArgs *)&args;
	Slider *slide = (Slider *)we->window;
	
	float val = (slide->getCurrentValue() * 10.0f);
	
	if(we->window->getName() == "slideSpeed"){	
		fParticleSpeedScale = val;
		DefaultWindow *text = (DefaultWindow *)winMgr->getWindow("txtSpeedInfo");
		text->setText("Particle Speed: " + toString(val));
	}else{
		fParticleSizeScale = val;
		DefaultWindow *text = (DefaultWindow *)winMgr->getWindow("txtSizeInfo");
		text->setText("Particle Size: " + toString(val));
	}
}

/*********************************************
		CEGUI setup - create the UI**********************************************/
void App::initGUI(){

	//Set some SDL stuff to make the GUI work nicely
	//SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	//Set up the GUI object
	mGUI = new CEGUI::OpenGLRenderer( 0, iScreenX, iScreenY );
	new CEGUI::System( mGUI );
	
	// initialise the required dirs for the DefaultResourceProvider
	CEGUI::DefaultResourceProvider* rp = (CEGUI::DefaultResourceProvider *)
							CEGUI::System::getSingleton().getResourceProvider();

	//Set the resource groups. This maps a name of a resource group to a type
    CEGUI::Imageset::setDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
    CEGUI::WindowManager::setDefaultResourceGroup("layouts");
    CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");

	//Tell the resource provider where things are
	rp->setResourceGroupDirectory("schemes", 	"./data/gui/schemes/");
	rp->setResourceGroupDirectory("imagesets", 	"./data/gui/imagesets/");
	rp->setResourceGroupDirectory("fonts", 		"./data/gui/fonts/");
	rp->setResourceGroupDirectory("layouts", 	"./data/gui/layouts/");
	rp->setResourceGroupDirectory("looknfeels", "./data/gui/looknfeel/");
	rp->setResourceGroupDirectory("lua_scripts","./data/gui/lua_scripts/");
	rp->setResourceGroupDirectory("schemas", 	"./data/gui/XMLRefSchema/");
	
	//CEGUI::XercesParser::setSchemaDefaultResourceGroup("schemas");
	
	//Load Arial to make sure we have at least one font
	if(!CEGUI::FontManager::getSingleton().isFontPresent( "arial" ) )
		CEGUI::FontManager::getSingleton().createFont( "arial.font" );

	//Load in the themee file
	CEGUI::SchemeManager::getSingleton().loadScheme( "SleekSpace.scheme" );
	
	// All windows and widgets are created via the WindowManager singleton.
    winMgr = &WindowManager::getSingleton();   
    
    //Create the root window
    root = (DefaultWindow*)winMgr->createWindow("DefaultWindow", "Root");
    System::getSingleton().setGUISheet(root); 
    		
    mProtoWindow = NULL;
    		
   	//Create the main menu buttons
   	makeMenuButtons();
		
	//Create the protcol toggle window
	makeProtocolWindow();
	
	//And the server window
	makeServerWindow();
	
	//And finally the options window
	makeOptionWindow();
		
	//Set the mouse cursor event.
	CEGUI::MouseCursor::getSingleton().subscribeEvent(
						MouseCursor::EventImageChanged, 
						Event::Subscriber(&App::onMouseCursorChanged, this) );
}

void App::resizeGUI(int x, int y){
	mGUI->restoreTextures();
    mGUI->setDisplaySize(CEGUI::Size(x, y));
}



/*********************************************
  Adds a named checkbox to the protocol wnd**********************************************/
void App::addProtocolEntry(string name, Color col, int index){

	if(!mProtoWindow){
		return;
	}

	Checkbox* cb = (Checkbox *)winMgr->createWindow("SleekSpace/Checkbox", toString(index));
	mProtoWindow->addChildWindow(cb);
	
	float ypos = (index / 2) * 0.08f;
	float xpos = (index % 2) * 0.4f;
	
	ypos += 0.1;
	xpos += 0.1;
	
	cb->setPosition(UVector2(cegui_reldim( xpos ), cegui_reldim( ypos )));
	cb->setSize(UVector2(cegui_reldim(0.35f), cegui_reldim( 0.05f)));
	cb->setText(name.c_str());
	cb->setProperty("NormalTextColour", col.toString()); 
	cb->setSelected(true);
	
	cb->subscribeEvent(Checkbox::EventCheckStateChanged, 
						Event::Subscriber(&App::onProtocolClicked, this) );
}


/*********************************************
	  Make the buttons down the bottom
	  of the screen.**********************************************/
void App::makeMenuButtons(){
 		
	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "btnServers"));
    root->addChildWindow(btn);
    btn->setText("Servers");
    btn->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
     
    btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "btnProtocols"));
    root->addChildWindow(btn);
    btn->setText("Protocols");
    btn->setPosition(UVector2(cegui_reldim(0.12f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "btnOptions"));
    root->addChildWindow(btn);
    btn->setText("Options");
    btn->setPosition(UVector2(cegui_reldim(0.23f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
}


/*********************************************
	Creates the protocol toggle window**********************************************/
void App::makeProtocolWindow(){
	          
    mProtoWindow = (FrameWindow*)winMgr->createWindow("SleekSpace/FrameWindow", "wndProtocol");
    root->addChildWindow(mProtoWindow);
    
    mProtoWindow->setPosition(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.25f)));
    mProtoWindow->setSize(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.5f)));  
    mProtoWindow->setMaxSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 1.0f)));
    mProtoWindow->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f))); 

    mProtoWindow->setText("Protocols");
    
    //for(int i=0;i<10;i++){   
	//	addProtocolEntry("protocol" + toString(i), Color(randFloat(0,1), randFloat(0,1), randFloat(0,1)), i);
   	//}
   	
   	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "btnHideAll"));
    mProtoWindow->addChildWindow(btn);
    btn->setText("Hide All");
    btn->setPosition(UVector2(cegui_reldim(0.06f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onProtocolButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "btnShowAll"));
    mProtoWindow->addChildWindow(btn);
    btn->setText("Show All");
    btn->setPosition(UVector2(cegui_reldim(0.54f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onProtocolButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    mProtoWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
    mProtoWindow->setAlpha(0.85f);
    mProtoWindow->setSizingEnabled(false);
    mProtoWindow->hide();
}

void App::makeServerWindow(){
	mServerWindow = (FrameWindow *)winMgr->createWindow("SleekSpace/FrameWindow", "wndServer");
    root->addChildWindow(mServerWindow);
    
    mServerWindow->setPosition(UVector2(cegui_reldim(0.22f), cegui_reldim( 0.3f)));
    mServerWindow->setSize(UVector2(cegui_reldim(0.5f), cegui_reldim( 0.5f)));  
    mServerWindow->setMaxSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 1.0f)));
    mServerWindow->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f))); 

    mServerWindow->setText("Servers");
    
    //for(int i=0;i<10;i++){   
	//	addProtocolEntry("protocol" + toString(i), Color(randFloat(0,1), randFloat(0,1), randFloat(0,1)), i);
   	//}
   	
   	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "btnRefresh"));
    mServerWindow->addChildWindow(btn);
    btn->setText("Refresh List");
    btn->setPosition(UVector2(cegui_reldim(0.06f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onServerButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "btnConnect"));
    mServerWindow->addChildWindow(btn);
    btn->setText("Connect");
    btn->setPosition(UVector2(cegui_reldim(0.54f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onServerButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    
   	Listbox* lb = (Listbox *)(winMgr->createWindow("SleekSpace/Listbox", "lbServers"));
   	mServerWindow->addChildWindow(lb);
   	lb->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.28f)));
    lb->setSize(UVector2(cegui_reldim(0.8f), cegui_reldim( 0.45f)));
   
    lb->addItem(new ListboxTextItem("paul-desktop:12345 (10.1.60.177:12345)"));
    lb->addItem(new ListboxTextItem("paul-laptop:12345 (10.1.60.123:12345)"));
    lb->addItem(new ListboxTextItem("localhost:12345 (127.0.0.1:12345)"));
        
    lb->subscribeEvent(Listbox::EventSelectionChanged, Event::Subscriber(&App::onServerListClicked, this));
    
    
    DefaultWindow* text = (DefaultWindow *)winMgr->createWindow("SleekSpace/StaticText", "txtServerInfo");
    mServerWindow->addChildWindow(text);
	text->setText("Currently connected to 'paul-desktop:12345'. \
					Select a different server from the list below or specify \
					a custom address manually.");
					
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.08f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.2f)));
	//text->setProperty("TextColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF0000 br:FFFF0000");
	text->setAlwaysOnTop(true);	
	text->setProperty("HorzFormatting", "WordWrapLeftAligned"); // LeftAligned, RightAligned, HorzCentred
				// HorzJustified, WordWrapLeftAligned, WordWrapRightAligned, WordWrapCentred, WordWrapJustified
				
				
				
				
	Editbox* edit = (Editbox *)winMgr->createWindow("SleekSpace/Editbox", "txtCustomServer");
    mServerWindow->addChildWindow(edit);
	edit->setText("");
					
	edit->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.75f)));
	edit->setSize(UVector2(cegui_reldim(0.8f), cegui_reldim( 0.1f)));
	//text->setProperty("TextColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF0000 br:FFFF0000");
	edit->setAlwaysOnTop(true);	
	
    
    mServerWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
    mServerWindow->setAlpha(0.85f);
    mServerWindow->setSizingEnabled(false);
    mServerWindow->hide();
}

void App::makeOptionWindow(){
          
    mOptionWindow = (FrameWindow*)winMgr->createWindow("SleekSpace/FrameWindow", "wndOption");
    root->addChildWindow(mOptionWindow);
    
    mOptionWindow->setPosition(UVector2(cegui_reldim(0.45f), cegui_reldim( 0.45f)));
    mOptionWindow->setSize(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.35f)));  
    mOptionWindow->setMaxSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 1.0f)));
    mOptionWindow->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f))); 
    mOptionWindow->setText("Options");    
    mOptionWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
    mOptionWindow->setAlpha(0.85f);
    mOptionWindow->setSizingEnabled(false);
    mOptionWindow->hide();
    
      
    DefaultWindow* text = (DefaultWindow *)winMgr->createWindow("SleekSpace/StaticText", "txtSpeedInfo");
    mOptionWindow->addChildWindow(text);
	text->setText("Particle Speed: 1.0");	
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.15f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.2f)));
    
    Slider *slide = (Slider *)(winMgr->createWindow("SleekSpace/Slider", "slideSpeed"));
    mOptionWindow->addChildWindow(slide);
    slide->setText("Connect");
    slide->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.3f)));
    slide->setSize(UVector2(cegui_reldim(0.8f), cegui_reldim( 0.15f)));
    slide->setAlwaysOnTop(true);	
	slide->subscribeEvent(Slider::EventValueChanged, Event::Subscriber(&App::onOptionSliderMoved, this));
	slide->setCurrentValue(fParticleSpeedScale / 10.0f);

    text = (DefaultWindow *)winMgr->createWindow("SleekSpace/StaticText", "txtSizeInfo");
    mOptionWindow->addChildWindow(text);
	text->setText("Particle Size: 1.0");	
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.5f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.2f)));
    
    slide = (Slider *)(winMgr->createWindow("SleekSpace/Slider", "slideSize"));
    mOptionWindow->addChildWindow(slide);
    slide->setText("Size");
    slide->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.65f)));
    slide->setSize(UVector2(cegui_reldim(0.8f), cegui_reldim( 0.15f)));
    slide->setAlwaysOnTop(true);	
    slide->subscribeEvent(Slider::EventValueChanged, Event::Subscriber(&App::onOptionSliderMoved, this));
    slide->setCurrentValue(fParticleSizeScale / 10.0f);
 
	text = (DefaultWindow *)winMgr->createWindow("SleekSpace/StaticText", "txtVersion");
    mOptionWindow->addChildWindow(text);
	text->setText("Version: 1.234");	
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.8f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.2f)));
    
    
}


/*********************************************
	Called as part of render2D**********************************************/
void App::renderGUI(){
	if(fGUITimeout > 0.0f){
		CEGUI::System::getSingleton().renderGUI();
		fGUITimeout -= fTimeScale;
	}
}








/*********************************************
	SDL mouse and key event translation stuff
	Stolen from the CEGUI wiki**********************************************/
bool handle_mouse_down(Uint8 button)
{
	bool ret = false;
	switch ( button )
	{
	// handle real mouse buttons
	case SDL_BUTTON_LEFT:
		ret = CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);
		break;
	case SDL_BUTTON_MIDDLE:
		ret = CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MiddleButton);
		break;
	case SDL_BUTTON_RIGHT:
		ret = CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::RightButton);
		break;
	
	// handle the mouse wheel
	case SDL_BUTTON_WHEELDOWN:
		ret = CEGUI::System::getSingleton().injectMouseWheelChange( -1 );
		break;
	case SDL_BUTTON_WHEELUP:
		ret = CEGUI::System::getSingleton().injectMouseWheelChange( +1 );
		break;
	}
	return ret;
}

bool handle_mouse_up(Uint8 button)
{
	bool ret = false; 
	switch ( button )
	{
	case SDL_BUTTON_LEFT:
		ret = CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::LeftButton);
		break;
	case SDL_BUTTON_MIDDLE:
		ret = CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MiddleButton);
		break;
	case SDL_BUTTON_RIGHT:
		ret = CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::RightButton);
		break;
	}
	
	return ret;
}

/*********************************************
 Translate a SDLKey to the proper CEGUI::Key
*********************************************/
 CEGUI::uint SDLKeyToCEGUIKey(SDLKey key)
 {
     using namespace CEGUI;
     switch (key)
     {
     case SDLK_BACKSPACE:    return Key::Backspace;
     case SDLK_TAB:          return Key::Tab;
     case SDLK_RETURN:       return Key::Return;
     case SDLK_PAUSE:        return Key::Pause;
     case SDLK_ESCAPE:       return Key::Escape;
     case SDLK_SPACE:        return Key::Space;
     case SDLK_COMMA:        return Key::Comma;
     case SDLK_MINUS:        return Key::Minus;
     case SDLK_PERIOD:       return Key::Period;
     case SDLK_SLASH:        return Key::Slash;
     case SDLK_0:            return Key::Zero;
     case SDLK_1:            return Key::One;
     case SDLK_2:            return Key::Two;
     case SDLK_3:            return Key::Three;
     case SDLK_4:            return Key::Four;
     case SDLK_5:            return Key::Five;
     case SDLK_6:            return Key::Six;
     case SDLK_7:            return Key::Seven;
     case SDLK_8:            return Key::Eight;
     case SDLK_9:            return Key::Nine;
     case SDLK_COLON:        return Key::Colon;
     case SDLK_SEMICOLON:    return Key::Semicolon;
     case SDLK_EQUALS:       return Key::Equals;
     case SDLK_LEFTBRACKET:  return Key::LeftBracket;
     case SDLK_BACKSLASH:    return Key::Backslash;
     case SDLK_RIGHTBRACKET: return Key::RightBracket;
     case SDLK_a:            return Key::A;
     case SDLK_b:            return Key::B;
     case SDLK_c:            return Key::C;
     case SDLK_d:            return Key::D;
     case SDLK_e:            return Key::E;
     case SDLK_f:            return Key::F;
     case SDLK_g:            return Key::G;
     case SDLK_h:            return Key::H;
     case SDLK_i:            return Key::I;
     case SDLK_j:            return Key::J;
     case SDLK_k:            return Key::K;
     case SDLK_l:            return Key::L;
     case SDLK_m:            return Key::M;
     case SDLK_n:            return Key::N;
     case SDLK_o:            return Key::O;
     case SDLK_p:            return Key::P;
     case SDLK_q:            return Key::Q;
     case SDLK_r:            return Key::R;
     case SDLK_s:            return Key::S;
     case SDLK_t:            return Key::T;
     case SDLK_u:            return Key::U;
     case SDLK_v:            return Key::V;
     case SDLK_w:            return Key::W;
     case SDLK_x:            return Key::X;
     case SDLK_y:            return Key::Y;
     case SDLK_z:            return Key::Z;
     case SDLK_DELETE:       return Key::Delete;
     case SDLK_KP0:          return Key::Numpad0;
     case SDLK_KP1:          return Key::Numpad1;
     case SDLK_KP2:          return Key::Numpad2;
     case SDLK_KP3:          return Key::Numpad3;
     case SDLK_KP4:          return Key::Numpad4;
     case SDLK_KP5:          return Key::Numpad5;
     case SDLK_KP6:          return Key::Numpad6;
     case SDLK_KP7:          return Key::Numpad7;
     case SDLK_KP8:          return Key::Numpad8;
     case SDLK_KP9:          return Key::Numpad9;
     case SDLK_KP_PERIOD:    return Key::Decimal;
     case SDLK_KP_DIVIDE:    return Key::Divide;
     case SDLK_KP_MULTIPLY:  return Key::Multiply;
     case SDLK_KP_MINUS:     return Key::Subtract;
     case SDLK_KP_PLUS:      return Key::Add;
     case SDLK_KP_ENTER:     return Key::NumpadEnter;
     case SDLK_KP_EQUALS:    return Key::NumpadEquals;
     case SDLK_UP:           return Key::ArrowUp;
     case SDLK_DOWN:         return Key::ArrowDown;
     case SDLK_RIGHT:        return Key::ArrowRight;
     case SDLK_LEFT:         return Key::ArrowLeft;
     case SDLK_INSERT:       return Key::Insert;
     case SDLK_HOME:         return Key::Home;
     case SDLK_END:          return Key::End;
     case SDLK_PAGEUP:       return Key::PageUp;
     case SDLK_PAGEDOWN:     return Key::PageDown;
     case SDLK_F1:           return Key::F1;
     case SDLK_F2:           return Key::F2;
     case SDLK_F3:           return Key::F3;
     case SDLK_F4:           return Key::F4;
     case SDLK_F5:           return Key::F5;
     case SDLK_F6:           return Key::F6;
     case SDLK_F7:           return Key::F7;
     case SDLK_F8:           return Key::F8;
     case SDLK_F9:           return Key::F9;
     case SDLK_F10:          return Key::F10;
     case SDLK_F11:          return Key::F11;
     case SDLK_F12:          return Key::F12;
     case SDLK_F13:          return Key::F13;
     case SDLK_F14:          return Key::F14;
     case SDLK_F15:          return Key::F15;
     case SDLK_NUMLOCK:      return Key::NumLock;
     case SDLK_SCROLLOCK:    return Key::ScrollLock;
     case SDLK_RSHIFT:       return Key::RightShift;
     case SDLK_LSHIFT:       return Key::LeftShift;
     case SDLK_RCTRL:        return Key::RightControl;
     case SDLK_LCTRL:        return Key::LeftControl;
     case SDLK_RALT:         return Key::RightAlt;
     case SDLK_LALT:         return Key::LeftAlt;
     case SDLK_LSUPER:       return Key::LeftWindows;
     case SDLK_RSUPER:       return Key::RightWindows;
     case SDLK_SYSREQ:       return Key::SysRq;
     case SDLK_MENU:         return Key::AppMenu;
     case SDLK_POWER:        return Key::Power;
     default:                return 0;
     }
     return 0;
 }


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

//Globals. Stolen from:
//http://www.cegui.org.uk/wiki/index.php/Using_CEGUI_with_SDL_and_OpenGL
bool handle_mouse_down(Uint8 button);
bool handle_mouse_up(Uint8 button);

float fGUITimeout = GUI_HIDE_DELAY;


/*********************************************
	Called by the SDL event loop to pass
	events off to CEGUI**********************************************/
bool App::processGUIEvent(SDL_Event e){

	//Whenever any even happens, make sure the GUI is shown
	fGUITimeout = GUI_HIDE_DELAY;

	bool handled = false;
	    
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
		CEGUI::System::getSingleton().injectKeyDown(e.key.keysym.scancode);

		// as for the character it's a litte more complicated. we'll use for translated unicode value.
		// this is described in more detail below.
		if (e.key.keysym.unicode != 0){
			CEGUI::System::getSingleton().injectChar(e.key.keysym.unicode);
		}
		break;

	case SDL_KEYUP:
		// like before we inject the scancode directly.
		CEGUI::System::getSingleton().injectKeyUp(e.key.keysym.scancode);
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
	if(senderID == "btnServers") target = NULL;
	else if(senderID == "btnProtocols") target = mProtoWindow;
	else if(senderID == "btnOptions") target = NULL;
	
	//Toggle the window if we found it. TODO: We could do fancy fades with 
	//setAlpha() and such...
	if(target){
		if(target->isVisible()){
			target->hide();
		}else{
			target->show();
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
	Called when we click a close button**********************************************/
bool App::onWndClose(const CEGUI::EventArgs &args){
	//Get the object that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	we->window->hide();
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


/*********************************************
	Called as part of render2D**********************************************/
void App::renderGUI(){
	if(fGUITimeout > 0.0f){
		CEGUI::System::getSingleton().renderGUI();
		fGUITimeout -= fTimeScale;
	}
}








/*********************************************
	SDL mouse event translation stuff**********************************************/
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

#include "CEGUI.h"

#include "main.h"

#include "RendererModules/OpenGLGUIRenderer/openglrenderer.h"
#include "CEGUIDefaultResourceProvider.h"
#include "XMLParserModules/XercesParser/CEGUIXercesParser.h"

using namespace CEGUI;


OpenGLRenderer*mGUI = NULL;
DefaultWindow *root = NULL;
WindowManager *winMgr = NULL;
FrameWindow *mProtoWindow = NULL;

void handle_mouse_down(Uint8 button)
{
switch ( button )
	{
	// handle real mouse buttons
	case SDL_BUTTON_LEFT:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);
		break;
	case SDL_BUTTON_MIDDLE:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MiddleButton);
		break;
	case SDL_BUTTON_RIGHT:
		CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::RightButton);
		break;
	
	// handle the mouse wheel
	case SDL_BUTTON_WHEELDOWN:
		CEGUI::System::getSingleton().injectMouseWheelChange( -1 );
		break;
	case SDL_BUTTON_WHEELUP:
		CEGUI::System::getSingleton().injectMouseWheelChange( +1 );
		break;
	}
}

void handle_mouse_up(Uint8 button)
{
switch ( button )
	{
	case SDL_BUTTON_LEFT:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::LeftButton);
		break;
	case SDL_BUTTON_MIDDLE:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MiddleButton);
		break;
	case SDL_BUTTON_RIGHT:
		CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::RightButton);
		break;
	}
}

void App::processGUIEvent(SDL_Event e){
	    
    switch( e.type ){
				      
	case SDL_MOUSEMOTION:
		// we inject the mouse position directly.
		CEGUI::System::getSingleton().injectMousePosition(
										e.motion.x,e.motion.y );
		break;

		// mouse down handler
	case SDL_MOUSEBUTTONDOWN:
		// let a special function handle the mouse button down event
		handle_mouse_down(e.button.button);
		break;

		// mouse up handler
	case SDL_MOUSEBUTTONUP:
		// let a special function handle the mouse button up event
		handle_mouse_up(e.button.button);
		break;


		// key down
	case SDL_KEYDOWN:
		// to tell CEGUI that a key was pressed, we inject the scancode.
		CEGUI::System::getSingleton().injectKeyDown(e.key.keysym.scancode);

		// as for the character it's a litte more complicated. we'll use for translated unicode value.
		// this is described in more detail below.
		if (e.key.keysym.unicode != 0){
			CEGUI::System::getSingleton().injectChar(e.key.keysym.unicode);
		}
		break;

		// key up
	case SDL_KEYUP:
		// like before we inject the scancode directly.
		CEGUI::System::getSingleton().injectKeyUp(e.key.keysym.scancode);
		break;
	}
}



/*********************************************
				CEGUI**********************************************/
void App::initGUI(){

	//Set some SDL stuff to make the GUI work nicely
	//SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	//Set up the GUI object
	mGUI = new CEGUI::OpenGLRenderer( 0, iScreenX, iScreenY );
	new CEGUI::System( mGUI );
	
	// initialise the required dirs for the DefaultResourceProvider
	CEGUI::DefaultResourceProvider* rp = (CEGUI::DefaultResourceProvider *)CEGUI::System::getSingleton().getResourceProvider();

	//Set the resource groups. This maps a name of a resource group to a type
    CEGUI::Imageset::setDefaultResourceGroup("imagesets");
    CEGUI::Font::setDefaultResourceGroup("fonts");
    CEGUI::Scheme::setDefaultResourceGroup("schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
    CEGUI::WindowManager::setDefaultResourceGroup("layouts");
    CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");

	//Tell the resource provider where things are
	rp->setResourceGroupDirectory("schemes", "./data/gui/schemes/");
	rp->setResourceGroupDirectory("imagesets", "./data/gui/imagesets/");
	rp->setResourceGroupDirectory("fonts", "./data/gui/fonts/");
	rp->setResourceGroupDirectory("layouts", "./data/gui/layouts/");
	rp->setResourceGroupDirectory("looknfeels", "./data/gui/looknfeel/");
	rp->setResourceGroupDirectory("lua_scripts", "./data/gui/lua_scripts/");

	// This is only needed if you are using Xerces and need to
	// specify the schemas location
	rp->setResourceGroupDirectory("schemas", "./data/gui/XMLRefSchema/");
	CEGUI::XercesParser::setSchemaDefaultResourceGroup("schemas");
	
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
    		
	//Create the buttons down the bottom of the screen
	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "Button1"));
    root->addChildWindow(btn);
    btn->setText("Servers");
    btn->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    //btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&Demo4Sample::handleQuit, this));
    btn->setAlwaysOnTop(true);	
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "Button2"));
    root->addChildWindow(btn);
    btn->setText("Protocols");
    btn->setPosition(UVector2(cegui_reldim(0.12f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    //btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&Demo4Sample::handleQuit, this));
    btn->setAlwaysOnTop(true);	
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "Button3"));
    root->addChildWindow(btn);
    btn->setText("Options");
    btn->setPosition(UVector2(cegui_reldim(0.23f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    //btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&Demo4Sample::handleQuit, this));
    btn->setAlwaysOnTop(true);	
		
	makeProtocolWindow();

}

void App::addProtocolEntry(string name, Color col, int index){
	Checkbox* cb = (Checkbox *)winMgr->createWindow("SleekSpace/Checkbox", "TextWindow/CB" + toString(index));
	mProtoWindow->addChildWindow(cb);
	
	float ypos = (index / 2) * 0.08f;
	float xpos = (index % 2) * 0.4f;
	
	ypos += 0.1;
	xpos += 0.1;
	
	cb->setPosition(UVector2(cegui_reldim( xpos ), cegui_reldim( ypos )));
	cb->setSize(UVector2(cegui_reldim(0.35f), cegui_reldim( 0.05f)));
	cb->setText(name.c_str());
	cb->setProperty("NormalTextColour", col.toString()); 
	
	//cb->subscribeEvent(Checkbox::EventCheckStateChanged, &formatChangedHandler);
}

void App::makeProtocolWindow(){
	          
    mProtoWindow = (FrameWindow*)winMgr->createWindow("SleekSpace/FrameWindow", "Demo Window");
    root->addChildWindow(mProtoWindow);
    
    mProtoWindow->setAlpha(0.95f);
    mProtoWindow->setPosition(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.25f)));
    mProtoWindow->setSize(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.5f)));  
    mProtoWindow->setMaxSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 1.0f)));
    mProtoWindow->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f))); 

    mProtoWindow->setText("Protocols");
    
    //for(int i=0;i<10;i++){   
	//	addProtocolEntry("protocol" + toString(i), Color(randFloat(0,1), randFloat(0,1), randFloat(0,1)), i);
   	//}
   	
   	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "HideAll"));
    mProtoWindow->addChildWindow(btn);
    btn->setText("Hide All");
    btn->setPosition(UVector2(cegui_reldim(0.06f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    //btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&Demo4Sample::handleQuit, this));
    btn->setAlwaysOnTop(true);	
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpace/Button", "ShowAll"));
    mProtoWindow->addChildWindow(btn);
    btn->setText("Show All");
    btn->setPosition(UVector2(cegui_reldim(0.54f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    //btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&Demo4Sample::handleQuit, this));
    btn->setAlwaysOnTop(true);	
}


void App::renderGUI(){
	CEGUI::System::getSingleton().renderGUI();
}

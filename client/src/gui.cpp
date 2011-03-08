/* 
 * This file is part of BSOD client
 *
 * Copyright (c) 2011 The University of Waikato, Hamilton, New Zealand.
 *
 * Author: Paul Hunkin
 *
 * Contributors: Shane Alcock
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND research
 * group. For further information please see http://www.wand.net.nz/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */


#include "config.h"
#include "main.h"

#ifdef ENABLE_GUI

//There are some nasty issues with this file pulling in gl.h and conflicting
//with glew.h, so we have it #included here instead of in libs.h




#if HAVE_CEGUI_07
	#ifdef __APPLE__
		#include "CEGUIBase/RendererModules/OpenGL/CEGUIOpenGLRenderer.h"
	#else
		#include "CEGUI/RendererModules/OpenGL/CEGUIOpenGLRenderer.h"
	#endif
#else
	#ifdef __APPLE__
		#include "CEGUIBase/RendererModules/OpenGLGUIRenderer/openglrenderer.h"
	#else
		#include "CEGUI/RendererModules/OpenGLGUIRenderer/openglrenderer.h"
	#endif
#endif

//These are alo apparently not included (properly) by CEGUI.h
#ifdef __APPLE__
	#include "CEGUIBase/CEGUIDefaultResourceProvider.h"
#else
	#include "CEGUI/CEGUIDefaultResourceProvider.h"
#endif
//#include "XMLParserModules/XercesParser/CEGUIXercesParser.h"

//Only for this file...
using namespace CEGUI;


/*********************************************
	CEGUI components
**********************************************/
OpenGLRenderer *mGUI = NULL;
DefaultWindow *root = NULL;
WindowManager *winMgr = NULL;
FrameWindow *mProtoWindow = NULL;
FrameWindow *mServerWindow = NULL;
FrameWindow *mOptionWindow = NULL;
FrameWindow *mMessageWindow = NULL;
FrameWindow *mDisconnectWindow = NULL;

//Globals. Stolen from:
//http://www.cegui.org.uk/wiki/index.php/Using_CEGUI_with_SDL_and_OpenGL
bool handle_mouse_down(Uint8 button);
bool handle_mouse_up(Uint8 button);
CEGUI::uint SDLKeyToCEGUIKey(SDLKey key);

bool bGuiIsShown = false;
bool bGlobalGuiEnable = true; //Turn all GUI stuff on and off


/*********************************************
	Server list stuff
**********************************************/
class ServerInfo{
public:
	string name;
	string port;
	string ip;
};

vector<ServerInfo> mServerInfoExplicit;
vector<ServerInfo> mServerInfoDisc;


/*********************************************
		CEGUI setup - create the UI
**********************************************/
void App::initGUI(){

	if(!bGlobalGuiEnable) return;
	
	fGUITimeout = 0.0f; //Start hidden

	//Set some SDL stuff to make the GUI work nicely
	//SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	//Set up the GUI object

#if HAVE_CEGUI_07
	mGUI = &(CEGUI::OpenGLRenderer::bootstrapSystem());
#else
	mGUI = new CEGUI::OpenGLRenderer( 0, iScreenX, iScreenY );
	new CEGUI::System( mGUI );
#endif	
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

#if HAVE_CEGUI_07
	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
		parser->setProperty("SchemaDefaultResourceGroup", "schemas");
#endif
	
	//CEGUI::XercesParser::setSchemaDefaultResourceGroup("schemas");
	
	//Load Arial to make sure we have at least one font
#if HAVE_CEGUI_07
	if(!CEGUI::FontManager::getSingleton().isDefined( "arial" ) )
		CEGUI::FontManager::getSingleton().create( "arial.font" );

	//Load in the themee file
	CEGUI::SchemeManager::getSingleton().create( "SleekSpaceBSOD.scheme" );
#else	

	if(!CEGUI::FontManager::getSingleton().isFontPresent( "arial" ) )
		CEGUI::FontManager::getSingleton().createFont( "arial.font" );

	//Load in the themee file
	CEGUI::SchemeManager::getSingleton().loadScheme( "SleekSpaceBSOD.scheme" );
#endif	
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
	
	//And the messagebox window
	makeMessageWindow();
   	
	//And the disconnected window
   	makeDisconnectedWindow(); 
    
		
		
	//Set the mouse cursor event.
	CEGUI::MouseCursor::getSingleton().subscribeEvent(
						MouseCursor::EventImageChanged, 
						Event::Subscriber(&App::onMouseCursorChanged, this) );
}


/*********************************************
	Called by the SDL event loop to pass
	events off to CEGUI
**********************************************/
bool App::processGUIEvent(SDL_Event e){

	if(!bGlobalGuiEnable) return false;
	
	bool handled = false;
	bool show = false;
	CEGUI::uint kc = 0;
	    
    switch( e.type ){
				      
	case SDL_MOUSEMOTION:
		// we inject the mouse position directly.
		CEGUI::System::getSingleton().injectMousePosition(
										e.motion.x,e.motion.y );
		show = true;
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

		// Use unicode value here if possible
		if (e.key.keysym.unicode != 0){
			CEGUI::System::getSingleton().injectChar(e.key.keysym.unicode);
		}
		
		show = true;
				
		break;

	case SDL_KEYUP:
		// like before we inject the scancode directly.
		kc = SDLKeyToCEGUIKey(e.key.keysym.sym);
		CEGUI::System::getSingleton().injectKeyUp(kc);
		break;
		
	case SDL_VIDEORESIZE:
		mGUI->grabTextures();
		fGUITimeout = 0.0f; //We don't unhide for this
		break;
		
	}
	
	if(show || handled){
		//Whenever any even happens, make sure the GUI is shown
		fGUITimeout = GUI_HIDE_DELAY;
	}
				
	return handled;
}

/*********************************************
	Called when we click a menu toggle
**********************************************/
bool App::onMenuButtonClicked(const EventArgs &args){

	if(!bGlobalGuiEnable) return false;
	
	//Get the control that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	String senderID = we->window->getName();
	FrameWindow *target = NULL;
	
	//From that we can figure out which window we want to toggle
	if(senderID == "btnServers") target = mServerWindow;
	else if(senderID == "btnProtocols") target = mProtoWindow;
	else if(senderID == "btnOptions") target = mOptionWindow;
	else if(senderID == "btnMessageOK") target = mMessageWindow;
	else if(senderID == "btnReconnectCancel") target = mDisconnectWindow;
	
	//Toggle the window if we found it. TODO: We could do fancy fades with 
	//setAlpha() and such...
	if(target){
		if(target->isVisible()){
			target->hide();
		}else{
			target->show();
			target->moveToFront();
			
			if(target == mServerWindow){
				renderServerList();
				beginDiscovery();
			}
		}
	}else{
		LOG("Bad target from button '%s' in onMenuButtonClicked\n", 
			senderID.c_str());	
	}
	return true;
}


/*********************************************
	Called when we click a protocol toggle
**********************************************/
bool App::onProtocolClicked(const EventArgs &args){
	
	if(!bGlobalGuiEnable) return false;

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
	
		//This is annoying. Look how many hoops we have to jump through. 
		//TODO: We should have a *single* showType() / hideType() or something
		getFD(id)->bShown = cb->isSelected();
		ps()->showColor(getFD(id)->mColor, cb->isSelected());		
		mFlowMgr->showType(getFD(id), cb->isSelected());
	}
	
	return true;	
}

/*********************************************
	Called when we click a protocol button
**********************************************/
bool App::onProtocolButtonClicked(const EventArgs &args){
	
	if(!bGlobalGuiEnable) return false;
	
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
		
		//Finally, set the flow manager toggle
		mFlowMgr->showType(getFD(i), show);
	}
		
	return true;
}

/*********************************************
			Pops up a dialog box
**********************************************/
void App::messagebox(string text, string title){

	if(!bGlobalGuiEnable) return;
	
	DefaultWindow* t = (DefaultWindow *)winMgr->getWindow("txtMessageBox");  	
	t->setText(text);
	
	mMessageWindow->setText(title);
		
    mMessageWindow->setPosition(UVector2(cegui_reldim(0.25f), 
    							cegui_reldim( 0.33f)));
	mMessageWindow->show();
	mMessageWindow->moveToFront();

}

void App::disconnectbox(string text, string title) {
	if (!bGlobalGuiEnable) return;

	DefaultWindow* t = (DefaultWindow *)winMgr->getWindow("txtDisconnectBox");  	
	t->setText(text);
	mDisconnectWindow->setText(title);
	mDisconnectWindow->setPosition(UVector2(cegui_reldim(0.25f),
			cegui_reldim( 0.33f)));
	mDisconnectWindow->show();
	mDisconnectWindow->moveToFront();
}

bool App::onReconnectButtonClicked(const EventArgs &args) {
	if(!bGlobalGuiEnable) return false;

	mDisconnectWindow->hide();

	if (!openSocket(false)) {
		/* If we failed to reconnect, pop the window back up */
		mDisconnectWindow->show();	
	}

	return true;

}

bool App::inExplicitList(string name, string port) {

	vector<ServerInfo>::iterator it;

	for (it = mServerInfoExplicit.begin(); it != mServerInfoExplicit.end();
			it ++) {
		if (((*it).name == name || (*it).ip == name) 
				&& (*it).port == port)
			return true;
	}

	return false;
}

void App::renderServerList() {

	vector<ServerInfo>::iterator it;
	Listbox *lb = (Listbox *)winMgr->getWindow("lbServers");

	lb->resetList();

	for (it = mServerInfoExplicit.begin(); it != mServerInfoExplicit.end();
			it ++) {

		lb->addItem(new ListboxTextItem((*it).name));

	}
	
}

/*********************************************
	Called when we click a server button
**********************************************/
bool App::onServerButtonClicked(const EventArgs &args){

	if(!bGlobalGuiEnable) return false;
	
	//Get the object that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	String senderID = we->window->getName();
	
	if(senderID == "btnRefresh"){
		renderServerList();
		beginDiscovery();
	}else if(senderID == "btnConnect" || senderID == "txtCustomServer"){
		Editbox *eb = (Editbox *)winMgr->getWindow("txtCustomServer");
		
		string text = toString(eb->getText());
		
		if(text.find(":") != string::npos){
		
			//Pull out the port and IP parts
			vector<string> split;
			splitString(text, ":", split);
		
			if(split.size() != 2){
				LOG("Bad entry in txtCustomServer!\n");
				return true;
			}
			
			//Set the config options. Probably should pass these to openSocket
			mServerAddr = split[0];				
			iServerPort = stringTo<int>(split[1]);	
			
			if(iServerPort <= 0){
				LOG("Bad port!\n");
				return true;
			}			
		}else{
			mServerAddr = text;
			iServerPort = DEFAULT_PORT;
		}
		
		bool toAdd = false;
		if (!inExplicitList(mServerAddr, toString(iServerPort)))
			toAdd = true;			
		
		if(openSocket(toAdd)){
			mServerWindow->hide();
		}else{
			messagebox("Could not connect to server '" + 
						mServerAddr + ":" + 
						toString(iServerPort) + "'", 
						"Connection Failed");

						
		}
		
	}else{	
		//Rogue button?
		return true;
	}
	
	return true;
}

/*********************************************
  Called when we click a server list entry
**********************************************/
bool App::onServerListClicked(const EventArgs &args){
	
	if(!bGlobalGuiEnable) return false;
	
	Editbox *eb = (Editbox *)winMgr->getWindow("txtCustomServer");
	Listbox *lb = (Listbox *)((WindowEventArgs *)&args)->window;
	ServerInfo *info = NULL;
		
	if(lb->getSelectedCount() <= 0){
		return true;
	}
	
	int index = lb->getItemIndex(lb->getFirstSelectedItem());
		
	//Sanity check
	if(index >= mServerInfoExplicit.size() + mServerInfoDisc.size() 
				|| index < 0){
		return true;
	}
	
	if (index < mServerInfoExplicit.size())
		info = &mServerInfoExplicit[index];
	else
		info = &mServerInfoDisc[index - mServerInfoExplicit.size()];
	
	eb->setText(info->ip + ":" + info->port);
		
	return true;
}

/*********************************************
	Called when we click a close button
**********************************************/
bool App::onWndClose(const CEGUI::EventArgs &args){
	
	if(!bGlobalGuiEnable) return false;

	//Get the object that sent this event
	WindowEventArgs *we = (WindowEventArgs *)&args;
	we->window->hide();
	
	return true;
}

bool App::onMouseCursorChanged(const CEGUI::EventArgs&){

	if(!bGlobalGuiEnable) return false;

	//If CEGUI has changed to a valid image, hide the SDL cursor. This ensures
	//that we do sensible things when we mouse over edit boxes and such, 
	//otherwise we have duplicate cursors. 
#ifndef _WINDOWS
#ifndef ENABLE_CGL_COMPAT
	if(CEGUI::MouseCursor::getSingleton().getImage()){
		SDL_ShowCursor(SDL_DISABLE);
	}else{
		SDL_ShowCursor(SDL_ENABLE);
	}
#endif
#endif
	
	return true;
}

bool App::onOptionSliderMoved(const CEGUI::EventArgs& args){
	
	if(!bGlobalGuiEnable) return false;

	WindowEventArgs *we = (WindowEventArgs *)&args;
	Slider *slide = (Slider *)we->window;
	
	float val = (slide->getCurrentValue() * 10.0f);
	
	if(we->window->getName() == "slideSpeed"){	
		fParticleSpeedScale = val;
		DefaultWindow *text = (DefaultWindow *)
								winMgr->getWindow("txtSpeedInfo");
		text->setText("Particle Speed: " + toString(val));
	}else{
		fParticleSizeScale = val;
		DefaultWindow *text = (DefaultWindow *)
								winMgr->getWindow("txtSizeInfo");
		text->setText("Particle Size: " + toString(val));
	}

	return true;
}

bool App::onDarknetCheckboxClicked(const CEGUI::EventArgs &args){

	if(!bGlobalGuiEnable) return false;

	WindowEventArgs *we = (WindowEventArgs *)&args;
	Checkbox *cb = (Checkbox *)we->window;
	
	if(cb->getName() == "cbDarknet"){
		bShowDarknet = cb->isSelected();
	}
	
	else if(cb->getName() == "cbNonDarknet"){
		bShowNonDarknet = cb->isSelected();
	}
	
	return true;
}

void App::shutdownGUI(){

	if(!bGlobalGuiEnable) return;

#if HAVE_CEGUI_07
	CEGUI::OpenGLRenderer::destroySystem();
#else	
	delete CEGUI::System::getSingletonPtr (); 
	delete mGUI;
#endif
}

void App::resizeGUI(int x, int y){

	if(!bGlobalGuiEnable) return;

	mGUI->restoreTextures();
    mGUI->setDisplaySize(CEGUI::Size(x, y));
}



/*********************************************
  Adds a named checkbox to the protocol wnd
**********************************************/
void App::addProtocolEntry(string name, Color col, int index){

	if(!bGlobalGuiEnable) return;

	if(!mProtoWindow){
		return;
	}

	Checkbox* cb = (Checkbox *)winMgr->createWindow("SleekSpaceBSOD/Checkbox", 
													toString(index) );											
	winMgr->getWindow("paneProto")->addChildWindow(cb);
	
	float ypos = (index / 2) * 0.1f;
	float xpos = (index % 2) * 0.5f;

#if HAVE_CEGUI_07

#else
	ypos += 0.1;
	xpos += 0.1;
#endif	
	cb->setPosition(UVector2(cegui_reldim( xpos ), cegui_reldim( ypos )));
	cb->setSize(UVector2(cegui_reldim(0.45f), cegui_reldim( 0.08f)));
	cb->setText(name.c_str());
	cb->setProperty("SelectedTextColour", col.toString()); 
	cb->setSelected(true);
	
	cb->subscribeEvent(Checkbox::EventCheckStateChanged, 
						Event::Subscriber(&App::onProtocolClicked, this) );
}

void App::clearProtocolEntries(){

	if(!bGlobalGuiEnable) return;

	for(int i=0;i<255;i++){		
		if(winMgr->isWindowPresent(toString(i))){
			winMgr->destroyWindow(toString(i));
		}	
	}

}

void App::addExplicitServerListEntry(string name, string IP, string port) {

	if(!bGlobalGuiEnable) return;
	
	ServerInfo info;
	info.name = name;
	info.ip = IP;
	info.port = port;

	mServerInfoExplicit.push_back(info);

}

void App::addDiscServerListEntry(string name, string IP, string port){
	
	if(!bGlobalGuiEnable) return;
	
	ServerInfo info;
	info.name = name;
	info.ip = IP;
	info.port = port;

	mServerInfoDisc.push_back(info);

	//Listbox* lb = (Listbox *)winMgr->getWindow("lbServers");  	
    	//lb->addItem(new ListboxTextItem(name));
}

void App::clearServerList(){
	Listbox* lb = (Listbox *)winMgr->getWindow("lbServers");  
	lb->resetList();
	
	Editbox *eb = (Editbox *)winMgr->getWindow("txtCustomServer");
	eb->setText("");
	
	mServerInfoDisc.clear();
	renderServerList();
}

void App::updateGUIConnectionStatus(){
	
	if(!bGlobalGuiEnable) return;
	
	DefaultWindow* text = (DefaultWindow *)winMgr->getWindow("txtServerInfo");  	

	if(isConnected()){
		text->setText("Currently connected to '" + mServerAddr + ":" + 
					  toString(iServerPort) + "'. " + 
					  "Select a server from the list\
					  below or specify a custom address manually.");
	}else{
	
		text->setText("Not currently connected to any server. \
					Select a different server from the list below or specify \
					a custom address manually.");
	}
}




/*********************************************
	Called as part of render2D
**********************************************/
void App::renderGUI(){
	
	if(!bGlobalGuiEnable) return;
	
	if(fGUITimeout > 0.0f){

		if(!bGuiIsShown){
			SDL_ShowCursor(SDL_ENABLE);
			bGuiIsShown = true;
		}

		CEGUI::System::getSingleton().renderGUI();
		fGUITimeout -= fTimeScale;
	}else if(bGuiIsShown){
		SDL_ShowCursor(SDL_DISABLE);
		bGuiIsShown = false;
	}
}

















/*********************************************
	  Make the buttons down the bottom
	  of the screen.
**********************************************/
void App::makeMenuButtons(){
 		
	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnServers"));
    root->addChildWindow(btn);
    btn->setText("Servers");
    btn->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
     
    btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnProtocols"));
    root->addChildWindow(btn);
    btn->setText("Protocols");
    btn->setPosition(UVector2(cegui_reldim(0.12f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnOptions"));
    root->addChildWindow(btn);
    btn->setText("Options");
    btn->setPosition(UVector2(cegui_reldim(0.23f), cegui_reldim( 0.95f)));
    btn->setSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.036f)));
    btn->setAlpha(0.9f);
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
}





/*********************************************
	Creates the protocol toggle window
**********************************************/
void App::makeProtocolWindow(){
	          
    mProtoWindow = (FrameWindow*)winMgr->createWindow("SleekSpaceBSOD/FrameWindow", "wndProtocol");
    root->addChildWindow(mProtoWindow);
    
    mProtoWindow->setPosition(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.25f)));
#if HAVE_CEGUI_07
    mProtoWindow->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.4f)));  
#else
    mProtoWindow->setSize(UVector2(cegui_reldim(0.5f), cegui_reldim( 0.5f)));  
#endif
    mProtoWindow->setMaxSize(UVector2(cegui_reldim(1.5f), cegui_reldim( 1.0f)));
    mProtoWindow->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f))); 

    mProtoWindow->setText("Protocols");
    
    ScrollablePane *pane = (ScrollablePane*)winMgr->createWindow("SleekSpaceBSOD/ScrollablePane", "paneProto");
    mProtoWindow->addChildWindow(pane);
#if HAVE_CEGUI_07
    pane->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.05f)));
    pane->setSize(UVector2(cegui_reldim(0.9f), cegui_reldim( 0.85f)));  
#else 
    pane->setPosition(UVector2(cegui_reldim(0.075f), cegui_reldim( 0.12f)));
    pane->setSize(UVector2(cegui_reldim(0.87f), cegui_reldim( 0.75f)));  
#endif
    //for(int i=0;i<30;i++){   
	//	addProtocolEntry("protocol" + toString(i), Color(randFloat(0,1), randFloat(0,1), randFloat(0,1)), i);
   	//}
   	
   	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnHideAll"));
    mProtoWindow->addChildWindow(btn);
    btn->setText("Hide All");
#if HAVE_CEGUI_07
    btn->setPosition(UVector2(cegui_reldim(0.06f), cegui_reldim( 0.90f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.1f)));
#else    
    btn->setPosition(UVector2(cegui_reldim(0.06f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
#endif
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onProtocolButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnShowAll"));
    mProtoWindow->addChildWindow(btn);
    btn->setText("Show All");
#if HAVE_CEGUI_07
    btn->setPosition(UVector2(cegui_reldim(0.54f), cegui_reldim( 0.90f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.1f)));
#else    
    btn->setPosition(UVector2(cegui_reldim(0.54f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
#endif
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onProtocolButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    mProtoWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
    mProtoWindow->setAlpha(0.85f);
    mProtoWindow->setSizingEnabled(false);
    mProtoWindow->hide();
}


/*********************************************
	Creates the message box window
**********************************************/
void App::makeMessageWindow(){
	mMessageWindow = (FrameWindow*)winMgr->createWindow("SleekSpaceBSOD/FrameWindow", "wndMessage");
    root->addChildWindow(mMessageWindow);
    
    mMessageWindow->setPosition(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.33f)));
    mMessageWindow->setSize(UVector2(cegui_reldim(0.5f), cegui_reldim( 0.25f)));  
    mMessageWindow->setText("Message");
    	
   	DefaultWindow* text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtMessageBox");
    mMessageWindow->addChildWindow(text);
	text->setText("This is an informational message. It is very long and annoying, and does nothing useful.");
					
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.08f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.7f)));
	//text->setProperty("TextColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF0000 br:FFFF0000");
	text->setProperty("HorzFormatting", "WordWrapLeftAligned"); // LeftAligned, RightAligned, HorzCentred
				// HorzJustified, WordWrapLeftAligned, WordWrapRightAligned, WordWrapCentred, WordWrapJustified
				
    
    mMessageWindow->hide();	
    
    PushButton *btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnMessageOK"));
    mMessageWindow->addChildWindow(btn);
    btn->setText("OK");
    btn->setPosition(UVector2(cegui_reldim(0.3f), cegui_reldim( 0.7f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.2f)));
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
    btn->setAlwaysOnTop(true);
    
    mMessageWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
    mMessageWindow->setAlpha(0.85f);
    mMessageWindow->setSizingEnabled(false);
}

void App::makeDisconnectedWindow() {
	mDisconnectWindow = (FrameWindow *)winMgr->createWindow("SleekSpaceBSOD/FrameWindow", "wndDisconnect");
	root->addChildWindow(mDisconnectWindow);
	mDisconnectWindow->setPosition(UVector2(cegui_reldim(0.25f), cegui_reldim( 0.33f)));
	mDisconnectWindow->setSize(UVector2(cegui_reldim(0.5f), cegui_reldim( 0.25f)));
	mDisconnectWindow->setText("Disconnected");

	DefaultWindow *text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtDisconnectBox");
	mDisconnectWindow->addChildWindow(text);
	text->setText("This is a special window for handling server disconnections\n");
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.08f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.7f)));
	text->setProperty("HorzFormatting", "WordWrapLeftAligned");

	mDisconnectWindow->hide();

	PushButton *recbtn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnReconnect"));
	mDisconnectWindow->addChildWindow(recbtn);
	recbtn->setText("Reconnect");
	recbtn->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.7f)));
	recbtn->setSize(UVector2(cegui_reldim(0.3f), cegui_reldim( 0.2f)));
	recbtn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onReconnectButtonClicked, this));
	recbtn->setAlwaysOnTop(true);

	PushButton *canbtn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnReconnectCancel"));
	mDisconnectWindow->addChildWindow(canbtn);
	canbtn->setText("Cancel");
	canbtn->setPosition(UVector2(cegui_reldim(0.6f), cegui_reldim( 0.7f)));		canbtn->setSize(UVector2(cegui_reldim(0.3f), cegui_reldim( 0.2f)));
	canbtn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onMenuButtonClicked, this));
	canbtn->setAlwaysOnTop(true);

	mDisconnectWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
	mDisconnectWindow->setAlpha(0.85f);
	mDisconnectWindow->setSizingEnabled(false);
}

/*********************************************
	Creates the server browser window
**********************************************/
void App::makeServerWindow(){
	mServerWindow = (FrameWindow *)winMgr->createWindow("SleekSpaceBSOD/FrameWindow", "wndServer");
    root->addChildWindow(mServerWindow);
    
    mServerWindow->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.3f)));
    mServerWindow->setSize(UVector2(cegui_reldim(0.5f), cegui_reldim( 0.5f)));  
    mServerWindow->setMaxSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 1.0f)));
    mServerWindow->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f))); 

    mServerWindow->setText("Servers");
    
    //for(int i=0;i<10;i++){   
	//	addProtocolEntry("protocol" + toString(i), Color(randFloat(0,1), randFloat(0,1), randFloat(0,1)), i);
   	//}
   	
   	PushButton* btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnRefresh"));
    mServerWindow->addChildWindow(btn);
    btn->setText("Refresh List");
    btn->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onServerButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    btn = (PushButton *)(winMgr->createWindow("SleekSpaceBSOD/Button", "btnConnect"));
    mServerWindow->addChildWindow(btn);
    btn->setText("Connect");
    btn->setPosition(UVector2(cegui_reldim(0.55f), cegui_reldim( 0.88f)));
    btn->setSize(UVector2(cegui_reldim(0.4f), cegui_reldim( 0.08f)));
    btn->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&App::onServerButtonClicked, this));
    btn->setAlwaysOnTop(true);	
    
    
   	Listbox* lb = (Listbox *)(winMgr->createWindow("SleekSpaceBSOD/Listbox", "lbServers"));
   	mServerWindow->addChildWindow(lb);
   	lb->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.28f)));
    lb->setSize(UVector2(cegui_reldim(0.9f), cegui_reldim( 0.45f)));
   
    lb->subscribeEvent(Listbox::EventSelectionChanged, Event::Subscriber(&App::onServerListClicked, this));
    
    
    DefaultWindow* text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtServerInfo");
    mServerWindow->addChildWindow(text);
	text->setText("Not currently connected to any server. \
					Select a server from the list below or specify \
					a custom address manually.");
					
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.08f)));
	text->setSize(UVector2(cegui_reldim(0.90f), cegui_reldim( 0.2f)));
	//text->setProperty("TextColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF0000 br:FFFF0000");
	text->setAlwaysOnTop(true);	
	text->setProperty("HorzFormatting", "WordWrapLeftAligned"); // LeftAligned, RightAligned, HorzCentred
				// HorzJustified, WordWrapLeftAligned, WordWrapRightAligned, WordWrapCentred, WordWrapJustified
				
				
				
	text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtSelected");
    mServerWindow->addChildWindow(text);
	text->setText("Address: ");
					
	text->setPosition(UVector2(cegui_reldim(0.05f), cegui_reldim( 0.70f)));
	text->setSize(UVector2(cegui_reldim(0.2f), cegui_reldim( 0.2f)));
	//text->setProperty("TextColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF0000 br:FFFF0000");
	text->setAlwaysOnTop(true);	
			
				
				
	Editbox* edit = (Editbox *)winMgr->createWindow("SleekSpaceBSOD/Editbox", "txtCustomServer");
    mServerWindow->addChildWindow(edit);
	edit->setText("");
					
	edit->setPosition(UVector2(cegui_reldim(0.2f), cegui_reldim( 0.75f)));
	edit->setSize(UVector2(cegui_reldim(0.75f), cegui_reldim( 0.1f)));
	//text->setProperty("TextColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF0000 br:FFFF0000");
	edit->setAlwaysOnTop(true);	
	edit->subscribeEvent(Editbox::EventTextAccepted, Event::Subscriber(&App::onServerButtonClicked, this));
    
    mServerWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
    mServerWindow->setAlpha(0.85f);
    mServerWindow->setSizingEnabled(false);
    mServerWindow->hide();
}



/*********************************************
	Creates the options window
**********************************************/
void App::makeOptionWindow(){
#if HAVE_CEGUI_07
	float opt_shift = 0.05;
#else
	float opt_shift = 0.0;
#endif
          
    mOptionWindow = (FrameWindow*)winMgr->createWindow("SleekSpaceBSOD/FrameWindow", "wndOption");
    root->addChildWindow(mOptionWindow);
    
    mOptionWindow->setPosition(UVector2(cegui_reldim(0.45f), cegui_reldim( 0.45f)));
    mOptionWindow->setSize(UVector2(cegui_reldim(0.30f), cegui_reldim( 0.5f)));  
    mOptionWindow->setMaxSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 1.0f)));
    mOptionWindow->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f))); 
    mOptionWindow->setText("Options");    
    mOptionWindow->subscribeEvent(FrameWindow::EventCloseClicked, Event::Subscriber(&App::onWndClose, this));
    mOptionWindow->setAlpha(0.85f);
    mOptionWindow->setSizingEnabled(false);
    mOptionWindow->hide();
    
      
    DefaultWindow* text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtSpeedInfo");
    mOptionWindow->addChildWindow(text);
	text->setText("Particle Speed: 1.0");	
	text->setPosition(UVector2(cegui_reldim(0.05f - opt_shift), cegui_reldim( 0.10f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.1f)));
    
    Slider *slide = (Slider *)(winMgr->createWindow("SleekSpaceBSOD/Slider", "slideSpeed"));
    mOptionWindow->addChildWindow(slide);
    slide->setText("Connect");
    slide->setPosition(UVector2(cegui_reldim(0.1f - opt_shift), cegui_reldim( 0.17f)));
    slide->setSize(UVector2(cegui_reldim(0.8f), cegui_reldim( 0.10f)));
    slide->setAlwaysOnTop(true);	
	slide->subscribeEvent(Slider::EventValueChanged, Event::Subscriber(&App::onOptionSliderMoved, this));
	slide->setCurrentValue(fParticleSpeedScale / 10.0f);

    text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtSizeInfo");
    mOptionWindow->addChildWindow(text);
	text->setText("Particle Size: 1.0");	
	text->setPosition(UVector2(cegui_reldim(0.05f - opt_shift), cegui_reldim( 0.25f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.2f)));
    
    slide = (Slider *)(winMgr->createWindow("SleekSpaceBSOD/Slider", "slideSize"));
    mOptionWindow->addChildWindow(slide);
    slide->setText("Size");
    slide->setPosition(UVector2(cegui_reldim(0.1f - opt_shift), cegui_reldim( 0.37f)));
    slide->setSize(UVector2(cegui_reldim(0.8f), cegui_reldim( 0.10f)));
    slide->setAlwaysOnTop(true);	
    slide->subscribeEvent(Slider::EventValueChanged, Event::Subscriber(&App::onOptionSliderMoved, this));
    slide->setCurrentValue(fParticleSizeScale / 10.0f);
    
    text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtDarkInfo");
    mOptionWindow->addChildWindow(text);
	text->setText("Darknet:");	
	text->setPosition(UVector2(cegui_reldim(0.05f - opt_shift), cegui_reldim( 0.5f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.1f)));
  
  	
  	
	Checkbox* cb = (Checkbox *)winMgr->createWindow("SleekSpaceBSOD/Checkbox", "cbDarknet");
	mOptionWindow->addChildWindow(cb);	
	cb->setPosition(UVector2(cegui_reldim( 0.1f  - opt_shift), cegui_reldim( 0.60f )));
	cb->setSize(UVector2(cegui_reldim(0.9f), cegui_reldim( 0.05f)));
	cb->setText("Show Darknet traffic");
	cb->setSelected(bShowDarknet);
	
	cb->subscribeEvent(Checkbox::EventCheckStateChanged, 
						Event::Subscriber(&App::onDarknetCheckboxClicked, this) );
						
						
	cb = (Checkbox *)winMgr->createWindow("SleekSpaceBSOD/Checkbox", "cbNonDarknet" );
	mOptionWindow->addChildWindow(cb);	
	cb->setPosition(UVector2(cegui_reldim( 0.1f  - opt_shift), cegui_reldim( 0.67f )));
	cb->setSize(UVector2(cegui_reldim(0.9f), cegui_reldim( 0.05f)));
	cb->setText("Show Non-darknet traffic");
	cb->setSelected(bShowNonDarknet);
	
	cb->subscribeEvent(Checkbox::EventCheckStateChanged, 
						Event::Subscriber(&App::onDarknetCheckboxClicked, this) );
  	
  	
 
	text = (DefaultWindow *)winMgr->createWindow("SleekSpaceBSOD/StaticText", "txtVersion");
    mOptionWindow->addChildWindow(text);
	text->setText("BSOD Client - version v" + toString(CLIENT_VERSION) + "\nBuilt " + __DATE__ + " at " + __TIME__);	
	text->setPosition(UVector2(cegui_reldim(0.05f - opt_shift), cegui_reldim( 0.75f)));
	text->setSize(UVector2(cegui_reldim(0.95f), cegui_reldim( 0.3f)));
    
    
}





/*********************************************
	SDL mouse and key event translation stuff
	Stolen from the CEGUI wiki
**********************************************/
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

#endif

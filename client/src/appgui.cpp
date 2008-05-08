#include "main.h"
#include "appgui.h"

vector<ModuleOption *> mOptions;
vector<ProtoOption *> mProtoOptions;

/*********************************************
		Handles the options button
**********************************************/
void handler(UIEvent evt, int eventData, void *userData){

	UIWindow *wnd = NULL;

	if(userData == 0){
		wnd = App::S()->wndOptions;
	}else{
		wnd = App::S()->wndProto;
	}

	bool hide = !wnd->isHidden();
	
	if(hide){
		wnd->setState(CLOSING);
	}else{
		wnd->setState(OPENING);		
		wnd->hide(false);
	}
	
}

/*********************************************
		Handles all option button presses
**********************************************/
void onModuleOptionPress(UIEvent evt, int eventData, void *userData){
	ModuleOption *opt = (ModuleOption *)userData;
	
	opt->enabled = !opt->enabled;
	
	if(opt->btn)
		opt->btn->setData(opt->enabled ? "true" : "false");			
	
	App::S()->setOption(opt->index, opt->enabled);
}

void onProtoOptionPress(UIEvent evt, int eventData, void *userData){
	ProtoOption *opt = (ProtoOption *)userData;	
	opt->shown = !opt->shown;	
	
	if(opt->btn)	
		opt->btn->setData(opt->shown ? "true" : "false");			
		
	opt->proto->bShown = opt->shown;	

	if(!opt->proto->bShown)	
		App::S()->ps()->delColor(opt->proto->mColor);
}

void onProtoOptionToggleAll(UIEvent evt, int eventData, void *userData){	
	bool show = false; //((int)userData == 1);	
	for(int i=0;i<(int)mProtoOptions.size();i++){
		ProtoOption *opt = mProtoOptions[i];
		opt->shown = show;	
		opt->btn->setData(show ? "true" : "false");		
		opt->proto->bShown = show;
	}
	
	App::S()->ps()->delAll();
}


/*********************************************
			Key handler for the root
**********************************************/
void onGlobalKeyPress(UIEvent evt, int eventData, void *userData){
	if(eventData == SDLK_ESCAPE){
		App::S()->utilShutdown(0);
	}else{
		LOG("Unhandled keypress for key %d (%c)\n", eventData, eventData);
	}
}

/*********************************************
			Click handler for the root
**********************************************/
void onGlobalClick(UIEvent evt, int eventData, void *userData){
	App::S()->mCurrentModule->onClick(eventData, App::S()->fMouseX, 
								App::S()->fMouseY, App::S()->fMouseZ);
								
	App::S()->endDrag();	
}

void onGlobalMouseDown(UIEvent evt, int eventData, void *userData){
	App::S()->beginDrag();
}

void App::addOption(int i, string text, UIWindow *wnd){

	ModuleOption *opt = new ModuleOption;

	UICheckbox *c = App::S()->guiCreate<UICheckbox>("chk", Vector2(10, i * 25 + 16), Vector2(16, 16), wnd);
	c->setData("false");	
	c->addEventHandler(EVENT_MOUSE_CLICK, (void *)opt, &onModuleOptionPress);
	
	UIText *t = App::S()->guiCreate<UIText>("txt", Vector2(40, i * 25 + 15), Vector2(128, 24), wnd);
	t->setData(text);
	t->addEventHandler(EVENT_MOUSE_CLICK, (void *)opt, &onModuleOptionPress);
	
	opt->name = text;
	opt->enabled = false;
	opt->btn = c;
	opt->index = i;
	
	mOptions.push_back(opt);	
}

/*********************************************
			Sets up the default GUI
**********************************************/
void App::makeGUI(){
	mUIRoot.initGeneric("screen", Vector2(0,0), Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
	mUIRoot.addEventHandler(EVENT_KEY_UP, NULL, &onGlobalKeyPress);
	mUIRoot.addEventHandler(EVENT_MOUSE_CLICK, NULL, &onGlobalClick);
	mUIRoot.addEventHandler(EVENT_MOUSE_DOWN, NULL, &onGlobalMouseDown);
	
	//Make a test GUI window
	wndOptions = guiCreate<UIWindow>("window", Vector2(-1, -1), Vector2(220, 170), &mUIRoot);
	wndOptions->setData("Camera options");
	wndOptions->hide(true); //start hidden
	
	addOption(1, "Rotate camera in X", wndOptions);
	addOption(2, "Rotate camera in Y", wndOptions);
	addOption(3, "Rotate camera in Z", wndOptions);
		
	ModuleOption *opt = new ModuleOption;
	
	UIButton *b = guiCreate<UIButton>("button", Vector2(40, 115), Vector2(130, 20), wndOptions);
	b->setData("Reset camera");		
	b->addEventHandler(EVENT_MOUSE_CLICK, (void *)opt, &onModuleOptionPress);
		
	opt->btn = NULL;
	opt->index = OPTION_RESET_ROTATION;	
	mOptions.push_back(opt);
	
		
	//Make the protocol window
	wndProto = guiCreate<UIWindow>("window", Vector2(-1, -1), Vector2(256, 40), &mUIRoot);
	wndProto->setData("Show Protocols");
	wndProto->hide(true); //start hidden
	
	b = guiCreate<UIButton>("button", Vector2(20, 45), Vector2(80, 20), wndProto);
	b->setData("Show All");	
	b->addEventHandler(EVENT_MOUSE_CLICK, (void *)1, &onProtoOptionToggleAll);
	
	b = guiCreate<UIButton>("button", Vector2(150, 45), Vector2(80, 20), wndProto);
	b->setData("Hide All");		
	b->addEventHandler(EVENT_MOUSE_CLICK, (void *)0, &onProtoOptionToggleAll);
		
	//Buttons down the bottom of the screen to show and hide windows
	UIButton *btnOptions = guiCreate<UIButton>("btnOptions", 
							Vector2(2, 5), Vector2(128, 20), &mUIRoot);
								btnOptions->setData("Camera");
								
	btnOptions->addEventHandler(EVENT_MOUSE_CLICK, (void *)0, &handler);
	
	
	UIButton *btnProto = guiCreate<UIButton>("btnProto", 
							Vector2(140, 5), Vector2(128, 20), &mUIRoot);							
	btnProto->setData("Protocols");
	btnProto->addEventHandler(EVENT_MOUSE_CLICK, (void *)1, &handler);
	
}

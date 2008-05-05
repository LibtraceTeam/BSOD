#include "main.h"
#include "appgui.h"

void App::addFlowDescriptor(byte id, Color c, string name){

	if(mFlowDescriptors[id]){
		return; //we already have one!
	}
	

	FlowDescriptor *f = new FlowDescriptor;
	f->id = id;
	f->mColor = c;
	f->mName = name;
	f->bShown = true; //show by default
	
	mFlowDescriptors[id] = f;
				
	//Add this to the GUI window			
	int y = (id / 2) * 25 + 80;
	int x = (id % 2) * 130 + 20;
						
	UIText *t = guiCreate<UIText>("text", Vector2(x + 30, y), Vector2(100, 20), wndProto);
	t->setData(f->mName);
	t->setColor(f->mColor);
				
	UICheckbox *b = guiCreate<UICheckbox>("button", Vector2(x + 5, y), Vector2(16, 16), wndProto);
				
	ProtoOption *opt = new ProtoOption();
	opt->btn = b;
	opt->shown = true;
	opt->proto = f;
		
	mProtoOptions.push_back(opt);
		
	b->setData("true");		
	b->addEventHandler(EVENT_MOUSE_CLICK, (void *)opt, &onProtoOptionPress);	
	t->addEventHandler(EVENT_MOUSE_CLICK, (void *)opt, &onProtoOptionPress);	
	
	wndProto->setSize(Vector2(wndProto->getSize().x, y + 60));
	
	if(wndProto->isHidden())
		wndProto->setPos(Vector2(100, 100)); 
}

FlowDescriptor *App::getFD(byte id){
	return mFlowDescriptors[id];
}

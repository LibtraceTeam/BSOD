struct ModuleOption{
	string name;	
	UICheckbox *btn;
	bool enabled;
	int index;
};

struct ProtoOption{
	FlowDescriptor *proto;
	UICheckbox *btn;
	bool shown;
};

extern vector<ModuleOption *> mOptions;
extern vector<ProtoOption *> mProtoOptions;

extern void onModuleOptionPress(UIEvent evt, int eventData, void *userData);
extern void onProtoOptionPress(UIEvent evt, int eventData, void *userData);

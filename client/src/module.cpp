#include "main.h"

typedef IModule *(*InitFunc)(App *);

void *getFunction(void *mHandle, string name){
	
	//find the address of function and data objects
	void *fptr = dlsym(mHandle, name.c_str());
	
	if(fptr){
	}else{
		printf("Couldn't get pointer for '%s': %s\n", name.c_str(),dlerror());
		return NULL;
	}
	
	return fptr;
}

/*********************************************
	Global loader. Sets the current module
**********************************************/
bool App::loadModule(string name){

	LOG("Loading module '%s'\n", name.c_str());
	
	string path = "./modules/" + name + ".so";
		
	//open the needed object
	void *mModuleHandle = dlopen(path.c_str(), RTLD_LAZY);
	
	if(mModuleHandle){
		//LOG("Got handle!\n");
	}else{
		ERR("Couldn't open page library: \n   %s\n", dlerror());
		return false;
	}
	
	if(mCurrentModule){
		mCurrentModule->shutdown();
		delete mCurrentModule;
	}
		
	InitFunc init = (InitFunc)getFunction(mModuleHandle, "init");
	
	if(!init){
		LOG("Couldn't run init() in module\n");
		return false;
	}
	
	//Use the global init to get the pointer to the module
	mCurrentModule = init(this);
	
	if(!mCurrentModule){
		LOG("Got a null module from init global!\n");
		return false;
	}
	
		
	//And start it up!
	mCurrentModule->init();
	
	return true;
}




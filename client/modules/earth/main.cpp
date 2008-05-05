#include "earth.h"

App *mApp = NULL;

EXPORT IModule *init(App *a){
	mApp = a;	
	App::setS(mApp);
	return (IModule *)(new EarthModule());
}
	
/*********************************************
		 	Entry
**********************************************/
bool EarthModule::init(){	

	LOG("Module Init!\n");
	
	mQuadric = gluNewQuadric();
	gluQuadricTexture(mQuadric, true);
	gluQuadricOrientation(mQuadric, GLU_OUTSIDE);
	gluQuadricNormals(mQuadric, GLU_SMOOTH);	
	
	mApp->camSetPos(0, 0, 20);
	mApp->camLookAt(0,0,0);
	
	mTex = mApp->texLoad("earthmap_2.jpg", 0);
	
	if(!mTex){
		LOG("Couldn't get the texture!\n");
		return false;
	}
	
	for(int i=0;i<3;i++){
		rotate[i] = false;
		angle[i] = false;
	}
	
	for(int i=0;i<NUM_STARS;i++){		
		Vector3 p = Vector3(mApp->randFloat(-1, 1), mApp->randFloat(-1, 1), mApp->randFloat(-1, 1));
		mStars[i].pos = p.normalized() * STAR_DISTANCE;		
	}
					
	return true;
}

/*********************************************
		 	Update
**********************************************/
void EarthModule::update(float currentTime, float timeDelta){

	fTimeScale = timeDelta;

	//rotate if necessary
	for(int i=0;i<3;i++)
		if(rotate[i]) angle[i] += timeDelta * 2.5;
		
	//for(int i=0;i<mFlowPoints.size();i++){
		
}

void EarthModule::newFlow(int flowID, IPaddress src, IPaddress dst, FlowProtocol type, FlowDirection dir){

	FlowPoint flow;
		
	byte *s = (byte *)&src.host; byte *d = (byte *)&dst.host;
		
	flow.pos1 = Vector3(s[0] - 128, s[1] - 128, s[2] - 128).normalized() * (RADIUS + 0.1f);
	flow.pos2 = Vector3(d[0] - 128, d[1] - 128, d[2] - 128).normalized() * (RADIUS + 0.1f);
	flow.type = type;
		
	mFlowPoints.push_back(flow);	
	
	mFlowMap[flowID] = &mFlowPoints[mFlowPoints.size() - 1];
	
	float speed = 1.0f;
	
	Vector3 v1 = flow.pos1.normalized() * speed;
	Vector3 v2 = flow.pos2.normalized() * speed;
		
	mApp->ps()->add(flow.pos1, Vector3(0,0,0), mApp->getProtocolColor(type), 0.5f, mApp->randFloat(15, 30));
	mApp->ps()->add(flow.pos2, Vector3(0,0,0), mApp->getProtocolColor(type), 0.5f, mApp->randFloat(15, 30));
}

void EarthModule::newPacket(int flowID, int size, float rtt){
	
	FlowPoint *flow = mFlowMap[flowID];	
	
	if(!flow){
		return;
	}
	
	Vector3 v1 = flow->pos1.normalized() * rtt;
	Vector3 v2 = flow->pos2.normalized() * rtt;
	
	mApp->ps()->add(flow->pos1, v1, mApp->getProtocolColor(flow->type), mApp->randFloat(0.5, 1.5), mApp->randFloat(5, 15));
	mApp->ps()->add(flow->pos2, v2, mApp->getProtocolColor(flow->type), mApp->randFloat(0.5, 1.5), mApp->randFloat(5, 15));

}

void EarthModule::delFlow(int flowID){
	mFlowMap[flowID] = NULL;
}

void EarthModule::delAll(){
	
}
	
	
/*********************************************
		 	3D rendering
**********************************************/
void EarthModule::render(){
		
	glRotatef(angle[0], 1, 0, 0);
	glRotatef(angle[1], 0, 1, 0);
	glRotatef(angle[2], 0, 0, 1);	
	
	glEnable(GL_CULL_FACE);
	
	glColor3f(1,1,1);
	
	mTex->bind();
	
	glRotatef(90, 1, 0, 0);
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	gluSphere( mQuadric,
			 	RADIUS,
			 	64,
			 	64);
			 	
	//'atmosphere' hack	
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glColor4f(0.15f, 0.15f, 0.15f, 0.15f);	 	
	gluSphere( mQuadric,
			 	RADIUS + 0.1f,
			 	32,
			 	32);
	glDisable(GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
	
	glColor3f(1,1,1);
	glPointSize(1.0f);
	glBegin(GL_POINTS);

	for(int i=0;i<NUM_STARS;i++){
		glVertex3f(mStars[i].pos.x, mStars[i].pos.y, mStars[i].pos.z);
	}
	glEnd();
	
	glEnd();
	
}



/*********************************************
		 	2D rendering
**********************************************/
void EarthModule::render2d(){

}

/*********************************************
		 	Shutdown
**********************************************/
void EarthModule::shutdown(){

	LOG("Shutdown\n");
	
}


/*********************************************
		 	Configuration
**********************************************/
bool EarthModule::setConfig(string key, string value){	
	
	if(key == "rotate_x")	rotate[0] = (value == "yes");
	if(key == "rotate_y")	rotate[1] = (value == "yes");
	if(key == "rotate_z")	rotate[2] = (value == "yes");
	
	if(key == "rotate_reset"){
		for(int i=0;i<3;i++){
			angle[i] = 0.0f;
		}
	}
	
	return true;
}

void EarthModule::getConfigExports(vector<string> &keys){
	keys.push_back("rotate_x|Rotate camera in X|no|yes");
	keys.push_back("rotate_y|Rotate camera in Y|no|yes");
	keys.push_back("rotate_z|Rotate camera in Z|no|yes");
	keys.push_back("rotate_reset|Reset rotation|Apply");
}



/*********************************************
		 	Interaction
**********************************************/
void EarthModule::onClick(int button, float x, float y, float z){
	
}


/*******************************************************************************
							BSOD2 Client - ps_classic.h
							
 This contains the 'classic' type of particle systems - a very simple one that
 renders each particle as triangles, and a slightly less simple one that uses
 pointsprites. Because these two are very similar, they share everything but
 their init() and render() methods.
*******************************************************************************/

/*********************************************
		Represents one particle
**********************************************/
class Particle{
public:
	bool active; //if it's alive or not
	
	float x,y,z; //position in 3-space relative to the origin of the parent
	float life; //how long left till it dies
	float r,g,b,a; //color
	float vx, vy, vz; //speed to move per second
	float size;	
	
	unsigned int index; //index into the parent particle array
};


/*********************************************
		A triangle-based particle system. 
**********************************************/
class PSClassic : public IParticleSystem{
protected:
	int iNumActive;
	int iLastColorChanges;
	
	//particle lists
	Particle mParticles[MAX_PARTICLES];	
	stack<Particle *> mFree; 
	vector<Particle *> mActive;

	void del(int i); //delete a specific particle	
	
	GLuint mDisplayList;
	bool bNeedRecompile;	
	
public:

	//overridden by PSSprites
	virtual bool init();
	virtual void render();	
	virtual int getType(){return PARTICLE_SYSTEM_CLASSIC;}
		
	//Particle system operations
	void add(Vector3 pos, Vector3 speed, Color col, float size, float life);
	void update();		
	void shutdown();	
	
	int getActive(){return iNumActive;}
	
	void delAll();
	void delColor(Color c);		
};


/*********************************************
		A point-sprite particle system
**********************************************/
class PSSprites : public PSClassic{

	void renderAll();
public:
	bool init();
	void render();
	int getType(){return PARTICLE_SYSTEM_POINTSPRITES;}
};


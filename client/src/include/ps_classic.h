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
	float x,y,z; //position in 3-space relative to the origin of the parent
	float life; //how long left till it dies
	float r,g,b,a; //color
	float vx, vy, vz; //speed to move per second
	float size;	
	float timestamp; //Used by shaders - the current time at creation 
	
	unsigned int index; //index into the parent particle array
	
	bool active; //if it's alive or not
};


/*********************************************
	Represents a group of particles that 
	share common colours and size
**********************************************/
class ParticleCollection{
public:
	vector<Particle *> mParticles;
	float fSize;
	Color mColor;
	bool bShown;
	uint32_t mList; 
	
	//For sorting
	float sum(){
		return mColor.sum() + (fSize * 100000);
	}	
	void del(int i);
};


/*********************************************
		A triangle-based particle system. 
**********************************************/
class PSClassic : public IParticleSystem{
protected:
	int iNumActive;
	int iLastColorChanges;
	float fTime;
	
	Texture *mTexture;
	
	//particle lists
	Particle mParticles[MAX_PARTICLES];	
	stack<Particle *> mFree; 
	
	//The collection of renderable particles. 
	map<float, ParticleCollection *> mParticleCollections;	
	ParticleCollection *getCollection(Color col, float size);
	
	//Display list
	GLuint mDisplayList;
	bool bNeedRecompile;	
	
	//Other util
	void del(ParticleCollection *col, int i);
	virtual float setSizeScale();
public:

	//overridden by PSSprites and PSShaders
	virtual bool init();
	virtual void render();	
	virtual void update();	
	virtual int getType(){return PARTICLE_SYSTEM_CLASSIC;}
	virtual void add(Vector3 pos, Vector3 speed, Color col, 
						float size, float life);
						
	//Other common particle system operations
	void showColor(Color c, bool bShow);	
	int getActive(){return iNumActive;}		
	void shutdown();		
	void delAll();	
	virtual void doPeriodicCleanup();

	virtual ~PSClassic(){}
};


/*********************************************
		A point-sprite particle system
**********************************************/
class PSSprites : public PSClassic{
protected:
	virtual float setSizeScale();
	float fMaxSize;
public:
	bool init();
	void render();
	virtual int getType(){return PARTICLE_SYSTEM_POINTSPRITES;}
};


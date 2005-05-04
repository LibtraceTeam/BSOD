#ifndef __PARTFLOW_H__
#define __PARTFLOW_H__

#ifdef _WIN32
#pragma warning( disable : 4996 )
#endif

#define PREALLOC 10 // Number of packets each flow preallocates memory for.

class CPartFlow;

typedef hash_map<unsigned int, CPartFlow *> FlowMap;
typedef list<FlowMap::const_iterator> IterList;

class CPartFlow
{
private:
	vector<Vector3f> vertices;
	//vector<Vector2f> tex_coords;
	//vector<byte> colours;
	byte colour[3];
	vector<Vector3f> jitter;
	//byte colours[3];
	CTexture *tex;

	vector<Vector3f> endpoint_vertices;
	vector<Vector2f> endpoint_tex_coords;

	Vector3f translation;
	Vector3f start;
	Vector3f destination;

	unsigned short offset;
	unsigned short sam_count;
	unsigned short gc_count;
	uint32 last_ts;

	float speed;
	float length;
	bool dark;
	int packets;
	IterList::iterator active_flow_ptr;
	byte flow_colour[3];
	unsigned char type;

public:
	CPartFlow();
	virtual ~CPartFlow();
	void Draw();
	void Update(float diff);
	//void Update_ServerTime(timestamp);

	bool AddParticle(unsigned char id, byte r, byte g, byte b, unsigned short size, float speed, bool dark);
	inline void ResetCounter() { sam_count = 0; }
	void CreateEndPoints();
	void ReInitialize();

	static float time_to_live;

	friend class CPartVis;
};

#endif

#ifndef __PARTFLOW_H__
#define __PARTFLOW_H__

class CPartFlow
{
private:
	vector<Vector3f> vertices;
	vector<Vector2f> tex_coords;
	vector<byte> colours;
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

	float speed;
	float length;
	bool dark;

public:
	CPartFlow();
	virtual ~CPartFlow();
	void Draw();
	void Update(float diff);
	//void Update_ServerTime(timestamp);

	void AddParticle(byte r, byte g, byte b, unsigned short size, float speed, bool dark);
	inline void ResetCounter() { sam_count = 0; }
	void CreateEndPoints();

	static float time_to_live;

	friend class CPartVis;
};

#endif
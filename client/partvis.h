#ifndef __PARTVIS_H__
#define __PARTVIS_H__

class CPartFlow
{
private:
    vector<Vector3f> vertices;
    vector<Vector2f> tex_coords;
    vector<byte> colours;
    CTexture *tex;
    
    Vector3f translation;
    Vector3f start;
    Vector3f destination;

    unsigned short offset;
    // CEntity ?
    
public:
    CPartFlow();
    virtual ~CPartFlow();
    void Draw();
    void Update(float diff);
//void Update_ServerTime(timestamp);
	
    void MoveParticles();
    void AddParticle(byte r, byte g, byte b, unsigned short size);

    static float time_to_live;

    friend class CPartVis;
};

typedef map<unsigned int, CPartFlow *> FlowMap;

class CPartVis : public CEntity
{
private:
    FlowMap flows;
    CTriangleFan *left, *right;
    uint32 last_timestamp;
    
public:
    CPartVis();
    virtual ~CPartVis() {}

    virtual void Draw();
    virtual void Update(float diff);

    void UpdateFlow(unsigned int flow_id, Vector3f v1, Vector3f v2);
    void UpdatePacket(unsigned int flow_id, uint32 timestamp, byte r,
	byte g, byte b, unsigned short size);
    void RemoveFlow(unsigned int id);
    uint32 GetLastTimestamp() { return last_timestamp; }

    CPartFlow * make_flow(int);
};

#endif // __PARTVIS_H__

/* $CVSID$ */ 
#ifndef __ENTITY_MANAGER_H__
#define __ENTITY_MANAGER_H__

class CEntity;
class CActor;
class CCamera;
class CPlayer;

class CEntityManager {
private:
public:
	CPlayer *player;
	list<CEntity *>	entities;
	list<CEntity *> temp_entities;

	CEntityManager();
	~CEntityManager();

	//CCamera* GetCamera();
	CPlayer* GetPlayer();
	
	void AddEntity(CEntity *ptr);
	void AddTempEntity(CEntity *e);
	void Update(float time);
	void DrawEntities();

	void UpdateEntity(int id, const Vector3f position, const Vector3f bearing, const Vector3f velocity);

	bool CollideEntities(CActor *subject, const Vector3f &futurePos, 
					Vector3f &collisionPoint, Vector3f &surfaceNorm);
};

#endif


/* $Header$ $Log$
/* $Header$ Revision 1.4  2004/02/17 01:59:55  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#include "stdafx.h"

#include "matrix.h"
#include "world.h"
#include "entity.h"
#include "camera.h"
#include "md3.h"
#include "reporter.h"
#include "quaternion.h"
#include "player.h"
#include "entity_manager.h"
#include "sound.h"

#include "partvis.h"

CEntityManager::CEntityManager() {
	// we need a camera to be element in the entity list and the pointer camera

	player = new CPlayer;
	player->GetCamera()->id = 0;
    // Could load an MD3 for the player here, but no point, as it isn't
    // displayed (don't want to see your own model, it only gets in the way)

	player->SetPosition(Vector3f(4.0f, 0.0f, 0.0f));
//	player->LookAt(Vector3f(0.0f, 0.0f, 0.0f));

	player->m_Ghost = true;
	AddEntity(player);

}

CEntityManager::~CEntityManager()
{
	list<CEntity *>::iterator i;
	for(i = entities.begin(); i != entities.end(); ++i)
	{
		delete *i;
	}

	for(i = temp_entities.begin(); i != temp_entities.end(); ++i)
	{
		delete *i;
	}
}


void CEntityManager::AddEntity(CEntity *obj) {
	entities.push_back(obj);
	CReporter::Report(CReporter::R_DEBUG, "Adding entity of id:%d to list.\n", obj->id);
}

CPlayer* CEntityManager::GetPlayer()
{
	return player;
}

//CCamera* CEntityManager::GetCamera() {
//	return player->GetCamera();
//}

void CEntityManager::DrawEntities() {
	list<CEntity *>::iterator i;
	for(i = entities.begin(); i != entities.end(); ++i)
	{
		(*i)->Draw();
	}

	for(i = temp_entities.begin(); i != temp_entities.end(); ++i)
	{
		(*i)->Draw();
	}
}

void CEntityManager::Update(float time) {
	list<CEntity *>::iterator i = entities.begin();
	for(; i != entities.end(); ++i)
	{
		(*i)->Update(time);
	}

	for(i = temp_entities.begin(); i != temp_entities.end(); ++i)
	{
		(*i)->Update(time);
	}
}

void CEntityManager::UpdateEntity(int id, const Vector3f position, const Vector3f bearing, const Vector3f velocity)
{
	list<CEntity *>::iterator i = entities.begin();
	for(; i != entities.end(); ++i)
	{
		if( (*i)->id == id )
		{
            Log("Updating id=%d", id);
			(*i)->SetBearing(bearing);
			(*i)->SetPosition(position);
			(*i)->SetVelocity(velocity);
			return;
		}
	}

	CActor *ent = new CActor;

	

	ent->id = id;
	
	CMD3 *mdl;
	mdl = new CMD3();

	world.resources->LoadMD3("players/tankjr", *mdl);
	
	//mdl->LoadMD3("tankjr");
	//mdl->collider = CCollider::Create();
//	mdl->collider->BuildCollisionInfo(mdl);

	/*CMD2 *act;
	act = new CMD2();
	act->LoadMD2("md2s/yohko/tris.md2");
	act->collider = CCollider::Create();
	act->collider->BuildCollisionInfo(act);*/
	ent->m_model = mdl;

	ent->SetPosition(position);
	ent->SetBearing(bearing);
	
	AddEntity(ent);
}

bool CEntityManager::CollideEntities(CActor *subject, const Vector3f &futurePos, 
					   Vector3f &collisionPoint, Vector3f &surfaceNorm) 
{
	// TODO: reimplement this function in a better way
/*	list<CEntity *>::iterator i = entities.begin();
	list< t3DModel * >::iterator iter_mdl_objs;

	for(; i != entities.end(); ++i) {
		
		float depth;
		if( subject != *i && ((CActor *)*i)->m_model) {
			CMD3 *obj = ((CActor *)*i)->m_model;
			iter_mdl_objs = obj->m_Objects.begin();
			for(; iter_mdl_objs != obj->m_Objects.end(); ++iter_mdl_objs) {
				for(int i = 0; i < (int)(*iter_mdl_objs)->pObject.size(); i++) {
					if(subject->m_model->collider->Collides(
						(*iter_mdl_objs)->pObject[i].collider, 
						futurePos, 
						collisionPoint, 
						//surfaceNorm, 
						depth ) ) {
						return true;
					}
				}
			}
		}

	}*/
	return false;
}

					   
void CEntityManager::AddTempEntity(CEntity *e)
{
	temp_entities.push_back(e);
}

/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#include "stdafx.h"

#include "matrix.h"
#include "world.h"
#include "entity.h"
#include "camera.h"
#include "reporter.h"
#include "quaternion.h"
#include "player.h"
#include "entity_manager.h"
#include "sound.h"

#include "partflow.h"
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
	
	/*CMD3 *mdl;
	mdl = new CMD3();

	world.resources->LoadMD3("players/tankjr", *mdl);*/
	
	//mdl->LoadMD3("tankjr");
	//mdl->collider = CCollider::Create();
//	mdl->collider->BuildCollisionInfo(mdl);

	/*CMD2 *act;
	act = new CMD2();
	act->LoadMD2("md2s/yohko/tris.md2");
	act->collider = CCollider::Create();
	act->collider->BuildCollisionInfo(act);*/
//	ent->m_model = mdl;

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

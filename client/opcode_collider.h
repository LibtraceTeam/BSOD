/* $CVSID$ */ 
#ifndef __OPCODE_COLLIDER__

class COpcodeCollider : public CCollider
{
private:
	Opcode::OPCODE_Model *ocModel;
	CMesh *mesh;
	
	static Opcode::AABBCollider			octreeCollider;
	static Opcode::SphereCollider		ocSphereCollider;

public:
	
	static void Initialise();

	static void CollisionCallback(uint32		  triangleindex,
								  Opcode::VertexPointers& triangle,
								  uint32		  userdata);
	
	COpcodeCollider() : ocModel(NULL), mesh(NULL) {}
	virtual ~COpcodeCollider() { delete ocModel; }
	virtual void BuildCollisionInfo(CMesh *mesh);
	virtual bool Collides(const CCollider *other, const Vector3f &position, 
						  Vector3f &collisionPoint, float &depth);
	virtual bool SphereCollide(const CCollider *other, const Vector3f &position, 
							   const float radius, Vector3f &collisionPoint,
							   float &depth);
	Opcode::OPCODE_Model *GetModel() { return ocModel; }
	CMesh *GetMesh() { return mesh; }

	virtual void WriteToDisk(FILE *out);
	virtual void LoadFromDisk(CReader *r, CMesh *m);
	virtual void Dump();
};

#endif // __OPCODE_COLLIDER__


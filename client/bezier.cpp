/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/bezier.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/bezier.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/bezier.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:55  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
#include "stdafx.h"
#include "bezier.h"
#include "world.h"
#include "display_manager.h"
#include "reporter.h"

#include <math.h>
#include <algorithm>

int			   CBezier::detail = 6;
vector<uint32> CBezier::indices;

bool CBezier::FitInBox(CBox &bbox) 
{
	vector<Vector3f>::iterator i = m_cpts.begin();

	for(; i != m_cpts.end(); ++i) {
		if(    (*i).x < bbox.m_min.x || (*i).x > bbox.m_max.x
			|| (*i).y < bbox.m_min.y || (*i).y > bbox.m_max.y
			|| (*i).z < bbox.m_min.z || (*i).z > bbox.m_max.z ) {

			return false;
		}
	}

	return true;
}

void CBezier::Draw()
{ 
	world.display->BindTexture(tex);
	// There are two different way to draw beziers!  One is via (detail - 1) triangle strips
	// and the other is via one big indexed triangle list.  Not sure which is faster, might
	// be worth investigating this some time.
	
/*
	int c = 0, d;
	for(int x = 0; x < detail - 1; x++) {
		world.display->DrawTriangleStrip(
			(float *)&vertices[c], 
			(float *)&texCoords[c], 
			(detail * 2) - 2);

		c += detail * 2;
	}
*/

	world.display->DrawIndexedTriangles2(
		(float *)&vertices[0],
		(float *)&texCoords[0],
		GetTriangleIndices(),
		GetNumberTriangles()); 

}

uint32 *CBezier::GetTriangleIndices()
{
	return &indices[0];
}

int CBezier::GetNumberTriangles()
{
	return (detail - 1) * ((detail - 1) * 2);
}

Vector2f CBezier::EvalBezier2D(float t, Vector2f &cp1, Vector2f &cp2, Vector2f &cp3)
{
	Vector2f q;

	q =       cp1 * (float)pow(1.0f - t, 2.0f)
			+ cp2 * (float)(2.0f * t * (1.0f - t))
			+ cp3 * (float)pow(t, 2.0f);

	return q;
}

Vector3f CBezier::EvalBezier3D(float t, Vector3f &cp1, Vector3f &cp2, Vector3f &cp3)
{
	Vector3f q;

	q =		  cp1 * (float)pow(1.0f - t, 2.0f)
			+ cp2 * (float)(2.0f * t * (1.0f - t))
			+ cp3 * (float)pow(t, 2.0f);

	return q;
}

void CBezier::InitIndices()
{
	indices.clear();
	/*for(int x = 0; x < detail; x++) {
		int last_size = 0;
		if(indices.size() != 0)
			last_size = indices.back() + 1;

		for(int y = 0; y < (detail * 2 - 2) * 3; y++) {
			indices.push_back( CMesh::strip_indices[y] + last_size );
		}
	}*/

	int count = 0;
	for(int num_x = 0; num_x < detail - 1; num_x++) {
		for(int num_y = 0; num_y < detail; num_y++) {
			if(num_y == 0) {
				indices.push_back(count); count++;	
				indices.push_back(count); count++;	
			} else {
				if(num_y == 1) {
					indices.push_back(count); count++;

					indices.push_back(count); 
					indices.push_back(count - 1);
					indices.push_back(count - 2);
					count++;	
				} else {
					indices.push_back(count - 2);		
					indices.push_back(count - 1);		
					indices.push_back(count); 
					count++;	
					indices.push_back(count); 
					indices.push_back(count - 1);
					indices.push_back(count - 2);
					count++;	
				}
			}
		}
	}

	/*int c = 0;
	for(int x = 0; x < detail - 1; x++) {
		world.display->DrawTriangleStrip(
			(float *)&vertices[c], 
			(float *)&texCoords[c], 
			(detail * 2) - 2);

		c += detail * 2;
	}*/
}

void CBezier::Tesselate()
{
	vector< Vector3f >::iterator cpts_iter = m_cpts.begin();
	vector< Vector2f >::iterator texpts_iter = m_tex_cpts.begin();

	Vector3f cpoints[3][3];
	Vector2f texpoints[3][3];
	int ypos;

	for( int y = 0; y < 3; y++) {
		ypos = y;
		for( int x = 0; x < 3; x++) {
			cpoints[x][ypos] = *cpts_iter;
			texpoints[x][ypos] = *texpts_iter;
			++cpts_iter;
			++texpts_iter;
		}
	}
	
	int num_x = 0, num_y = 0;
	float incr = 1.0f / (float)(detail - 1);
	vector<Vector3f> coords(detail * detail);
	vector<Vector2f> tex_coords(detail * detail);

	for(float t = 0; num_y < detail; t += incr, num_y++)
	{
		num_x = 0;
		for(float v = 0; num_x < detail; v += incr, num_x++)
		{
			// Physical coordinates:
			Vector3f s0, s1, s2, q;
        
			s0 = EvalBezier3D(t, cpoints[0][0], cpoints[1][0], cpoints[2][0]);
			s1 = EvalBezier3D(t, cpoints[0][1], cpoints[1][1], cpoints[2][1]);
			s2 = EvalBezier3D(t, cpoints[0][2], cpoints[1][2], cpoints[2][2]);

			q = EvalBezier3D(v, s0, s1, s2);

			coords[num_x + num_y * detail] = q;

			// Texture coordinates:
			Vector2f t0, t1, t2, r;

			t0 = EvalBezier2D(t, texpoints[0][0], texpoints[1][0], texpoints[2][0]);
			t1 = EvalBezier2D(t, texpoints[0][1], texpoints[1][1], texpoints[2][1]);
			t2 = EvalBezier2D(t, texpoints[0][2], texpoints[1][2], texpoints[2][2]);

			r = EvalBezier2D(v, t0, t1, t2);

			tex_coords[num_x + num_y * detail] = r;

		}
	}

	for(num_x = 0; num_x < detail - 1; num_x++) {
		for(num_y = 0; num_y < detail; num_y++) {
			vertices.push_back( coords[num_x + num_y * detail] );
			vertices.push_back( coords[(num_x + 1) + num_y * detail] );

			texCoords.push_back( tex_coords[num_x + num_y * detail] );
			texCoords.push_back( tex_coords[(num_x + 1) + num_y * detail] );
		}
	}
}

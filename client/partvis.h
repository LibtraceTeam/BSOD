/*
 * This file is part of bsod-client
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Authors: Sam Jansen
 *	    Sebastian Dusterwald
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * bsod-client includes software developed by Sam Jansen and Jesse Baker 
 * (see http://www.wand.net.nz/~stj2/bung).
 */

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
#ifndef __PARTVIS_H__
#define __PARTVIS_H__

#ifdef _WIN32
#pragma warning( disable : 4996 )
#endif

#define SELECT_BUFFER_SIZE 1024

#include "gui.h"

struct FlowDescriptor 
{
	byte colour[3];
	char name[256];
	bool show;
};

union IPUnion
{
	byte quartet[4];
	uint32 integer;
};

inline void IPInt2Byte( uint32 ip, byte *q1, byte *q2, byte *q3, byte *q4 )
{
	IPUnion ipu;
	ipu.integer = ip;

	*q1 = ipu.quartet[0];
	*q2 = ipu.quartet[1];
	*q3 = ipu.quartet[2];
	*q4 = ipu.quartet[3];
}

inline bool PointInRect( int px, int py, int rx, int ry, int rwidth, int rheight )
{
	if( px < rx )
		return( false );
	if( py < ry )
		return( false );
	if( px > (rx+rwidth) )
		return( false );
	if( py > (ry+rheight) )
		return( false );

	return( true );
}

typedef map<unsigned int, CPartFlow *> FlowMap;
typedef list<CPartFlow*> FlowList;
typedef map<unsigned char, FlowDescriptor *> FlowDescMap;
typedef list<unsigned int> FlowIDList;

class CPartVis : public CEntity
{
private:
    FlowMap flows;
    CTriangleFan *left, *right;
    uint32 last_timestamp;
    bool paused;
	CTexture *wandlogo;
	IterList active_nodes;
	FlowList partflow_pool;
	vector<Vector2f> tex_coords;
	FlowIDList flows_to_remove;
    
public:
    CPartVis( bool mm );
    virtual ~CPartVis() {}

    virtual void Draw( bool picking );
    virtual void Update(float diff);

    void UpdateFlow(unsigned int flow_id, Vector3f v1, Vector3f v2, uint32 ip1, uint32 ip2 );
    void UpdatePacket(unsigned int flow_id, uint32 timestamp, byte id_num, 
		unsigned short size, float speed, bool dark);
    void RemoveFlow(unsigned int id);
    uint32 GetLastTimestamp() { return last_timestamp; }
    void BeginUpdate();
    void EndUpdate();
    void TogglePaused();
	void ToggleFilter();
	void ToggleShowDark();
	void ToggleBackFilter();
	int NumFlows();
	void ChangeSpeed( bool faster );
	void KillAll();
	IterList::iterator AddActiveFlow( const FlowMap::iterator i );
	void RemoveActiveFlow( const FlowMap::iterator i );
	CPartFlow *AllocPartFlow();
	void FreePartFlow( CPartFlow *flow );
	void GCPartFlows();
	void BillboardBegin();
	void BillboardEnd();

	float colour_table[256]; // Lookup for colour/255.0f to get 0.0f-1.0f

	int packetsFrame;
	//int packetsFrameIn; // Temp var to check packets in/out ratios. Rox.
	float diff;
	float fps;
	float last_gc;
	bool do_gcc;
	bool matrix_mode;
	bool no_gui;
	bool singularity;
	
	short int show_dark;
	//int filter_state;

	float global_size;
	float global_speed;
	float global_alpha;
	bool jitter;
	bool billboard;
	string particle_img;
	float maxPointSize;
	uint32 tehMax;
	byte ip[4];
	bool isHit;
	bool click;

	uint32 pSelBuff[SELECT_BUFFER_SIZE];

    CPartFlow * make_flow(int);
	FlowDescMap fdmap;
	CGui *pGui;

	friend class CPartFlow;
};

#endif // __PARTVIS_H__

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

typedef hash_map<unsigned int, CPartFlow *> FlowMap;

class CPartVis : public CEntity
{
private:
    FlowMap flows;
    CTriangleFan *left, *right;
    uint32 last_timestamp;
    bool paused;
	int filter_state;
	short int show_dark;
	CTexture *wandlogo;
	CTexture *helptex;
	float colour_table[256]; // Lookup for colour/255.0f to get 0.0f-1.0f
	bool show_help;
    
public:
    CPartVis();
    virtual ~CPartVis() {}

    virtual void Draw();
    virtual void Update(float diff);

    void UpdateFlow(unsigned int flow_id, Vector3f v1, Vector3f v2);
    void UpdatePacket(unsigned int flow_id, uint32 timestamp, byte r,
	byte g, byte b, unsigned short size, float speed, bool dark);
    void RemoveFlow(unsigned int id);
    uint32 GetLastTimestamp() { return last_timestamp; }
    void BeginUpdate();
    void EndUpdate();
    void TogglePaused();
	void ToggleFilter();
	void ToggleShowDark();
	void ToggleBackFilter();
	void ToggleHelp();
	int NumFLows();

	int packetsFrame;
	float diff;
	float fps;

    CPartFlow * make_flow(int);

	friend class CPartFlow;
};

#endif // __PARTVIS_H__

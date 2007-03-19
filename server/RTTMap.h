/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Authors: Brendon Jones
 *	    Daniel Lawson
 *	    Sebastian Dusterwald
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * bsod-server is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * bsod-server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bsod-server; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 *
 */


#ifndef RTTMAP_H
#define RTTMAP_H

using namespace std;

#define UINT16 unsigned short int
#define UINT32 unsigned long int
#define BYTE unsigned char

struct Flow
{
	UINT32 ip1;
	UINT32 ip2;
	UINT16 port1;
	UINT16 port2;

	Flow(void)
	{
		ip1 = 0;
		ip2 = 0;
		port1 = 0;
		port2 = 0;
	}

	Flow( UINT32 ip1, UINT32 ip2, UINT16 port1, UINT16 port2 )
	{
		this->ip1 = ip1;
		this->ip2 = ip2;
		this->port1 = port1;
		this->port2 = port2;
	}
};

struct PacketTS
{
	UINT32 ts;
	UINT32 ts_echo;

};

inline int cmp( const Flow &a, const Flow &b )
{
	if (a.ip1 != b.ip1)
		return a.ip1 - b.ip1;

	if (a.ip2 != b.ip2)
		return a.ip2 - b.ip2;
	
	if (a.port1 != b.port1)
		return a.port1 - b.port1;

	return a.port2 - b.port2;
}

inline bool operator < (const Flow &a, const Flow &b )
{
	return( cmp(a,b) < 0 );
}

typedef std::map< UINT32, double > TraceMap; 
typedef std::map< Flow, TraceMap > FlowMap;

class RTTMap
{
	public:
		RTTMap(void);
		~RTTMap(void);
		void Add( Flow *flow, UINT32 time_stamp, double now );
		int Flush( double now );
		PacketTS GetTimeStamp( libtrace_packet_t *packet );
		double Retrieve( Flow *flow, UINT32 time_stamp );
		void Update( Flow *flow, UINT32 time_stamp, UINT32 new_time_stamp );

	protected:
		// TraceMap *m_traces;
		FlowMap *m_flows;
		int GetNextOption( BYTE **ptr, BYTE *type, int *len, BYTE *optlen, BYTE **data );
};

#endif

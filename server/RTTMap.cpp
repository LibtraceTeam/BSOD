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


// Desc: Class that handles tracking of RTT data needed to calc packet speed.

#include <map>
#include <iostream>

using namespace std;

#include "libtrace.h"
#include "RTTMap.h"


RTTMap::RTTMap(void)
{
	m_flows = new FlowMap();
}

RTTMap::~RTTMap(void)
{
	delete m_flows;
}

void RTTMap::Add( Flow *flow, unsigned long int time_stamp, double now )
{
	FlowMap::iterator i = NULL;
	if( (i = m_flows->find( *flow )) != m_flows->end() )
	{
		TraceMap::iterator j = NULL;
		TraceMap *tmpTMap = &i->second;
		//if( (j = i->first->find( time_stamp )) != i->end() )
		if( ( j = tmpTMap->find( time_stamp ) ) != tmpTMap->end() )
		{
			// Found time_stamp.
			//spew( "Insertion exists. Deleting and returning value." );
			//tmpTMap->erase( j );
			//return( now - time_stamp );
			return; // Already exists. Done.
		}
		else
		{
			// Didn't find a matching timestamp.
			tmpTMap->insert( pair< unsigned long int, double >( time_stamp, now ) );
			return;
		}
	}
	else
	{
		TraceMap tmap;
		tmap.insert( pair< unsigned long int, double >( time_stamp, now ) );
		m_flows->insert( pair< Flow, TraceMap >( *flow, tmap ) );
	}
	return;
}

// Flush out any packets that have been timed out:
int RTTMap::Flush( double now )
{
	FlowMap::iterator i = m_flows->begin();
	int count = 0;

	//spew( "Flushing..." );
	for( ; i != m_flows->end(); i++ )
	{
		//spew( "Loopy1" );
		TraceMap &tmap = i->second;
		TraceMap::iterator j = tmap.begin();

		for( ; j != tmap.end(); j++ )
		{
			if( (now - j->second) > 180 )
			{
				//spew( "Erasing old entry..." );
				//TraceMap::iterator tmp = j;
				//tmp++;
				tmap.erase( j );
				//j = tmp;
				//spew( "Done" );
				count++;
			}
			//else
			//	j++;
		}

		if( tmap.size() < 1 )
			m_flows->erase( i );
	}

	spew( "Flows: " << this->m_flows->size() );
	return( count );
	//spew( "Done." );
}

int RTTMap::GetNextOption( BYTE **ptr, BYTE *type, int *len, BYTE *optlen, BYTE **data )
{
	if( *len <= 0 )
		return( 0 );
	*type = **ptr;
	switch( *type )
	{
		case 0:
			return( 0 );
		case 1:
			(*ptr)++;
			(*len)--;
			return( 1 );
		default:
			*optlen = *(*ptr+1);
			if( *optlen < 2 )
				return( 0 );
			(*len) -= *optlen;
			(*data) = (*ptr+2);
			(*ptr) += *optlen;
			if( *len < 0 )
				return( 0 );
			return( 1 );
	}
	return( -1 ); // Shouldn't hit this EVER!
}

// Take a packet, go through its fields and get the timestamp:
PacketTS RTTMap::GetTimeStamp( libtrace_packet_t *packet )
{
	struct libtrace_tcp *tcpptr = trace_get_tcp( packet );
	if( !tcpptr )
	{
		PacketTS ret;
		return( ret ); // Fail.
	}
	
	BYTE *pkt = (BYTE*)tcpptr + sizeof(*tcpptr);
	int plen = (tcpptr->doff*4-sizeof *tcpptr);
	BYTE type = 0;
	BYTE optlen = 0;
	BYTE *data = NULL;

	while( GetNextOption( &pkt, &type, &plen, &optlen, &data ) )
	{
		if( type != 8 )
			continue;

		UINT32 *ts = (UINT32*)&data[0];
		UINT32 *tsecho = (UINT32*)&data[4];

		//spew( "--ts: " << *ts << " --ts_echo: " << *tsecho );

		PacketTS ret( *ts, *tsecho );
		return( ret );
	}

	//spew( "No timestamp!" );
	PacketTS ret;
	return( ret );
}

// Checks if the particular trace exists and if it does it deletes it and returns its original timestamp.
// If it does not find it the function simply returns -1.
double RTTMap::Retrieve( Flow *flow, unsigned long int time_stamp )
{
	FlowMap::iterator i = NULL;
	if( (i = m_flows->find(*flow) ) != m_flows->end() )
	{
		TraceMap &tmap = i->second;
		TraceMap::iterator j = tmap.begin();
		if( (j = tmap.find( time_stamp )) != tmap.end() )
		{
			double ret = j->second;
			tmap.erase( j );
			if( tmap.size() < 1 )
				m_flows->erase( i );
			return( ret );
		}
	}

	return( -1.0 );
}

void RTTMap::Update( Flow *flow, UINT32 time_stamp, UINT32 new_time_stamp )
{
	FlowMap::iterator i = NULL;
	if( (i = m_flows->find(*flow) ) != m_flows->end() )
	{
		//spew( "Found matching flow" );
		TraceMap &tmap = i->second;
		TraceMap::iterator j = tmap.begin();
		if( (j = tmap.find( time_stamp )) != tmap.end() )
		{
			double time = j->second;
			tmap.erase( j );
			tmap.insert( pair<UINT32,double>(new_time_stamp,time) );
			//spew( "Updated packet" );

			//j->first = new_time_stamp;
			return;
		}
	}
}
#ifndef RTTMAP_H
#define RTTMAP_H

using namespace std;

#define DEBUG

#ifdef DEBUG
#define spew(x) cout << x << endl;
#else
#define spew(x) 
#endif


#define UINT16 unsigned short int
#define UINT32 unsigned long int
#define BYTE unsigned char

inline void* operator new( size_t size )
{
	void *p = malloc( size );
	if( p==0 )
		throw std::bad_alloc(); // For ANSI/ISO compliance.
	return( p );
}

inline void operator delete ( void *ptr )
{
	if( ptr != NULL )
	{
		free( ptr );
		ptr = NULL;
	}
}

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

	PacketTS(void)
	{
		ts = 0;
		ts_echo = 0;
	}

	PacketTS( UINT32 ts, UINT32 ts_echo )
	{
		this->ts = ts;
		this->ts_echo = ts_echo;
	}

	void operator = (PacketTS other)
	{
		this->ts = other.ts;
		this->ts_echo = other.ts_echo;
	}
};

inline int cmp( const Flow &a, const Flow &b )
{
	if( a.ip1 < b.ip1 )
		return( -1 );
	if( a.ip1 > b.ip1 )
		return( 1 );
	
	if( a.ip2 < b.ip2 )
		return( -1 );
	if( a.ip2 > b.ip2 )
		return( 1 );

	if( a.port1 < b.port1 )
		return( -1 );
	if( a.port1 > b.port1 )
		return( 1 );

	if( a.port2 < b.port2 )
		return( -1 );
	if( a.port2 > b.port2 )
		return( 1 );

	return( 0 );
}

inline bool operator < (const Flow &a, const Flow &b )
{
	return( cmp(a,b) < 0 );
}

inline bool operator > (const Flow &a, const Flow &b )
{
	return( cmp(a,b) > 0 );
}

inline bool operator == (const Flow &a, const Flow &b )
{
	return( cmp(a,b) == 0 );
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

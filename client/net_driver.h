/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef _NET_DRIVER_H_
#define _NET_DRIVER_H_

class CNetDriver
{
private:

public:
	virtual void Connect(string address) = 0;
	virtual void ReceiveData() = 0;
	virtual ~CNetDriver() {}

	static CNetDriver *Create();
};


#endif


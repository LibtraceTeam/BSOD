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


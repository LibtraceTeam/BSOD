#ifndef BLACKLIST_H
#define BLACKLIST_H

#include "string"
#include "stdio.h"
#include "stdlib.h"
#include "libtrace.h"
#include "time.h"
#include <arpa/inet.h>

class blacklist
{
public:
	blacklist(const char *path, int bl_cnt, int save_int);
	~blacklist();

	bool poke(libtrace_packet_t *packet);
private:
	struct BL
	{
		std::string FName;
		bool InUse;
		bool List[256][256];
		BL *next;
	}*head,*cur;
	
	std::string BLpath;
	int BL_cnt;

	double  ptime,
		ctime,
		Save_int;

	bool read(uint8_t *ip);
	bool write(uint8_t *ip);
	bool load();
	bool save(double);
	void delBL(BL*die);
	char* itoa(int input);
};

#endif

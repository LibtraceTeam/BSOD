#include "stdafx.h"
#include "misc.h"
#include <stdarg.h>

///////////////////////////////////////////////////////////////////

string bsprintf(const char *str, ...)
{
	va_list marker;
	int count = -1;
	string message;
	
	va_start(marker, str);
	while(count == -1) {
		char buf[1024];

		// _vsnprintf returns -1 when it doesn't finish writing, or number of chars in buf
		count = 
#ifdef LINUX
			vsnprintf(buf, 1023, str, marker);
#else
			_vsnprintf(buf, 1023, str, marker);
#endif
		message += buf;
	}

	return message;
}

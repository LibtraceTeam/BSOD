/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/misc.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/misc.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/misc.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
#include "stdafx.h"
#include "misc.h"
#include <stdarg.h>

///////////////////////////////////////////////////////////////////
// TODO: TODO: TODO: get rid of this shit ass code!
// H4X0R! TEMPORARY! REMOVE ME!
// WTF ?????///?/
//#include "collider.h"
//CCollider * CCollider::Create() { return new CDummyCollider; }
//void CCollider::Initialise() {}

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

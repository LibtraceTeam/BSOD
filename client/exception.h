/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "reporter.h"

class CException
{
protected:
	string msg;
public:
	
	virtual string GetMessage() { return msg; }
	CException(string message) { 
		msg = message; 
		CReporter::Report(CReporter::R_ERROR, "Exception: %s\n", message.c_str()); 
	}
	CException(CReporter::ReportLevel level, string message) {
		msg = message;
		CReporter::Report(level, "Exception: %s\n", message.c_str());
	}
	virtual ~CException() {}
};

class CEOFException : public CException
{
public:
	CEOFException(string m) : CException(m) {}
	virtual string GetMessage() {
		return string("CEOFException: ") + msg;
	}

	virtual ~CEOFException() {}
};

#endif


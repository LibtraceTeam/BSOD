/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#include "stdafx.h"
#include "reporter.h"
#include "world.h"
#include "system_driver.h"
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h> // For OutputDebugString
#endif

static CReporter cr;
static char *reportLevelName[] = { "Fatal Error", "", "Error", "Warning", "Msg", "Dbg" };
const int CReporter::reportMaxLogSize = 10;

void CReporter::SetReportLevel(int iReportLevel)
{
	cr.reportLevel = iReportLevel;
}

void CReporter::SetOutput(FILE *newOut)
{
	cr.out = newOut;
}

void CReporter::Report(ReportLevel dbgLevel, const string &msg)
{
	Report(dbgLevel, msg.c_str());
}

const list<string> &CReporter::GetLog()
{
	return cr.reportLog;
}

void CReporter::Report(ReportLevel dbgLevel, const char *msg, ...)
{
	if(dbgLevel > cr.reportLevel)
		return;

	va_list marker;
	int count = -1;
	string message;
	
	va_start(marker, msg);
	while(count == -1) {
		char buf[1024];

		// _vsnprintf returns -1 when it doesn't finish writing, or number of chars in buf
		count = 
#ifdef LINUX
			vsnprintf(buf, 1023, msg, marker);
#else
			_vsnprintf(buf, 1023, msg, marker);
#endif
		message += buf;
	}

	if(dbgLevel == R_FATAL) {
		world.sys->ErrorMessageBox("Fatal Error!", message.c_str());
		world.sys->Quit();
	}

	va_end(marker);

	SimpleReport(dbgLevel, message);
}

void CReporter::SimpleReport(ReportLevel dbgLevel, const string &msg)
{
	if(dbgLevel > cr.reportLevel)
		return;

	//fputs(reportLevelName[dbgLevel], cr.out);
	//fputs(": ", cr.out);
	fputs(msg.c_str(), cr.out);

	fflush(cr.out);

	cr.reportLog.push_front(msg);
	if(cr.reportLog.size() > cr.reportMaxLogSize)
		cr.reportLog.pop_back();

#ifdef _WIN32
	OutputDebugString(reportLevelName[dbgLevel]);
	OutputDebugString(": ");
	OutputDebugString(msg.c_str());
#endif
}

void Log(const char *fmt, ...)
{
	va_list marker;
	int count = -1;
	string message;
	
	va_start(marker, fmt);
	while(count == -1) {
		char buf[4096];

		// _vsnprintf returns -1 when it doesn't finish writing, or number of chars in buf
		count = 
#ifdef LINUX
			vsnprintf(buf, 4095, fmt, marker);
#else
			_vsnprintf(buf, 4095, fmt, marker);
#endif
		message += buf;
	}

	va_end(marker);

	CReporter::SimpleReport(CReporter::R_DEBUG, message);
}


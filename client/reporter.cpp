/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#include "stdafx.h"
#include "reporter.h"
#include "world.h"
#include "system_driver.h"
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h> // For OutputDebugString
#endif

static CReporter cr;
//static char *reportLevelName[] = { "Fatal Error", "", "Error", "Warning", "Msg", "Dbg" };
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
	if(cr.reportLog.size() > (unsigned)cr.reportMaxLogSize)
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


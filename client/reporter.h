/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef _REPORTER_H_
#define _REPORTER_H_

class CReporter
{
public:
	CReporter() { reportLevel = R_DEBUG; out = stdout; }
	CReporter(int iReportLevel) { reportLevel = iReportLevel; }

	enum ReportLevel { R_FATAL, R_NONE, R_ERROR, R_WARNING, R_MESSAGE, R_DEBUG };

	static void Report(ReportLevel dbgLevel, const char *msg, ...);
	static void Report(ReportLevel dbgLevel, const string &msg);
	static void SimpleReport(ReportLevel dbgLevel, const string &msg);
	
	static void SetOutput(FILE *newOut);
	static void SetReportLevel(int iReportLevel);
	static const list<string> &GetLog();

private:
	int reportLevel;
	FILE *out;
	list<string> reportLog;
	static const int reportMaxLogSize;
};

void Log(const char *msg, ...);

#endif


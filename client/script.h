/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef __SCRIPT_H__
#define __SCRIPT_H__

class CScript
{
public:
	virtual void ExecuteString(const string &s) = 0;
	virtual void ExecuteFile(const string &filename) = 0;

	virtual void Begin() = 0;
	virtual void End() = 0;

	virtual bool GetGlobal(const string &name, int *val) = 0;
	virtual bool GetGlobal(const string &name, float *val) = 0;
	virtual bool GetGlobal(const string &name, string *val) = 0;
	virtual bool GetGlobal(const string &name, Vector3f *val) = 0;
	virtual bool GetGlobal(const string &name, bool *val) = 0;

	static CScript *Create();
};

#endif // __SCRIPT_H__


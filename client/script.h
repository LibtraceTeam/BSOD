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


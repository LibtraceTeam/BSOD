/* $CVSID$ */ 
#ifndef _CONFIG_H_
#define _CONFIG_H_

// Because this is a derivative of IParserCallback we must include this
// file after xml_parse.h ... ?
#include "xml_parse.h"

class CConfig : public IParserCallback
{
private:
	struct Attribute
	{
		string		value;
		bool		boolVal;
		int			intVal;
		float		floatVal;
		Vector3f	vector3fVal;
		bool		parsed;

		Attribute() { boolVal = false; intVal = 0; parsed = false; }
	};

	map<string, Attribute> attrMap;
	void		ParseAttr(Attribute &attr);

public:
	virtual void BeginTagFunc(const string &tag, const map<string, string, string_less> &attrs);
	virtual void EndTagFunc(const string &contents);

	void		ParseFile(string fileName);

	bool		GetBoolean(const string attr);
	int			GetInt(const string attr);
	float		GetFloat(const string attr);
	Vector3f	GetVector3f(const string attr);
	string		GetString(const string attr);

	virtual ~CConfig() {}
};


#endif // _CONFIG_H_


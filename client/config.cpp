/* $CVSID$ */ 
#include "stdafx.h"
#include "exception.h"
#include "config.h"
#include "vfs.h"
#include "misc.h"

void CConfig::ParseFile(string fileName)
{
	CXMLParser parser;
	CReader *reader = CVFS::LoadFile(fileName);

	parser.Parse( reader, this );

	delete reader;
}

void CConfig::BeginTagFunc(const string &tag, const map<string, string, string_less> &attrs)
{
	map<string, string, string_less>::const_iterator i = attrs.begin();
	for(; i != attrs.end(); ++i)
	{
		Attribute at;
		at.value = (*i).second;

		attrMap[ (*i).first ] = at;
	}
}

void CConfig::EndTagFunc(const string &contents)
{
}

void CConfig::ParseAttr(Attribute &attr)
{
	attr.parsed = true;
	
	if( attr.value.length() == 0 )
		return;
	
	attr.intVal = atoi( attr.value.c_str() );
	attr.floatVal = (float)atof( attr.value.c_str() );
	
	string boolCheck = attr.value;
	uint32 i = 0;
	while(i < boolCheck.size()) {
		boolCheck[i] = tolower(boolCheck[i]);
		i++;
	}
	
	if(boolCheck == "true" || boolCheck == "yes")
		attr.boolVal = true;
	else
		attr.boolVal = false;
	
	sscanf( attr.value.c_str(), "(%f,%f,%f)", &attr.vector3fVal.x, 
		&attr.vector3fVal.y, &attr.vector3fVal.z);
}

bool CConfig::GetBoolean(const string attr)
{
	map<string, Attribute>::iterator i = attrMap.find(attr);
	
	if(i == attrMap.end()) {
		throw CException(CReporter::R_WARNING, bsprintf("GetBoolean('%s') failed.", attr.c_str()));
	} else {
		if( !(*i).second.parsed )
		{
			// Lazy evaluation: only parse if needed!
			ParseAttr((*i).second);
		}
	
		return (*i).second.boolVal;
	}
}

int CConfig::GetInt(const string attr)
{
	map<string, Attribute>::iterator i = attrMap.find(attr);
	
	if(i == attrMap.end()) {
		throw CException(CReporter::R_WARNING, bsprintf("GetInt('%s') failed.", attr.c_str()));
	} else {
		if( !(*i).second.parsed )
		{
			// Lazy evaluation: only parse if needed!
			ParseAttr((*i).second);
		}
	
		return (*i).second.intVal;
	}
}

float CConfig::GetFloat(const string attr)
{
	map<string, Attribute>::iterator i = attrMap.find(attr);
	
	if(i == attrMap.end()) {
		throw CException(CReporter::R_WARNING, bsprintf("GetFloat('%s') failed.", attr.c_str()));
	} else {
		if( !(*i).second.parsed )
		{
			// Lazy evaluation: only parse if needed!
			ParseAttr((*i).second);
		}
	
		return (*i).second.floatVal;
	}
}

Vector3f CConfig::GetVector3f(const string attr)
{
	map<string, Attribute>::iterator i = attrMap.find(attr);
	
	if(i == attrMap.end()) {
		throw CException(CReporter::R_WARNING, bsprintf("GetVector3f('%s') failed.", attr.c_str()));
	} else {
		if( !(*i).second.parsed )
		{
			// Lazy evaluation: only parse if needed!
			ParseAttr((*i).second);
		}
	
		return (*i).second.vector3fVal;
	}
}

string CConfig::GetString(const string attr)
{
	map<string, Attribute>::iterator i = attrMap.find(attr);
	
	if(i == attrMap.end()) {
		throw CException(CReporter::R_WARNING, bsprintf("GetString('%s') failed.", attr.c_str()));
	} else {
		return (*i).second.value;
	}
}

/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#include "stdafx.h"
#include "xml_parse.h"
#include "vfs.h"
#include "reporter.h"
#include "exception.h"

const string CXMLParser::whitespace(" \t\r\n");

bool CXMLParser::IsEndTag(string tag)
{
	if(tag[0] == '/')
		return true;

	return false;
}

bool CXMLParser::IsWhiteSpace(const char c)
{
	if( whitespace.find(c) != string::npos )
		return true;

	return false;
}

void CXMLParser::FindAttrs(string &tag, Attribute &attrs)
{
	const char *t = tag.c_str();
	string key, value, current;
	string::size_type in;

	in = tag.find_first_of(whitespace);
	if((int)in < 0 || (int)in > (int)tag.size())
		return;

	while(*t)
	{
		if(*t == '/')
		{
			const char *check = t;
			while(*check && *check != '>' && IsWhiteSpace(*check))
				check++;
			if(*check == '>')
				break;
		}
		
		if(*t == '=')
		{
			DoKeyValue(current, key, value, attrs);
		}
		else
		{
			current += *t;
		}
		
		t++;
	}

	DoKeyValue(current, key, value, attrs);
}

bool CXMLParser::IsSingleTag(string tag)
{
	const char *t = tag.c_str(), *start = t;
	bool ret = false;

	t += strlen(t) - 1;

	while(IsWhiteSpace(*t) && t > start)
		t--;

	if(*t == '/')
		ret = true;

	return ret;
}

string CXMLParser::StripWhiteSpace(const string &str)
{
	const char *s = str.c_str();
	string ret;
	
	while(IsWhiteSpace(*s))
		s++;

	ret = str.substr( s - str.c_str(), str.size() );

	s = ret.c_str() + ret.size() - 1;

	while(IsWhiteSpace(*s))
		s--;

	ret = ret.substr( 0, s - ret.c_str() + 1 );

	return ret;
}

void CXMLParser::DoKeyValue(string &current, string &key, string &value,
		Attribute &attrs)
{
	if(key.empty())
	{
		string::size_type index;

		index = current.find_last_of(whitespace);

		if( current[index + 1] == '\0' )
		{
			current = current.substr(0, current.find_last_not_of(whitespace) + 1);
			index = current.find_last_of(whitespace);
		}
	
		if((int)index < 0 || (int)index > (int)current.size())
			return;

		key = current.substr(index + 1, current.size()).c_str();
		key = StripWhiteSpace(key);
	}
	else
	{
		string::size_type index;

		index = current.find_last_of(whitespace);

		if( current[index + 1] == '\0' )
		{
			current = current.substr(0, current.find_last_not_of(whitespace) + 1);
			index = current.find_last_of(whitespace);
		}
	
		if(index < 0 || (int)index > (int)current.size())
			value = current;
		else
            value = current.substr(0, index).c_str();

		value = StripWhiteSpace(value);

		attrs[key] = value;

		if(index < 0 || (int)index > (int)current.size())
			key = "";
		else
		{
			key = current.substr(index + 1, current.size()).c_str();
			key = StripWhiteSpace(key);
		}
	}

	current = "";
}

/** Parses the given XML file.  begin_tag_func is called whenever a tag
 * begins, end_tag_func whenever one ends.  The appropriate data will be
 * sent as function arguments.  Note that this parser does absolutely no
 * validation: it expects a valid XML file, and one suited to this parser.
 * Don't expect every XML file to be parsed by this class without a hitch.
 *
 * reader	The input file reader -- this should point to some sort of
 *			XML file somewhere.
 * callback->BeginTagFunc() The callback function for whenever a beginning
 *			tag is found.  This will be given two parameters: a tag
 *			parameter, defining which tag is beginning, and an Attribute
 *			paramater, defining all the other info inside the tag such
 *			as 'height=20' and so on.  This is stored in an STL map to
 *			associate the keys and values.  In the above example
 *			attrs["height"] would yeild string("20").
 * callback->EndTagFunc() The callback for whenever and end tag is found.
 *			Note that this is called for those special XML single tags
 *			'<example height=20 />' after the corresponding 
 *			begin_tag_func.  Also, note that the end tag name wont
 *			necessarily match up with the begin tag name if the XML
 *			file is incorrect.
 */
void CXMLParser::Parse(CReader *reader, IParserCallback *callback)
{
	int file_len = 0, counter = 0;
	char c;
	string contents, tag;
	list<string> contents_stack, tag_stack;
	enum { InTag, InNormal, Out } state = Out;

	file_len = reader->GetLength();

	/* HACK for now: ignore first line. This will be the annoying
	  <?xml version="1.0"> line that appears in all XML documents. */
	while(counter != file_len)
	{
		reader->Read(&c, 1);
		counter++;

		if(c == '\n')
			break;
	}

	while(counter != file_len)
	{
		reader->Read(&c, 1);
		counter++;

		switch(c)
		{
		case '<':
			state = InTag;
			break;
		case '>':
		{
			if(state == Out || state == InNormal)
			{
				// Error
				throw CException(CReporter::R_FATAL, "Badly formed XML file.");
			}

			Attribute attrs;
			
			if(IsEndTag(tag))
			{
				callback->EndTagFunc(contents);
					
				if(contents_stack.size() > 0)
				{
					contents = *(--contents_stack.end());
					tag_stack.pop_back();
					contents_stack.pop_back();
					tag = "";
				}
				else {
					contents = "";
					tag = "";
				}
			}
			else if(IsSingleTag(tag))
			{
				FindAttrs(tag, attrs);
				callback->BeginTagFunc(tag, attrs);
				callback->EndTagFunc(string("single tag"));
				tag = "";
			}
			else
			{
				FindAttrs(tag, attrs);
				callback->BeginTagFunc(tag, attrs);
				tag_stack.push_back(tag);
				contents_stack.push_back(contents);
				contents = "";
				tag = "";
			}

			if(contents_stack.size() > 0)
				state = InNormal;
			else
				state = Out;
			
			break;
		}
		default:
			if(state == Out || state == InNormal)
			{
				contents += c;
			}
			else if(state == InTag)
			{
				tag += c;
			}
		}
	}
}

#ifndef __XML_PARSE_H__
#define __XML_PARSE_H__

#include <map>
#include <algorithm>

/** This struct is an intentional hack so the code will compile. 
 * This is equivalent to what would be the default 3rd argument
 * to map<string, string>.  less<string> should be fine, but
 * MSVC refuses to compile due to compiler bugs.  Also, a
 * forward declaration here for string_less will not work.
 * Again, I can only site compiler bugs because it should be
 * fine to declare the struct here and define it in the cpp file.
 * g++ compiles correctly without the need for this function,
 * this function; however it should still work in g++, as this
 * function merely simplifies things for the compiler by having
 * less template stuff to figure out. - STJ 27/2/02
 */
struct string_less : binary_function<string, string, bool> {
	bool operator()(const string& _X, const string& _Y) const
	{ return ((_X.compare(_Y) < 0) ? true : false); }
};

class CReader;

/** Anything wishing to use the XML parser must implement this
 * interface. */
struct IParserCallback
{
public:
	virtual void BeginTagFunc(const string &tag, const map<string, string, string_less> &attrs) = 0;
	virtual void EndTagFunc(const string &contents) = 0;
};

/** A very, VERY simple XML parser.  It is deliberately simple,
 * as the needs for parsing map files are very limited.  It is
 * also designed so very little memory (dependant only upon the
 * amount of tag nesting present) is used at any one time.  Also
 * speed is a factor: this parser is fairly quick, though I'm
 * sure it could be optimised quite a log more.
 */
class CXMLParser
{
public:
	typedef map<string, string, string_less> Attribute;
	static const string whitespace;

	/** Parses the entire xml file given, calling begin_tag_func
	 * and end_tag_func when necessary. */
	void Parse(
		CReader *reader,
		IParserCallback *callback
			);
	
private:
	void FindAttrs(string &tag, Attribute &attrs);
	void DoKeyValue(string &current, string &key, string &value,
		Attribute &attrs);
	bool IsSingleTag(string tag);
	bool IsWhiteSpace(const char c);
	bool IsEndTag(string tag);
	string StripWhiteSpace(const string &str);
};

#endif


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
	bool operator()(const string& bsod_X, const string& bsod_Y) const
	{ return ((bsod_X.compare(bsod_Y) < 0) ? true : false); }
};

class CReader;

/** Anything wishing to use the XML parser must implement this
 * interface. */
struct IParserCallback
{
public:
	virtual void BeginTagFunc(const string &tag, const map<string, string, string_less> &attrs) = 0;
	virtual void EndTagFunc(const string &contents) = 0;
	virtual ~IParserCallback() {};
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


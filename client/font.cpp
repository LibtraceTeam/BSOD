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
#include "font.h"
#include "vfs.h"
#include "texture_manager.h"
#include "reporter.h"
#include "exception.h"

CFont::CFont()
	: max_char(0), min_char(0), italic_start(0), italic_end(0)
	, bold_start(0), bold_end(0)
	, cur_italic(false), cur_bold(false), start(0), tex(0)
{
}

void CFont::Load(const string &fontName)
{
	auto_ptr<CReader> reader(CVFS::LoadFile(fontName + ".xml"));

	CXMLParser parser;
	parser.Parse(reader.get(), this);

	// TODO: Hmm, shouldn't assume .png here
	tex = CTextureManager::tm.LoadTexture(fontName + string(".png"));
}

/* void ReportAttr(pair<string, string> sam)
{
	CReporter::Report(CReporter::R_WARNING,
		"1: '%s' 2: '%s'", sam.first.c_str(), sam.second.c_str());
} */

void CFont::BeginTagFunc(const string &tag, const map<string, string, string_less> &attrs)
{
	string t = tag;
	//for_each(t.begin(), t.end(), tolower);

	map<string, string, string_less>::const_iterator i;
	//for_each(attrs.begin(), attrs.end(), ReportAttr);

	if(t.find("font") != string::npos)
	{
		i = attrs.find(string("bold_end"));
		if(i == attrs.end())
			throw CException("Jiab Jiab Jiab!");

		bold_end = atoi((*i).second.c_str());

		i = attrs.find("bold_start");
		bold_start = atoi((*i).second.c_str());

		i = attrs.find("max_char");
		max_char = atoi((*i).second.c_str());
		
		i = attrs.find("min_char");
		min_char = atoi((*i).second.c_str());
		
		i = attrs.find("italic_start");
		italic_start = atoi((*i).second.c_str());
		
		i = attrs.find("italic_end");
		italic_end = atoi((*i).second.c_str());
	}
	else if(t.find("glyph") != string::npos)
	{
		GlyphInfo gi;
		
		i = attrs.find("height");
		gi.height = atoi((*i).second.c_str());
		i = attrs.find("width");
		gi.width = atoi((*i).second.c_str());
		
		i = attrs.find("xoff");
		gi.xoff = atoi((*i).second.c_str());
		i = attrs.find("yoff");
		gi.yoff = atoi((*i).second.c_str());

		glyphs.push_back(gi);
	}
}

void CFont::EndTagFunc(const string &contents)
{
}

void CFont::SetBold(bool bold)
{
	cur_bold = bold;
	if(bold)
		start = bold_start;
	else
		start = 0;
}

void CFont::SetItalic(bool italic)
{
	cur_italic = italic;
	if(italic)
		start = italic_start;
	else
		start = 0;
}

uint32 CFont::GetHeight() const
{
	// All characters always have the same height currently.  This isn't really
	// much of a limitation, most fonts work this way.
	return glyphs[0].height;
}

uint32 CFont::MeasureString(const string &str) const
{
	string::const_iterator i = str.begin();
	uint32 length = 0;
	int top, left, width, height;
	for(; i != str.end(); ++i)
	{
		GetGlyph(*i, top, left, width, height);
		length += width;
	}

	return length;
}

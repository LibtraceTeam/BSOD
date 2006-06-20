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
#ifndef _FONT_H_
#define _FONT_H_

#include "xml_parse.h"

#include <assert.h>

class CFont : public IParserCallback
{
private:
	struct GlyphInfo
	{
		int width, height, xoff, yoff;
	};
	vector<GlyphInfo> glyphs;
	
	int max_char, min_char, italic_start, italic_end, bold_start, bold_end;
	bool cur_italic, cur_bold;
	int start;

public:
	CFont();
	virtual ~CFont() {}
	
	void Load(const string &fontName);
	void GetGlyph(const char a, int &top, int &left, int &width, int &height) const
	{
		if (a<min_char) {
			top=left=height=width=0;
			return;
		}
		int s   = a - min_char + start;
		assert(s>=0);
		assert(s<glyphs.size());
		assert(a<=max_char);
		assert(a>=min_char);
		top	= glyphs[s].yoff;
		left	= glyphs[s].xoff;
		height	= glyphs[s].height;
		width	= glyphs[s].width;
	}
	void SetItalic(bool italic);
	void SetBold(bool bold);
	uint32 MeasureString(const string &str) const;
	uint32 GetHeight() const;

	// IParserCallback implementation:
	virtual void BeginTagFunc(const string &tag, const map<string, string, string_less> &attrs);
	virtual void EndTagFunc(const string &contents);
	
	CTexture *tex;
};

#endif


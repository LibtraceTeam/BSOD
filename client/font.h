/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef _FONT_H_
#define _FONT_H_

#include "xml_parse.h"

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
		int s   = a - min_char + start;
		top		= glyphs[s].yoff;
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


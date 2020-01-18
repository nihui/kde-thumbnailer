%{
/*
    rtf.ll  -  A simple RTF Parser (Flex code)

    Copyright (c) 2002 by Vladimir Shutoff   <vovan@shutoff.ru>    (original code)
    Copyright (c) 2004 by Thiago S. Barcelos <barcelos@ime.usp.br> (Kopete port)
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************

update rtf.cc:
flex -olex.yy.c  `test -f rtf.ll || echo './'`rtf.ll
sed '/^#/ s|lex.yy\.c|rtf.cc|' lex.yy.c >rtf.cc
rm -f lex.yy.c

*/

#define UP				1
#define DOWN			2
#define CMD				3
#define TXT				4
#define HEX				5
#define IMG				6
#define UNICODE_CHAR	7
#define SKIP			8
#define SLASH			9
#define S_TXT			10
#define URL				11
#define SMILE			12

//#define YY_NEVER_INTERACTIVE	1
//#define YY_ALWAYS_INTERACTIVE	0
//#define YY_MAIN			0

%}

%option nounput
%option nostack
%option prefix="rtf"
%option noyywrap

%%

"{"								{ return UP; }
"}"								{ return DOWN; }
"<SMILE>"[ ]*"id"[ ]*"="[ ]*[0-9]+[^<]*"</SMILE>"	{ return SMILE; }
"<smile>"[ ]*"id"[ ]*"="[ ]*[0-9]+[^<]*"</smile>"	{ return SMILE; }
"\\"[\\\{\}]					{ return SLASH; }
"\\u"[0-9]{3,7}[ ]?"?"			{ return UNICODE_CHAR; }
"\\"[A-Za-z]+[0-9]*[ ]? 		{ return CMD; }
"\\'"[0-9A-Fa-f][0-9A-Fa-f]		{ return HEX; }
"<##"[^>]+">"					{ return IMG; }
"http://"[^ \r\n\t\\]+			{ return URL; }
[^ \\{}\r\n\t]+					{ return TXT; }
[ \t]+							{ return TXT; }
.								{ return S_TXT; }
%%

#include "rtfparser.h"

#include <QDebug>
#include <QTextCodec>
#include <QTextCharFormat>
#include <QUrl>


RtfLevel::RtfLevel()
{
}

RtfLevel::RtfLevel(RtfParser* p) :
	parser(p),
	m_bFontTable(false),
	m_bColors(false),
	m_nFont(0),
	m_nEncoding(0)
{
	init();
}

RtfLevel::RtfLevel(const RtfLevel &l) :
	parser(l.parser),
	previousCharFormat(l.previousCharFormat),
	m_bFontTable(l.m_bFontTable),
	m_bColors(l.m_bColors),
	m_nFont(l.m_nFont),
	m_nEncoding(l.m_nEncoding)
	
{
	init();
}

RtfLevel::~RtfLevel()
{
	if (m_textSet)
		m_html += finishTags();
}

void RtfLevel::init()
{
	m_nFontColor = 0;
	m_nBackgroundColor = 0;
	m_nFontBgColor = 0;
	m_nFontSize = 0;
	m_bBold = false;
	m_bItalic = false;
	m_bUnderline = false;

	m_isUrlTag = false;
	m_isUrlEditing = false;
	m_isFontTag = false;
	m_isFontEditing = false;
	m_isStyleAttribute = false;

	m_textSet = false;

	codec = QTextCodec::codecForName("cp1251");
}

void RtfLevel::setFont(int nFont)
{
	if (nFont <= 0)
		return;

	if (m_bFontTable)
	{
		if (nFont > parser->fonts.size() + 1)
		{
			qDebug() << "Invalid font index (" << nFont << ") while parsing font table";
			return;
		}
		
		if (nFont > parser->fonts.size())
		{
			FontDef f;
			f.charset = 0;
			parser->fonts.push_back(f);
		}
		
		m_nFont = nFont;
	}
	else
	{
		if (nFont > parser->fonts.size())
		{
			qDebug() << "Invalid font index (" << nFont << ")";
			return;
		}
		
		if (m_nFont == nFont)
			return;
		
		m_nFont = nFont;
		m_nEncoding = parser->fonts[nFont-1].charset;
		
		setFont(QString(parser->fonts[nFont-1].name));
	}
}

void RtfLevel::setFont(QString fontFamily)
{
	if (parser->m_type == 0)
	{
		QTextCharFormat fmt;
		//qDebug() << "font name = " << parser->fonts[nFont-1].name;
		fmt.setFontFamily(fontFamily);
		parser->cursor.mergeCharFormat(fmt);
	}
	else if (parser->m_type == 1)
		m_html += openFontTag() + openStyleAttribute() + "font-family: " + fontFamily + QString(";");
}

void RtfLevel::setText(const char* str)
{
	if (m_bColors)
	{
		if (m_bColorInit)
		{
			QColor c(m_nRed, m_nGreen, m_nBlue);
			parser->colors.push_back(c);
			resetColors();
		}
	}
	else if (m_bFontTable)
	{
		if ((m_nFont <= 0) || (m_nFont > parser->fonts.size()))
			return;

		//qDebug() << "setting font = " << str;

		FontDef& def = parser->fonts[m_nFont-1];

		const char* pp = strchr(str, ';');
		unsigned size;
		if (pp != NULL)
		{
			size = (pp - str);
			def.name = m_fontName + QByteArray(str, size);
			m_fontName.clear();
		}
		else
		{
			size = strlen(str);
			m_fontName += QByteArray(str, size);
		}
	}
	else
	{
		QString text = codec->toUnicode(str);
		//qDebug() << "insert text " << text;
		if (parser->m_type == 0)
			parser->cursor.insertText(text);
		else if (parser->m_type == 1)
		{
			m_html += closeTags() + text;
			m_textSet = true;
			parser->m_html += text;
		}
	}
}

void RtfLevel::setUrl(const char* str)
{
	QString text = codec->toUnicode(str);
	QUrl url(text);
	if (!url.isValid())
	{
		qDebug() << "url is invalid";
		setText(str);
		return;
	}
	if (parser->m_type == 0)
	{
		QTextCharFormat cf = parser->cursor.charFormat();
		QTextCharFormat fmt = cf;
		fmt.setAnchorHref(text);
		fmt.setAnchor(true);
		fmt.setForeground(Qt::blue);
		fmt.setFontUnderline(true);
		parser->cursor.setCharFormat(fmt);
		parser->cursor.insertText(text);
		parser->cursor.setCharFormat(cf);
	}
	else if (parser->m_type == 1)
	{
		m_html += finishUrlTag() + closeTags() + "<a href=\"" + text + "\">" + text + "</a>";
		m_textSet = true;
		parser->m_html += "<a href=\"" + text + "\">" + text + "</a>";
/*		m_isUrlTag = true;
		m_isUrlEditing = true;*/
	}
}

void RtfLevel::startParagraph()
{
	if (parser->m_type == 0)
		parser->cursor.insertBlock();
}

void RtfLevel::setItalic(bool b)
{
	if (parser->m_type == 0)
	{
		QTextCharFormat fmt;
		fmt.setFontItalic(b);
		parser->cursor.mergeCharFormat(fmt);
	}
	else if (parser->m_type == 1)
	{
		if (b)
			m_html += closeTags() + "<i>";
		else if (m_bItalic)
			m_html += closeTags() + "</i>";
		m_bItalic = b;
	}
}

void RtfLevel::setBold(bool b)
{
	if (parser->m_type == 0)
	{
		QTextCharFormat fmt;
		fmt.setFontWeight(b? QFont::Bold : QFont::Normal);
		parser->cursor.mergeCharFormat(fmt);
	}
	else if (parser->m_type == 1)
	{
		if (b)
			m_html += closeTags() + "<b>";
		else if (m_bBold)
			m_html += closeTags() + "</b>";
		m_bBold = b;
	}
}

void RtfLevel::setUnderline(bool b)
{
	if (parser->m_type == 0)
	{
		QTextCharFormat fmt;
		fmt.setFontUnderline(b);
		parser->cursor.mergeCharFormat(fmt);
	}
	else if (parser->m_type == 1)
	{
		if (b)
			m_html += closeTags() + "<u>";
		else if (m_bUnderline)
			m_html += closeTags() + "</u>";
		m_bUnderline = b;
	}
}

void RtfLevel::setFontColor(unsigned short nColor)
{
	if (m_nFontColor == nColor) return;
	
	//qDebug() << "RtfLevel::setFontColor, nColor = " << nColor;
	
	if (nColor > parser->colors.size()) return;
	m_nFontColor = nColor;
	QColor c;
	if (m_nFontColor)
		c = parser->colors[m_nFontColor-1];
	
	//qDebug() << "color = " << c;

	if (parser->m_type == 0)
	{
		QTextCharFormat fmt;
		fmt.setForeground(c);
		parser->cursor.mergeCharFormat(fmt);
	}
	else if (parser->m_type == 1)
	{
		m_html += openStyleAttribute() + "color: rgb(" + QString::number(c.red()) + ", " + QString::number(c.green()) + ", " + QString::number(c.blue()) + QString(");");
	}
}

void RtfLevel::setBackgroundColor(unsigned short nColor)
{
	if (m_nBackgroundColor == nColor) return;
	
	//qDebug() << "RtfLevel::setFontColor, nColor = " << nColor;
	
	if (nColor > parser->colors.size()) return;
	m_nBackgroundColor = nColor;
	QColor c;
	if (m_nBackgroundColor)
		c = parser->colors[m_nBackgroundColor-1];
	
	//qDebug() << "color = " << c;

	if (parser->m_type == 0)
	{
		QTextCharFormat fmt;
		fmt.setBackground(c);
		parser->cursor.mergeCharFormat(fmt);
	}
}

void RtfLevel::setFontSizeHalfPoints(unsigned short sizeInHalfPoints)
{
	if (parser->m_type == 0)
	{
		QTextCharFormat fmt;
		fmt.setFontPointSize(sizeInHalfPoints/2);
		parser->cursor.mergeCharFormat(fmt);
	}
	else if (parser->m_type == 1)
	{
		if (m_nFontSize != sizeInHalfPoints)
			m_html += openStyleAttribute() + "font-size: " + QString::number(sizeInHalfPoints + 10) + QString("pt;");
		m_nFontSize = sizeInHalfPoints;
	}
}

void RtfLevel::storeCharFormat()
{
	if (parser->m_type == 0)
		previousCharFormat = parser->cursor.charFormat();
}

void RtfLevel::restoreCharFormat()
{
	if (parser->m_type == 0)
		parser->cursor.setCharFormat(previousCharFormat);
}

QString RtfLevel::openFontTag()
{
	if (!m_isFontTag)
	{
		QString res = finishTags();
		m_isFontTag = true;
		m_isFontEditing = true;
		return res + "<font";
	}
	else
		return "";
}

QString RtfLevel::closeFontTag()
{
	if (m_isFontTag)
	{
		m_isFontTag = false;
		return closeStyleAttribute() + ">";
	}
	else
		return "";
}

QString RtfLevel::openStyleAttribute()
{
	if (!m_isStyleAttribute)
	{
		m_isStyleAttribute = true;
		return openFontTag() + " style=\"";
	}
	else
		return "";
}

QString RtfLevel::closeStyleAttribute()
{
	if (m_isStyleAttribute)
	{
		m_isStyleAttribute = false;
		return QString("\"");
	}
	else
		return "";
}

QString RtfLevel::finishFontTag()
{
	if (m_isFontEditing)
	{
		m_isFontEditing = false;
		return closeFontTag() + "</font>";
	}
	else
		return "";
}

QString RtfLevel::finishUrlTag()
{
	if (m_isUrlEditing)
	{
		m_isUrlEditing = false;
		return closeUrlTag() + "</a>";
	}
	else
		return "";
}

QString RtfLevel::finishTags()
{
	return finishUrlTag() + finishFontTag();
}

QString RtfLevel::closeTags()
{
	if (m_isFontTag || m_isUrlTag)
	{
		m_isFontTag = false;
		m_isUrlTag = false;
		return closeStyleAttribute() + ">";
	}
	else
		return "";
}

QString RtfLevel::closeUrlTag()
{
	if (m_isUrlTag)
	{
		m_isUrlTag = false;
		return QString(">");
	}
	else
		return "";
}


RtfParser::RtfParser()
	: curLevel(this)
{
}

RtfParser::~RtfParser()
{
}

const unsigned FONTTBL		= 0;
const unsigned COLORTBL		= 1;
const unsigned RED			= 2;
const unsigned GREEN		= 3;
const unsigned BLUE			= 4;
const unsigned CF			= 5;
const unsigned FS			= 6;
const unsigned HIGHLIGHT	= 7;
const unsigned PARD			= 8;
const unsigned PAR			= 9;
const unsigned I			= 10;
const unsigned B			= 11;
const unsigned UL			= 12;
const unsigned F			= 13;
const unsigned FCHARSET		= 14;
const unsigned FNAME		= 15;
const unsigned ULNONE		= 16;
const unsigned LTRPAR		= 17;
const unsigned RTLPAR		= 18;
const unsigned LINE			= 19;
const unsigned CB			= 20;

static char cmds[] =
    "fonttbl\x00"
    "colortbl\x00"
    "red\x00"
    "green\x00"
    "blue\x00"
    "cf\x00"
    "fs\x00"
    "highlight\x00"
    "pard\x00"
    "par\x00"
    "i\x00"
    "b\x00"
    "ul\x00"
    "f\x00"
    "fcharset\x00"
    "fname\x00"
    "ulnone\x00"
    "ltrpar\x00"
    "rtlpar\x00"
    "line\x00"
    "cb\x00"
    "\x00";

static char h2d(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return (c - 'A') + 10;
    if ((c >= 'a') && (c <= 'f'))
        return (c - 'a') + 10;
    return 0;
}


void RtfParser::parse(QByteArray rtf, int defR, int defG, int defB, int defSize, QString fontFamily)
{
	if (m_type == 0)
		cursor = QTextCursor(m_doc);
	
	YY_BUFFER_STATE yy_current_buffer = yy_scan_bytes(rtf.data(), rtf.size());
	for (;;)
	{
		int res = yylex();
		if (!res) break;

		/*if (defR > -1)
			curLevel.setRed(defR);
		if (defG > -1)
			curLevel.setRed(defG);
		if (defB > -1)
			curLevel.setRed(defB);*/

		if (defSize > -1)
			curLevel.setFontSizeHalfPoints(defSize * 2);

		switch (res)
		{
		case UP:
			curLevel.storeCharFormat();
			levels.push(curLevel);
			break;
		case DOWN:
			if (!levels.empty())
			{
				curLevel = levels.top();
				levels.pop();
				curLevel.restoreCharFormat();
			}
			break;
		case HEX:
			{
				char s[2];
				s[0] = (h2d(yytext[2]) << 4) + h2d(yytext[3]);
				s[1] = 0;
				curLevel.setText(s);
				break;
			}
		case UNICODE_CHAR:
			{
				if (m_type == 0)
					cursor.insertText(QChar((unsigned short)(atol(yytext + 2))));
				else if (m_type == 1)
					m_html += QChar((unsigned short)(atol(yytext + 2)));
				break;
			}
		case SLASH:
			curLevel.setText(yytext+1);
			break;
		case TXT:
			curLevel.setText(yytext);
			break;
		case URL:
			curLevel.setUrl(yytext);
			break;
		case CMD:
			{
				const char* cmd = yytext + 1;
				unsigned n_cmd = 0;
				unsigned cmd_size = 0;
				int cmd_value = -1;
				const char* p;
				for (p = cmd; *p; p++, cmd_size++)
					if (((*p >= '0') && (*p <= '9')) || (*p == ' ')) break;
				if (*p && (*p != ' ')) cmd_value = atol(p);
				for (p = cmds; *p; p += strlen(p) + 1, n_cmd++)
				{
					if (strlen(p) >  cmd_size) continue;
					if (!memcmp(p, cmd, cmd_size)) break;
				}
				cmd += strlen(p);
				
				switch (n_cmd)
				{
				case FONTTBL:
					curLevel.setFontTbl();
					break;
				case F:
					// RTF fonts are 0-based; our font index is 1-based.
					if (fontFamily == "")
						curLevel.setFont(cmd_value+1);
					else
						curLevel.setFont(fontFamily);
					break;
				case FS:
					if (defSize < 0)
						curLevel.setFontSizeHalfPoints(cmd_value);
					break;
				case COLORTBL:
					curLevel.setColors();
					break;
				case RED:
					if (defR < 0)
						curLevel.setRed(cmd_value);
					else
						curLevel.setRed(defR);
					break;
				case GREEN:
					if (defG < 0)
						curLevel.setGreen(cmd_value);
					else
						curLevel.setGreen(defG);
					break;
				case BLUE:
					if (defB < 0)
						curLevel.setBlue(cmd_value);
					else
						curLevel.setBlue(defB);
					break;
				case CF:
					curLevel.setFontColor(cmd_value);
					break;
				case CB:
					curLevel.setBackgroundColor(cmd_value);
					break;
				case PAR:
					curLevel.startParagraph();
					break;
				case I:
					curLevel.setItalic(cmd_value != 0);
					break;
				case B:
					curLevel.setBold(cmd_value != 0);
					break;
				case UL:
					curLevel.setUnderline(cmd_value != 0);
					break;
				case ULNONE:
					curLevel.setUnderline(false);
					break;
				}
			}
			break;
		}
	}
	yy_delete_buffer(yy_current_buffer);
	yy_current_buffer = NULL;
	
	levels.clear();
}

void RtfParser::parseToTextDocument(QByteArray rtf, QTextDocument* doc, int defR, int defG, int defB, int defSize, QString fontFamily)
{
	m_type = 0;
	m_doc = doc;
	parse(rtf, defR, defG, defB, defSize, fontFamily);
}

void RtfParser::parseToHTML(QByteArray rtf, QString& html)
{
	m_type = 1;
	parse(rtf);
	html = m_html;
}

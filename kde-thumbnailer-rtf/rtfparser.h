/***************************************************************************
 *   Copyright (C) 2008 by Alexander Volkov                                *
 *   volkov0aa@gmail.com                                                   *
 *                                                                         *
 *   This file is part of instant messenger MyAgent-IM                     *
 *                                                                         *
 *   MyAgent-IM is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   MyAgent-IM is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef RTFPARSER_H
#define RTFPARSER_H

#include <QByteArray>
#include <QStack>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QPointer>

class QTextDocument;
class QTextCodec;

struct FontDef
{
	int charset;
	QByteArray name;
};

class RtfParser;

class RtfLevel
{
public:
	RtfLevel();
	RtfLevel(RtfParser* p);
	RtfLevel(const RtfLevel&);
	~RtfLevel();

	void setFontTbl() { m_bFontTable = true; }
	void setColors() { m_bColors = true; resetColors(); }
	void setRed(unsigned char val) { setColor(val, &m_nRed); }
	void setGreen(unsigned char val) { setColor(val, &m_nGreen); }
	void setBlue(unsigned char val) { setColor(val, &m_nBlue); }
	void setText(const char* str);
	void setUrl(const char* str);
	void setFont(int nFont);
	void setFont(QString fontFamily);
	void setFontColor(unsigned short color);
	void setBackgroundColor(unsigned short color);
	void setFontSizeHalfPoints(unsigned short sizeInHalfPoints);
	void setBold(bool);
	void setItalic(bool);
	void setUnderline(bool);
	void startParagraph();
	bool isParagraphOpen() const;
	void clearParagraphFormatting();
	void storeCharFormat();
	void restoreCharFormat();

private:
	RtfParser* parser;
	QTextCharFormat previousCharFormat;
	QTextCodec* codec;	

	void init();
	void resetColors() { m_nRed = m_nGreen = m_nBlue = 0; m_bColorInit = false; }
	void setColor(unsigned char val, unsigned char *p)
	{ *p = val; m_bColorInit = true; }

	QString openFontTag();
	QString openStyleAttribute();
	QString closeFontTag();
	QString closeStyleAttribute();
	QString finishFontTag();
	QString finishUrlTag();
	QString finishTags();
	QString closeTags();
	QString closeUrlTag();

// True when parsing the fonts table
	bool m_bFontTable;
// True when parsing the colors table.
	bool m_bColors;

	unsigned char m_nRed;
	unsigned char m_nGreen;
	unsigned char m_nBlue;
	bool m_bColorInit;
	int m_nFont; // 1-based
	int m_nEncoding;
	int m_nFontColor; // 1-based
	int m_nBackgroundColor; // 1-based
	int m_nFontSize;
	int m_nFontBgColor; // 1-based
	QByteArray m_fontName;
	bool m_bBold;
	bool m_bItalic;
	bool m_bUnderline;

	bool m_isUrlTag;
	bool m_isUrlEditing;
	bool m_isFontTag;
	bool m_isFontEditing;
	bool m_isStyleAttribute;

	bool m_textSet;

	QString m_html;
};

class RtfParser
{
	friend class RtfLevel;
public:
	RtfParser();
	~RtfParser();
	
	void parse(QByteArray rtf, int defR = -1, int defG = -1, int defB = -1, int defSize = 1, QString fontFamily = "");
	void parseToTextDocument(QByteArray rtf, QTextDocument* doc, int defR = -1, int defG = -1, int defB = -1, int defSize = -1, QString fontFamily = "");
	void parseToHTML(QByteArray rtf, QString& html);
	
private:	
	// Fonts table.
	QVector<FontDef> fonts;
    // Colors table.
	QVector<QColor> colors;
	
	QTextCursor cursor;
	
	RtfLevel curLevel;
	QStack<RtfLevel> levels;

	QTextDocument* m_doc;
	QString m_html;

	int m_type;
};

#endif

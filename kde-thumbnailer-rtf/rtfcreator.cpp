/*
 *  This file is part of kde-thumbnailer-rtf
 *  Copyright (C) 2012 Ni Hui <shuizhuyuanluo@126.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rtfcreator.h"

#include <QAbstractTextDocumentLayout>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QTextDocument>
#include <kdemacros.h>

#include "rtfparser.h"

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new RTFCreator;
    }
}

RTFCreator::RTFCreator()
{
}

RTFCreator::~RTFCreator()
{
}

bool RTFCreator::create( const QString& path, int width, int height, QImage& img )
{
    QFile in( path );
    in.open( QIODevice::ReadOnly );

    RtfParser c;
    QTextDocument doc;
    c.parseToTextDocument( in.readAll(), &doc );

    QSize size = doc.documentLayout()->documentSize().toSize();
    if ( size.width() > width ) {
        doc.setTextWidth( width );
    }

    QImage image( doc.documentLayout()->documentSize().toSize(), QImage::Format_RGB32 );
    image.fill( Qt::white );
    QPainter painter( &image );

    QAbstractTextDocumentLayout::PaintContext ctx;
    doc.documentLayout()->draw( &painter, ctx );

    if ( image.height() > height )
        img = image.copy( 0, 0, image.width(), (double)height / width * image.width() );
    else
        img = image;

    return true;
}

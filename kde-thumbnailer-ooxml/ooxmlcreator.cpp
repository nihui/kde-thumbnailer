/*
 *  This file is part of kde-thumbnailer-ooxml
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

#include "ooxmlcreator.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <KZip>
#include <KZipFileEntry>
#include <kdemacros.h>

#include "libkowmf/WmfPainterBackend.h"

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new OOXmlCreator;
    }
}

OOXmlCreator::OOXmlCreator()
{
}

OOXmlCreator::~OOXmlCreator()
{
}

bool OOXmlCreator::create( const QString& path, int width, int height, QImage& img )
{
    KZip zip( path );
    if ( !zip.open( QIODevice::ReadOnly ) )
        return false;

    const KArchiveEntry* relsEntry = zip.directory()->entry( "_rels/.rels" );
    const KZipFileEntry* relsFile = static_cast<const KZipFileEntry*>(relsEntry);

    if ( !relsFile )
        return false;

    QDomDocument doc;
    doc.setContent( relsFile->data() );

    QString type( "http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail" );
    QString target;

    QDomElement docElem = doc.firstChildElement( "Relationships" );
    QDomElement rel = docElem.firstChildElement( "Relationship" );
    while ( !rel.isNull() ) {
        if ( rel.attribute( "Type" ) == type ) {
            target = rel.attribute( "Target" );
            break;
        }
        rel = rel.nextSiblingElement( "Relationship" );
    }

    if ( target.isEmpty() )
        return false;

    const KArchiveEntry* imageEntry = zip.directory()->entry( target );
    const KZipFileEntry* imageFile = static_cast<const KZipFileEntry*>(imageEntry);

    if ( !imageFile )
        return false;

    if ( target.endsWith( ".wmf" ) ) {
        QImage image( width, height, QImage::Format_ARGB32_Premultiplied );
        image.fill( Qt::transparent );
        QPainter p(&image);
        Libwmf::WmfPainterBackend wmf( &p, image.size() );
        if ( !wmf.load( imageFile->data() ) ) {
            return false;
        }
        wmf.play();

        QSize ts = wmf.boundingRect().size();
        ts.scale( width, height, Qt::KeepAspectRatio );
        img = image.scaled( ts );
    }
    else {
        img.loadFromData( imageFile->data() );
    }

    return true;
}

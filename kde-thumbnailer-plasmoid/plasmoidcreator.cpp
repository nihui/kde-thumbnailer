/*
 *  This file is part of kde-thumbnailer-plasmoid
 *  Copyright (C) 2011-2012 Ni Hui <shuizhuyuanluo@126.com>
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

#include "plasmoidcreator.h"

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QTextStream>
#include <KIconLoader>
#include <KZip>
#include <KZipFileEntry>
#include <kdemacros.h>

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new PlasmoidCreator;
    }
}

PlasmoidCreator::PlasmoidCreator()
{
}

PlasmoidCreator::~PlasmoidCreator()
{
}

bool PlasmoidCreator::create( const QString& path, int width, int height, QImage& img )
{
    KZip zip( path );
    if ( !zip.open( QIODevice::ReadOnly ) )
        return false;

    const KArchiveEntry* entry = zip.directory()->entry( "metadata.desktop" );
    const KZipFileEntry* metadataFile = static_cast<const KZipFileEntry*>(entry);

    if ( !metadataFile )
        return false;

    QByteArray data = metadataFile->data();
    QTextStream ss( data );
    QString line;
    QString iconName;
    do {
        line = ss.readLine();
        if ( line.startsWith( "Icon=" ) ) {
            iconName = line.mid( 5 );
            break;
        }
    }
    while ( !line.isNull() );

    if ( iconName.isEmpty() )
        return false;

    int size = qMin( width, height );
    QImage pix( size, size, QImage::Format_ARGB32_Premultiplied );
    pix.fill( Qt::transparent );

    const KArchiveEntry* iconEntry = zip.directory()->entry( iconName );
    const KZipFileEntry* iconFile = static_cast<const KZipFileEntry*>(iconEntry);

    if ( iconFile ) {
        if ( iconName.endsWith( ".svg" ) || iconName.endsWith( ".svgz" ) ) {
            //
            QPainter p( &pix );

            QSvgRenderer r( iconFile->data() );
            r.render( &p );
        }
        else {
            //
            pix.loadFromData( iconFile->data() );
        }
    }
    else {
        pix = KIconLoader::global()->loadIcon( iconName, KIconLoader::Desktop, size,
                                               KIconLoader::DefaultState, QStringList(), 0,
                                               true ).toImage();
        if ( pix.isNull() )
            return false;
    }

    img = pix;

    return true;
}

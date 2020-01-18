/*
 *  This file is part of kde-thumbnailer-webp
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

#include "webpcreator.h"

#include <stdio.h>
#include <stdlib.h>

#include <webp/decode.h>

#include <QImage>

#include <kdemacros.h>

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new WebpCreator;
    }
}

WebpCreator::WebpCreator()
{
}

WebpCreator::~WebpCreator()
{
}

bool WebpCreator::create( const QString& path, int width, int height, QImage& img )
{
    FILE* in = fopen( path.toLocal8Bit().constData(), "rb" );
    if ( !in ) {
        return false;
    }
    fseek( in, 0, SEEK_END );
    uint32_t data_size = ftell( in );
    fseek( in, 0, SEEK_SET );
    uint8_t* data = (uint8_t*)malloc( data_size );
    int ok = ( fread( data, data_size, 1, in ) == 1 );
    fclose( in );
    if ( !ok ) {
        free( data );
        return false;
    }

    int w, h;
    if ( !WebPGetInfo( data, data_size, &w, &h ) ) {
        free( data );
        return false;
    }

    QImage outimg( w, h, QImage::Format_RGB888 );
    uint8_t* bits = WebPDecodeRGBInto( data, data_size, outimg.bits(), outimg.byteCount(), outimg.bytesPerLine() );
    free( data );

    if ( bits != outimg.bits() ) {
        return false;
    }

    if ( width < w || height < h )
        img = outimg.scaled( width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    else
        img = outimg;

    return true;
}

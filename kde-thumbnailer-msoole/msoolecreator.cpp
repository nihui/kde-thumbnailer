/*
 *  This file is part of kde-thumbnailer-msoole
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

#include "msoolecreator.h"

#include <gsf/gsf.h>
#include <gsf/gsf-clip-data.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-meta-names.h>
#include <gsf/gsf-msole-utils.h>
#include <gsf/gsf-utils.h>

#include <QByteArray>
#include <QImage>
#include <QString>

#include <kdemacros.h>

#include "libkowmf/WmfPainterBackend.h"

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new MSOOLECreator;
    }
}

static bool load_dib( const void* data, int size, QImage& img )
{
    typedef struct _BMPFILEHEADER {
        char    bfType[2];
        qint32  bfSize;
        qint16  bfReserved1;
        qint16  bfReserved2;
        qint32  bfOffBits;
    } BMPFILEHEADER;
    const int BMPFILEHEADERSIZE = 14;
    typedef struct _BMPINFOHEADER {
        qint32  biSize;
        qint32  biWidth;
        qint32  biHeight;
        qint16  biPlanes;
        qint16  biBitCount;
        qint32  biCompression;
        qint32  biSizeImage;
        qint32  biXPelsPerMeter;
        qint32  biYPelsPerMeter;
        qint32  biClrUsed;
        qint32  biClrImportant;
    } BMPINFOHEADER;

    BMPINFOHEADER* infohdr = (BMPINFOHEADER*)data;

    BMPFILEHEADER filehdr;
    memcpy(filehdr.bfType, "BM", 2);
    filehdr.bfReserved1 = 0;
    filehdr.bfReserved2 = 0;
    int colorCount;
    switch (infohdr->biBitCount) {
        case 32:
        case 24:
        case 16:
            colorCount = 0;
            break;
        default:
            colorCount = infohdr->biClrUsed ? infohdr->biClrUsed : 1 << infohdr->biBitCount;
            break;
    }
    filehdr.bfOffBits = BMPFILEHEADERSIZE + infohdr->biSize + colorCount * 4;
    filehdr.bfSize = BMPFILEHEADERSIZE + size;

    QByteArray pattern((const char*)&filehdr, BMPFILEHEADERSIZE);
    pattern.append((const char*)data, size);

    img.loadFromData( pattern, "BMP" );

    return true;
}

static bool load_wmf( const void* data, int size, QImage& img, int width, int height )
{
    QByteArray ba((const char*)data, size);
    QImage image( width, height, QImage::Format_ARGB32_Premultiplied );
    image.fill( Qt::transparent );
    QPainter p(&image);
    Libwmf::WmfPainterBackend wmf( &p, image.size() );
    if ( !wmf.load( ba ) ) {
        return false;
    }
    wmf.play();

    QSize ts = wmf.boundingRect().size();
    ts.scale( width, height, Qt::KeepAspectRatio );
    img = image.scaled( ts );

    return true;
}

MSOOLECreator::MSOOLECreator()
{
    gsf_init();
}

MSOOLECreator::~MSOOLECreator()
{
    gsf_shutdown();
}

bool MSOOLECreator::create( const QString& path, int width, int height, QImage& img )
{
    GsfInput* in = gsf_input_mmap_new( path.toUtf8().constData(), NULL );
    in = gsf_input_uncompress( in );
    GsfInfile* infile = gsf_infile_msole_new( in, NULL );

    GsfInput* stream = gsf_infile_child_by_name( infile, "\05SummaryInformation" );
    if ( !stream ) {
        g_object_unref( infile );
        g_object_unref( in );
        return false;
    }

    GsfDocMetaData* metadata = gsf_doc_meta_data_new();
    if ( gsf_msole_metadata_read( stream, metadata ) ) {
        g_object_unref( metadata );
        g_object_unref( stream );
        g_object_unref( infile );
        g_object_unref( in );
        return false;
    }

    GsfDocProp* prop = gsf_doc_meta_data_lookup( metadata, GSF_META_NAME_THUMBNAIL );
    if ( !prop ) {
        g_object_unref( metadata );
        g_object_unref( stream );
        g_object_unref( infile );
        g_object_unref( in );
        return false;
    }

    const GValue* propvalue = gsf_doc_prop_get_val( prop );
    if ( !propvalue ) {
        g_object_unref( metadata );
        g_object_unref( stream );
        g_object_unref( infile );
        g_object_unref( in );
        return false;
    }

    GsfClipData* cd = GSF_CLIP_DATA(g_value_get_object( propvalue ));

    bool ok = false;

    GsfClipFormat format = gsf_clip_data_get_format( cd );
    if ( format == GSF_CLIP_FORMAT_WINDOWS_CLIPBOARD ) {
        gsize size;
        gconstpointer dp = gsf_clip_data_peek_real_data( cd, &size, NULL );

        GsfClipFormatWindows fw = gsf_clip_data_get_windows_clipboard_format( cd, NULL );
        switch ( fw ) {
            case GSF_CLIP_FORMAT_WINDOWS_METAFILE:
                ok = load_wmf( dp, size, img, width, height );
                break;
            case GSF_CLIP_FORMAT_WINDOWS_DIB:
                ok = load_dib( dp, size, img );
                break;
            case GSF_CLIP_FORMAT_WINDOWS_ENHANCED_METAFILE:
                break;
            default:
                break;
        }
    }

    g_object_unref( cd );

    g_object_unref( metadata );
    g_object_unref( stream );
    g_object_unref( infile );
    g_object_unref( in );

    return ok;
}

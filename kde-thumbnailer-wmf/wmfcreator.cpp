/*
 *  This file is part of kde-thumbnailer-wmf
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
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

#include "wmfcreator.h"

#include <math.h>

#include <libwmf/api.h>
#include <libwmf/gd.h>

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QVarLengthArray>

#include <kdemacros.h>

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new WmfCreator;
    }
}

/// The WMF-Scalefactor (the lower the faster)
#define SCALE 1.0
/// The average png image size
#define AVGIMGSIZE 30720
static QVarLengthArray<char, AVGIMGSIZE> m_imageData;

static int sink( void* context, char* buffer, int length )
{
    m_imageData.append( buffer, length );
    return m_imageData.count();
}

WmfCreator::WmfCreator()
{
}

WmfCreator::~WmfCreator()
{
}

bool WmfCreator::create( const QString& path, int width, int height, QImage& img )
{
    wmfAPI_Options m_options;
    m_options.function = wmf_gd_function;

    wmfAPI* m_API;
    wmf_gd_t* m_ddata;

    wmfD_Rect bbox;
    wmf_error_t error;

    m_imageData.clear();

    unsigned long flags = WMF_OPT_FUNCTION | WMF_OPT_IGNORE_NONFATAL;
    error = wmf_api_create( &m_API, flags, &m_options );
    if ( error != wmf_E_None ) {
        // is the API pointer now really valid?
        wmf_api_destroy( m_API );
        return false;
    }

    m_ddata = WMF_GD_GetData( m_API );

    if ( ( m_ddata->flags & WMF_GD_SUPPORTS_PNG ) == 0 ) {
        wmf_api_destroy( m_API );
        return false;
    }

    error = wmf_file_open( m_API, path.toLocal8Bit() );
    if ( error != wmf_E_None ) {
        wmf_api_destroy( m_API );
        return false;
    }

    error = wmf_scan( m_API, 0, &bbox );
    if ( error != wmf_E_None ) {
        wmf_file_close( m_API );
        wmf_api_destroy( m_API );
        return false;
    }

    m_ddata->type = wmf_gd_png;
    m_ddata->flags |= WMF_GD_OUTPUT_MEMORY | WMF_GD_OWN_BUFFER;
    m_ddata->sink.function = &sink;
    m_ddata->file = NULL;

    m_ddata->bbox.TL.x = bbox.TL.x * SCALE;
    m_ddata->bbox.TL.y = bbox.TL.y * SCALE;
    m_ddata->bbox.BR.x = bbox.BR.x * SCALE;
    m_ddata->bbox.BR.y = bbox.BR.y * SCALE;

    m_ddata->width  = (unsigned int)ceil(m_ddata->bbox.BR.x - m_ddata->bbox.TL.x);
    m_ddata->height = (unsigned int)ceil(m_ddata->bbox.BR.y - m_ddata->bbox.TL.y);

    error = wmf_play( m_API, 0, &m_ddata->bbox );
    if ( error != wmf_E_None ) {
        wmf_file_close( m_API );
        wmf_api_destroy( m_API );
        return false;
    }

    wmf_file_close( m_API );
    wmf_api_destroy( m_API );

    QByteArray ba( m_imageData.data(), m_imageData.count() );
    img.loadFromData( ba );

    return true;
}

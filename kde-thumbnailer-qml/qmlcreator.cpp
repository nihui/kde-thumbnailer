/*
 *  This file is part of kde-thumbnailer-qml
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

#include "qmlcreator.h"

#include <QDeclarativeError>
#include <QDeclarativeView>
#include <QImage>
#include <QUrl>
#include <kdemacros.h>

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new QMLCreator;
    }
}

QMLCreator::QMLCreator()
{
}

QMLCreator::~QMLCreator()
{
}

bool QMLCreator::create( const QString& path, int width, int height, QImage& img )
{
    QDeclarativeView canvas;
    canvas.setSource( QUrl::fromLocalFile( path ) );

    if (!canvas.errors().isEmpty() || canvas.size().isEmpty()) {
        return false;
    }

    QImage pix( canvas.size(), QImage::Format_ARGB32_Premultiplied );
    canvas.QWidget::render( &pix );

    if ( width < canvas.width() || height < canvas.height() )
        img = pix.scaled( width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    else
        img = pix;

    return true;
}

/*
 *  This file is part of kde-thumbnailer-w3x
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

#include "w3xcreator.h"

#include <stdio.h>
#include <string.h>

#include <libmpq/mpq.h>

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <kdemacros.h>

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new W3xCreator;
    }
}

typedef struct blp_header
{
    char ident[4];
    long compression;
    long flags;
    long width;
    long height;
    long pictype;
    long picsubtype;
    long mipmapoffset[16];
    long mipmapsize[16];
} __attribute__((__packed__)) blp_header_t;

typedef struct tga_header
{
    char  identsize;
    char  colourmaptype;
    char  imagetype;

    short colourmapstart;
    short colourmaplength;
    char  colourmapbits;

    short xstart;
    short ystart;
    short width;
    short height;
    char  bits;
    char  descriptor;
} __attribute__((__packed__)) tga_header_t;

static void load_image_from_blp( const unsigned char* blpdata, off_t blpsize, QImage& img, bool& ok )
{
    const blp_header_t* header = (const blp_header_t*)blpdata;

    if ( strncmp( header->ident, "BLP1", 4 ) != 0 ) {
        ok = false;
    }

    switch ( header->compression ) {
        case 0: {
            /// BLP JPEG
            long offset = header->mipmapoffset[0];
            long size = header->mipmapsize[0];

            long jpegheadersize;
            memcpy( &jpegheadersize, blpdata + sizeof(blp_header_t), sizeof(long) );

            unsigned char* tdata = (unsigned char*)malloc( jpegheadersize + size );
            memcpy( tdata, blpdata + sizeof(blp_header_t) + sizeof(long), jpegheadersize );

            const unsigned char* jpegpixdata = blpdata + offset;
            memcpy( tdata + jpegheadersize, jpegpixdata, size );

            img.loadFromData( tdata, jpegheadersize + size, "JPEG" );

            free( tdata );
            ok = true;
            break;
        }
        case 1: {
            switch ( header->pictype ) {
                case 3:
                case 4: {
                    /// uncompressed 1, alpha channel
                    ok = false;
                    break;
                }
                case 5: {
                    /// uncompressed 2, no alpha channel
                    ok = false;
                    break;
                }
                default: {
//                     printf( "unsupported pictype\n" );
                    ok = false;
                    break;
                }
            }
            break;
        }
        default: {
//             printf( "unsupported compression\n" );
            ok = false;
            break;
        }
    }
}

W3xCreator::W3xCreator()
{
    libmpq__init();
}

W3xCreator::~W3xCreator()
{
    libmpq__shutdown();
}

bool W3xCreator::create( const QString& path, int width, int height, QImage& img )
{
    mpq_archive_s* a;
    if ( libmpq__archive_open( &a, path.toLocal8Bit(), -1 ) != 0 ) {
        return false;
    }

    int war3mapPreview = -1;
    int war3mapMap = -1;
    unsigned int listfile_number;
    if ( libmpq__file_number( a, "(listfile)", &listfile_number ) == 0 ) {
        /// unpack internal listfile
        off_t listfile_size;
        libmpq__file_unpacked_size( a, listfile_number, &listfile_size );

        /// read listfile content into memory
        unsigned char* listfile = (unsigned char*)malloc( listfile_size );
        libmpq__file_read( a, listfile_number, listfile, listfile_size, NULL );
        int i = 0;
        char* filename = strtok( (char*)listfile, "\r\n" );
        while ( filename ) {
            if ( strncmp( filename, "war3mapPreview.tga", 18 ) == 0 )
                war3mapPreview = i;
            if ( strncmp( filename, "war3mapMap.blp", 14 ) == 0 )
                war3mapMap = i;
            filename = strtok( NULL, "\r\n" );
            ++i;
        }
        free( listfile );
    }

    if ( war3mapPreview == -1 && war3mapMap == -1 ) {
        /// not found

        /// iterator all files and guess the preview image
        unsigned int total = 0;
        libmpq__archive_files( a, &total );

//         off_t blpmaxsize = 0;
//         off_t tgamaxsize = 0;

        unsigned int i = 0;
        off_t outsize;
        for ( ; i < total; ++i ) {
            /// extract file and guess the magic
            libmpq__file_unpacked_size( a, i, &outsize );
            if ( outsize < 1000 ) {
                /// skip small files
                continue;
            }

            unsigned char* tmp = (unsigned char*)malloc( outsize );
            libmpq__file_read( a, i, tmp, outsize, NULL );

            /// guess it as blp image
            blp_header_t* blpheader = (blp_header_t*)tmp;
            if ( strncmp( blpheader->ident, "BLP1", 4 ) == 0 ) {
                /// blp1 format
                if ( blpheader->width == 256 && blpheader->height == 256 ) {
//                     printf( "GOT BLP1 %d\n", i );
//                     if ( outsize >= blpmaxsize ) {
//                         blpmaxsize = outsize;
                        war3mapMap = i;
//                     }
                }
            }

            /// guess it as tga image
            tga_header_t* tgaheader = (tga_header_t*)tmp;
            if ( tgaheader->identsize == 0x00
                && tgaheader->imagetype == 0x02
                && tgaheader->xstart == 0
                && tgaheader->ystart == 0
                && tgaheader->width == 256
                && tgaheader->height == 256 ) {
                /// tga format
//                 if ( outsize >= tgamaxsize ) {
//                     tgamaxsize = outsize;
                    war3mapPreview = i;
//                 printf( "GOT TGA %d\n", i );
//                 }
            }

            free( tmp );
        }
    }

    int previewfile_number = ( war3mapPreview != -1 ) ? war3mapPreview : war3mapMap;

//     printf( "preview image number: %d\n", previewfile_number );

    if ( previewfile_number == -1 ) {
        libmpq__archive_close( a );
        return false;
    }

    /// read preview image file
    off_t previewfile_size;
    libmpq__file_unpacked_size( a, previewfile_number, &previewfile_size );
    unsigned char* previewfile = (unsigned char*)malloc( previewfile_size );
    libmpq__file_read( a, previewfile_number, previewfile, previewfile_size, NULL );

    if ( war3mapPreview != -1 )
        img.loadFromData( previewfile, previewfile_size, "TGA" );
    else {
        /// load image data from blp format
        bool ok = false;
        load_image_from_blp( previewfile, previewfile_size, img, ok );
        if ( !ok ) {
            free( previewfile );
            libmpq__archive_close( a );
            return false;
        }
    }
    free( previewfile );

    libmpq__archive_close( a );
    return true;
}

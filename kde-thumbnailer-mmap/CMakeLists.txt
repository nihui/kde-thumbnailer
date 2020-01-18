project(kde-thumbnailer-mmap)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

set(mmapthumbnail_SRCS mmapcreator.cpp)

kde4_add_plugin(mmapthumbnail ${mmapthumbnail_SRCS})
target_link_libraries(mmapthumbnail
    ${KDE4_KIO_LIBS}
)

install(TARGETS mmapthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES mmapthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES mmap.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

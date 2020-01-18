project(kde-thumbnailer-webp)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(KDE4 REQUIRED)
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)
find_package(LIBWEBP REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

set(webpthumbnail_SRCS webpcreator.cpp)

kde4_add_plugin(webpthumbnail ${webpthumbnail_SRCS})
target_link_libraries(webpthumbnail
    ${KDE4_KIO_LIBS}
    ${LIBWEBP_LIBRARY}
)

install(TARGETS webpthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES webpthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES webp.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

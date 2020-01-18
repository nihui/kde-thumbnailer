project(kde-thumbnailer-w3x)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(KDE4 REQUIRED)
set(SHARED_MIME_INFO_MINIMUM_VERSION "0.30")
find_package(SharedMimeInfo REQUIRED)
find_package(LIBMPQ REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

add_definitions(${LIBMPQ_DEFINITIONS})

set(w3xthumbnail_SRCS w3xcreator.cpp)

kde4_add_plugin(w3xthumbnail ${w3xthumbnail_SRCS})
target_link_libraries(w3xthumbnail
    ${KDE4_KIO_LIBS}
    ${LIBMPQ_LIBRARIES}
)

install(TARGETS w3xthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES w3xthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES w3x.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

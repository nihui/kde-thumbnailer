project(kde-thumbnailer-wmf)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(KDE4 REQUIRED)
find_package(Freetype REQUIRED)
find_package(LIBWMF REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

include_directories(${FREETYPE_INCLUDE_DIRS})

set(wmfthumbnail_SRCS wmfcreator.cpp)

kde4_add_plugin(wmfthumbnail ${wmfthumbnail_SRCS})
target_link_libraries(wmfthumbnail
    ${KDE4_KIO_LIBS}
    ${LIBWMF_LIBRARIES}
)

install(TARGETS wmfthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES wmfthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

project(kde-thumbnailer-rtf)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

set(rtfthumbnail_SRCS rtfcreator.cpp rtf.cc)

kde4_add_plugin(rtfthumbnail ${rtfthumbnail_SRCS})
target_link_libraries(rtfthumbnail
    ${KDE4_KIO_LIBS}
)

install(TARGETS rtfthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES rtfthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

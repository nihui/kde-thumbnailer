project(kde-thumbnailer-plasmoid)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

set(plasmoidthumbnail_SRCS plasmoidcreator.cpp)

kde4_add_plugin(plasmoidthumbnail ${plasmoidthumbnail_SRCS})
target_link_libraries(plasmoidthumbnail
    ${KDE4_KIO_LIBS}
)

install(TARGETS plasmoidthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasmoidthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

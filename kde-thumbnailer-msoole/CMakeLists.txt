project(kde-thumbnailer-msoole)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(KDE4 REQUIRED)

find_package(GObject REQUIRED)
find_package(GLIB2 REQUIRED)
find_package(LIBGSF REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

include_directories(
    ${GOBJECT_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIR}
    ${LIBGSF_INCLUDE_DIR}
)

set(libkowmf_SRCS
    libkowmf/WmfAbstractBackend.cpp
    libkowmf/WmfDeviceContext.cpp
    libkowmf/WmfPainterBackend.cpp
    libkowmf/WmfParser.cpp
    libkowmf/WmfStack.cpp
)

set(msoolethumbnail_SRCS msoolecreator.cpp ${libkowmf_SRCS})

kde4_add_plugin(msoolethumbnail ${msoolethumbnail_SRCS})
target_link_libraries(msoolethumbnail
    ${KDE4_KIO_LIBS}
    ${GOBJECT_LIBRARIES}
    ${LIBGSF_LIBRARIES}
)

install(TARGETS msoolethumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES msoolethumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

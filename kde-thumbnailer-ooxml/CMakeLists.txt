project(kde-thumbnailer-ooxml)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

set(libkowmf_SRCS
    libkowmf/WmfAbstractBackend.cpp
    libkowmf/WmfDeviceContext.cpp
    libkowmf/WmfPainterBackend.cpp
    libkowmf/WmfParser.cpp
    libkowmf/WmfStack.cpp
)

set(ooxmlthumbnail_SRCS ooxmlcreator.cpp ${libkowmf_SRCS})

kde4_add_plugin(ooxmlthumbnail ${ooxmlthumbnail_SRCS})
target_link_libraries(ooxmlthumbnail
    ${KDE4_KIO_LIBS}
    ${QT_QTXML_LIBRARY}
)

install(TARGETS ooxmlthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ooxmlthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

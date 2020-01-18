project(kde-thumbnailer-qml)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

set(qmlthumbnail_SRCS qmlcreator.cpp)

kde4_add_plugin(qmlthumbnail ${qmlthumbnail_SRCS})
target_link_libraries(qmlthumbnail
    ${KDE4_KIO_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
)

install(TARGETS qmlthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES qmlthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})

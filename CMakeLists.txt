cmake_minimum_required(VERSION 3.16)

project(FrontierExtractorUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Multimedia Widgets)

qt_standard_project_setup(REQUIRES 6.5)

qt6_add_resources(RESOURCE_FILES resources.qrc)

qt_add_executable(appFrontierExtractorUI
    main.cpp
    ${RESOURCE_FILES}
)

qt_add_qml_module(appFrontierExtractorUI
    URI FrontierExtractorUI
    VERSION 1.0
    QML_FILES Main.qml OGGSurgeon.qml
    SOURCES zapfileexplorer.h zapfileexplorer.cpp
    SOURCES ZAP_FILE.h
    SOURCES oggformat.h
    SOURCES ZAP_FILE.cpp
    SOURCES audioeditoritem.h audioeditoritem.cpp
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appFrontierExtractorUI PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appFrontierExtractorUI
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appFrontierExtractorUI
    PRIVATE Qt6::Quick Qt6::Widgets
    PRIVATE Qt::Multimedia
)

include(GNUInstallDirs)
install(TARGETS appFrontierExtractorUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

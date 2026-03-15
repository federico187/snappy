cmake_minimum_required(VERSION 3.16)
project(snappy VERSION 1.0.0 LANGUAGES CXX DESCRIPTION "Snappy - Screenshot Tool")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 (or Qt6 if preferred)
find_package(Qt5 COMPONENTS Core Gui Widgets Svg QUIET)
if(NOT Qt5_FOUND)
    find_package(Qt6 COMPONENTS Core Gui Widgets SvgWidgets REQUIRED)
    set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::SvgWidgets)
else()
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg)
endif()

set(SOURCES
    src/main.cpp
    src/application.cpp
    src/editorwindow.cpp
    src/screenshotmanager.cpp
    src/snipoverlay.cpp
    src/canvasview.cpp
    src/toolbar.cpp
    src/preferencesdialog.cpp
    src/annotationitems.cpp
    src/globalhotkey.cpp
)

set(HEADERS
    src/application.h
    src/editorwindow.h
    src/screenshotmanager.h
    src/snipoverlay.h
    src/canvasview.h
    src/toolbar.h
    src/preferencesdialog.h
    src/annotationitems.h
    src/globalhotkey.h
)

set(RESOURCES
    resources/resources.qrc
)

# Windows-specific: add RC file for icon + version info
if(WIN32)
    set(WIN_RC resources/snappy.rc)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES} ${WIN_RC})
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${QT_LIBRARIES})

# Set executable properties for Windows
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "snappy"
)

# Windows: link user32 for RegisterHotKey
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32)
endif()

# Install rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

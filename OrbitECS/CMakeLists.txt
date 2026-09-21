cmake_minimum_required(VERSION 3.16)

project(OrbitECS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.9.3/macos/")
find_package(Qt6 REQUIRED COMPONENTS
    Core Qml Network Test Sql
    Multimedia MultimediaWidgets
    Quick QuickControls2
)



qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appOrbitECS
    main.cpp
    src/Loader/BodyLoader.cpp
    src/Front/FrontManager.cpp
    src/Back/BackManager.cpp
)

qt_add_qml_module(appOrbitECS
    URI OrbitECS
    VERSION 1.0
    QML_FILES
        Main.qml
)

set_target_properties(appOrbitECS PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(appOrbitECS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Back/ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Loader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Back
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bridge
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Front
)

target_link_libraries(appOrbitECS
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appOrbitECS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

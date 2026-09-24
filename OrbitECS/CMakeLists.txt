cmake_minimum_required(VERSION 3.16)

project(OrbitECS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.9.3/macos/")

# Release par défaut si rien n'est précisé
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ORBIT_NATIVE_ARCH "Optimiser pour le CPU de la machine (non portable)" ON)
option(ORBIT_FAST_MATH   "Activer -ffast-math (plus rapide, moins précis)"    ON)
option(ORBIT_LTO         "Activer la LTO en Release"                          ON)

include(CheckCXXCompilerFlag)
include(CheckIPOSupported)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Qml
    Network
    Test
    Sql
    Multimedia
    MultimediaWidgets
    Quick
    Quick3D
    QuickControls2
    
)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appOrbitECS
    main.cpp

    # Debug
    DebugPrint.cpp

    # Loader
    src/Loader/BodyLoader.cpp

    # Front
    src/Front/FrontManager.cpp

    # Back
    src/Back/BackManager.cpp

    # Bridge
    src/Bridge/BufferExchange.cpp

    # ECS Systems
    src/Back/ECS/Systems/GravitySystem.cpp
    src/Back/ECS/Systems/IntegrationSystem.cpp
    src/Front/BodyInstancing.cpp
)

# --- Optimisations du back (Release uniquement) ---
target_compile_options(appOrbitECS PRIVATE
    $<$<CONFIG:Release>:-O3>
    # Sans danger pour la physique : pas de errno ni de traps sur les maths
    $<$<CONFIG:Release>:-fno-math-errno>
    $<$<CONFIG:Release>:-fno-trapping-math>
)

if(ORBIT_NATIVE_ARCH)
    check_cxx_compiler_flag(-march=native HAS_MARCH_NATIVE)
    check_cxx_compiler_flag(-mcpu=native  HAS_MCPU_NATIVE)
    if(HAS_MARCH_NATIVE)
        target_compile_options(appOrbitECS PRIVATE $<$<CONFIG:Release>:-march=native>)
    elseif(HAS_MCPU_NATIVE)   # Apple Silicon avec certains Apple clang
        target_compile_options(appOrbitECS PRIVATE $<$<CONFIG:Release>:-mcpu=native>)
    endif()
endif()

if(ORBIT_FAST_MATH)
    target_compile_options(appOrbitECS PRIVATE $<$<CONFIG:Release>:-ffast-math>)
endif()

if(ORBIT_LTO)
    check_ipo_supported(RESULT HAS_IPO OUTPUT IPO_ERR)
    if(HAS_IPO)
        set_property(TARGET appOrbitECS PROPERTY
            INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    endif()
endif()

qt_add_qml_module(appOrbitECS
    URI OrbitECS
    VERSION 1.0

    QML_FILES
        Main.qml
)

set_target_properties(appOrbitECS PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(appOrbitECS PRIVATE

    # Back
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Back

    # Back/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Back/Core

    # ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Back/ECS

    # ECS Components
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Back/ECS/Components

    # ECS Systems
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Back/ECS/Systems

    # Bridge
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bridge

    # Front
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Front

    # Loader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Loader
)

target_link_libraries(appOrbitECS
    PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Network
        Qt6::Sql
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Quick3D
)

include(GNUInstallDirs)

install(TARGETS appOrbitECS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
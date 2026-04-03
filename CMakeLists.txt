cmake_minimum_required(VERSION 3.16)

project(SW_Widgets VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

option(SW_BUILD_EXAMPLES "Build examples" ON)
if(SW_BUILD_EXAMPLES)
    add_subdirectory(examples/demo_app/)
endif()

add_library(SW_Widgets STATIC
    src/SWFluentUI/SWToggleSwitch.cpp
    include/SWFluentUI/swtoggleswitch.hpp
)

target_include_directories(SW_Widgets
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/SWFluentUI>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(SW_Widgets
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

# 🔥 Exportación (clave)
include(GNUInstallDirs)

install(TARGETS SW_Widgets
    EXPORT SW_WidgetsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT SW_WidgetsTargets
    FILE SW_WidgetsTargets.cmake
    NAMESPACE SW::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SW_Widgets
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    SW_WidgetsConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SW_WidgetsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SW_WidgetsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SW_Widgets
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SW_WidgetsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SW_WidgetsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SW_Widgets
)

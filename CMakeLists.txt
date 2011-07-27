project(loopy)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
 
find_package(KDE4 REQUIRED)

include (MacroLibrary)
include (KDE4Defaults)

find_package(MSGFMT REQUIRED)

include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
 
add_subdirectory(themes)
add_subdirectory(desktop)
add_subdirectory(src)
macro_optional_add_subdirectory(po)

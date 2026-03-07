cmake_minimum_required (VERSION 3.14)

#main project
project (RBMK)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set (RBMK_MSVC true)
	add_compile_options(/utf-8)
else()
	set (RBMK_MSVC false)
endif()

set( CMAKE_EXPORT_COMPILE_COMMANDS on)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(contrib)
add_subdirectory(src)

set(FMT_MODULE OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

set_property(TARGET fmt PROPERTY FOLDER "contrib")
set_property(TARGET fmt-c PROPERTY FOLDER "contrib")
set_property(TARGET SDL_uclibc PROPERTY FOLDER "contrib")
set_property(TARGET SDL3-shared PROPERTY FOLDER "contrib")

set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)

include(InstallRequiredSystemLibraries)

install(TARGETS Engine)
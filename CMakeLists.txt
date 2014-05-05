cmake_minimum_required(VERSION 2.8)
project(Asteroids)
include(ExternalProject)

set(Asteroids_INSTALL_DIR "${Asteroids_SOURCE_DIR}/bin")
# Use modified FindSDL2 and FindGLEW that will work with my windows setup
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Asteroids_SOURCE_DIR}/cmake")

set(entityx_DIR "${Asteroids_SOURCE_DIR}/external/entityx")
# Later should handle multiple possible build configurations per:
# https://stackoverflow.com/questions/6351609/cmake-linking-to-library-downloaded-from-externalproject-add
set(entityx_LIBRARY "${entityx_DIR}/lib/libentityx.a")

ExternalProject_Add(entityx
	PREFIX ${entityx_DIR}
	GIT_REPOSITORY https://github.com/alecthomas/entityx.git
	INSTALL_DIR ${entityx_DIR}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${entityx_DIR}
)

# Bump up warning levels appropriately for each compiler
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -ggdb -std=c++11 -DGLM_FORCE_RADIANS")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Zi")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DGLM_FORCE_RADIANS")
endif()

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
# On windows we need to find GLM too
if (WIN32)
	find_package(GLM REQUIRED)
	include_directories(${GLM_INCLUDE_DIRS})
endif()

include_directories(include ${entityx_DIR}/include ${SDL2_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
add_subdirectory(src)


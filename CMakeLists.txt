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
set(entityx_INCLUDE_DIR "${entityx_DIR}/include")

ExternalProject_Add(entityx
	PREFIX ${entityx_DIR}
	GIT_REPOSITORY https://github.com/alecthomas/entityx.git
	GIT_TAG origin/0.1.x
	INSTALL_DIR ${entityx_DIR}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${entityx_DIR}
)

set(lfwatch_DIR "${Asteroids_SOURCE_DIR}/external/lfwatch")
set(lfwatch_LIBRARY "${lfwatch_DIR}/lib/liblfwatch.a")
set(lfwatch_INCLUDE_DIR "${lfwatch_DIR}/include")
ExternalProject_Add(lfwatch
	PREFIX ${lfwatch_DIR}
	GIT_REPOSITORY https://github.com/Twinklebear/lfwatch.git
	INSTALL_DIR ${lfwatch_DIR}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${lfwatch_DIR}
)

set(stb_image_INCLUDE_DIR "${Asteroids_SOURCE_DIR}/external/stb_image/include")
file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
	"${stb_image_INCLUDE_DIR}/stb_image.h")

# Bump up warning levels appropriately for each compiler
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -ggdb -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Zi")
	endif()
endif()

add_definitions(-DGLM_FORCE_RADIANS)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
# On windows we need to find GLM too
if (WIN32)
	find_package(GLM REQUIRED)
	include_directories(${GLM_INCLUDE_DIRS})
endif()

include_directories(include ${entityx_INCLUDE_DIR} ${lfwatch_INCLUDE_DIR}
   ${SDL2_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${stb_image_INCLUDE_DIR})
add_subdirectory(src)


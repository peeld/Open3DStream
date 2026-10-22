cmake_minimum_required(VERSION  3.13)

# Wire protocol version. See CHANGELOG.md for what changed in each version.
set(O3DS_VERSION_TAG "1.0.6")

project(Open3DStream VERSION ${O3DS_VERSION_TAG} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX d)

option(O3DS_BUILD_TESTS     "Build the test programs"                          ON)
option(O3DS_BUILD_APPS      "Build the command line apps"                      ON)
option(O3DS_BUILD_FBXSTREAM "Build FbxStream (needs the Autodesk FBX SDK)"     ON)
option(O3DS_BUILD_MOBU      "Build the MotionBuilder plugin (needs MoBu SDK)"  ON)
option(O3DS_BUILD_MAYA      "Build the Maya plugin (needs Maya devkit)"        OFF)

# nng's TLS support is built on mbedtls; nng's exported targets depend on it
find_package(MbedTLS CONFIG REQUIRED HINTS ${CMAKE_CURRENT_LIST_DIR}/usr)

add_subdirectory(src)

if(O3DS_BUILD_APPS)
	add_subdirectory(apps/SubscribeTest)
endif()

if(O3DS_BUILD_FBXSTREAM)
	add_subdirectory(apps/FbxStream)
endif()

if(O3DS_BUILD_MOBU)
	add_subdirectory(plugins/mobu)
endif()

if(O3DS_BUILD_MAYA)
	add_subdirectory(plugins/maya)
endif()

if(O3DS_BUILD_TESTS)
	add_subdirectory(apps/Test1)
	add_subdirectory(apps/Test2)
endif()

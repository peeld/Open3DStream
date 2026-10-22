cmake_minimum_required(VERSION 3.13)

project(FbxStream)

# Streams an fbx file on a loop - useful for testing connectivity.
# Needs the Autodesk FBX SDK. Point FBX_SDK_ROOT at the install if it is not
# in one of the default locations; FbxStream is skipped if it is not found.

if(WIN32)
	set(_fbx_default "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1")
else()
	set(_fbx_default "$ENV{HOME}/fbxsdk")
endif()
set(FBX_SDK_ROOT "${_fbx_default}" CACHE PATH "Autodesk FBX SDK install directory")

find_path(FBX_SDK_INCLUDE_DIR fbxsdk.h HINTS ${FBX_SDK_ROOT}/include NO_DEFAULT_PATH)
find_library(FBX_SDK_LIBRARY NAMES libfbxsdk fbxsdk
	HINTS
		${FBX_SDK_ROOT}/lib/gcc/x64/release
		${FBX_SDK_ROOT}/lib/vs2017/x64/release
		${FBX_SDK_ROOT}/lib/vs2019/x64/release
		${FBX_SDK_ROOT}/lib/vs2022/x64/release
	NO_DEFAULT_PATH)

if(NOT FBX_SDK_INCLUDE_DIR OR NOT FBX_SDK_LIBRARY)
	message(STATUS "FBX SDK not found in FBX_SDK_ROOT (${FBX_SDK_ROOT}) - skipping FbxStream")
	return()
endif()

find_package(nng CONFIG REQUIRED HINTS ${CMAKE_CURRENT_LIST_DIR}/../../usr)

add_executable(FbxStream main.cpp fbxloader.cpp fbxloader.h)

target_include_directories(FbxStream PRIVATE ${FBX_SDK_INCLUDE_DIR})
target_link_libraries(FbxStream PRIVATE o3dsnet o3dscore nng::nng ${FBX_SDK_LIBRARY})
if(UNIX)
	target_link_libraries(FbxStream PRIVATE xml2)
endif()

if(WIN32)
	# Ship the fbx sdk dll alongside the exe
	get_filename_component(_fbx_libdir ${FBX_SDK_LIBRARY} DIRECTORY)
	add_custom_command(TARGET FbxStream POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_fbx_libdir}/libfbxsdk.dll $<TARGET_FILE_DIR:FbxStream>)
	install(FILES ${_fbx_libdir}/libfbxsdk.dll DESTINATION bin OPTIONAL)

	set_target_properties(FbxStream PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS
		"${CMAKE_CURRENT_LIST_DIR}/../../test_data/beta_fight.fbx tcp://127.0.0.1:3001")
endif()

install(TARGETS FbxStream DESTINATION bin)

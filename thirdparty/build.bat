@ECHO OFF
CALL %~DP0..\build_env.bat

REM if NOT EXIST flatbuffers\CMakeLists.txt git submodule update --init --recursive
IF NOT EXIST nng\CMakeLists.txt cd nng &&  git fetch --tags && git checkout v1.12.3
REM if NOT EXIST websocketpp\CMakeLists.txt git submodule update --init --recursive
IF NOT EXIST mbedtls\CMakeLists.txt  cd mbedtls && git fetch --tags && git checkout v3.6.4 && git submodule update --init

REM git submodule update --recursive


ECHO =================
ECHO MBEDTLS
ECHO =================


if NOT EXIST build_mbedtls mkdir build_mbedtls

cmake -Hmbedtls -Bbuild_mbedtls -G %CMAKE_VS_VERSION% -A x64 -DCMAKE_INSTALL_PREFIX=%~DP0..\usr -DGEN_FILES=1

cmake --build build_mbedtls --config Debug 
cmake --build build_mbedtls --config RelWithDebInfo 
cmake --install build_mbedtls --config Debug 
cmake --install build_mbedtls --config RelWithDebInfo 



:nng

REM thirdparty\nng\cmake\FindmbedTLS.cmake
REM file(STRINGS ${MBEDTLS_INCLUDE_DIR}/mbedtls/version.h _MBEDTLS_VERLINE
REM file(STRINGS ${MBEDTLS_INCLUDE_DIR}/mbedtls/build_info.h _MBEDTLS_VERLINE

ECHO =================
ECHO NNG
ECHO =================


if NOT EXIST build_nng mkdir build_nng

cmake -Hnng -Bbuild_nng -G %CMAKE_VS_VERSION% -A x64 ^
		-DNNG_ENABLE_TLS=ON ^
		-DCMAKE_PREFIX_PATH=%~DP0..\usr  ^
		-DCMAKE_INSTALL_PREFIX=%~DP0..\usr

cmake --build build_nng --config Debug 
cmake --build build_nng --config RelWithDebInfo 
cmake --install build_nng --config Debug 
cmake --install build_nng --config RelWithDebInfo 



ECHO =================
ECHO FLATBUFFERS
ECHO =================

if NOT EXIST build_fb mkdir build_fb

cmake -Hflatbuffers -Bbuild_fb -G %CMAKE_VS_VERSION% -A x64 -DCMAKE_INSTALL_PREFIX=%~DP0..\usr

cmake --build build_fb --config Debug 
cmake --build build_fb --config RelWithDebInfo 
cmake --install build_fb --config Debug 
cmake --install build_fb --config RelWithDebInfo 

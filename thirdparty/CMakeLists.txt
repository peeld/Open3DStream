cmake_minimum_required(VERSION  3.13)
project(O3DS3rdParty)

set(CMAKE_DEBUG_POSTFIX d)

add_subdirectory(flatbuffers)

# nng's TLS transport (used for PSK) needs mbedtls. mbedtls is built and
# installed by build.sh first, so nng finds it with find_package.
set(NNG_ENABLE_TLS ON CACHE BOOL "" FORCE)
add_subdirectory(nng)
add_subdirectory(crccpp)

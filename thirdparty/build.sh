cmake -Hmbedtls -Bbuild_medtls -DCMAKE_INSTALL_PREFIX=../usr -DGEN_FILES=1
cmake --build build_mbedtls
cmake --install build_mbedtls

cmake -Hflatbuffers -Bbuild_fb -DCMAKE_INSTALL_PREFIX=../usr
cmake --build build_fb
cmake --install build_fb

cmake -Hnng -Bbuild_nng -DCMAKE_INSTALL_PREFIX=../usr -DCMAKE_PREFIX_PATH=../usr -DNNG_ENABLE_TLS=ON
cmake --build build_nng
cmake --install build_nng

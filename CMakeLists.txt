cmake_minimum_required(VERSION 3.13)
project(nitrokey-fido2)

SET(CMAKE_C_COMPILER /usr/local/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/local/bin/g++)

message(STATUS Compiler: ${CMAKE_C_COMPILER})
message(STATUS CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE})
message(STATUS CMAKE_LINKER_FLAGS_DEBUG: ${CMAKE_LINKER_FLAGS_DEBUG})
message(STATUS CMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG})
SET(VERSION_FLAG -DSOLO_VERSION_MAJ=2 -DSOLO_VERSION_MIN=2 -DSOLO_VERSION_PATCH=2)
add_definitions(-DAES256=1 -DAPP_CONFIG=\"app.h\" ${VERSION_FLAG} -Wall) # -DENABLE_U2F_EXTENSIONS
#add_definitions(-DAES256=1 -DAPP_CONFIG=\"app.h\" -DIS_BOOTLOADER)

add_definitions(-DNK_TEST_MODE -DCBOR_PARSER_MAX_RECURSIONS=3 -DNK_SIMULATION)

SET(USE_ASAN FALSE)

#set(CMAKE_C_FLAGS_DEBUG " -c -fdump-rtl-dfinish -fstack-usage ")

IF (${USE_ASAN})
    message(STATUS "Enable asan")
    set(CMAKE_C_FLAGS_DEBUG " -fdata-sections -ffunction-sections  ${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -g -O1 -lasan")
    set(CMAKE_LINKER_FLAGS_DEBUG "-Wl,--gc-sections ${CMAKE_LINKER_FLAGS_DEBUG} -lasan")
    set(ADD_LIBS asan)
ENDIF (${USE_ASAN})

#set (CMAKE_C_COMPILER  /usr/bin/clang)
#set (CMAKE_BUILD_TYPE Debug)
#"-Wl,--gc-sections tinycbor/lib/libtinycbor.a -lasan"


include_directories(crypto/micro-ecc)
include_directories(crypto/sha256)
include_directories(crypto/tiny-AES-c)
include_directories(crypto/cifra/src)
include_directories(crypto/cifra/src/ext)
include_directories(fido2)
include_directories(fido2/extensions)
include_directories(pc)
include_directories(tests)
include_directories(tinycbor/src)

add_executable(nitrokey-fido2-simulation pc/main.c targets/stm32l432/src/app-common.h)
target_link_libraries(nitrokey-fido2-simulation libsimulation ${ADD_LIBS})

#add_library(catch_main tests/catch_main.cpp)

add_library(libsimulation
        fido2/extensions/webcrypt/ext_webcrypt.c
        fido2/extensions/webcrypt/ext_webcrypt.h
        fido2/extensions/webcrypt/wc_origin.c
        fido2/extensions/webcrypt/wc_origin.h
        fido2/extensions/webcrypt/wc_device.c
        fido2/extensions/webcrypt/wc_device.h
        fido2/extensions/webcrypt/wc_state.c
        fido2/extensions/webcrypt/wc_state.h
        fido2/extensions/webcrypt/wc_buffer.c
        fido2/extensions/webcrypt/wc_buffer.h
        fido2/extensions/webcrypt/wc_cbor.c
        fido2/extensions/webcrypt/wc_cbor.h
        fido2/extensions/webcrypt/wc_keys.c
        fido2/extensions/webcrypt/wc_keys.h
        targets/stm32l432/src/memory_layout.h
        fido2/device.c
        pc/device.c
        crypto/aes-gcm/aes_gcm.c
        crypto/micro-ecc/types.h
        crypto/micro-ecc/uECC.c
        crypto/micro-ecc/uECC.h
        crypto/micro-ecc/uECC_vli.h
        crypto/sha256/sha256.c
        crypto/sha256/sha256.h
        crypto/tiny-AES-c/aes.c
        crypto/tiny-AES-c/aes.h
        crypto/tiny-AES-c/aes.hpp
    crypto/cifra/src/pbkdf2.c
    crypto/cifra/src/hmac.c
    crypto/cifra/src/chash.c
    crypto/cifra/src/sha256.c
    crypto/cifra/src/blockwise.c
        fido2/extensions/extensions.c
        fido2/extensions/extensions.h
        fido2/extensions/solo.c
        fido2/extensions/solo.h
        fido2/extensions/wallet.c
        fido2/extensions/wallet.h
        fido2/cose_key.h
        fido2/crypto.c
        fido2/crypto.h
        fido2/ctap.c
        fido2/ctap.h
        fido2/ctap_errors.h
        fido2/ctap_parse.c
        fido2/ctap_parse.h
        fido2/ctaphid.c
        fido2/ctaphid.h
        fido2/device.h
        fido2/log.c
        fido2/log.h
        fido2/storage.h
        fido2/stubs.c
        fido2/test_power.c
        fido2/u2f.c
        fido2/u2f.h
        fido2/util.c
        fido2/util.h
        fido2/version.h
        pc/app.h
        tinycbor/src/cbor.h
        tinycbor/src/cborencoder.c
        tinycbor/src/cborencoder_close_container_checked.c
        tinycbor/src/cborerrorstrings.c
        tinycbor/src/cborinternal_p.h
        tinycbor/src/cborjson.h
        tinycbor/src/cborparser.c
        tinycbor/src/cborparser_dup_string.c
        tinycbor/src/cborpretty.c
        tinycbor/src/cborpretty_stdio.c
        tinycbor/src/cbortojson.c
        tinycbor/src/cborvalidation.c
        tinycbor/src/compilersupport_p.h
        tinycbor/src/open_memstream.c
        tinycbor/src/tinycbor-version.h
    tinycbor/src/utf8_p.h
        fido2/extensions/webcrypt/wc_commands.c fido2/extensions/webcrypt/wc_commands.h)

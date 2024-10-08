cmake_minimum_required(VERSION 3.14)
set(CMAKE_CONFIGURATION_TYPES "Release;Debug;ASAN")
project(spotify)

# This CMake file implements building spotify with CMAKE
#
# Note that this is *NOT* the official way to build spotify on anything
# other than Windows/mingw despite working on other platforms too. You will need
# to add -DUNSUPPORTED_BUILDS=true to build on non Windows platforms.
#
# This cmake also makes a few assertions like lzo, lz4 being used
# and OpenSSL having version 1.1.1+ and generally does not offer the same
# configurability like autoconf

find_package(PkgConfig REQUIRED)
include(CheckSymbolExists)
include(CheckIncludeFiles)
include(CheckCCompilerFlag)
include(CheckLinkerFlag OPTIONAL)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CTest)

option(UNSUPPORTED_BUILDS "Allow unsupported builds" OFF)

if (NOT WIN32 AND NOT ${UNSUPPORTED_BUILDS})
    message(FATAL_ERROR "Note: on Unix platform the official and supported build method is using autoconfig. CMake based build should be only used for Windows and internal testing/development.")
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    message(FATAL_ERROR "The top level source directory has a config.h file. Note that you can't mix in-tree autoconfig builds with out-of-tree cmake builds.")
endif ()

option(MBED "BUILD with mbed" OFF)
set(MBED_INCLUDE_PATH "" CACHE STRING "Path to mbed TLS include directory")
set(MBED_LIBRARY_PATH "" CACHE STRING "Path to mbed library directory")
option(WOLFSSL "BUILD with wolfSSL" OFF)
option(ENABLE_LZ4 "BUILD with lz4" ON)
option(ENABLE_LZO "BUILD with lzo" ON)
option(ENABLE_PKCS11 "BUILD with pkcs11-helper" ON)
option(USE_WERROR "Treat compiler warnings as errors (-Werror)" ON)

set(PLUGIN_DIR /usr/local/lib/spotify/plugins CACHE FILEPATH "Location of the plugin directory")

# Create machine readable compile commands
option(ENABLE_COMPILE_COMMANDS "Generate compile_commands.json and a symlink for clangd to find it" OFF)
if (ENABLE_COMPILE_COMMANDS)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/build AND NOT IS_SYMLINK ${CMAKE_CURRENT_SOURCE_DIR}/build)
        message(FATAL_ERROR "The top level source directory contains a 'build' file or directory. Please remove or rename it. CMake creates a symlink with that name during build.")
    endif()
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
    add_custom_target(
        symlink-build-dir ALL
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/build
        )
endif ()

# AddressSanitize - use CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=asan to build with ASAN
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address,undefined -fno-sanitize-recover=all -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_ASAN
    "-fsanitize=address,undefined -fno-sanitize-recover=all -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds."
    FORCE)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_WINSOCK_DEPRECATED_NO_WARNINGS)
    if (USE_WERROR)
        add_compile_options(/WX)
    endif ()
    add_compile_options(
        /MP
        /W2
        /sdl
        /Qspectre
        /guard:cf
        /FC
        /ZH:SHA_256
        "$<$<CONFIG:Release>:/GL>"
        "$<$<CONFIG:Release>:/Oi>"
        "$<$<CONFIG:Release>:/Gy>"
        "$<$<CONFIG:Release>:/Zi>"
        )
    add_link_options(
        /Brepro
        "$<$<CONFIG:Release>:/LTCG:incremental>"
        "$<$<CONFIG:Release>:/DEBUG:FULL>"
        "$<$<CONFIG:Release>:/OPT:REF>"
        "$<$<CONFIG:Release>:/OPT:ICF>"
        )
    if (${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64" OR ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x86")
        add_link_options("$<$<CONFIG:Release>:/CETCOMPAT>")
    endif()
else ()
    add_compile_options(-Wall -Wuninitialized)
    check_c_compiler_flag(-Wno-stringop-truncation NoStringOpTruncation)

    if (${NoStringOpTruncation})
        add_compile_options(-Wno-stringop-truncation)
    endif()
    # We are not ready for this
    #add_compile_options(-Wconversion -Wno-sign-conversion -Wsign-compare)
    if (USE_WERROR)
        add_compile_options(-Werror)
    endif ()
endif ()

find_program(PYTHON NAMES python3 python)
execute_process(
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/parse-version.m4.py ${CMAKE_CURRENT_SOURCE_DIR}/version.m4
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
include(${CMAKE_CURRENT_BINARY_DIR}/version.cmake)

set(spotify_VERSION_MAJOR ${PRODUCT_VERSION_MAJOR})
set(spotify_VERSION_MINOR ${PRODUCT_VERSION_MINOR})
set(spotify_VERSION_PATCH ${PRODUCT_VERSION_PATCH})
set(spotify_VERSION_RESOURCE ${PRODUCT_VERSION_RESOURCE})

set(CMAKE_C_STANDARD 11)

# Set the various defines for config.h.cmake.in
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(TARGET_LINUX YES)
    set(ENABLE_ASYNC_PUSH YES)
    set(ENABLE_LINUXDCO YES)
    set(ENABLE_SITNL YES)
    set(HAVE_DECL_SO_MARK YES)
    set(ENABLE_FEATURE_TUN_PERSIST 1)
    set(HAVE_LINUX_TYPES_H 1)
    set(ENABLE_DCO YES)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    set(TARGET_FREEBSD YES)
    set(ENABLE_DCO YES)
    link_libraries(-lnv)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    set(TARGET_OPENBSD YES)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
    set(TARGET_SOLARIS YES)
    set(HAVE_SYS_SOCKIO_H 1)
    link_libraries(-lnsl -lsocket -lresolv)
elseif (WIN32)
    set(ENABLE_DCO YES)
elseif (APPLE)
    set(TARGET_DARWIN YES)
    set(HAVE_NET_IF_UTUN_H YES)
else()
    message(FATAL_ERROR "Unknown system name: \"${CMAKE_SYSTEM_NAME}\"")
endif ()

if (UNIX)
    set(PATH_SEPARATOR /)
    set(ENABLE_PORT_SHARE YES)
    set(HAVE_SA_FAMILY_T YES)
elseif (WIN32)
    set(PATH_SEPARATOR \\\\)
    set(TARGET_WIN32 YES)
endif ()

check_symbol_exists(chroot unistd.h HAVE_CHROOT)
check_symbol_exists(chdir unistd.h HAVE_CHDIR)
check_symbol_exists(dup unistd.h HAVE_DUP)
check_symbol_exists(dup2 unistd.h HAVE_DUP2)
check_symbol_exists(fork unistd.h HAVE_FORK)
check_symbol_exists(execve unistd.h HAVE_EXECVE)
check_symbol_exists(ftruncate unistd.h HAVE_FTRUNCATE)
check_symbol_exists(nice unistd.h HAVE_NICE)
check_symbol_exists(setgid unistd.h HAVE_SETGID)
check_symbol_exists(setuid unistd.h HAVE_SETUID)
check_symbol_exists(setsid unistd.h HAVE_SETSID)
check_symbol_exists(getpeereid "unistd.h;sys/socket.h" HAVE_GETPEEREID)

check_symbol_exists(epoll_create sys/epoll.h HAVE_EPOLL_CREATE)

check_symbol_exists(gettimeofday sys/time.h HAVE_GETTIMEOFDAY)
check_symbol_exists(basename libgen.h HAVE_BASENAME)
check_symbol_exists(chsize io.h HAVE_CHSIZE)
check_symbol_exists(daemon "unistd.h;stdlib.h" HAVE_DAEMON)
check_symbol_exists(dirname libgen.h HAVE_DIRNAME)
check_symbol_exists(getrlimit sys/resource.h HAVE_GETRLIMIT)
check_symbol_exists(mlockall sys/mman.h HAVE_MLOCKALL)

check_symbol_exists(sendmsg sys/socket.h HAVE_SENDMSG)
check_symbol_exists(recvmsg sys/socket.h HAVE_RECVMSG)
check_symbol_exists(openlog syslog.h HAVE_OPENLOG)
check_symbol_exists(syslog syslog.h HAVE_SYSLOG)
check_symbol_exists(getgrnam grp.h HAVE_GETGRNAM)
check_symbol_exists(getpwnam pwd.h HAVE_GETPWNAM)
check_symbol_exists(getsockname sys/socket.h HAVE_GETSOCKNAME)
check_symbol_exists(getrlimit "sys/time.h;sys/resource.h" HAVE_GETRLIMIT)

# Checking for existence of structs with check_symbol_exists does not work,
# so we use check_struct_hash_member with a member instead
check_struct_has_member("struct cmsghdr" cmsg_len sys/socket.h HAVE_CMSGHDR)


# Some OS (e.g. FreeBSD) need some basic headers to allow
# including network headers
set(NETEXTRA sys/types.h)
check_include_files("${NETEXTRA};netinet/in.h" HAVE_NETINET_IN_H)

if (HAVE_NETINET_IN_H)
    list(APPEND NETEXTRA netinet/in.h)
endif ()

check_include_files("${NETEXTRA};netinet/in6.h" HAVE_NETINET_IN_H)
check_include_files(linux/if_tun.h HAVE_LINUX_IF_TUN_H)
check_include_files(linux/sockios.h HAVE_LINUX_SOCKIOS_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(dmalloc.h HAVE_DMALLOC_H)
check_include_files(err.h HAVE_ERR_H)
check_include_files(sys/epoll.h HAVE_SYS_EPOLL_H)
check_include_files(poll.h HAVE_POLL_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(netdb.h HAVE_NETDB_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(sys/un.h HAVE_SYS_UN_H)
check_include_files(libgen.h HAVE_LIBGEN_H)
check_include_files(net/if.h HAVE_NET_IF_H)
check_include_files("${NETEXTRA};netinet/ip.h" HAVE_NETINET_IP_H)
check_include_files(arpa/inet.h HAVE_ARPA_INET_H)
check_include_files(net/if_utun.h HAVE_NET_UTUN_H)
check_include_files(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_files(sys/inotify.h HAVE_SYS_INOTIFY_H)
check_include_files("${NETEXTRA};sys/uio.h" HAVE_SYS_UIO_H)
check_include_files(syslog.h HAVE_SYSLOG_H)
check_include_files(sys/wait.h HAVE_SYS_WAIT_H)
check_include_files(grp.h HAVE_GRP_H)
check_include_files(pwd.h HAVE_PWD_H)
check_include_files(sys/mman.h HAVE_SYS_MMAN_H)


check_include_files("${NETEXTRA};resolv.h" HAVE_RESOLV_H)
check_include_files("${NETEXTRA};net/if_tun.h" HAVE_NET_IF_TUN_H)

set(CMAKE_EXTRA_INCLUDE_FILES netinet/ip.h)
check_type_size("struct in_pktinfo" IN_PKTINFO)
check_struct_has_member("struct in_pktinfo" ipi_spec_dst netinet/ip.h HAVE_IPI_SPEC_DST)
check_type_size("struct msghdr" MSGHDR)
set(CMAKE_EXTRA_INCLUDE_FILES)

find_program(IFCONFIG_PATH ifconfig)
find_program(IPROUTE_PATH ip)
find_program(ROUTE_PATH route)

if (${ENABLE_LZ4})
    pkg_search_module(liblz4 liblz4 REQUIRED IMPORTED_TARGET)
endif ()

if (${ENABLE_LZO})
    pkg_search_module(lzo2 lzo2 REQUIRED IMPORTED_TARGET)
endif ()

if (${ENABLE_PKCS11})
    pkg_search_module(pkcs11-helper libpkcs11-helper-1 REQUIRED IMPORTED_TARGET)
endif ()

function(check_mbed_configuration)
    if (NOT (MBED_INCLUDE_PATH STREQUAL "") )
        set(CMAKE_REQUIRED_INCLUDES ${MBED_INCLUDE_PATH})
    endif ()
    if (NOT (MBED_LIBRARY_PATH STREQUAL ""))
        set(CMAKE_REQUIRED_LINK_OPTIONS "-L${MBED_LIBRARY_PATH}")
    endif ()
    set(CMAKE_REQUIRED_LIBRARIES "mbedtls;mbedx509;mbedcrypto")
    check_symbol_exists(mbedtls_ctr_drbg_update_ret mbedtls/ctr_drbg.h HAVE_MBEDTLS_CTR_DRBG_UPDATE_RET)
    check_symbol_exists(mbedtls_ssl_conf_export_keys_ext_cb mbedtls/ssl.h HAVE_MBEDTLS_SSL_CONF_EXPORT_KEYS_EXT_CB)
    check_include_files(psa/crypto.h HAVE_MBEDTLS_PSA_CRYPTO_H)
endfunction()

if (${MBED})
    check_mbed_configuration()
endif()

function(add_library_deps target)
    if (${MBED})
        if (NOT (MBED_INCLUDE_PATH STREQUAL "") )
            target_include_directories(${target} PRIVATE ${MBED_INCLUDE_PATH})
        endif ()
        if(NOT (MBED_LIBRARY_PATH STREQUAL ""))
            target_link_directories(${target} PRIVATE ${MBED_LIBRARY_PATH})
        endif ()

        target_link_libraries(${target} PRIVATE -lmbedtls -lmbedx509 -lmbedcrypto)
    elseif (${WOLFSSL})
        pkg_search_module(wolfssl wolfssl REQUIRED)
        target_link_libraries(${target} PUBLIC ${wolfssl_LINK_LIBRARIES})
        target_include_directories(${target} PRIVATE ${wolfssl_INCLUDE_DIRS}/wolfssl)
    else ()
        set(ENABLE_X509ALTUSERNAME YES)

        find_package(OpenSSL REQUIRED)
        target_link_libraries(${target} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
        if (WIN32)
            target_link_libraries(${target} PUBLIC
                ws2_32.lib crypt32.lib fwpuclnt.lib iphlpapi.lib
                wininet.lib setupapi.lib rpcrt4.lib wtsapi32.lib ncrypt.lib bcrypt.lib)
        endif ()

    endif ()

    if (MINGW)
        target_compile_definitions(${target} PRIVATE
                -DWIN32_LEAN_AND_MEAN
                -DNTDDI_VERSION=NTDDI_VISTA -D_WIN32_WINNT=_WIN32_WINNT_VISTA
        )
    endif()

    # optional dependencies
    target_link_libraries(${target} PUBLIC
        $<TARGET_NAME_IF_EXISTS:PkgConfig::liblz4>
        $<TARGET_NAME_IF_EXISTS:PkgConfig::lzo2>
        $<TARGET_NAME_IF_EXISTS:PkgConfig::pkcs11-helper>
        )

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        pkg_search_module(libcapng REQUIRED libcap-ng IMPORTED_TARGET)
        pkg_search_module(libnl REQUIRED libnl-genl-3.0 IMPORTED_TARGET)

        target_link_libraries(${target} PUBLIC PkgConfig::libcapng PkgConfig::libnl)
    endif ()

endfunction()

if (${MBED})
    set(ENABLE_CRYPTO_MBEDTLS YES)
elseif (${WOLFSSL})
    set(ENABLE_CRYPTO_OPENSSL YES)
    set(ENABLE_CRYPTO_WOLFSSL YES)
    set(ENABLE_X509ALTUSERNAME YES)
else ()
    set(ENABLE_CRYPTO_OPENSSL YES)
    set(ENABLE_X509ALTUSERNAME YES)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} src/compat include)

add_custom_command(
    OUTPUT always_rebuild config-version.h
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/git-version.py
    )
set(HAVE_CONFIG_VERSION_H YES)

configure_file(config.h.cmake.in config.h)
configure_file(include/spotify-plugin.h.in spotify-plugin.h)
# TODO we should remove the need for this, and always include config.h
add_definitions(-DHAVE_CONFIG_H)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(doc)
add_subdirectory(src/spotifymsica)
add_subdirectory(src/spotifyserv)
add_subdirectory(src/tapctl)

set(SOURCE_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/config-version.h
    ${CMAKE_CURRENT_BINARY_DIR}/spotify-plugin.h

    src/compat/compat-basename.c
    src/compat/compat-daemon.c
    src/compat/compat-dirname.c
    src/compat/compat-gettimeofday.c
    src/compat/compat-strsep.c
    src/spotify/argv.c
    src/spotify/argv.h
    src/spotify/base64.c
    src/spotify/base64.h
    src/spotify/basic.h
    src/spotify/buffer.c
    src/spotify/buffer.h
    src/spotify/circ_list.h
    src/spotify/clinat.c
    src/spotify/clinat.h
    src/spotify/common.h
    src/spotify/comp-lz4.c
    src/spotify/comp-lz4.h
    src/spotify/comp.c
    src/spotify/comp.h
    src/spotify/compstub.c
    src/spotify/console.c
    src/spotify/console_builtin.c
    src/spotify/console.h
    src/spotify/crypto.c
    src/spotify/crypto.h
    src/spotify/crypto_backend.h
    src/spotify/crypto_openssl.c
    src/spotify/crypto_openssl.h
    src/spotify/crypto_mbedtls.c
    src/spotify/crypto_mbedtls.h
    src/spotify/cryptoapi.c
    src/spotify/cryptoapi.h
    src/spotify/dco.c
    src/spotify/dco.h
    src/spotify/dco_win.c
    src/spotify/dco_win.h
    src/spotify/dco_linux.c
    src/spotify/dco_linux.h
    src/spotify/dco_freebsd.c
    src/spotify/dco_freebsd.h
    src/spotify/dhcp.c
    src/spotify/dhcp.h
    src/spotify/dns.c
    src/spotify/dns.h
    src/spotify/errlevel.h
    src/spotify/env_set.c
    src/spotify/env_set.h
    src/spotify/error.c
    src/spotify/error.h
    src/spotify/event.c
    src/spotify/event.h
    src/spotify/fdmisc.c
    src/spotify/fdmisc.h
    src/spotify/forward.c
    src/spotify/forward.h
    src/spotify/fragment.c
    src/spotify/fragment.h
    src/spotify/gremlin.c
    src/spotify/gremlin.h
    src/spotify/helper.c
    src/spotify/helper.h
    src/spotify/httpdigest.c
    src/spotify/httpdigest.h
    src/spotify/init.c
    src/spotify/init.h
    src/spotify/integer.h
    src/spotify/interval.c
    src/spotify/interval.h
    src/spotify/list.c
    src/spotify/list.h
    src/spotify/lladdr.c
    src/spotify/lladdr.h
    src/spotify/lzo.c
    src/spotify/lzo.h
    src/spotify/manage.c
    src/spotify/manage.h
    src/spotify/mbuf.c
    src/spotify/mbuf.h
    src/spotify/memdbg.h
    src/spotify/misc.c
    src/spotify/misc.h
    src/spotify/mroute.c
    src/spotify/mroute.h
    src/spotify/mss.c
    src/spotify/mss.h
    src/spotify/mstats.c
    src/spotify/mstats.h
    src/spotify/mtcp.c
    src/spotify/mtcp.h
    src/spotify/mtu.c
    src/spotify/mtu.h
    src/spotify/mudp.c
    src/spotify/mudp.h
    src/spotify/multi.c
    src/spotify/multi.h
    src/spotify/ntlm.c
    src/spotify/ntlm.h
    src/spotify/occ.c
    src/spotify/occ.h
    src/spotify/spotify.c
    src/spotify/spotify.h
    src/spotify/spotify_win32_resources.rc
    src/spotify/options.c
    src/spotify/options.h
    src/spotify/options_util.c
    src/spotify/options_util.h
    src/spotify/otime.c
    src/spotify/otime.h
    src/spotify/ovpn_dco_win.h
    src/spotify/packet_id.c
    src/spotify/packet_id.h
    src/spotify/perf.c
    src/spotify/perf.h
    src/spotify/ping.c
    src/spotify/ping.h
    src/spotify/pkcs11.c
    src/spotify/pkcs11.h
    src/spotify/pkcs11_backend.h
    src/spotify/pkcs11_openssl.c
    src/spotify/pkcs11_mbedtls.c
    src/spotify/platform.c
    src/spotify/platform.h
    src/spotify/plugin.c
    src/spotify/plugin.h
    src/spotify/pool.c
    src/spotify/pool.h
    src/spotify/proto.c
    src/spotify/proto.h
    src/spotify/proxy.c
    src/spotify/proxy.h
    src/spotify/ps.c
    src/spotify/ps.h
    src/spotify/push.c
    src/spotify/push.h
    src/spotify/pushlist.h
    src/spotify/reflect_filter.c
    src/spotify/reflect_filter.h
    src/spotify/reliable.c
    src/spotify/reliable.h
    src/spotify/route.c
    src/spotify/route.h
    src/spotify/run_command.c
    src/spotify/run_command.h
    src/spotify/schedule.c
    src/spotify/schedule.h
    src/spotify/session_id.c
    src/spotify/session_id.h
    src/spotify/shaper.c
    src/spotify/shaper.h
    src/spotify/sig.c
    src/spotify/sig.h
    src/spotify/socket.c
    src/spotify/socket.h
    src/spotify/socks.c
    src/spotify/socks.h
    src/spotify/ssl.c
    src/spotify/ssl.h
    src/spotify/ssl_backend.h
    src/spotify/ssl_common.h
    src/spotify/ssl_openssl.c
    src/spotify/ssl_openssl.h
    src/spotify/ssl_mbedtls.c
    src/spotify/ssl_mbedtls.h
    src/spotify/ssl_verify.c
    src/spotify/ssl_verify.h
    src/spotify/ssl_verify_backend.h
    src/spotify/ssl_verify_openssl.c
    src/spotify/ssl_verify_openssl.h
    src/spotify/ssl_verify_mbedtls.c
    src/spotify/ssl_verify_mbedtls.h
    src/spotify/status.c
    src/spotify/status.h
    src/spotify/syshead.h
    src/spotify/tls_crypt.c
    src/spotify/tun.c
    src/spotify/tun.h
    src/spotify/tun_afunix.c
    src/spotify/tun_afunix.h
    src/spotify/networking_sitnl.c
    src/spotify/networking_freebsd.c
    src/spotify/auth_token.c
    src/spotify/auth_token.h
    src/spotify/ssl_ncp.c
    src/spotify/ssl_ncp.h
    src/spotify/ssl_pkt.c
    src/spotify/ssl_pkt.h
    src/spotify/ssl_util.c
    src/spotify/ssl_util.h
    src/spotify/vlan.c
    src/spotify/vlan.h
    src/spotify/wfp_block.c
    src/spotify/wfp_block.h
    src/spotify/win32.c
    src/spotify/win32-util.c
    src/spotify/win32.h
    src/spotify/win32-util.h
    src/spotify/xkey_helper.c
    src/spotify/xkey_provider.c
    )

add_executable(spotify ${SOURCE_FILES})

add_library_deps(spotify)

if(MINGW)
    target_compile_options(spotify PRIVATE -municode -UUNICODE)
    target_link_options(spotify PRIVATE -municode)
endif()

if (MSVC)
    # we have our own manifest
    target_link_options(spotify PRIVATE /MANIFEST:NO)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(spotify PUBLIC -ldl)
endif ()

if (NOT WIN32)
    target_compile_options(spotify PRIVATE -DPLUGIN_LIBDIR=\"${PLUGIN_DIR}\")

    find_library(resolv resolv)
    # some platform like BSDs already include resolver functionality in the libc and not have an extra resolv library
    if (${resolv} OR APPLE)
        target_link_libraries(spotify PUBLIC -lresolv)
    endif ()
endif ()


if (BUILD_TESTING)
    find_package(cmocka CONFIG)
    if (TARGET cmocka::cmocka)
        set(CMOCKA_LIBRARIES cmocka::cmocka)
    else ()
        pkg_search_module(cmocka cmocka REQUIRED IMPORTED_TARGET)
        set(CMOCKA_LIBRARIES PkgConfig::cmocka)
    endif ()

    set(unit_tests
        "test_auth_token"
        "test_buffer"
        "test_crypto"
        "test_misc"
        "test_ncp"
        "test_packet_id"
        "test_pkt"
        "test_provider"
        "test_ssl"
        "test_user_pass"
        )

    if (WIN32)
        list(APPEND unit_tests
            "test_cryptoapi"
            )
    endif ()

    # MSVC and Apple's LLVM ld do not support --wrap
    # This test requires cmake >= 3.18, so check if check_linker_flag is
    # available
    if (COMMAND check_linker_flag)
        check_linker_flag(C -Wl,--wrap=parse_line LD_SUPPORTS_WRAP)
    endif()

    if (${LD_SUPPORTS_WRAP})
        list(APPEND unit_tests
            "test_argv"
            "test_tls_crypt"
            )
    endif ()

    # These tests work on only on Linux since they depend on special Linux features
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        list(APPEND unit_tests
            "test_networking"
            )
    endif ()

    if (NOT WIN32 AND ${ENABLE_PKCS11})
        set(_HAVE_SOFTHSM2 YES)
        find_program(P11TOOL p11tool)
        find_program(SOFTHSM2_UTIL softhsm2-util)
        find_library(SOFTHSM2_MODULE softhsm2 PATH_SUFFIXES softhsm)

        if (P11TOOL STREQUAL "P11TOOL-NOTFOUND")
            message(STATUS "p11tool not found, pkcs11 UT disabled")
            set(_HAVE_SOFTHSM2 NO)
        elseif (SOFTHSM2_UTIL STREQUAL "SOFTHSM2_UTIL-NOTFOUND")
            message(STATUS "softhsm2-util not found, pkcs11 UT disabled")
            set(_HAVE_SOFTHSM2 NO)
        elseif (SOFTHSM2_MODULE STREQUAL "SOFTHSM2_MODULE-NOTFOUND")
            message(STATUS "softhsm2 module not found, pkcs11 UT disabled")
            set(_HAVE_SOFTHSM2 NO)
        endif ()

        if (_HAVE_SOFTHSM2)
            message(VERBOSE "pkcs11 UT enabled")
            list(APPEND unit_tests
                "test_pkcs11"
                )
        endif ()
    endif ()

    foreach (test_name ${unit_tests})
        # test_networking needs special environment
        if (NOT ${test_name} STREQUAL "test_networking")
            add_test(${test_name} ${test_name})
            # for compat with autotools make check
            set(_UT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_tests/spotify)
            set_tests_properties(${test_name} PROPERTIES
                ENVIRONMENT "srcdir=${_UT_SOURCE_DIR}")
        endif ()
        add_executable(${test_name}
            tests/unit_tests/spotify/${test_name}.c
            tests/unit_tests/spotify/mock_msg.c
            tests/unit_tests/spotify/mock_msg.h
            src/spotify/platform.c
            src/spotify/win32-util.c
            src/compat/compat-gettimeofday.c
            )

        add_library_deps(${test_name})
        target_link_libraries(${test_name} PUBLIC ${CMOCKA_LIBRARIES})

        target_include_directories(${test_name} PRIVATE src/spotify)

        # for compat with IDEs like Clion that ignore the tests properties
        # for the environment variable srcdir when running tests as fallback
        target_compile_definitions(${test_name} PRIVATE "-DUNIT_TEST_SOURCEDIR=\"${CMAKE_SOURCE_DIR}/tests/unit_tests/spotify\"")

        if (NOT ${test_name} STREQUAL "test_buffer")
            target_sources(${test_name} PRIVATE
                src/spotify/buffer.c
                )
        endif ()

    endforeach()

    target_sources(test_auth_token PRIVATE
        src/spotify/base64.c
        src/spotify/crypto_mbedtls.c
        src/spotify/crypto_openssl.c
        src/spotify/crypto.c
        src/spotify/otime.c
        src/spotify/packet_id.c
        )

    target_sources(test_buffer PRIVATE
        tests/unit_tests/spotify/mock_get_random.c
        )

    target_sources(test_crypto PRIVATE
        src/spotify/crypto_mbedtls.c
        src/spotify/crypto_openssl.c
        src/spotify/crypto.c
        src/spotify/otime.c
        src/spotify/packet_id.c
        src/spotify/mtu.c
        src/spotify/mss.c
        )

    target_sources(test_ssl PRIVATE
            tests/unit_tests/spotify/mock_management.c
            tests/unit_tests/spotify/mock_ssl_dependencies.c
            tests/unit_tests/spotify/mock_win32_execve.c
            src/spotify/argv.c
            src/spotify/base64.c
            src/spotify/crypto.c
            src/spotify/crypto_mbedtls.c
            src/spotify/crypto_openssl.c
            src/spotify/cryptoapi.c
            src/spotify/env_set.c
            src/spotify/mss.c
            src/spotify/mtu.c
            src/spotify/options_util.c
            src/spotify/otime.c
            src/spotify/packet_id.c
            src/spotify/run_command.c
            src/spotify/ssl_mbedtls.c
            src/spotify/ssl_openssl.c
            src/spotify/ssl_util.c
            src/spotify/ssl_verify_mbedtls.c
            src/spotify/ssl_verify_openssl.c
            src/spotify/xkey_helper.c
            src/spotify/xkey_provider.c
    )

    target_sources(test_misc PRIVATE
        tests/unit_tests/spotify/mock_get_random.c
        src/spotify/options_util.c
        src/spotify/ssl_util.c
        src/spotify/list.c
        )

    target_sources(test_ncp PRIVATE
        src/spotify/crypto_mbedtls.c
        src/spotify/crypto_openssl.c
        src/spotify/crypto.c
        src/spotify/otime.c
        src/spotify/packet_id.c
        src/spotify/ssl_util.c
        src/compat/compat-strsep.c
        )

    target_sources(test_packet_id PRIVATE
        tests/unit_tests/spotify/mock_get_random.c
        src/spotify/otime.c
        src/spotify/packet_id.c
        src/spotify/reliable.c
        src/spotify/session_id.c
        )

    target_sources(test_pkt PRIVATE
        tests/unit_tests/spotify/mock_win32_execve.c
        src/spotify/argv.c
        src/spotify/base64.c
        src/spotify/crypto_mbedtls.c
        src/spotify/crypto_openssl.c
        src/spotify/crypto.c
        src/spotify/env_set.c
        src/spotify/otime.c
        src/spotify/packet_id.c
        src/spotify/reliable.c
        src/spotify/run_command.c
        src/spotify/session_id.c
        src/spotify/ssl_pkt.c
        src/spotify/tls_crypt.c
        )

    target_sources(test_provider PRIVATE
        tests/unit_tests/spotify/mock_get_random.c
        src/spotify/xkey_provider.c
        src/spotify/xkey_helper.c
        src/spotify/base64.c
        )

    target_sources(test_user_pass PRIVATE
        tests/unit_tests/spotify/mock_get_random.c
        tests/unit_tests/spotify/mock_win32_execve.c
        src/spotify/base64.c
        src/spotify/console.c
        src/spotify/env_set.c
        src/spotify/run_command.c
        )

    if (TARGET test_argv)
        target_link_options(test_argv PRIVATE -Wl,--wrap=parse_line)
        target_sources(test_argv PRIVATE
            tests/unit_tests/spotify/mock_get_random.c
            src/spotify/argv.c
            )
    endif ()

    if (TARGET test_cryptoapi)
        target_sources(test_cryptoapi PRIVATE
            tests/unit_tests/spotify/mock_get_random.c
            tests/unit_tests/spotify/cert_data.h
            tests/unit_tests/spotify/pkey_test_utils.c
            src/spotify/xkey_provider.c
            src/spotify/xkey_helper.c
            src/spotify/base64.c
            )
    endif ()

    if (TARGET test_networking)
        target_link_options(test_networking PRIVATE -Wl,--wrap=parse_line)
        target_compile_options(test_networking PRIVATE -UNDEBUG)
        target_sources(test_networking PRIVATE
            src/spotify/networking_sitnl.c
            src/spotify/crypto_mbedtls.c
            src/spotify/crypto_openssl.c
            src/spotify/crypto.c
            src/spotify/otime.c
            src/spotify/packet_id.c
            )
    endif ()

    if (TARGET test_tls_crypt)
        target_link_options(test_tls_crypt PRIVATE -Wl,--wrap=parse_line)
        target_link_options(test_tls_crypt PRIVATE
            -Wl,--wrap=buffer_read_from_file
            -Wl,--wrap=buffer_write_file
            -Wl,--wrap=rand_bytes)
        target_sources(test_tls_crypt PRIVATE
            tests/unit_tests/spotify/mock_win32_execve.c
            src/spotify/argv.c
            src/spotify/base64.c
            src/spotify/crypto_mbedtls.c
            src/spotify/crypto_openssl.c
            src/spotify/crypto.c
            src/spotify/env_set.c
            src/spotify/otime.c
            src/spotify/packet_id.c
            src/spotify/run_command.c
            )
    endif ()

    if (TARGET test_pkcs11)
        target_compile_options(test_pkcs11 PRIVATE
            -DP11TOOL_PATH=\"${P11TOOL}\"
            -DSOFTHSM2_MODULE_PATH=\"${SOFTHSM2_MODULE}\"
            -DSOFTHSM2_UTIL_PATH=\"${SOFTHSM2_UTIL}\"
            )
        target_sources(test_pkcs11 PRIVATE
            tests/unit_tests/spotify/mock_get_random.c
            tests/unit_tests/spotify/pkey_test_utils.c
            src/spotify/argv.c
            src/spotify/base64.c
            src/spotify/env_set.c
            src/spotify/otime.c
            src/spotify/pkcs11.c
            src/spotify/pkcs11_openssl.c
            src/spotify/run_command.c
            src/spotify/xkey_helper.c
            src/spotify/xkey_provider.c
            )
    endif ()

endif (BUILD_TESTING)

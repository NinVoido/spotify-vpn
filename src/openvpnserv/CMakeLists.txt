if (NOT WIN32)
    return ()
endif ()

project(spotifyserv)

add_executable(spotifyserv)

target_include_directories(spotifyserv PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../
    ../../include/
    ../spotify/
    ../compat/
    )
target_sources(spotifyserv PRIVATE
    common.c
    interactive.c
    service.c service.h
    validate.c validate.h
    ../spotify/wfp_block.c ../spotify/wfp_block.h
    spotifyserv_resources.rc
    ../spotify/ring_buffer.h
    )
target_compile_options(spotifyserv PRIVATE
    -D_UNICODE
    -UNTDDI_VERSION
    -D_WIN32_WINNT=_WIN32_WINNT_VISTA
    )
target_link_libraries(spotifyserv
    advapi32.lib userenv.lib iphlpapi.lib fwpuclnt.lib rpcrt4.lib shlwapi.lib netapi32.lib ws2_32.lib ntdll.lib)
if (MINGW)
    target_compile_options(spotifyserv PRIVATE -municode)
    target_link_options(spotifyserv PRIVATE -municode)
endif ()

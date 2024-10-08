if (NOT WIN32)
    return ()
endif ()

project(spotifymsica)

add_library(spotifymsica SHARED)

target_include_directories(spotifymsica PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../
    ../../include/
    ../compat/
    )
target_sources(spotifymsica PRIVATE
    dllmain.c
    msiex.c msiex.h
    msica_arg.c msica_arg.h
    spotifymsica.c spotifymsica.h
    ../tapctl/basic.h
    ../tapctl/error.c ../tapctl/error.h
    ../tapctl/tap.c ../tapctl/tap.h
    spotifymsica_resources.rc
    )
target_compile_options(spotifymsica PRIVATE
    -D_UNICODE
    -UNTDDI_VERSION
    -D_WIN32_WINNT=_WIN32_WINNT_VISTA
    )

if (MSVC)
    target_compile_options(spotifymsica PRIVATE
        "$<$<CONFIG:Release>:/MT>"
        "$<$<CONFIG:Debug>:/MTd>"
        )
endif ()

target_link_libraries(spotifymsica
    advapi32.lib ole32.lib msi.lib setupapi.lib iphlpapi.lib shell32.lib shlwapi.lib version.lib newdev.lib)
if (MINGW)
    target_compile_options(spotifymsica PRIVATE -municode)
    target_link_options(spotifymsica PRIVATE -municode)
    target_link_options(spotifymsica PRIVATE
        -Wl,--kill-at)
endif ()

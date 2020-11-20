include(chip-lib)
include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

pw_add_module_library(pw_sys_io.nrfconnect
    SOURCES
      sys_io_nrfconnect.cc
)

target_include_directories(pw_sys_io.nrfconnect PUBLIC $ENV{ZEPHYR_BASE}/include)

target_link_libraries(pw_sys_io.nrfconnect PUBLIC
    zephyr_interface
    pw_sys_io
)

target_compile_options(pw_sys_io.nrfconnect PUBLIC
    -Wno-error=redundant-decls
)

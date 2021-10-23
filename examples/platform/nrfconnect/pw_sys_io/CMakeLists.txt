include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

add_library(suppress_zephyr_warnings INTERFACE)
target_compile_options(suppress_zephyr_warnings INTERFACE
    -Wno-redundant-decls
    -Wno-missing-field-initializers
)

pw_add_module_library(pw_sys_io.nrfconnect
    SOURCES
      sys_io_nrfconnect.cc
    PRIVATE_DEPS
      pw_sys_io
      suppress_zephyr_warnings
)

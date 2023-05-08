include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

add_library(suppress_zephyr_warnings INTERFACE)
target_compile_options(suppress_zephyr_warnings INTERFACE
    -Wno-redundant-decls
    -Wno-missing-field-initializers
    -Wno-cast-qual
    -Wno-undef
)

pw_add_library(pw_sys_io.telink STATIC
    SOURCES
      sys_io_telink.cc
    PUBLIC_INCLUDES
      public
    PRIVATE_DEPS
      pw_sys_io
      suppress_zephyr_warnings
      pw_sys_io.default_putget_bytes
)

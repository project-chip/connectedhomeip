include($ENV{PW_ROOT}/pw_build/pigweed.cmake)

# This is compiled in gn build, just provide headers and let linker sort it out.
pw_add_library(pw_sys_io.bee INTERFACE
    PRIVATE_DEPS
      pw_sys_io
    HEADERS
      public
)

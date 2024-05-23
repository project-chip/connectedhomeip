idf_component_register(SRCS "repo/c/qrcodegen.c"
                       INCLUDE_DIRS "repo/c")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-unknown-pragmas")

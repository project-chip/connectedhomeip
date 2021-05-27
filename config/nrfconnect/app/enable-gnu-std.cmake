add_library(gnu14 INTERFACE)
target_compile_options(gnu14 INTERFACE -std=gnu++14 -D_SYS__PTHREADTYPES_H_)
target_link_libraries(app PRIVATE gnu14)


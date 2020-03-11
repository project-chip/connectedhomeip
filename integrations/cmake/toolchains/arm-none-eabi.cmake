# generated cmake toolchain file for
include (CMakeForceCompiler)

# We are cross compiling so we don't want compiler tests to run, as they will fail
set(CMAKE_SYSTEM_NAME Generic)
# Set processor type
set(CMAKE_SYSTEM_PROCESSOR arm)

SET(CMAKE_CROSSCOMPILING 1)

set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")   # Pass: -x assembler-with-cpp
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_LINKER "arm-none-eabi-g++")         # Always link C++ with g++

# -ffunction-sections -fdata-sections -fno-strict-aliasing -fshort-enums --specs=nosys.specs

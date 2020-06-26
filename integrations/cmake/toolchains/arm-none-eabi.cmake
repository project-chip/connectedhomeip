# generated cmake toolchain file for
include (CMakeForceCompiler)

# We are cross compiling so we don't want compiler tests to run, as they will fail
set(CMAKE_SYSTEM_NAME Generic)
# Set processor type
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_CROSSCOMPILING 1)

set(ARM_GCC_INSTALL_ROOT $ENV{ARM_GCC_INSTALL_ROOT})
set(CROSSCOMPILE_PREFIX "${ARM_GCC_INSTALL_ROOT}/arm-none-eabi-")

set(CMAKE_ASM_COMPILER "${CROSSCOMPILE_PREFIX}gcc")   # Pass: -x assembler-with-cpp
set(CMAKE_C_COMPILER "${CROSSCOMPILE_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${CROSSCOMPILE_PREFIX}g++")
set(CMAKE_LINKER "${CROSSCOMPILE_PREFIX}g++")         # Always link C++ with g++ rather than gcc or ld

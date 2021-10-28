cmake_minimum_required(VERSION 3.6)

project(chip-gn)

set(chip_dir "${ameba_matter_root}")
set(chip_dir_output "${matter_output_path}/chip")
set(chip_c_flags "")
set(chip_cpp_flags "")
set(chip-gn chip-gn)

get_filename_component(CHIP_ROOT ${chip_dir} REALPATH)
get_filename_component(CHIP_OUTPUT ${chip_dir_output} REALPATH)
get_filename_component(LIB_ROOT ${prj_root}/GCC-RELEASE/project_hp/asdk/lib/application REALPATH)

include(ExternalProject)

# FOR CHIP
string(APPEND CHIP_GN_ARGS)

list(
    APPEND CHIP_CFLAGS

    -DCHIP_PROJECT=1
    -DCONFIG_PLATFORM_8721D
    -DCONFIG_USE_MBEDTLS_ROM_ALG
    -DCONFIG_FUNCION_O0_OPTIMIZE
    -DDM_ODM_SUPPORT_TYPE=32
    -DCHIP_DEVICE_LAYER_TARGET=Ameba
    -DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>
    -D_POSIX_REALTIME_SIGNALS
)

list(
    APPEND CHIP_CXXFLAGS

    -DFD_SETSIZE=10
    -Wno-sign-compare
    -Wno-unused-function
    -Wno-unused-but-set-variable
    -Wno-unused-variable
    -Wno-deprecated-declarations
    -Wno-unused-parameter
    -Wno-format
)

list(
    APPEND CHIP_INC

    ${CHIP_ROOT}/config/ameba
    ${CHIP_ROOT}/src/include
    ${CHIP_ROOT}/src/lib
    ${CHIP_ROOT}/src
    ${CHIP_ROOT}/src/system
    ${CHIP_ROOT}/src/app
    ${CHIP_ROOT}/third_party/nlassert/repo/include
    ${CHIP_ROOT}/third_party/nlio/repo/include
    ${CHIP_ROOT}/third_party/nlunit-test/repo/src
)

execute_process(
    COMMAND echo "mkdir CHIP output folder ..."
    COMMAND mkdir -p ${CHIP_OUTPUT}
)

foreach(tmp IN LISTS CHIP_CFLAGS)
	string(CONCAT appended "\"" ${tmp} "\", ")
	string(APPEND chip_c_flags "${appended}")
endforeach()
foreach(tmp IN LISTS GLOBAL_C_FLAGS)
	string(CONCAT appended "\"" ${tmp} "\", ")
	string(APPEND chip_c_flags "${appended}")
endforeach()
foreach(tmp IN LISTS inc_path)
	string(CONCAT appended "\"-I" ${tmp} "\", ")
	string(APPEND chip_c_flags "${appended}")
endforeach()
foreach(tmp IN LISTS CHIP_INC)
	string(CONCAT appended "\"-I" ${tmp} "\", ")
	string(APPEND chip_c_flags "${appended}")
endforeach()
foreach(tmp IN LISTS CHIP_CXXFLAGS)
	string(CONCAT appended "\"" ${tmp} "\", ")
	string(APPEND chip_cpp_flags "${appended}")
endforeach()
foreach(tmp IN LISTS GLOBAL_CPP_FLAGS)
	string(CONCAT appended "\"" ${tmp} "\", ")
	string(APPEND chip_cpp_flags "${appended}")
endforeach()
string(APPEND chip_cpp_flags "${chip_c_flags}")

set(import_str "import(\"//args.gni\")\n" )

string(APPEND CHIP_GN_ARGS "${import_str}")
string(APPEND CHIP_GN_ARGS "target_cflags_c = [${chip_c_flags}]\n")
string(APPEND CHIP_GN_ARGS "target_cflags_cc = [${chip_cpp_flags}]\n")
string(APPEND CHIP_GN_ARGS "ameba_ar = \"arm-none-eabi-ar\"\n")
string(APPEND CHIP_GN_ARGS "ameba_cc = \"arm-none-eabi-gcc\"\n")
string(APPEND CHIP_GN_ARGS "ameba_cxx = \"arm-none-eabi-c++\"\n")
string(APPEND CHIP_GN_ARGS "ameba_cpu = \"ameba\"\n")

file(GENERATE OUTPUT ${CHIP_OUTPUT}/args.gn CONTENT ${CHIP_GN_ARGS})

ExternalProject_Add(
    chip-gn
    PREFIX                  ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR              ${CHIP_ROOT}
    BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND       gn --root=${CHIP_ROOT}/config/ameba gen --check --fail-on-unused-args ${CHIP_OUTPUT}
    BUILD_COMMAND           ninja -C ${CHIP_OUTPUT}
    INSTALL_COMMAND         ""
    BUILD_BYPRODUCTS        -lCHIP
    CONFIGURE_ALWAYS        TRUE
    BUILD_ALWAYS            TRUE
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD     TRUE
)

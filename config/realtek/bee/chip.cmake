cmake_minimum_required(VERSION 3.20)

project(chip-gn)

set(chip_dir "${bee_matter_root}")
set(chip_dir_output "${matter_output_path}/chip")
set(chip_c_flags "")
set(chip_cpp_flags "")
set(chip-gn chip-gn)

set(matter_enable_rotating_id 1)

get_filename_component(CHIP_OUTPUT ${chip_dir_output} REALPATH)

include(ExternalProject)

# FOR CHIP
string(APPEND CHIP_GN_ARGS)

list(
    APPEND CHIP_CFLAGS

    -DCHIP_PROJECT=1
    -DCONFIG_USE_MBEDTLS_ROM_ALG
    -DDM_ODM_SUPPORT_TYPE=32
    -D_POSIX_REALTIME_SIGNALS
    -DCHIP_SHELL_MAX_TOKENS=11
)
# Build Matter otcli
if (matter_enable_otcli)
list(APPEND CHIP_CFLAGS -DCHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI=1)
endif (matter_enable_otcli)

if (matter_enable_factory_data)
list(APPEND CHIP_CFLAGS -DCONFIG_FACTORY_DATA=1)
endif (matter_enable_factory_data)

if (matter_enable_dlps)
list(APPEND CHIP_CFLAGS -DDLPS_EN=1)
endif (matter_enable_dlps)

list(
    APPEND CHIP_CXXFLAGS

    -DFD_SETSIZE=10
    -Wno-sign-compare
    -Wno-unused-function
    -Wno-unused-but-set-variable
    -Wno-unused-variable
    -Wno-deprecated-declarations
    -Wno-unused-parameter
    -Wno-unused-label
    -Wno-format
    -Wno-stringop-truncation
    -Wno-format-nonliteral
    -Wno-format-security
    -std=gnu++17
)

list(
    APPEND CHIP_INC

	${inc_path}
    ${CHIP_ROOT}/config/realtek/bee
    ${CHIP_ROOT}/src/include
    ${CHIP_ROOT}/src/lib
    ${CHIP_ROOT}/src
    ${CHIP_ROOT}/src/system
    ${CHIP_ROOT}/src/app
    ${CHIP_ROOT}/src/platform/realtek/BEE
    ${CHIP_ROOT}/src/platform/OpenThread
    ${CHIP_ROOT}/third_party/nlassert/repo/include
    ${CHIP_ROOT}/third_party/nlio/repo/include
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
foreach(tmp IN LISTS GLOBAL_CXX_FLAGS)
	string(CONCAT appended "\"" ${tmp} "\", ")
	string(APPEND chip_cpp_flags "${appended}")
endforeach()
string(APPEND chip_cpp_flags "${chip_c_flags}")

set(import_str "import(\"//config/realtek/bee/args.gni\")\n" )

string(APPEND CHIP_GN_ARGS "${import_str}")

string(APPEND CHIP_GN_ARGS "target_cflags_c = [${chip_c_flags}]\n")
string(APPEND CHIP_GN_ARGS "target_cflags_cc = [${chip_cpp_flags}]\n")
string(APPEND CHIP_GN_ARGS "bee_ar = \"arm-none-eabi-ar\"\n")
string(APPEND CHIP_GN_ARGS "bee_cc = \"arm-none-eabi-gcc\"\n")
string(APPEND CHIP_GN_ARGS "bee_cxx = \"arm-none-eabi-c++\"\n")
string(APPEND CHIP_GN_ARGS "bee_cpu = \"arm\"\n")
string(APPEND CHIP_GN_ARGS "chip_enable_openthread = true\n")
string(APPEND CHIP_GN_ARGS "chip_inet_config_enable_ipv4 = false\n")
string(APPEND CHIP_GN_ARGS "chip_use_transitional_commissionable_data_provider = false\n")
string(APPEND CHIP_GN_ARGS "chip_logging = true\n")
string(APPEND CHIP_GN_ARGS "chip_error_logging = true\n")
string(APPEND CHIP_GN_ARGS "chip_progress_logging = true\n")
string(APPEND CHIP_GN_ARGS "chip_detail_logging= true\n")

# project config
string(APPEND CHIP_GN_ARGS "chip_project_config_include_dirs = [\"${matter_example_path}/main/include\"]\n")
string(APPEND CHIP_GN_ARGS "chip_device_project_config_include = \"<CHIPProjectConfig.h>\"\n")
string(APPEND CHIP_GN_ARGS "chip_project_config_include = \"<CHIPProjectConfig.h>\"\n")
string(APPEND CHIP_GN_ARGS "chip_system_project_config_include = \"<CHIPProjectConfig.h>\"\n")

# Enable persistent storage audit
if (matter_enable_persistentstorage_audit)
string(APPEND CHIP_GN_ARGS "chip_support_enable_storage_api_audit = true\n")
endif (matter_enable_persistentstorage_audit)
#endif

# Build RPC
if (matter_enable_rpc)
string(APPEND CHIP_GN_ARGS "chip_build_pw_rpc_lib = true\n")
string(APPEND CHIP_GN_ARGS "pw_log_BACKEND = \"//third_party/pigweed/repo/pw_log_basic\"\n")
string(APPEND CHIP_GN_ARGS "pw_assert_BACKEND = \"//third_party/pigweed/repo/pw_assert_log:check_backend\"\n")
string(APPEND CHIP_GN_ARGS "pw_sys_io_BACKEND = \"//examples/platform/realtek/pw_sys_io:pw_sys_io_bee\"\n")
string(APPEND CHIP_GN_ARGS "dir_pw_third_party_nanopb = \"//third_party/nanopb/repo\"\n")
string(APPEND CHIP_GN_ARGS "pw_build_LINK_DEPS = [\"//third_party/pigweed/repo/pw_assert:impl\", \"//third_party/pigweed/repo/pw_log:impl\"]\n")
string(APPEND CHIP_GN_ARGS "pw_rpc_CONFIG = \"//third_party/pigweed/repo/pw_rpc:disable_global_mutex\"")
endif (matter_enable_rpc)

if(matter_enable_ftd)
string(APPEND CHIP_GN_ARGS "chip_openthread_ftd = true\n")
string(APPEND CHIP_GN_ARGS "chip_enable_icd_server = false\n")
endif()

if(matter_enable_mtd)
string(APPEND CHIP_GN_ARGS "chip_openthread_ftd = false\n")
if(matter_enable_med)
string(APPEND CHIP_GN_ARGS "chip_enable_icd_server = false\n")
else()
string(APPEND CHIP_GN_ARGS "chip_enable_icd_server = true\n")
endif()
endif()

# Build Matter Shell
if (matter_enable_shell)
string(APPEND CHIP_GN_ARGS "chip_build_libshell = true\n")
endif (matter_enable_shell)

# Build ota-requestor
if (matter_enable_ota_requestor)
string(APPEND CHIP_GN_ARGS "chip_enable_ota_requestor = true\n")
endif (matter_enable_ota_requestor)

# Rotating ID
if (matter_enable_rotating_id)
    string(APPEND CHIP_GN_ARGS "chip_enable_additional_data_advertising = true\n")
    string(APPEND CHIP_GN_ARGS "chip_enable_rotating_device_id = true\n")
else (matter_enable_rotating_id)
    string(APPEND CHIP_GN_ARGS "chip_enable_additional_data_advertising = false\n")
    string(APPEND CHIP_GN_ARGS "chip_enable_rotating_device_id = false\n")
endif (matter_enable_rotating_id)

file(GENERATE OUTPUT ${CHIP_OUTPUT}/args.gn CONTENT ${CHIP_GN_ARGS})

ExternalProject_Add(
    chip-gn
    PREFIX                  ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR              ${CHIP_ROOT}
    BINARY_DIR              ${CMAKE_CURRENT_BINARY_DIR}
    CONFIGURE_COMMAND       gn
                            --root=${CHIP_ROOT}
                            --root-target=${CHIP_ROOT}/config/realtek/bee
                            --dotfile=${CHIP_ROOT}/config/realtek/bee/.gn
                            gen
                            --check
                            --fail-on-unused-args
                            "${CHIP_OUTPUT}"
    BUILD_COMMAND           ninja -C "${CHIP_OUTPUT}" bee
    INSTALL_COMMAND         ""
    BUILD_BYPRODUCTS        -lCHIP -lPwRpc
    CONFIGURE_ALWAYS        TRUE
    BUILD_ALWAYS            TRUE
    USES_TERMINAL_CONFIGURE TRUE
    USES_TERMINAL_BUILD     TRUE
)

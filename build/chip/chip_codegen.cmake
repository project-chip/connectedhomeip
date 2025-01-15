# Run chip code generation. 
#
# Example usage:
#   chip_codegen("app"
#      INPUT     "some_file.matter"
#      GENERATOR "bridge"
#      OUTPUTS
#            "bridge/OnOff.h"
#            "bridge/LevelControl.h"
#            "bridge/Switch.h"
#            # ... more outputs
#     OUTPUT_PATH  DIR_NAME_VAR
#     OUTPUT_FILES  FILE_NAMES_VAR
#   )
#
# Arguments:
#   INPUT     - the name of the ".matter" file to use for generation
#   GENERATOR - generator to use for codegen.py
#   OUTPUTS   - EXPECTED output names. MUST match actual outputs
# 
#   OUTPUT_PATH  - [OUT] output variable will contain the directory where the
#                  files will be generated
#   OUTPUT_FILES - [OUT] output variable will contain the path of generated files.
#                  suitable to be added within a build target
#
function(chip_codegen TARGET_NAME)
    cmake_parse_arguments(ARG 
         "" 
         "INPUT;GENERATOR;OUTPUT_PATH;OUTPUT_FILES" 
         "OUTPUTS" 
         ${ARGN}
    )

    set(CHIP_CODEGEN_PREGEN_DIR "" CACHE PATH "Pre-generated directory to use instead of compile-time code generation.")

    if ("${CHIP_CODEGEN_PREGEN_DIR}" STREQUAL "")
        set(GEN_FOLDER "${CMAKE_BINARY_DIR}/gen/${TARGET_NAME}/${ARG_GENERATOR}")

        string(REPLACE ";" "\n" OUTPUT_AS_NEWLINES "${ARG_OUTPUTS}")

        file(MAKE_DIRECTORY "${GEN_FOLDER}")
        file(GENERATE
            OUTPUT "${GEN_FOLDER}/expected.outputs"
            CONTENT "${OUTPUT_AS_NEWLINES}"
        )


        set(OUT_NAMES)
        foreach(NAME IN LISTS ARG_OUTPUTS)
            list(APPEND OUT_NAMES "${GEN_FOLDER}/${NAME}")
        endforeach()

        # Python is expected to be in the path
        #
        # find_package(Python3 REQUIRED)
        add_custom_command(
            OUTPUT ${OUT_NAMES}
            COMMAND "${CHIP_ROOT}/scripts/codegen.py"
            ARGS "--generator" "${ARG_GENERATOR}"
                 "--output-dir" "${GEN_FOLDER}"
                 "--expected-outputs" "${GEN_FOLDER}/expected.outputs"
                 "${ARG_INPUT}"
            DEPENDS
                "${ARG_INPUT}"
            VERBATIM
        )

        add_custom_target(${TARGET_NAME} DEPENDS "${OUT_NAMES}")

        # Forward outputs to the parent
        set(${ARG_OUTPUT_FILES} "${OUT_NAMES}" PARENT_SCOPE)
        set(${ARG_OUTPUT_PATH} "${GEN_FOLDER}" PARENT_SCOPE)
    else()
        # Gets a path such as:
        #    examples/lock-app/lock-common/lock-app.matter
        file(RELATIVE_PATH MATTER_FILE_PATH "${CHIP_ROOT}" ${ARG_INPUT})

        # Removes the trailing file extension to get something like:
        #    examples/lock-app/lock-common/lock-app
        string(REGEX REPLACE "\.matter$" "" CODEGEN_DIR_PATH "${MATTER_FILE_PATH}")


        # Build the final location within the pregen directory
        set(GEN_FOLDER "${CHIP_CODEGEN_PREGEN_DIR}/${CODEGEN_DIR_PATH}/codegen/${ARG_GENERATOR}")

        # TODO: build a fake target of ${TARGET_NAME}

        # Here we have ${CHIP_CODEGEN_PREGEN_DIR}
        set(OUT_NAMES)
        foreach(NAME IN LISTS ARG_OUTPUTS)
            list(APPEND OUT_NAMES "${GEN_FOLDER}/${NAME}")
        endforeach()


        set(${ARG_OUTPUT_FILES} "${OUT_NAMES}" PARENT_SCOPE)
        set(${ARG_OUTPUT_PATH} "${GEN_FOLDER}" PARENT_SCOPE)

        # allow adding dependencies to a phony target since no codegen is done
        add_custom_target(${TARGET_NAME})
    endif()
endfunction()

# Run chip code generation using zap
#
# Example usage:
#   chip_codegen("app"
#      INPUT     "some_file.zap"
#      GENERATOR "app-templates"
#      OUTPUTS
#            "zap-generated/access.h",
#            "zap-generated/endpoint_config.h",
#            "zap-generated/gen_config.h",
#            "zap-generated/IMClusterCommandHandler.cpp"
#     OUTPUT_PATH  DIR_NAME_VAR
#     OUTPUT_FILES  FILE_NAMES_VAR
#   )
#
# Arguments:
#   INPUT     - the name of the ".zap" file to use for generation
#   GENERATOR - generator to use, like "app-templates"
#   TEMPLATES_JSON - 
#   OUTPUTS   - EXPECTED output names
#
#   OUTPUT_PATH  - [OUT] output variable will contain the directory where the
#                  files will be generated
#
#   OUTPUT_FILES - [OUT] output variable will contain the path of generated files.
#                  suitable to be added within a build target
#
function(chip_zapgen TARGET_NAME)
    cmake_parse_arguments(ARG
         ""
         "INPUT;GENERATOR;OUTPUT_PATH;OUTPUT_FILES;TEMPLATES_JSON"
         "OUTPUTS"
         ${ARGN}
    )

    set(CHIP_CODEGEN_PREGEN_DIR "" CACHE PATH "Pre-generated directory to use instead of compile-time code generation.")

    if ("${CHIP_CODEGEN_PREGEN_DIR}" STREQUAL "")
        set(GEN_FOLDER "${CMAKE_BINARY_DIR}/gen/${TARGET_NAME}/zapgen/${ARG_GENERATOR}")

        string(REPLACE ";" "\n" OUTPUT_AS_NEWLINES "${ARG_OUTPUTS}")

        file(MAKE_DIRECTORY "${GEN_FOLDER}")
        file(GENERATE
            OUTPUT "${GEN_FOLDER}/expected.outputs"
            CONTENT "${OUTPUT_AS_NEWLINES}"
        )

        set(OUT_NAMES)
        foreach(NAME IN LISTS ARG_OUTPUTS)
            list(APPEND OUT_NAMES "${GEN_FOLDER}/${NAME}")
        endforeach()

        if ("${ARG_GENERATOR}" STREQUAL "app-templates")

            if ("${ARG_TEMPLATES_JSON}" STREQUAL "")
                SET(TEMPLATE_PATH "${CHIP_ROOT}/src/app/zap-templates/app-templates.json")
            else()
                SET(TEMPLATE_PATH "${ARG_TEMPLATES_JSON}")
            endif()

            # TODO: unclear how to maintain these: there is no parser that can figure
            #       out links of template files and zap files and such
            SET(EXTRA_DEPENDENCIES
                "${CHIP_ROOT}/src/app/zap-templates/partials/header.zapt"
                "${CHIP_ROOT}/src/app/zap-templates/templates/app/access.zapt"
                "${CHIP_ROOT}/src/app/zap-templates/templates/app/endpoint_config.zapt"
                "${CHIP_ROOT}/src/app/zap-templates/templates/app/gen_config.zapt"
                "${CHIP_ROOT}/src/app/zap-templates/templates/app/im-cluster-command-handler.zapt"
           )
           SET(OUTPUT_SUBDIR "zap-generated")
        else()
            message(SEND_ERROR "Unsupported zap generator: ${ARG_GENERATOR}")
        endif()

        # Python is expected to be in the path
        #
        # find_package(Python3 REQUIRED)
        #
        # TODO: lockfile support should be removed as this serializes zap
        # (slower), however this is currently done because on Darwin zap startup
        # may conflict and error out with:
        #    Error: EEXIST: file already exists, mkdir '/var/folders/24/8k48jl6d249_n_qfxwsl6xvm0000gn/T/pkg/465fcc8a6282e28dc7a166859d5814d34e2fb94249a72fa9229033b5b32dff1a'
        add_custom_command(
            OUTPUT ${OUT_NAMES}
            COMMAND "${CHIP_ROOT}/scripts/tools/zap/generate.py"
            ARGS
                "--no-prettify-output"
                "--templates" "${TEMPLATE_PATH}"
                "--output-dir" "${GEN_FOLDER}/${OUTPUT_SUBDIR}"
                "--lock-file" "${CMAKE_BINARY_DIR}/zap_gen.lock"
                "--parallel"
                "${ARG_INPUT}"
            DEPENDS
                "${ARG_INPUT}"
                ${EXTRA_DEPENDENCIES}
            VERBATIM
        )

        add_custom_target(${TARGET_NAME} DEPENDS "${OUT_NAMES}")

        # Forward outputs to the parent
        set(${ARG_OUTPUT_FILES} "${OUT_NAMES}" PARENT_SCOPE)
        set(${ARG_OUTPUT_PATH} "${GEN_FOLDER}" PARENT_SCOPE)
    else()
        # Gets a path such as:
        #    examples/lock-app/lock-common/lock-app.zap
        file(RELATIVE_PATH MATTER_FILE_PATH "${CHIP_ROOT}" ${ARG_INPUT})

        # Removes the trailing file extension to get something like:
        #    examples/lock-app/lock-common/lock-app
        string(REGEX REPLACE "\.zap$" "" CODEGEN_DIR_PATH "${MATTER_FILE_PATH}")


        # Build the final location within the pregen directory
        set(GEN_FOLDER "${CHIP_CODEGEN_PREGEN_DIR}/${CODEGEN_DIR_PATH}/zap/${ARG_GENERATOR}")

        # TODO: build a fake target of ${TARGET_NAME}

        # Here we have ${CHIP_CODEGEN_PREGEN_DIR}
        set(OUT_NAMES)
        foreach(NAME IN LISTS ARG_OUTPUTS)
            list(APPEND OUT_NAMES "${GEN_FOLDER}/${NAME}")
        endforeach()


        set(${ARG_OUTPUT_FILES} "${OUT_NAMES}" PARENT_SCOPE)
        set(${ARG_OUTPUT_PATH} "${GEN_FOLDER}" PARENT_SCOPE)

        # allow adding dependencies to a phony target since no codegen is done
        add_custom_target(${TARGET_NAME})
    endif()
endfunction()

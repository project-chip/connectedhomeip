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

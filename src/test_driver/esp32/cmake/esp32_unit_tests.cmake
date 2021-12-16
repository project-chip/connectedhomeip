
# Defines a stand-alone elf unit test that can be executed in QEMU
#
# Parameters:
#    NAME    - the elf file name
#    LIBRARY - the library that contains the registered unit tests. 
#              for libASN1Tests.a use ASN1Tests as name.
#    EXTRA_LIBRAIRES - what else to add to link libraries, generally dependencies
#                      of $LIBRARY   
#
# TODO: several paths are hard-coded here and could use some updates:
#      - always links to idf::main
#      - assumes esp-idf/chip/lib is where the built libraries reside
#      - assumes a "dummy.c" source exists to be able to "add_executable"
macro(esp32_unit_test)
    cmake_parse_arguments(
        UNIT_TEST
          ""                 # options
          "NAME;LIBRARY"     # one value arguments
          "EXTRA_LIBRARIES"  # multi value arguments
          ${ARGN}
    )

    ######################## Elf  binary #######################

    add_executable(${UNIT_TEST_NAME} dummy.c)

    target_link_directories(${UNIT_TEST_NAME} PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR}/esp-idf/chip/lib
    )

    target_link_libraries(${UNIT_TEST_NAME} PUBLIC
        idf::main 
        -Wl,--whole-archive ${UNIT_TEST_LIBRARY} -Wl,--no-whole-archive
        ${UNIT_TEST_EXTRA_LIBRARIES}
        nlunit-test
        nlfaultinjection
    )
    idf_build_executable(${UNIT_TEST_NAME})

    ######################## flashable image #######################

    # This is very hacky: ESP build system will generate this, but only for the FIRST
    # executable that is used

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}.bin_timestamp"
      COMMAND ${ESPTOOLPY} elf2image ${ESPTOOLPY_FLASH_OPTIONS} ${esptool_elf2image_args}
          -o "${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}.bin" "${UNIT_TEST_NAME}"
      COMMAND ${CMAKE_COMMAND} -E echo "Generated ${UNIT_TEST_NAME}.bin"
      COMMAND ${CMAKE_COMMAND} -E md5sum "${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}" > "${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}.bin_timestamp"
      DEPENDS ${UNIT_TEST_NAME}
      VERBATIM                                                                                                                                                  
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating binary image from ${UNIT_TEST_NAME}"
    )                                                                                                                                                         
    add_custom_target(gen_project_binary_${UNIT_TEST_NAME} ALL DEPENDS "${build_dir}/${UNIT_TEST_NAME}.bin_timestamp")

endmacro()


# FIXME: elfbuilder ?
#    mkdir -p build/chip/
#    echo "#!/bin/sh" > build/chip/esp32_elf_builder.sh
#    echo set -e >> build/chip/esp32_elf_builder.sh
#    echo set -x >> build/chip/esp32_elf_builder.sh
#    echo $(CXX) $(CXXFLAGS) $(CPPFLAGS) -L$(PROJECT_PATH)/build/chip/lib -Wl,--whole-archive '$$1' -Wl,--no-whole-archive \
#         -lnlunit-test $(LDFLAGS) -lnlfaultinjection '$$2' -o $(PROJECT_PATH)/build/chip-tests.elf -Wl,-Map=$(APP_MAP) >> build/chip/esp32_elf_builder.sh

#    echo $(ESPTOOLPY) elf2image $(ESPTOOL_FLASH_OPTIONS) $(ESPTOOL_ELF2IMAGE_OPTIONS) \
#         -o $(PROJECT_PATH)/build/chip/chip-tests.bin $(PROJECT_PATH)/build/chip-tests.elf >> build/chip/esp32_elf_builder.sh
#    ln -sf $(PROJECT_PATH)/build/partitions.bin $(PROJECT_PATH)/build/chip/partitions.bin
#    mkdir -p build/chip/bootloader
#    ln -sf $(PROJECT_PATH)/build/bootloader/bootloader.bin $(PROJECT_PATH)/build/chip/bootloader.bin
#    ln -sf $(PROJECT_PATH)/idf.sh $(PROJECT_PATH)/build/chip/env.sh


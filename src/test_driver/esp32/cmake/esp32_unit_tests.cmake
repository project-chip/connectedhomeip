
# Defines a stand-alone elf unit test that can be executed in QEMU
#
# Parameters:
#    NAME    - the elf file name
#    LIBRARY - the library that contains the registered unit tests. 
#              for libASN1Tests.a use ASN1Tests as name.
#    EXTRA_LIBRAIRES - what else to add to link libraries, generally dependencies
#                      of $LIBRARY   
#
# The list ESP32_TEST_IMAGES keeps track of all output images that could
# be used for testing
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
        -lSupportTesting
        nlunit-test
        nlfaultinjection
    )

    add_dependencies(${UNIT_TEST_NAME} idf::main)
    add_dependencies(${UNIT_TEST_NAME} idf::chip)
    # TODO:
    #   - this does NOT properly handle dependencies  on UNIT_TEST_LIBRARY and such,
    #     so changes in the tests themselves will not re-gen

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
    add_custom_target(gen_binary_${UNIT_TEST_NAME} ALL DEPENDS "${build_dir}/${UNIT_TEST_NAME}.bin_timestamp")


    ###################### Image executable in QEMU #################


    # A runnable image is a 4MB file with:
    #   bootloader at 0x1000
    #   partition table at 0x8000
    #   image at 0x10000
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}.img_timestamp"

      COMMAND dd if=/dev/zero bs=1024 count=4096 of=${UNIT_TEST_NAME}.img
      COMMAND dd if=${CMAKE_CURRENT_BINARY_DIR}/bootloader/bootloader.bin bs=1 seek=4096 of=${UNIT_TEST_NAME}.img conv=notrunc
      COMMAND dd if=${CMAKE_CURRENT_BINARY_DIR}/partition_table/partition-table.bin bs=1 seek=32768 of=${UNIT_TEST_NAME}.img conv=notrunc
      COMMAND dd if=${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}.bin bs=1 seek=65536 of=${UNIT_TEST_NAME}.img conv=notrunc

      COMMAND ${CMAKE_COMMAND} -E echo "Generated ${UNIT_TEST_NAME}.img"
      COMMAND ${CMAKE_COMMAND} -E md5sum "${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}.img" > "${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_NAME}.img_timestamp"

      DEPENDS gen_binary_${UNIT_TEST_NAME}
      VERBATIM
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating binary image from ${UNIT_TEST_NAME}"
    )
    add_custom_target(gen_image_${UNIT_TEST_NAME} ALL DEPENDS "${build_dir}/${UNIT_TEST_NAME}.img_timestamp")

    LIST(APPEND ESP32_TEST_IMAGES ${UNIT_TEST_NAME}.img)


    # IMAGE CAN BE RUN AS
    # $QEMU_ESP32 -nographic -no-reboot -machine esp32 -drive file=out/esp32-qemu-tests/testASN1.img,if=mtd,format=raw
endmacro()

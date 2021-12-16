
# Defines a stand-alone elf unit test that can be executed in QEMU
#
# Parameters:
#    NAME    - the elf file name
#    LIBRARY - the library that contains the registered unit tests. 
#              for libASN1Tests.a use ASN1Tests as name.
#    EXTRA_LIBRAIRES - what else to add to link libraries, generally dependencies
#                      of $LIBRARY   
macro(esp32_unit_test)
    cmake_parse_arguments(
        UNIT_TEST
          ""                 # options
          "NAME;LIBRARY"     # one value arguments
          "EXTRA_LIBRARIES"  # multi value arguments
          ${ARGN}
    )

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
endmacro()
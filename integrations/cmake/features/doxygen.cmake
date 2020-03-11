#
#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the CHIP SDK CMake template for adding the `doc` target
#      for building Doxygen documentaion.
#

option(BUILD_DOC "Build documentation" ON)

if (BUILD_DOC)

  # Map cmake variables to names used by Doxyfile.in
  set(PACKAGE_NAME ${PROJECT_NAME})
  set(PACKAGE_VERSION ${PROJECT_VERSION})
  set(abs_srcdir ${CHIP_PROJECT_SOURCE_DIR}/docs)
  set(abs_builddir ${CHIP_PROJECT_BINARY_DIR}/docs)
  set(abs_top_srcdir ${CHIP_PROJECT_SOURCE_DIR})
  set(abs_top_builddir ${CHIP_PROJECT_BINARY_DIR})

  # check if Doxygen is installed
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
      # set input and output files
      set(DOXYGEN_IN ${CHIP_PROJECT_SOURCE_DIR}/docs/Doxyfile.in)
      set(DOXYGEN_OUT ${CHIP_PROJECT_BINARY_DIR}/docs/Doxyfile)

      # request to configure the file
      configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})
      message("Doxygen build started")

      # note the option ALL which allows to build the docs together with the application
      add_custom_target( doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CHIP_PROJECT_BINARY_DIR/docs}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)

endif (BUILD_DOC)

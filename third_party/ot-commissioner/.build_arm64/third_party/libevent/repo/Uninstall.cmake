# https://gitlab.kitware.com/cmake/community/wikis/FAQ#can-i-do-make-uninstall-with-cmake

if(NOT EXISTS "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: /Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/install_manifest.txt")
endif(NOT EXISTS "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/install_manifest.txt")

file(READ "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach(file ${files})
  message(STATUS "Uninstalling $ENV{DESTDIR}${file}")
  if(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
    exec_program(
      "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/.environment/cipd/pigweed/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
      OUTPUT_VARIABLE rm_out
      RETURN_VALUE rm_retval
      )
    if(NOT "${rm_retval}" STREQUAL 0)
      message(FATAL_ERROR "Problem when removing $ENV{DESTDIR}${file}")
    endif(NOT "${rm_retval}" STREQUAL 0)
  else(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
    message(STATUS "File $ENV{DESTDIR}${file} does not exist.")
  endif(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
endforeach(file)

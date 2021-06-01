# - Config file for the Libevent package
# It defines the following variables
#  LIBEVENT_INCLUDE_DIRS - include directories for FooBar
#  LIBEVENT_LIBRARIES    - libraries to link against

# Get the path of the current file.
get_filename_component(LIBEVENT_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Set the include directories.
set(LIBEVENT_INCLUDE_DIRS "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include;/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/include")

# Include the project Targets file, this contains definitions for IMPORTED targets.
include(${LIBEVENT_CMAKE_DIR}/LibeventTargets.cmake)

# IMPORTED targets from LibeventTargets.cmake
set(LIBEVENT_LIBRARIES event event_core event_extra)


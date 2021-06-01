#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "event_core_static" for configuration "Release"
set_property(TARGET event_core_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(event_core_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libevent_core.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS event_core_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_event_core_static "${_IMPORT_PREFIX}/lib/libevent_core.a" )

# Import target "event_extra_static" for configuration "Release"
set_property(TARGET event_extra_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(event_extra_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libevent_extra.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS event_extra_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_event_extra_static "${_IMPORT_PREFIX}/lib/libevent_extra.a" )

# Import target "event_pthreads_static" for configuration "Release"
set_property(TARGET event_pthreads_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(event_pthreads_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libevent_pthreads.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS event_pthreads_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_event_pthreads_static "${_IMPORT_PREFIX}/lib/libevent_pthreads.a" )

# Import target "event_static" for configuration "Release"
set_property(TARGET event_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(event_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libevent.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS event_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_event_static "${_IMPORT_PREFIX}/lib/libevent.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

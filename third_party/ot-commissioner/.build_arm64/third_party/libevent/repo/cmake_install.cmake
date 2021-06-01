# Install script for directory: /Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/lib/libevent_core.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/event2" TYPE FILE FILES
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/keyvalq_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/listener.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/thread.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/util.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/visibility.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/include/event2/event-config.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/pkgconfig/libevent_core.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib/pkgconfig" TYPE FILE FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/libevent_core.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/lib/libevent_extra.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/event2" TYPE FILE FILES
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/keyvalq_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/listener.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/thread.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/util.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/visibility.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/include/event2/event-config.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/pkgconfig/libevent_extra.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib/pkgconfig" TYPE FILE FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/libevent_extra.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/lib/libevent_pthreads.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/event2" TYPE FILE FILES
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/keyvalq_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/listener.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/thread.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/util.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/visibility.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/include/event2/event-config.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/pkgconfig/libevent_pthreads.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib/pkgconfig" TYPE FILE FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/libevent_pthreads.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/lib/libevent.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/event2" TYPE FILE FILES
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/bufferevent_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/buffer_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/dns_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/event_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/http_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/keyvalq_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/listener.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/rpc_struct.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/tag_compat.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/thread.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/util.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event2/visibility.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/include/event2/event-config.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/pkgconfig/libevent.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib/pkgconfig" TYPE FILE FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/libevent.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/evdns.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/evrpc.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/event.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/evhttp.h"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/include/evutil.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/cmake/libevent/LibeventConfig.cmake;/usr/local/lib/cmake/libevent/LibeventConfigVersion.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib/cmake/libevent" TYPE FILE FILES
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo//CMakeFiles/LibeventConfig.cmake"
    "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/LibeventConfigVersion.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libevent/LibeventTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libevent/LibeventTargets.cmake"
         "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/CMakeFiles/Export/lib/cmake/libevent/LibeventTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libevent/LibeventTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libevent/LibeventTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libevent" TYPE FILE FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/CMakeFiles/Export/lib/cmake/libevent/LibeventTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libevent" TYPE FILE FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/.build_arm64/third_party/libevent/repo/CMakeFiles/Export/lib/cmake/libevent/LibeventTargets-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xruntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/Users/ritikananda/AndroidStudioProjects/connectedhomeipproject/third_party/ot-commissioner/repo/third_party/libevent/repo/event_rpcgen.py")
endif()


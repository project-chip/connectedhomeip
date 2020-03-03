/*
 *
 *    Copyright (c) 2016-2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file contains default compile-time configuration
 *      constants for the Nest Weave System Layer, a common
 *      abstraction layer for the system networking components
 *      underlying the various Weave target network layers.
 *
 *      Package integrators that wish to override these values should
 *      either use preprocessor definitions or create a project-
 *      specific SystemProjectConfig.h header and then assert
 *      HAVE_SYSTEMPROJECTCONFIG_H via the package configuration tool
 *      via --with-weave-system-project-includes=DIR where DIR is
 *      the directory that contains the header.
 *
 *      NOTE WELL: On some platforms, this header is included by
 *      C-language programs.
 */

#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

/* Platform include headers */
#include <BuildConfig.h>

/* Include a Weave project-specific configuration file, if defined.
 *
 * An application or module that incorporates Weave can define a project
 * configuration file to override standard Weave configuration with
 * application-specific values.  The WeaveProjectConfig.h file is typically
 * located outside the OpenWeave source tree, alongside the source code for the
 * application.  The config file is included here to enable certain system-wide
 * configuration options, primarily related to logging and error reporting.
 */
#ifdef WEAVE_PROJECT_CONFIG_INCLUDE
#include WEAVE_PROJECT_CONFIG_INCLUDE
#endif

/* Include a Weave platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard Weave
 * configuration that are specific to the platform or OS on which Weave is
 * running.  It is typically provided as apart of an adaptation layer that
 * adapts OpenWeave to the target environment.  This adaptation layer may be
 * included in the OpenWeave source tree itself or implemented externally.  The
 * config file is included here to enable certain system-wide configuration
 * options, primarily related to logging and error reporting.
 */
#ifdef WEAVE_PLATFORM_CONFIG_INCLUDE
#include WEAVE_PLATFORM_CONFIG_INCLUDE
#endif

/* Include a SystemLayer project-specific configuration file, if defined.
 *
 * An application or module that incorporates Weave can define a project configuration
 * file to override standard System Layer configuration with application-specific values.
 * The project config file is typically located outside the OpenWeave source tree,
 * alongside the source code for the application.
 */
#ifdef SYSTEM_PROJECT_CONFIG_INCLUDE
#include SYSTEM_PROJECT_CONFIG_INCLUDE
#endif // SYSTEM_PROJECT_CONFIG_INCLUDE

/* Include a SystemLayer platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard System Layer configuration
 * that are specific to the platform or OS on which Weave is running.  It is typically
 * provided as apart of an adaptation layer that adapts OpenWeave to the target
 * environment.  This adaptation layer may be included in the OpenWeave source tree
 * itself or implemented externally.
 */
#ifdef SYSTEM_PLATFORM_CONFIG_INCLUDE
#include SYSTEM_PLATFORM_CONFIG_INCLUDE
#endif

/*--- Sanity check on the build configuration logic. ---*/

#if !(WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS)
#error "REQUIRED: WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS"
#endif // !(WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS)

#if WEAVE_SYSTEM_CONFIG_USE_LWIP && WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#error "FORBIDDEN: WEAVE_SYSTEM_CONFIG_USE_LWIP && WEAVE_SYSTEM_CONFIG_USE_SOCKETS"
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP && WEAVE_SYSTEM_CONFIG_USE_SOCKETS

// clang-format off

/**
 *  @def WEAVE_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
 *
 *  @brief
 *    This boolean configuration option is (1) if the obsolescent features of the Weave System Layer are provided.
 */
#ifndef WEAVE_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#define WEAVE_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES 0
#endif //  WEAVE_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

/**
 *  @def WEAVE_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION
 *
 *  @brief
 *      Define as 1 to transfer the project configuration variable definitions from InetProjectConfig.h into the corresponding
 *      variables for the Weave System Layer.
 */
#ifndef WEAVE_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION
#define WEAVE_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION 0
#endif // WEAVE_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION

#if WEAVE_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION
#ifdef INET_PROJECT_CONFIG_INCLUDE
#if WEAVE_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
/*
 * NOTE WELL: the `INET_LWIP` and `INET_SOCKETS` configuration parameters used to be generated directly by the `autoconf` system.
 * Historically, those definitions appeared in `$WEAVE/src/include/BuildConfig.h` and the build configuration logic in some systems
 * that have `InetProjectConfig.h` may still be relying on these definitions already being present in the logic prior to the
 * inclusion of <InetProjectConfig.h> and they must accordingly be defined here to provide for transferring the contents of the
 * INET layer configuration properly.
 */
#ifndef INET_LWIP
#define INET_LWIP WEAVE_SYSTEM_CONFIG_USE_LWIP
#endif // !defined(INET_LWIP)

#ifndef INET_SOCKETS
#define INET_SOCKETS WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#endif // !defined(INET_SOCKETS)
#endif // WEAVE_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#include INET_PROJECT_CONFIG_INCLUDE
#endif // INET_PROJECT_CONFIG_INCLUDE

#if !defined(WEAVE_SYSTEM_CONFIG_POSIX_LOCKING) && defined(INET_CONFIG_POSIX_LOCKING)
#define WEAVE_SYSTEM_CONFIG_POSIX_LOCKING INET_CONFIG_POSIX_LOCKING
#endif // !defined(WEAVE_SYSTEM_CONFIG_POSIX_LOCKING) && defined(INET_CONFIG_POSIX_LOCKING)

#if !defined(WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING) && defined(INET_CONFIG_FREERTOS_LOCKING)
#define WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING INET_CONFIG_FREERTOS_LOCKING
#endif // !defined(WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING) && defined(INET_CONFIG_FREERTOS_LOCKING)

#if !defined(WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC) && defined(INET_CONFIG_NUM_BUFS)
#define WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC INET_CONFIG_NUM_BUFS
#endif // !defined(WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC) && defined(INET_CONFIG_NUM_BUFS)

#if !defined(WEAVE_SYSTEM_CONFIG_NUM_TIMERS) && defined(INET_CONFIG_NUM_TIMERS)
#define WEAVE_SYSTEM_CONFIG_NUM_TIMERS INET_CONFIG_NUM_TIMERS
#endif // !defined(WEAVE_SYSTEM_CONFIG_NUM_TIMERS) && defined(INET_CONFIG_NUM_TIMERS)

#endif // WEAVE_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION

/* Standard include headers */
#ifndef WEAVE_SYSTEM_CONFIG_ERROR_TYPE
#include <stdint.h>
#endif /* WEAVE_SYSTEM_CONFIG_ERROR_TYPE */

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#include <lwip/opt.h>
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

/* Configuration option variables defined below */

/**
 *  @def WEAVE_SYSTEM_CONFIG_NO_LOCKING
 *
 *  @brief
 *      Disable the use of locking within the system layer..
 *
 *      Unless you are simulating an LwIP-based system on a Unix-style host, this value should be left at its default.
 */
#ifndef WEAVE_SYSTEM_CONFIG_NO_LOCKING
#define WEAVE_SYSTEM_CONFIG_NO_LOCKING 0
#endif /* WEAVE_SYSTEM_CONFIG_NO_LOCKING */

/**
 *  @def WEAVE_SYSTEM_CONFIG_POSIX_LOCKING
 *
 *  @brief
 *      Use POSIX locking. This is enabled by default when not compiling for BSD sockets.
 *
 *      Unless you are simulating an LwIP-based system on a Unix-style host, this value should be left at its default.
 */
#ifndef WEAVE_SYSTEM_CONFIG_POSIX_LOCKING
#define WEAVE_SYSTEM_CONFIG_POSIX_LOCKING 1
#endif /* WEAVE_SYSTEM_CONFIG_POSIX_LOCKING */

/**
 *  @def WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING
 *
 *  @brief
 *      Use FreeRTOS locking.
 *
 *      This should be generally asserted (1) for FreeRTOS + LwIP-based systems and deasserted (0) for BSD sockets-based systems.
 *
 *      However, if you are simulating an LwIP-based system atop POSIX threads and BSD sockets, this should also be deasserted (0).
 */
#ifndef WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING
#define WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#endif /* WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING */

/**
 *  @def WEAVE_SYSTEM_CONFIG_NO_LOCKING
 *
 *  @brief
 *      Disable the use of locking within the system layer.
 *
 *      This value is mutually exclusive with WEAVE_SYSTEM_CONFIG_POSIX_LOCKING and WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING.
 */
#ifndef WEAVE_SYSTEM_CONFIG_NO_LOCKING
#define WEAVE_SYSTEM_CONFIG_NO_LOCKING 0
#endif /* WEAVE_SYSTEM_CONFIG_NO_LOCKING */

#if !(WEAVE_SYSTEM_CONFIG_POSIX_LOCKING || WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING || WEAVE_SYSTEM_CONFIG_NO_LOCKING)
#error "REQUIRED: WEAVE_SYSTEM_CONFIG_POSIX_LOCKING || WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING || WEAVE_SYSTEM_CONFIG_NO_LOCKING"
#endif // !(WEAVE_SYSTEM_CONFIG_POSIX_LOCKING || WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING || WEAVE_SYSTEM_CONFIG_NO_LOCKING)

#if WEAVE_SYSTEM_CONFIG_NO_LOCKING && (WEAVE_SYSTEM_CONFIG_POSIX_LOCKING || WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING)
#error "FORBIDDEN: WEAVE_SYSTEM_CONFIG_NO_LOCKING && (WEAVE_SYSTEM_CONFIG_POSIX_LOCKING || WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING)"
#endif // WEAVE_SYSTEM_CONFIG_NO_LOCKING && (WEAVE_SYSTEM_CONFIG_POSIX_LOCKING || WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING)

#if WEAVE_SYSTEM_CONFIG_POSIX_LOCKING && WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING
#error "FORBIDDEN: WEAVE_SYSTEM_CONFIG_POSIX_LOCKING && WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING"
#endif // WEAVE_SYSTEM_CONFIG_POSIX_LOCKING && WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING

#ifndef WEAVE_SYSTEM_CONFIG_ERROR_TYPE

/**
 *  @def WEAVE_SYSTEM_CONFIG_ERROR_TYPE
 *
 *  @brief
 *      This defines the data type used to represent errors for the Weave System Layer subsystem.
 */
#define WEAVE_SYSTEM_CONFIG_ERROR_TYPE int32_t

/**
 *  @def WEAVE_SYSTEM_CONFIG_NO_ERROR
 *
 *  @brief
 *      This defines the Weave System Layer error code for no error or success.
 */
#ifndef WEAVE_SYSTEM_CONFIG_NO_ERROR
#define WEAVE_SYSTEM_CONFIG_NO_ERROR 0
#endif /* WEAVE_SYSTEM_CONFIG_NO_ERROR */

/**
 *  @def WEAVE_SYSTEM_CONFIG_ERROR_MIN
 *
 *  @brief
 *      This defines the base or minimum Weave System Layer error number range.
 */
#ifndef WEAVE_SYSTEM_CONFIG_ERROR_MIN
#define WEAVE_SYSTEM_CONFIG_ERROR_MIN 7000
#endif /* WEAVE_SYSTEM_CONFIG_ERROR_MIN */

/**
 *  @def WEAVE_SYSTEM_CONFIG_ERROR_MAX
 *
 *  @brief
 *      This defines the top or maximum Weave System Layer error number range.
 */
#ifndef WEAVE_SYSTEM_CONFIG_ERROR_MAX
#define WEAVE_SYSTEM_CONFIG_ERROR_MAX 7999
#endif /* WEAVE_SYSTEM_CONFIG_ERROR_MAX */

/**
 *  @def _WEAVE_SYSTEM_CONFIG_ERROR
 *
 *  @brief
 *      This defines a mapping function for Weave System Layer errors that allows mapping such errors into a platform- or
 *      system-specific range.
 */
#ifndef _WEAVE_SYSTEM_CONFIG_ERROR
#define _WEAVE_SYSTEM_CONFIG_ERROR(e) (WEAVE_SYSTEM_CONFIG_ERROR_MIN + (e))
#endif /* _WEAVE_SYSTEM_CONFIG_ERROR */

#endif /* WEAVE_SYSTEM_CONFIG_ERROR_TYPE */

/**
 *  @def WEAVE_SYSTEM_HEADER_RESERVE_SIZE
 *
 *  @brief
 *      The number of bytes to reserve in a network packet buffer to contain
 *      the Weave message and exchange headers.
 *
 *      This number was calculated as follows:
 *
 *      Weave Message Header:
 *
 *          2 -- Frame Length
 *          2 -- Message Header
 *          4 -- Message Id
 *          8 -- Source Node Id
 *          8 -- Destination Node Id
 *          2 -- Key Id
 *
 *      Weave Exchange Header:
 *
 *          1 -- Application Version
 *          1 -- Message Type
 *          2 -- Exchange Id
 *          4 -- Profile Id
 *          4 -- Acknowleged Message Id
 *
 *    @note A number of these fields are optional or not presently used. So most headers will be considerably smaller than this.
 */
#ifndef WEAVE_SYSTEM_HEADER_RESERVE_SIZE
#define WEAVE_SYSTEM_HEADER_RESERVE_SIZE 38
#endif /* WEAVE_SYSTEM_HEADER_RESERVE_SIZE */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
 *
 *  @brief
 *      This is the total number of packet buffers for the BSD sockets configuration.
 *
 *      This may be set to zero (0) to enable unbounded dynamic allocation using malloc.
 */
#ifndef WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
#define WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC 15
#endif /* WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
 *
 *  @brief
 *      The maximum size an application can use with \c PacketBuffer. This is not the raw memory size consumed by
 *      a \c PacketBuffer object.
 *
 *  @note
 *      Only socket platforms can override the default value. On LwIP-based platforms, the size is derived from the PBUF size
 *      and overriding the value will result in a compile-time error.
 *
 *      This value should be set large enough to accomodate the usage of PacketBuffer in the system. In particular, for the use
 *      in Weave, the value should be set to accomodate the desired path MTU (i.e. the largest IP packet that can be sent over
 *      the network interface) plus any protocol overhead.
 *
 *      For example, sending an IP packet over the tunnel requires additional overheads that depend on platform's network
 *      interface. On socket platforms, the tunnel protocol overhead is 47 bytes; on LwIP platforms the overhead is 101 bytes,
 *      plus any "sub-Ethernet" data structure space (e.g. the linked list pointers used by some interfaces to queue packets).
 *
 *      The overheads are calculated as follows:
 *
 *          (variable) -- "Sub-Ethernet" data structures (LwIP-only)
 *          14 -- Ethernet Header (LwIP-only)
 *          20 -- IPv4 Header (LwIP-only)
 *          20 -- TCP Header (LwIP-only)
 *          2  -- Weave Message Length
 *          24 -- Weave Header
 *          1  -- Tunnel Header
 *          20 -- Crypto Trailer
 *
 *      The size of PacketBuffer structure does not need to be included in this value.
 */
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#ifdef WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
#error "WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX cannot be defined on an LwIP-based platform."
#endif /* WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX */
#else /* !WEAVE_SYSTEM_CONFIG_USE_LWIP */
#ifndef WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
#define WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX 1583
#endif /* WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX */
#endif /* !WEAVE_SYSTEM_CONFIG_USE_LWIP */

#if WEAVE_SYSTEM_CONFIG_USE_LWIP

/**
 *  @def WEAVE_SYSTEM_CONFIG_LWIP_EVENT_TYPE
 *
 *  @brief
 *      This defines the type for Weave System Layer event types, typically an integral type.
 */
#ifndef WEAVE_SYSTEM_CONFIG_LWIP_EVENT_TYPE
#define WEAVE_SYSTEM_CONFIG_LWIP_EVENT_TYPE int
#endif /* WEAVE_SYSTEM_CONFIG_LWIP_EVENT_TYPE */

/**
 *  @def WEAVE_SYSTEM_CONFIG_LWIP_EVENT_UNRESERVED_CODE
 *
 *  @brief
 *      This defines the first number in the default event code space not reserved for use by the Weave System Layer.
 *      Event codes used by each layer must not overlap.
 */
#ifndef WEAVE_SYSTEM_CONFIG_LWIP_EVENT_UNRESERVED_CODE
#define WEAVE_SYSTEM_CONFIG_LWIP_EVENT_UNRESERVED_CODE  32
#endif /* WEAVE_SYSTEM_CONFIG_LWIP_EVENT_UNRESERVED_CODE */

/**
 *  @def _WEAVE_SYSTEM_CONFIG_LWIP_EVENT
 *
 *  @brief
 *      This defines a mapping function for Weave System Layer codes for describing the types of events for the LwIP dispatcher,
 *      which allows mapping such event types into a platform- or system-specific range.
 */
#ifndef _WEAVE_SYSTEM_CONFIG_LWIP_EVENT
#define _WEAVE_SYSTEM_CONFIG_LWIP_EVENT(e) (e)
#endif /* _WEAVE_SYSTEM_CONFIG_LWIP_EVENT */

/**
 *  @def WEAVE_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE
 *
 *  @brief
 *      This defines the type of Weave System Layer event objects or "messages" for the LwIP dispatcher.
 *
 *      Such types are not directly used by the Weave System Layer but are "passed through". Consequently a forward declaration and
 *      a const pointer or reference are appropriate.
 */
#ifndef WEAVE_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE
namespace nl {
namespace Weave {
namespace System {

struct LwIPEvent;

} // namespace System
} // namespace Weave
} // namespace nl

#define WEAVE_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE const struct nl::Weave::System::LwIPEvent*
#endif /* WEAVE_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE */

#endif /* WEAVE_SYSTEM_CONFIG_USE_LWIP */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS
 *
 *  @brief
 *      This defines whether (1) or not (0) your platform will provide the following platform- and system-specific functions:
 *      - nl::Weave::System::MapErrorPOSIX
 *      - nl::Weave::System::DescribeErrorPOSIX
 *      - nl::Weave::System::IsErrorPOSIX
 */
#ifndef WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS
#define WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS 0
#endif /* WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_POSIX_ERROR_FUNCTIONS */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS
 *
 *  @brief
 *      This defines whether (1) or not (0) your platform will provide the following system-specific functions:
 *      - nl::Weave::System::MapErrorLwIP
 *      - nl::Weave::System::DescribeErrorLwIP
 *      - nl::Weave::System::IsErrorLwIP
 */
#ifndef WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS
#define WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS 0
#endif /* WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_LWIP_ERROR_FUNCTIONS */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS
 *
 *  @brief
 *      This defines whether (1) or not (0) your platform will provide the following platform-specific functions:
 *      - nl::Weave::System::Platform::Layer::WillInit
 *      - nl::Weave::System::Platform::Layer::WillShutdown
 *      - nl::Weave::System::Platform::Layer::DidInit
 *      - nl::Weave::System::Platform::Layer::DidShutdown
 */
#ifndef WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS
#define WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS 0
#endif /* WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS
 *
 *  @brief
 *      This defines whether (1) or not (0) your platform will provide the following platform-specific functions:
 *      - nl::Weave::System::Platform::Layer::PostEvent
 *      - nl::Weave::System::Platform::Layer::DispatchEvents
 *      - nl::Weave::System::Platform::Layer::DispatchEvent
 *      - nl::Weave::System::Platform::Layer::StartTimer
 */
#ifndef WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS
#define WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS 0
#endif /* WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS */

/**
 *  @def WEAVE_SYSTEM_CONFIG_NUM_TIMERS
 *
 *  @brief
 *      This is the total number of available timers.
 */
#ifndef WEAVE_SYSTEM_CONFIG_NUM_TIMERS
#define WEAVE_SYSTEM_CONFIG_NUM_TIMERS 32
#endif /* WEAVE_SYSTEM_CONFIG_NUM_TIMERS */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PROVIDE_STATISTICS
 *
 *  @brief
 *      This defines whether (1) or not (0) the Weave System Layer provides logic for gathering and reporting statistical
 *      information for diagnostic purposes.
 */
#ifndef WEAVE_SYSTEM_CONFIG_PROVIDE_STATISTICS
#define WEAVE_SYSTEM_CONFIG_PROVIDE_STATISTICS 0
#endif // WEAVE_SYSTEM_CONFIG_PROVIDE_STATISTICS

/**
 *  @def WEAVE_SYSTEM_CONFIG_TEST
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable testing aids.
 */
#ifndef WEAVE_SYSTEM_CONFIG_TEST
#define WEAVE_SYSTEM_CONFIG_TEST 0
#endif

// clang-format on

// Configuration parameters with header inclusion dependencies

/**
 * @def WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE
 *
 * @brief
 *  The number of bytes to reserve in a network packet buffer to contain all the possible protocol encapsulation headers before the
 *  application message text. On POSIX sockets, this is WEAVE_SYSTEM_HEADER_RESERVE_SIZE. On LwIP, additional space is required for
 *  the all the headers from layer-2 up to the TCP or UDP header.
 */
#ifndef WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#define WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE \
    (PBUF_LINK_HLEN + PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN + WEAVE_SYSTEM_HEADER_RESERVE_SIZE)
#else /* !WEAVE_SYSTEM_CONFIG_USE_LWIP */
#define WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE (WEAVE_SYSTEM_HEADER_RESERVE_SIZE)
#endif /* !WEAVE_SYSTEM_CONFIG_USE_LWIP */
#endif /* WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE */

/**
 *  @def WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
 *
 *  @brief
 *      Use platform-supplied functions for System Layer clock functions.
 *
 *  Determines whether (1) or not (0) the underlying platform provides implementations for
 *  the System Layer GetClock/SetClock functions.
 */
#ifndef WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
#define WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 0
#endif // WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

/**
 *  @def WEAVE_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
 *
 *  @brief
 *      Use the POSIX time functions for System Layer clock functions.
 *
 *  Use the POSIX time functions (clock_gettime, gettimeofday, etc.) to implement the
 *  System Layer GetClock/SetClock functions.
 *
 *  Defaults to enabled if the system is using sockets.
 *
 *  This configuration is overridden if WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME is set.
 */
#ifndef WEAVE_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#define WEAVE_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 1
#else // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#define WEAVE_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 0
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#endif // WEAVE_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

/**
 *  @def WEAVE_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
 *
 *  @brief
 *      Use LwIP time function for System Layer monotonic clock functions.
 *
 *  Use the LwIP sys_now() function to implement the System Layer GetClock_Monotonic... functions.
 *
 *  Defaults to enabled if the system is using LwIP and not sockets.
 *
 *  This configuration is overridden if WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME is set.
 */
#ifndef WEAVE_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
#if WEAVE_SYSTEM_CONFIG_USE_LWIP && !WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#define WEAVE_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME 1
#else
#define WEAVE_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME 0
#endif
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

/**
 *  @def WEAVE_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD
 *
 *  @brief
 *      The earliest time at which the real time clock is considered to be valid.
 *
 *  The value is given in Unix time scaled to seconds.
 *
 *  The default value corresponds to 2000/01/01 00:00:00
 */
#ifndef WEAVE_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD
#define WEAVE_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD 946684800
#endif // WEAVE_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD

#endif // defined(SYSTEMCONFIG_H)

/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2018 Nest Labs, Inc.
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
 *      constants for the  CHIP System Layer, a common
 *      abstraction layer for the system networking components
 *      underlying the various CHIP target network layers.
 *
 *      Package integrators that wish to override these values should
 *      either use preprocessor definitions or create a project-
 *      specific SystemProjectConfig.h header and then assert
 *      HAVE_SYSTEMPROJECTCONFIG_H via the package configuration tool
 *      via --with-chip-system-project-includes=DIR where DIR is
 *      the directory that contains the header.
 *
 *      NOTE WELL: On some platforms, this header is included by
 *      C-language programs.
 */

#pragma once

/* Platform include headers */
#if CHIP_HAVE_CONFIG_H
#include <platform/CHIPDeviceBuildConfig.h>
#include <system/SystemBuildConfig.h>
#endif

/* Include a CHIP project-specific configuration file, if defined.
 *
 * An application or module that incorporates CHIP can define a project
 * configuration file to override standard CHIP configuration with
 * application-specific values.  The CHIPProjectConfig.h file is typically
 * located outside the CHIP source tree, alongside the source code for the
 * application.  The config file is included here to enable certain system-wide
 * configuration options, primarily related to logging and error reporting.
 */
#ifdef CHIP_PROJECT_CONFIG_INCLUDE
#include CHIP_PROJECT_CONFIG_INCLUDE
#endif

/* Include a CHIP platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard CHIP
 * configuration that are specific to the platform or OS on which CHIP is
 * running.  It is typically provided as apart of an adaptation layer that
 * adapts CHIP to the target environment.  This adaptation layer may be
 * included in the CHIP source tree itself or implemented externally.  The
 * config file is included here to enable certain system-wide configuration
 * options, primarily related to logging and error reporting.
 */
#ifdef CHIP_PLATFORM_CONFIG_INCLUDE
#include CHIP_PLATFORM_CONFIG_INCLUDE
#endif

/* Include a System::Layer project-specific configuration file, if defined.
 *
 * An application or module that incorporates CHIP can define a project configuration
 * file to override standard System Layer configuration with application-specific values.
 * The project config file is typically located outside the CHIP source tree,
 * alongside the source code for the application.
 */
#ifdef SYSTEM_PROJECT_CONFIG_INCLUDE
#include SYSTEM_PROJECT_CONFIG_INCLUDE
#endif // SYSTEM_PROJECT_CONFIG_INCLUDE

/* Include a System::Layer platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard System Layer configuration
 * that are specific to the platform or OS on which CHIP is running.  It is typically
 * provided as apart of an adaptation layer that adapts CHIP to the target
 * environment.  This adaptation layer may be included in the CHIP source tree
 * itself or implemented externally.
 */
#ifdef SYSTEM_PLATFORM_CONFIG_INCLUDE
#include SYSTEM_PLATFORM_CONFIG_INCLUDE
#endif

/*--- Sanity check on the build configuration logic. ---*/

#if !(CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK ||                 \
      CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT)
#error "REQUIRED: CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK"
#endif // !(CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_USE_NETWORK_FRAMEWORK)

#if CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_USE_SOCKETS
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_USE_SOCKETS"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK && CHIP_SYSTEM_CONFIG_USE_SOCKETS
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK && CHIP_SYSTEM_CONFIG_USE_SOCKETS"
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK && CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT &&                                                                                 \
    (CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK || CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_LWIP)
#error                                                                                                                             \
    "FORBIDDEN: CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT && ( CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK || CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_LWIP )"
#endif

#if CHIP_SYSTEM_CONFIG_MULTICAST_HOMING && !CHIP_SYSTEM_CONFIG_USE_SOCKETS
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_MULTICAST_HOMING CAN ONLY BE USED WITH SOCKET IMPL"
#endif

#if CHIP_SYSTEM_CONFIG_MULTICAST_HOMING && CHIP_SYSTEM_CONFIG_USE_SOCKETS && __ZEPHYR__
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_MULTICAST_HOMING WAS NOT TESTED WITH ZEPHYR"
#endif

// clang-format off

/**
 *  @def CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION
 *
 *  @brief
 *      Define as 1 to transfer the project configuration variable definitions from InetProjectConfig.h into the corresponding
 *      variables for the CHIP System Layer.
 */
#ifndef CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION
#define CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION 0
#endif // CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION

#if CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION
#ifdef INET_PROJECT_CONFIG_INCLUDE
#include INET_PROJECT_CONFIG_INCLUDE
#endif // INET_PROJECT_CONFIG_INCLUDE

#if !defined(CHIP_SYSTEM_CONFIG_POSIX_LOCKING) && defined(INET_CONFIG_POSIX_LOCKING)
#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING INET_CONFIG_POSIX_LOCKING
#endif // !defined(CHIP_SYSTEM_CONFIG_POSIX_LOCKING) && defined(INET_CONFIG_POSIX_LOCKING)

#if !defined(CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING) && defined(INET_CONFIG_FREERTOS_LOCKING)
#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING INET_CONFIG_FREERTOS_LOCKING
#endif // !defined(CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING) && defined(INET_CONFIG_FREERTOS_LOCKING)

#if !defined(CHIP_SYSTEM_CONFIG_MBED_LOCKING) && defined(INET_CONFIG_MBED_LOCKING)
#define CHIP_SYSTEM_CONFIG_MBED_LOCKING INET_CONFIG_MBED_LOCKING
#endif // !defined(CHIP_SYSTEM_CONFIG_MBED_LOCKING) && defined(INET_CONFIG_MBED_LOCKING)

#if !defined(CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING) && defined(INET_CONFIG_CMSIS_RTOS_LOCKING)
#define CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING INET_CONFIG_CMSIS_RTOS_LOCKING
#endif // !defined(CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING) && defined(INET_CONFIG_CMSIS_RTOS_LOCKING)

#if !defined(CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE) && defined(INET_CONFIG_NUM_BUFS)
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE INET_CONFIG_NUM_BUFS
#endif // !defined(CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE) && defined(INET_CONFIG_NUM_BUFS)

#endif // CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#ifdef TARGET_MCU_STM32L4
// [ MBED HACK ]
// We have to undefine |SUCCESS| here to prevent compilation error due to
// conflict with |ErrorStatus| enum type values defined in CMSIS/stm32l4xx.h.
// This problem is only related for when tests are build and nlunit-test.h is used.
#undef SUCCESS
#endif
#include <lwip/opt.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/* Configuration option variables defined below */

/**
 *  @def CHIP_SYSTEM_CONFIG_POSIX_LOCKING
 *
 *  @brief
 *      Use POSIX locking. This is enabled by default when not compiling for BSD sockets.
 *
 *      Unless you are simulating an LwIP-based system on a Unix-style host, this value should be left at its default.
 */
#ifndef CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 1
#endif /* CHIP_SYSTEM_CONFIG_POSIX_LOCKING */

/**
 *  @def CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
 *
 *  @brief
 *      Use FreeRTOS locking.
 *
 *      This should be generally asserted (1) for FreeRTOS + LwIP-based systems and deasserted (0) for BSD sockets-based systems.
 *
 *      However, if you are simulating an LwIP-based system atop POSIX threads and BSD sockets, this should also be deasserted (0).
 */
#ifndef CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#endif /* CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING */

/**
 *  @def CHIP_SYSTEM_CONFIG_MBED_LOCKING
 *
 *  @brief
 *      Use Mbed OS locking.
 *
 *      This should be generally asserted (1) for Mbed OS.
 *
 *      However, if you are simulating an LwIP-based system atop POSIX threads and BSD sockets, this should also be deasserted (0).
 */
#ifndef CHIP_SYSTEM_CONFIG_MBED_LOCKING
#define CHIP_SYSTEM_CONFIG_MBED_LOCKING 0
#endif /* CHIP_SYSTEM_CONFIG_MBED_LOCKING */

/**
 *  @def CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING
 *
 *  @brief
 *      Use CMSIS-RTOS locking.
 *
 *      This should be generally asserted (1) for CMSIS-RTOS.
 *
 *      However, if you are simulating an LwIP-based system atop POSIX threads and BSD sockets, this should also be deasserted (0).
 */
#ifndef CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING
#define CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING 0
#endif /* CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING */

/**
 *  @def CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING
 *
 *  @brief
 *      Use Zephyr native locking primitives.
 *
 *      This is recommended and enabled by default for Zephyr RTOS.
 *      Alternatively, you can use CHIP_SYSTEM_CONFIG_POSIX_LOCKING and Zephyr POSIX compatibility
 *      layer, but that solution has higher memory overhead.
 */
#ifndef CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING
#define CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING 0
#endif /* CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING */

/**
 *  @def CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
 *
 *  @brief
 *      Allocate Pool from Heap for large systems (e.g. Linux).
 */
#ifndef CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 0
#endif /* CHIP_SYSTEM_CONFIG_POOL_USE_HEAP */

/**
 *  @def CHIP_SYSTEM_CONFIG_NO_LOCKING
 *
 *  @brief
 *      Disable the use of locking within the system layer.
 *
 *      This value is mutually exclusive with CHIP_SYSTEM_CONFIG_POSIX_LOCKING and CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING and CHIP_SYSTEM_CONFIG_MBED_LOCKING.
 */
#ifndef CHIP_SYSTEM_CONFIG_NO_LOCKING
#define CHIP_SYSTEM_CONFIG_NO_LOCKING 0
#endif /* CHIP_SYSTEM_CONFIG_NO_LOCKING */

#if !(CHIP_SYSTEM_CONFIG_POSIX_LOCKING || CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING || CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)
#error "REQUIRED: CHIP_SYSTEM_CONFIG_POSIX_LOCKING || CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING || CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING"
#endif // !(CHIP_SYSTEM_CONFIG_POSIX_LOCKING || CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING || CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING && (CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING || CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_POSIX_LOCKING && (CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING || CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)"
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING && (CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING || CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)

#if CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING && (CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING && (CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)"
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING && (CHIP_SYSTEM_CONFIG_MBED_LOCKING || CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)

#if CHIP_SYSTEM_CONFIG_MBED_LOCKING && (CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_MBED_LOCKING && (CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)"
#endif // CHIP_SYSTEM_CONFIG_MBED_LOCKING && (CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING || CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)

#if CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING && (CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING && (CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)"
#endif // CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING && (CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING || CHIP_SYSTEM_CONFIG_NO_LOCKING)

#if CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING && CHIP_SYSTEM_CONFIG_NO_LOCKING
#error "FORBIDDEN: CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING && CHIP_SYSTEM_CONFIG_NO_LOCKING"
#endif // CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING && CHIP_SYSTEM_CONFIG_NO_LOCKING

/**
 *  @def CHIP_SYSTEM_HEADER_RESERVE_SIZE
 *
 *  @brief
 *      The number of bytes to reserve in a network packet buffer to contain
 *      the CHIP message and exchange headers.
 *
 *      This number was calculated as follows:
 *
 *      CHIP Crypto Header:
 *
 *          4 -- Length of encrypted block
 *          4 -- Reserve
 *          8 -- Initialization Vector
 *          8 -- Encryption Tag
 */
#ifndef CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE
#define CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE 24
#endif

/**
 *  @def CHIP_SYSTEM_HEADER_RESERVE_SIZE
 *
 *  @brief
 *      The number of bytes to reserve in a network packet buffer to contain
 *      the CHIP message and exchange headers.
 *
 *      This number was calculated as follows:
 *
 *      CHIP Message Header:
 *
 *          2 -- Frame Length
 *          2 -- Message Header
 *          4 -- Message Id
 *          8 -- Source Node Id
 *          8 -- Destination Node Id
 *          2 -- Key Id
 *
 *      CHIP Exchange Header:
 *
 *          1 -- Application Version
 *          1 -- Message Type
 *          2 -- Exchange Id
 *          4 -- Profile Id
 *          4 -- Acknowledged Message Id
 *
 *    @note A number of these fields are optional or not presently used. So most headers will be considerably smaller than this.
 */
#ifndef CHIP_SYSTEM_HEADER_RESERVE_SIZE
#define CHIP_SYSTEM_HEADER_RESERVE_SIZE (38 + CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE)
#endif /* CHIP_SYSTEM_HEADER_RESERVE_SIZE */

/**
 *  @def CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE
 *
 *  @brief
 *      This is the total number of packet buffers for the BSD sockets configuration.
 *
 *      This may be set to zero (0) to enable unbounded dynamic allocation using malloc.
 */
#ifndef CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 15
#endif /* CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE */

/**
 *  @def CHIP_SYSTEM_CONFIG_PACKETBUFFER_LWIP_PBUF_TYPE
 *
 *  @brief
 *      LwIP @pbuf_type for System::PacketBuffer allocations.
 *
 *      Note that this does not affect allocations by LwIP itself, e.g. the normal receive path.
 */
#ifndef CHIP_SYSTEM_CONFIG_PACKETBUFFER_LWIP_PBUF_TYPE
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_LWIP_PBUF_TYPE PBUF_POOL
#endif /* CHIP_SYSTEM_CONFIG_PACKETBUFFER_LWIP_PBUF_TYPE */

/**
 *  @def CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
 *
 *  @brief
 *      The maximum size an application can use with \c PacketBuffer. This is not the raw memory size consumed by
 *      a \c PacketBuffer object.
 *
 *  @note
 *      Only socket platforms can override the default value. On LwIP-based platforms, the size is derived from the PBUF size
 *      and overriding the value will result in a compile-time error.
 *
 *      This value should be set large enough to accommodate the usage of PacketBuffer in the system. In particular, for the use
 *      in CHIP, the value should be set to accommodate the desired path MTU (i.e. the largest IP packet that can be sent over
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
 *          2  -- CHIP Message Length
 *          24 -- CHIP Header
 *          1  -- Tunnel Header
 *          20 -- Crypto Trailer
 *
 *      The size of PacketBuffer structure does not need to be included in this value.
 */
#if CHIP_SYSTEM_CONFIG_USE_LWIP && !defined(DOXYGEN)
#ifdef CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
#error "CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX cannot be defined on an LwIP-based platform."
#endif /* CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX */
#else /* !CHIP_SYSTEM_CONFIG_USE_LWIP */
#ifndef CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX 1583
#endif /* CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX */

/**
 *  @def _CHIP_SYSTEM_CONFIG_LWIP_EVENT
 *
 *  @brief
 *      This defines a mapping function for CHIP System Layer codes for describing the types of events for the LwIP dispatcher,
 *      which allows mapping such event types into a platform- or system-specific range.
 */
#ifndef _CHIP_SYSTEM_CONFIG_LWIP_EVENT
#define _CHIP_SYSTEM_CONFIG_LWIP_EVENT(e) (e)
#endif /* _CHIP_SYSTEM_CONFIG_LWIP_EVENT */

#endif /* !CHIP_SYSTEM_CONFIG_USE_LWIP */

/**
 *  @def CHIP_SYSTEM_CONFIG_EVENT_TYPE
 *
 *  @brief
 *      This defines the type for CHIP System Layer event types, typically an integral type.
 */
#ifndef CHIP_SYSTEM_CONFIG_EVENT_TYPE
#define CHIP_SYSTEM_CONFIG_EVENT_TYPE int
#endif /* CHIP_SYSTEM_CONFIG_EVENT_TYPE */

/**
 *  @def CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE
 *
 *  @brief
 *      This defines the type of CHIP System Layer event objects or "messages".
 *
 *      Such types are not directly used by the CHIP System Layer but are "passed through". Consequently a forward declaration and
 *      a const pointer or reference are appropriate.
 */
#ifndef CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE

#if CHIP_SYSTEM_CONFIG_USE_LWIP
namespace chip {
namespace System {

struct LwIPEvent;

} // namespace System
} // namespace chip

#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct chip::System::LwIPEvent*

#else /* CHIP_SYSTEM_CONFIG_USE_LWIP */

#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *

#endif /* CHIP_SYSTEM_CONFIG_USE_LWIP */

#endif /* CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE */

#if CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  @def _CHIP_SYSTEM_CONFIG_LWIP_EVENT
 *
 *  @brief
 *      This defines a mapping function for CHIP System Layer codes for describing the types of events for the LwIP dispatcher,
 *      which allows mapping such event types into a platform- or system-specific range.
 */
#ifndef _CHIP_SYSTEM_CONFIG_LWIP_EVENT
#define _CHIP_SYSTEM_CONFIG_LWIP_EVENT(e) (e)
#endif /* _CHIP_SYSTEM_CONFIG_LWIP_EVENT */

#endif /* CHIP_SYSTEM_CONFIG_USE_LWIP */

/**
 *  @def CHIP_SYSTEM_CONFIG_NUM_TIMERS
 *
 *  @brief
 *      This is the total number of available timers, for configurations that use a fixed timer pool.
 */
#ifndef CHIP_SYSTEM_CONFIG_NUM_TIMERS
#define CHIP_SYSTEM_CONFIG_NUM_TIMERS 32
#endif /* CHIP_SYSTEM_CONFIG_NUM_TIMERS */

/**
 *  @def CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS
 *
 *  @brief
 *      This defines whether (1) or not (0) the CHIP System Layer provides logic for gathering and reporting statistical
 *      information for diagnostic purposes.
 */
#ifndef CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS
#define CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS 0
#endif // CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS

/**
 *  @def CHIP_SYSTEM_CONFIG_TEST
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable testing aids.
 */
#ifndef CHIP_SYSTEM_CONFIG_TEST
#define CHIP_SYSTEM_CONFIG_TEST 0
#endif

// clang-format on

// Configuration parameters with header inclusion dependencies

/**
 * @def CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE
 *
 * @brief
 *  The number of bytes to reserve in a network packet buffer to contain all the possible protocol encapsulation headers before the
 *  application message text. On POSIX sockets, this is CHIP_SYSTEM_HEADER_RESERVE_SIZE. On LwIP, additional space is required for
 *  the all the headers from layer-2 up to the TCP or UDP header.
 */
#ifndef CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE                                                                                     \
    (PBUF_LINK_HLEN + PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN + CHIP_SYSTEM_HEADER_RESERVE_SIZE)
#else /* !CHIP_SYSTEM_CONFIG_USE_LWIP */
#define CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE (CHIP_SYSTEM_HEADER_RESERVE_SIZE)
#endif /* !CHIP_SYSTEM_CONFIG_USE_LWIP */
#endif /* CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE */

/**
 *  @def CHIP_SYSTEM_CONFIG_PLATFORM_LOG
 *
 *  @brief
 *      Defines whether (1) or not (0) the system uses a platform-specific logging implementation.
 *
 *  See CHIPLogging.h for details.
 */
#ifndef CHIP_SYSTEM_CONFIG_PLATFORM_LOG
#define CHIP_SYSTEM_CONFIG_PLATFORM_LOG 0
#endif // CHIP_SYSTEM_CONFIG_PLATFORM_LOG

/**
 *  @def CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
 *
 *  @brief
 *      Use platform-supplied functions for System Layer clock functions.
 *
 *  Determines whether (1) or not (0) the underlying platform provides implementations for
 *  the System Layer GetClock/SetClock functions.
 */
#ifndef CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 0
#endif // CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
 *
 *  @brief
 *      Use the POSIX time functions for System Layer clock functions.
 *
 *  Use the POSIX time functions (clock_gettime, gettimeofday, etc.) to implement the
 *  System Layer GetClock/SetClock functions.
 *
 *  Defaults to enabled if the system is using sockets.
 *
 *  This configuration is overridden if CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME is set.
 */
#ifndef CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#define CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 1
#else // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#define CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 0
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
 *
 *  @brief
 *      Use LwIP time function for System Layer monotonic clock functions.
 *
 *  Use the LwIP sys_now() function to implement the System Clock functions.
 *
 *  Defaults to enabled if the system is using LwIP and not sockets.
 *
 *  This configuration is overridden if CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME is set.
 */
#ifndef CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_SOCKETS
#define CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME 1
#else
#define CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME 0
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_LWIP_SKIP_INIT
 *
 *  @brief
 *      Skip LwIP initalization during network setup.
 *
 *  The platform can provide its own LwIP initialization.
 *  This option allows skipping initialization steps in network setup e.g. in unit tests.
 *
 *  This configuration is overridden if CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT is set.
 */
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#ifndef CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT
#define CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT 0
#endif // CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  @def CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD
 *
 *  @brief
 *      The earliest time at which the real time clock is considered to be valid.
 *
 *  The value is given in Unix time scaled to seconds.
 *
 *  The default value corresponds to 2000/01/01 00:00:00
 */
#ifndef CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD
#define CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD 946684800
#endif // CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE
 *
 *  @brief
 *      Use the POSIX pipe() function.
 *
 *  Use the POSIX pipe() function to create an anonymous data stream.
 *
 *  Defaults to enabled if the system is using sockets (except for Zephyr RTOS).
 */
#ifndef CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE
#if (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK) && !__ZEPHYR__ && !__MBED__
#define CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE 1
#else
#define CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE 0
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS
 *
 *  @brief
 *      Include missing functions for Zephyr sockets
 *
 *  Zephyr socket API lacks some of the functions required by CHIP.
 *  If this value is set CHIP will provide the missing functions.
 *
 *  Defaults to enabled on Zephyr platforms using sockets
 */
#ifndef CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && __ZEPHYR__
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS 1
#else
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS 0
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
 *
 *  @brief
 *      Use platform API to join and leave multicast groups.
 *
 *  In case a given platform does not support adding and removing multicast
 *  addresses to a network interface using the generic network API like LWIP
 *  or sockets, this setting allows the platform layer to inject handlers
 *  which achieve the goal by other means.
 *
 *  Defaults to enabled on Zephyr platforms using sockets
 */
#ifndef CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && __ZEPHYR__
#define CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API 1
#else
#define CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API 0
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
 *
 *  @brief
 *      Use Zephyr net_if API to enumerate available network interfaces
 *
 *  Defaults to enabled on Zephyr platforms using sockets
 */
#ifndef CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && __ZEPHYR__
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF 1
#else
#define CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF 0
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

/**
 *  @def CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
 *
 *  @brief
 *      Use BSD ifaddrs.h API to enumerate available network interfaces
 *
 *  Defaults to enabled on Unix/Linux platforms using sockets
 */
#ifndef CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#if (CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK) && !__ZEPHYR__
#define CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS 1
#else
#define CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS 0
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

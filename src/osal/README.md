# CHIP Operating System Abstraction Layer (OSAL)

OSKI (Operating System Kernel Interface) is an OSAL used within Project CHIP.

The name OSKI is used to diambiguate the CHIP OSAL from external OSAL used by
other projects or vendor SDKs.

## Introduction

The CHIP OSAL is designed to provide a thin adaptation layer for portability of
the example applications and common portions of the DeviceLayer to a range of
Real Time Operating Systems (RTOS). The intent is to leverage native OS
primitives as much as possible while providing a unified interface surface for
those primitives to the rest of the CHIP system, and to deliver common OS
functionality suitable for deeply embedded environments.

CHIP OSAL provides abstractions for:

-   [tasks](#Task) (aka threads)
-   [mutex](#Mutex)
-   [semaphores](#Semaphore)
-   [queues](#Queue)
-   [timers](#Timer)
-   [time](#Time)

CHIP OSAL currently supports the above abstractions for the following OS
targets:

-   POSIX (linux) - Linux and other standard POSIX systems
-   POSIX (apple) - Apple and other Mach kernel systems using dispatch

Implementations for the following targets are planned:

-   FreeRTOS
-   Zephyr

### Motivation

Project CHIP has explicit goals to provide a unifying, interoperable, versatile,
low overhead, and robust connected home solution with explicit focus on
time-to-market. These goals require the platform layer design to be highly
scalable, allowing disparate and diverse platforms to be integrated with high
velocity. To support rapid integration of new platforms in a scalable and
maintainable way requires:

-   Maximum reuse of code, verification, and testing
-   Minimum code fragmentation, forking, and conditional compilation
-   Adaptable and thin pathway to optimized native APIs

Device platforms tend to be highly unique on the first order, but also pivot on
three major areas of common functionality. These areas of commonality define a
three dimensional matrix of possible device configurations where a shared point
on any one axis can allow code reuse across otherwise disparate platorms:

-   Device (board)

    -   Platforms that share a specific choice of chip combinations and wiring
        at the PCB level can share a common DeviceLayer port.
    -   The DeviceLayer provides the minimum interface required to interface
        CHIP to all the HW-specific details of a device such as BLE, WiFi,
        storage.
    -   Target examples are silicon vendor development boards or final product
        PCBs.
    -   A DeviceLayer is able to own all decisions about a device and impose
        hard assumptions on the particular combination of board + os + hw.
    -   A DeviceLayer is free to use an abstraction of the underlying OS or HW
        layers to provide better portability between different RTOS environments
        or across a family of SoCs.

-   Operating System (os)

    -   Platforms that share a common OS or RTOS can share a common OSAL port.
    -   Target examples are FreeRTOS, Zephyr, Linux, Mac, ...
    -   A given app or device layer port codebase may want to be retargetted
        from one OS to another. This could happen during an upgrade cycle for
        instance, or a new product may want to use a particular device layer
        port that CHIP provides, but port it to the RTOS they typically use.

-   Hardware (hw / chip)
    -   Platforms that share a common chipset, SoC, or silicon can share a
        common Hardware Abstraction Layer (HAL) from the vendor SDK.
    -   By leveraging a HAL, the same application can be retargetted to
        different chipsets across a family of similar SoCs.

The primary motivation of the CHIP OSAL is to enable code sharing and reuse in
the example applications and DeviceLayer. Rather than have a separate example
app each combination of hw + os + board, the CHIP OSAL allows an example app to
be written in a common way and be retargeted for different OS/RTOS such as
FreeRTOS, Zephyr, Linux, etc. The CHIP OSAL is intended to help make the system
more scalable as the matrix of combinations of hw + os + app increases over
time.

### Context

There is a long history of OSAL layers. Why does CHIP need its own?

While it is true the CHIP OSAL is "Yet Another OSAL", it is one that was
designed to meet the specific requirements for Project CHIP. Other OSAL projects
were considered, some with common contributors to CHIP OSAL, but each had gaps
relative the requirements for Project CHIP:

-   [nler](https://github.com/nestlabs/nler) - Nest Labs Embedded Runtime

    -   Uses event queues with event pointer semantics whereas CHIP uses message
        queues with copy semantics.
    -   Has an inconsistant API namespace. All CHIP OSAL functions predictably
        begin with `chip_os`.
    -   Was designed to enforce a particular embedded programming philosophy.
        Semaphores are notably missing for example.
    -   Imposes its own centralized timer system rather than providing a thin
        pass-through to native OS timers.
    -   Centralized timer system relies on nler event queues, which are
        antithetical to the CHIP messsage queue paradigm.

-   [npl](https://github.com/apache/mynewt-nimble/tree/master/porting/npl) -
    MyNewt NimBLE Platform Layer
    -   Uses event queues with event pointer semantics whereas CHIP uses message
        queues with copy semantics.
    -   Is embedded within a larger BLE stack project and as such isn't easily
        composable as a submodule.
    -   Uses a consistent, but domain specic API namespace: `ble_npl`

---

## Reference

### Task

A task is an independent context of code execution that runs without any
dependency on other concurrent tasks within the system. Only one task runs at
any given time. The scheduler starts and stops tasks as necessary to manage
resources according to the priorities and policies of the system. A task has no
knowledge of the underlying scheduler activity and can be swapped in and out,
but will always run with a consistent execution context and stack.

### Mutex

A mutex provides a locking mechanism for enforcing mutual exclusion and
protection of shared resources between independent paths of execution such as
different tasks or interrupts.

### Semaphore

A semaphore is a synchronization primitive which provides a means to block one
task until it is released by a signal from another task or interrupt.

### Queue

A queue is a basic primitive for intertask communication. A queue can be used to
send messages from a task or interrupt producer to a consumer task using copy
semantics.

### Timer

A timer triggers a callback function after a given amount of time has passed.

### Time

A collection of utility functions for getting current system time and converting
between milliseconds and CPU ticks is provided.

## Porting guide

The CHIP OSAL module is structured as follows:

| File / Folder           | Contents                                                            |
| ----------------------- | ------------------------------------------------------------------- |
| src/include/chip/osal.h | Public header with complete CHIP OSAL API                           |
| src/osal/include        | Common headers and utilitied that can be shared by ports            |
| src/osal/tests          | Implements portable tests of CHIP OSAL APIs                         |
| src/osal/<port>         | Implements OSAL API for a given platform <port>                     |
| <port>/chip/os_port.h   | Maps OSAL types to platform-specific definitions for a given <port> |

### Layout Details

```
.
├── src/osal              - Top level of CHIP OSAL submodule
├── include
│   └── Ring.h            - A portable ring buffer class for common use
├── README.md             - This document
└── tests
    ├── Makefile.am       - Primary automake file
    ├── Makefile.mk       - Developer utility make file for running the tests
    ├── test_os_mutex.c   - Test of chip_os_mutex
    ├── test_os_queue.c   - Test of chip_os_queue
    ├── test_os_sem.c     - Test of chip_os_sem
    ├── test_os_task.c    - Test of chip_os_task
    ├── test_os_timer.c   - Test of chip_os_timer
    ├── test_ring.cpp     - Test of Ring.h
    └── test_util.h       - Common test utilities
```

#### POSIX Port Layout

The POSIX port includes both Linux and Apple implementations.

Linux uses the standard POSIX APIs for all functionality.

Apple uses POSIX pthreads but also Grand Central dispatch for the timer and
semaphore implementations.

| OS    | Task    | Mutex | Sem      | Timer    | Time  | Queue          |
| ----- | ------- | ----- | -------- | -------- | ----- | -------------- |
| Linux | pthread | posix | posix    | posix    | posix | pthread + ring |
| MacOS | pthread | posix | dispatch | dispatch | mach  | pthread + ring |

```
├── src/osal               - Top level of CHIP OSAL submodule
├── posix
│   ├── chip
│   │   ├── os_port.h      - Primary port-specific header to hook into public src/include/chip/osal.h
│   │   ├── os_time.h      - Types related to time and timers
│   │   └── os_types.h     - All other CHIP OSAL types
│   ├── os_mutex.c         - Implementation of chip_os_mutex
│   ├── os_queue.cc        - Implementation of chip_os_queue
│   ├── os_sem.c           - Implementation of chip_os_sem
│   ├── os_task.c          - Implementation of chip_os_task
│   ├── os_time.c          - Implementation of chip_os_time
│   ├── os_timer.c         - Implementation of chip_os_timer
│   ├── os_utils.c         - Shared code for the posix port, most notably error mapping.
│   ├── os_utils.h         - Shared header for the posix port, most notably nlassert code utility macros.
│   └── RingPthread.h      - Thread-safe version of Ring using posix mutex and cond
```

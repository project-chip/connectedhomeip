# CHIP Operating System Abstraction Layer (OSAL)

## Introduction

The CHIP OSAL is designed to provide a thin adaptation layer for portability of
the example applications and common portions of the DeviceLayer to a range of
Real Time Operating Systems (RTOS). The intent is to leverage native OS
primitives as much as possible while providing a unified interface surface for
those primitives to the rest of the CHIP system, and to deliver common OS
functionality suitable for deeply embedded environments.

CHIP OSAL provides abstractions for:

-   tasks (aka threads)
-   mutex
-   semaphores
-   queues
-   timers
-   time

CHIP OSAL currently supports the above abstractions for the following OS
targets:

-   POSIX (linux) - Linux and other standard POSIX systems
-   POSIX (apple) - Apple and other Mach kernel systems using dispatch

Implementations for the following targets are planned:

-   FreeRTOS
-   Zephyr

## Motivation

Project CHIP has explicit goals to provide a unifying, interoperable, 
versatile, low overhead, and robust connected home solution with explicit 
focus on time-to-market. These goals require the platform layer design to be 
highly scalable, allowing disparate and diverse platforms to be integrated 
with high velocity. To support rapid integration of new platforms in a scalable 
and maintainable way requires:

    * Maximum reuse of code, verification, and testing
    * Minimum code fragmentation, forking, and conditional compilation

Device platforms tend to be highly unique on the first order, but also pivot 
on three major areas of common functionality. These areas of commonality define
a three dimensional matrix of possible device configurations where a shared
point on any one axis can allow code reuse across otherwise disparate platorms: 

    * Hardware (hw / chip)
        - Platforms that share a common chipset, SoC, or silicon.
        - Target examples are: efr32, esp32, nrf52, ...
        - Sharing examples are: devices that share a particular chipset selection.
        - A given app or codebase may want to be retargetted from one OS to another.

    * Operating System (os) 
        - Platforms that share a common OS or RTOS.
        - Target examples are freertos, zephyr, linux, macos, ...
        - A given app or codebase may want to be retargetted from one OS to another.

    * Device (board)
        - Platforms that share specific combinations and choices at a PCB level.
        - Target examples are: nrf52840dk, 

The primary motivation of the CHIP OSAL is to enable code sharing and reuse in the example
applications and DeviceLayer.  Rather than have a separate example app each combination of
hw+board+os, the CHIP OSAL allows example apps to be written in a common way and be targeted 
for different OS/RTOS such as FreeRTOS, Zephyr, Linux, etc. 

The DeviceLayer is abstracting the HW-specific details such as BLE, WiFi, storage.  The OSAL is 
providing a common program model to consolidate generic DeviceLayer code and example apps that 
could be targeted for different OS/RTOS such as FreeRTOS, Zephyr, Linux, etc.

It is intended to make the system scalable as the matrix of combinations of hw+os+app increases over time.


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

CHIP Operating System Abstraction Layer (OSAL)
==============================================

# Introduction

The CHIP OSAL is designed to provide a thin adaptation layer for portability
of the example applications and common portions of the DeviceLayer to a
range of Real Time Operating Systems (RTOS). The intent is to leverage native
OS primitives as much as possible while providing a unified interface surface
for those primitives to the rest of the CHIP system, and to deliver common OS
functionality suitable for deeply embedded environments.

CHIP OSAL provides abstractions for:

* tasks (aka threads)
* mutex
* semaphores
* queues
* timers
* time

CHIP OSAL currently supports the above abstractions for the following OS targets:

* POSIX (pthread)

Implementations for the following targets are planned:

* FreeRTOS
* Zephyr
* MacOS / Darwin - Adjustments to POSIX to support MacOS (mainly timer_t).

# Reference

## Task

A task is an independent context of code execution that runs without any
dependency on other concurrent tasks within the system. Only one task runs
at any given time. The scheduler starts and stops tasks as necessary to manage
resources according to the priorities and policies of the system. A task has no
knowledge of the underlying scheduler activity and can be swapped in and out,
but will always run with a consistent execution context and stack.

## Mutex

A mutex provides a locking mechanism for enforcing mutual exclusion and
protection of shared resources between independent paths of execution
such as different tasks or interrupts.

## Semaphore

A semaphore is a synchronization primitive which provides a means to block
one task until it is released by a signal from another task or interrupt.

## Queue

A queue is a basic primitive for intertask communication.  A queue
can be used to send messages from a task or interrupt producer to
a consumer task using copy semantics.

## Timer

A timer triggers a callback function after a given amount of time has passed.

## Time

A collection of utility functions for getting current system time and
converting between milliseconds and CPU ticks is provided.

# Porting guide

The CHIP OSAL module is structured as follows:

| File / Folder              | Contents                                                            |
| ---------------------------|---------------------------------------------------------------------|
| src/include/chip/osal.h    | Public header with complete CHIP OSAL API                           |
| src/osal/include           | Common headers and utilitied that can be shared by ports            |
| src/osal/tests             | Implements portable tests of CHIP OSAL APIs                         |
| src/osal/<port>            | Implements OSAL API for a given platform <port>                     |
| <port>/chip/os_port.h      | Maps OSAL types to platform-specific definitions for a given <port> |

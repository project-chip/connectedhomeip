/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Defines a global data structure containing FreeRTOS configuration
 *          information that is used by the SEGGER FreeRTOS GDB extension to
 *          enable FreeRTOS-aware debugging.
 *
 *          The file is included directly in FreeRTOS's tasks.c source file
 *          by means of the configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H
 *          configuration option.
 */

#ifndef FREERTOS_TASKS_C_ADDITIONS_H
#define FREERTOS_TASKS_C_ADDITIONS_H

/*
 * Version of the FreeRTOSDebugConfig structure
 */
#define FREERTOS_DEBUG_CONFIG_MAJOR_VERSION 1
#define FREERTOS_DEBUG_CONFIG_MINOR_VERSION 1

/*
 * The FreeRTOS memory allocation scheme in use.
 *
 * Must correspond to the heap_x.c source file included in the build (e.g. heap_1.c, heap_2.c, etc.).
 */
#ifndef FREERTOS_MEMORY_SCHEME
#define FREERTOS_MEMORY_SCHEME 1
#endif

/*
 * FreeRTOS debugging configuration information structure.
 */
const uint8_t FreeRTOSDebugConfig[] __attribute__((section(".rodata"))) = {
    FREERTOS_DEBUG_CONFIG_MAJOR_VERSION,
    FREERTOS_DEBUG_CONFIG_MINOR_VERSION,
    tskKERNEL_VERSION_MAJOR,
    tskKERNEL_VERSION_MINOR,
    tskKERNEL_VERSION_BUILD,
    FREERTOS_MEMORY_SCHEME,
    offsetof(struct tskTaskControlBlock, pxTopOfStack),
#if (tskKERNEL_VERSION_MAJOR > 8)
    offsetof(struct tskTaskControlBlock, xStateListItem),
#else
    offsetof(struct tskTaskControlBlock, xGenericListItem),
#endif
    offsetof(struct tskTaskControlBlock, xEventListItem),
    offsetof(struct tskTaskControlBlock, pxStack),
    offsetof(struct tskTaskControlBlock, pcTaskName),
    offsetof(struct tskTaskControlBlock, uxTCBNumber),
    offsetof(struct tskTaskControlBlock, uxTaskNumber),
    configMAX_TASK_NAME_LEN,
    configMAX_PRIORITIES,
    0 /* padding */
};

#endif /* FREERTOS_TASKS_C_ADDITIONS_H */

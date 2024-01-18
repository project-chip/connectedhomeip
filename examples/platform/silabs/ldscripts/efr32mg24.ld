/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
/***************************************************************************//**
 * @file     gcc_EFR32MG24.ld
 * @brief    GNU Linker Script for Cortex-M based device
 * @version  V2.2.0
 * @date     16. December 2020
 * Linker script for Silicon Labs EFR32MG24 devices
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories, Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 * 
 * The licensor of this software is Silicon Laboratories Inc.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08006000, LENGTH = 0x17A000
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x40000
}

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __StackSeal      (only if ARMv8-M stack sealing is used)
 */

ENTRY(Reset_Handler)

SECTIONS
{
  .text :
  {
    KEEP(*(.vectors))
    *(.text*)

    KEEP(*(.init))
    KEEP(*(.fini))

    /* .ctors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    /* .dtors */
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)
  
    __code_classification_validator_start__ = .;
    . = . + 0x20;
    *(code_classification_validator)
    . = ALIGN(32);
    __code_classification_validator_end__ = .;
    *(.rodata*)
  
    KEEP(*(.eh_frame*))
  } > FLASH
  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH

  __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > FLASH
  __exidx_end = .;

  .copy.table :
  {
    . = ALIGN(4);
    __copy_table_start__ = .;

    LONG (__etext)
    LONG (__data_start__)
    LONG ((__data_end__ - __data_start__) / 4)

    __copy_table_end__ = .;
  } > FLASH

  .zero.table :
  {
    . = ALIGN(4);
    __zero_table_start__ = .;
    __zero_table_end__ = .;
    __etext = ALIGN(4);
  } > FLASH

  .data : AT (__etext)
  {
    __data_start__ = .;
    *(vtable)
    *(.data*)
    . = ALIGN (4);
    PROVIDE (__ram_func_section_start = .);
    *(.ram)
    PROVIDE (__ram_func_section_end = .);

    . = ALIGN(4);
    /* preinit data */
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(4);
    /* finit data */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);

    KEEP(*(.jcr*))
    . = ALIGN(4);
    /* All data end */
    __data_end__ = .;

  } > RAM

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM AT > RAM

  __ramfuncs_start__ = .;

  __vma_ramfuncs_start__ = .;
  __lma_ramfuncs_start__ = __etext + SIZEOF(.data);

  __text_application_ram_offset__ = . - __vma_ramfuncs_start__;
  text_application_ram . : AT(__lma_ramfuncs_start__ + __text_application_ram_offset__)
  {
    . = ALIGN(4);
    __text_application_ram_start__ = .;
    *(text_application_ram)
    . = ALIGN(4);
    __text_application_ram_end__ = .;
  } > RAM

  . = ALIGN(4);
  __vma_ramfuncs_end__ = .;
  __lma_ramfuncs_end__ = __lma_ramfuncs_start__ + __text_application_ram_offset__ + SIZEOF(text_application_ram);

  __ramfuncs_end__ = .;

  .heap (COPY):
  {
    __HeapBase = .;
    __end__ = .;
    end = __end__;
    _end = __end__;
    KEEP(*(.heap*))
    __HeapLimit = .;
  } > RAM

  __main_flash_end__ = ORIGIN(FLASH) + LENGTH(FLASH);

  /* .stack_dummy section doesn't contains any symbols. It is only
   * used for linker to calculate size of stack sections, and assign
   * values to stack symbols later */
  .stack_dummy (COPY):
  {
    KEEP(*(.stack*))
 /* KEEP(*(.stackseal*))*/
  } > RAM

  /* Set stack top to end of RAM, and stack limit move down by
   * size of stack_dummy section */
  __StackTop = ORIGIN(RAM) + LENGTH(RAM);
  __StackLimit = __StackTop - SIZEOF(.stack_dummy);
  PROVIDE(__stack = __StackTop);

   .nvm (DSECT) : {
    KEEP(*(.simee*))
  } > FLASH

  /* Last page of flash is reserved for the manufacturing token space */
  linker_nvm_end = __main_flash_end__ - 8192;
  linker_nvm_begin = linker_nvm_end - SIZEOF(.nvm);
  linker_nvm_size = SIZEOF(.nvm);
  __nvm3Base = linker_nvm_begin;
  __attestation_credentials_base = linker_nvm_end;

  /* Check if data + heap + stack exceeds RAM limit */
  /*ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")*/
  ASSERT( (linker_nvm_begin + SIZEOF(.nvm)) <= __main_flash_end__, "NVM3 is excessing the flash size !")

  /* Check if FLASH usage exceeds FLASH size */
  ASSERT( LENGTH(FLASH) >= (__etext + SIZEOF(.data)), "FLASH memory overflowed !")
}

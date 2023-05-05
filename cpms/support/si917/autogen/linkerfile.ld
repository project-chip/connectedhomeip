/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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


MEMORY
{
	FLASH (rx)  : ORIGIN = 0x08012000, LENGTH = 0x400000
	RAM (rwx) : ORIGIN = 0xC, LENGTH = 0x4FC00
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
 *   __HeapBase
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */
ENTRY(Reset_Handler)

SECTIONS
{
  .text :
  {
    linker_vectors_begin = .;
    KEEP(*(.vectors))
    linker_vectors_end = .;

    __Vectors_End = .;
    __Vectors_Size = __Vectors_End - linker_vectors_begin;

    linker_code_begin = .;
    *(SORT_BY_ALIGNMENT(.text*))
    linker_code_end = .;

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

    *(.rodata*)
    *(.eh_frame*)
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

    /* Add each additional data section here */
/*
    LONG (__etext2)
    LONG (__data2_start__)
    LONG ((__data2_end__ - __data2_start__) / 4)
*/
    __copy_table_end__ = .;
  } > FLASH

  .zero.table :
  {
    . = ALIGN(4);
    __zero_table_start__ = .;
    /* Add each additional bss section here */
/*
    LONG (__bss2_start__)
    LONG ((__bss2_end__ - __bss2_start__) / 4)
*/
    __zero_table_end__ = .;
  } > FLASH

  __etext = .;

  /* Start placing output sections which are loaded into RAM */
  . = ORIGIN(RAM);

  .stack ALIGN(8) (NOLOAD):
  {
    __StackLimit = .;
    KEEP(*(.stack*))
    . = ALIGN(4);
    __StackTop = .;
    PROVIDE(__stack = __StackTop);
  } > RAM


  .noinit . (NOLOAD):
  {
    *(.noinit*);
  } > RAM

  .data . : AT (__etext)
  {
    . = ALIGN(4);
    __data_start__ = .;
    _sdata = __data_start__;

    *(vtable)
    *(.data*)
    *(SORT_BY_ALIGNMENT(.data*))
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

    . = ALIGN(4);
    /* All data end */
    __data_end__ = .;
    _edata = __data_end__;

  } > RAM

  .bss . :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    _sbss = __bss_start__;
    *(SEGGER_RTT)
    *(SORT_BY_ALIGNMENT(.bss*))
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    _ebss = __bss_end__;
  } > RAM

  .heap (COPY) :
  {
    __HeapBase = .;
    __end__ = .;
    end = __end__;
    _end = __end__;
    KEEP(*(.heap*))
    . = ORIGIN(RAM) + LENGTH(RAM);
    __HeapLimit = .;
  } > RAM
 
  __heap_size = __HeapLimit - __HeapBase;
  __ram_end__ = 0x20000000 + 0x40000;
  __main_flash_end__ = ORIGIN(FLASH) + LENGTH(FLASH);

  /* .stack_dummy section doesn't contains any symbols. It is only
   * used for linker to calculate size of stack sections, and assign
   * values to stack symbols later */
   .stack_dummy (COPY):
  {
    KEEP(*(.stack*))
  } > RAM

  /* Set stack top to end of RAM, and stack limit move down by
   * size of stack_dummy section */
  __StackTop = ORIGIN(RAM) + LENGTH(RAM);
  __StackLimit = __StackTop - SIZEOF(.stack_dummy);
  PROVIDE(__stack = __StackTop);

   /* This is where we handle flash storage blocks. We use dummy sections for finding the configured
   * block sizes and then "place" them at the end of flash when the size is known. */
  .internal_storage (DSECT) : {
    KEEP(*(.internal_storage*))
  } > FLASH

  .nvm (DSECT) : {
    KEEP(*(.simee*))
  } > FLASH

  /* Last page of flash is reserved for the manufacturing token space */
  linker_nvm_end = __main_flash_end__;
  linker_nvm_begin = linker_nvm_end - SIZEOF(.nvm);
  linker_nvm_size = SIZEOF(.nvm);
  linker_storage_end = linker_nvm_begin;
  __nvm3Base = linker_nvm_begin;
  linker_storage_begin = linker_storage_end - SIZEOF(.internal_storage);
  linker_storage_size = SIZEOF(.internal_storage);

  /* Check if data + heap + stack exceeds RAM limit */
  /*ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")*/
  ASSERT( (linker_nvm_begin + SIZEOF(.nvm)) <= __main_flash_end__, "NVM3 is excessing the flash size !")

  /* Check if FLASH usage exceeds FLASH size */
/*  ASSERT( LENGTH(FLASH) >= (__etext), "FLASH memory overflowed !") */ /* Build error need to be checked for CCP */
  ASSERT((linker_storage_begin >= (__etext + SIZEOF(.data))), "FLASH memory overflowed !")
}

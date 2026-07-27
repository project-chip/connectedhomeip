/***************************************************************************//**
 * GCC Linker script for Silicon Labs devices
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
   FLASH      (rx)  : ORIGIN = 0x8006000, LENGTH = 0x178000
   RAM        (rwx) : ORIGIN = 0x20000004, LENGTH = 0x3fffc
   BOOTLOADER_RESET_REGION (rwx) : ORIGIN = 0x20000000, LENGTH = 0x4
 }

ENTRY(Reset_Handler)

SECTIONS
{

  .vectors :
  {
    linker_vectors_begin = .;
    KEEP(*(.vectors))
    linker_vectors_end = .;

    __Vectors_End = .;
    __Vectors_Size = __Vectors_End - __Vectors;
    __lma_ramfuncs_start__ = .;
  } > FLASH
  .bootloader_reset_section (NOLOAD):
  {
    __ResetReasonStart__ = .;
    . = . + 4;
    . = ALIGN(4);
    __ResetReasonEnd__ = .;
  } > BOOTLOADER_RESET_REGION

  .stack (NOLOAD):
  {
    . = ALIGN(8);
    __StackLimit = .;
    KEEP(*(.stack*))
    . = ALIGN(4);
    __StackTop = .;
    PROVIDE(__stack = __StackTop);
  } > RAM

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(SORT_BY_ALIGNMENT(.bss*))
    *(COMMON)
    . = ALIGN(32);
    __bss_end__ = .;
  } > RAM


  .noinit (NOLOAD):
  {
    *(.noinit*);
    . = ALIGN(32);
  } > RAM


  text_application_ram :
  {
    . = ALIGN(32);
    __vma_ramfuncs_start__ = .;
    __text_application_ram_start__ = .;

    *(text_application_ram)

    . = ALIGN(32);
    __vma_ramfuncs_end__ = .;
    __text_application_ram_end__ = .;
  } > RAM AT > FLASH

  .rodata :
  {
    __lma_ramfuncs_end__ = .;
    __rodata_start__ = .;
    __rodata_end__ = .;
  } > FLASH

  .text :
  {
    linker_code_begin = .;
    *(SORT_BY_ALIGNMENT(.text*))
    *(SORT_BY_ALIGNMENT(text_*))
    . = ALIGN(32);
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
    __etext = .;
  } > FLASH

  .data :
  {
    . = ALIGN(4);
    __data_start__ = .;
    *(vtable)
    *(SORT_BY_ALIGNMENT(.data*))
    . = ALIGN(4);

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
    *(SEGGER_RTT)

    . = ALIGN(4);
    /* All data end */
    __data_end__ = .;

  } > RAM AT > FLASH


/* Calculate heap size based on RAM limits. */
heap_limit = ORIGIN(RAM) + LENGTH(RAM);  /* End of RAM */
heap_size = heap_limit - __HeapBase;
  .memory_manager_heap (NOLOAD):
  {
    . = ALIGN(8);
    __HeapBase = .;
    . += heap_size;
    __end__ = .;
    end = __end__;
    _end = __end__;
    KEEP(*(.memory_manager_heap*))
    __HeapLimit = ORIGIN(RAM) + LENGTH(RAM);
  } > RAM

  __heap_size = __HeapLimit - __HeapBase;
  __ram_end__ = 0x20000004 + 0x3fffc;
  __main_flash_end__ = 0x8006000 + 0x178000;

   /* This is where we handle flash storage blocks. We use dummy sections for finding the configured
   * block sizes and then "place" them at the end of flash when the size is known. */
  .internal_storage (DSECT) : {
    KEEP(*(.internal_storage*))
  } > FLASH


  .nvm (DSECT) : {
    KEEP(*(.simee*))
  } > FLASH

  __ramfuncs_start__ = __vma_ramfuncs_start__;
  __ramfuncs_end__ = __vma_ramfuncs_end__;

  linker_nvm_end = __main_flash_end__;
  linker_nvm_begin = linker_nvm_end - SIZEOF(.nvm);
  linker_storage_end = linker_nvm_begin;
  __nvm3Base = linker_nvm_begin;

  linker_storage_begin = linker_storage_end - SIZEOF(.internal_storage);
  ASSERT((linker_storage_begin >= (__etext + SIZEOF(.data))), "FLASH memory overflowed !")


  app_flash_end = 0x8006000 + 0x178000;
  ASSERT( (linker_nvm_begin + SIZEOF(.nvm)) <= app_flash_end, "NVM3 is excessing the flash size !")
}

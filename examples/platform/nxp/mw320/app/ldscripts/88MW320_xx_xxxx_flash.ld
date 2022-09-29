/*
** ###################################################################
**     Processor:           88MW320_A0_NAPC
**     Compiler:            GNU C Compiler
**     Reference manual:    88MW320-88MW322, Rev.3, July 2020
**     Version:             rev. 1.0, 2020-09-07
**     Build:               b210107
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2021 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/



/* Entry Point */
ENTRY(Reset_Handler)

HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 0x0400;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0400;
M_VECTOR_RAM_SIZE = DEFINED(__ram_vector_table__) ? 0x00000180 : 0;
_nvram_start = 0x480C0000;

/* The SRAM region [0x15EFC0-0x15EFFF] is reserved for entropy store. */
/* The SRAM region [0x15F000-0x15FFFF] is reserved for keystore. */

/* Specify the memory areas */
MEMORY
{
  m_interrupts          (RX)  : ORIGIN = 0x1F000100, LENGTH = 0x00000180
  m_text                (RX)  : ORIGIN = 0x1F000280, LENGTH = 0x000FFD80
  m_data0               (RW)  : ORIGIN = 0x00100000, LENGTH = 0x0005EFC0
  m_data1               (RW)  : ORIGIN = 0x20000000, LENGTH = 0x00020000
  m_nvram               (RW)  : ORIGIN = 0x480C0044, LENGTH = 0x00000FBC
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into internal flash */
  .interrupts :
  {
    . = ALIGN(4);
    __VECTOR_TABLE = .;
    __Vectors = .;
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > m_interrupts

  /* The program code and other data goes into internal flash */
  .text :
  {
    . = ALIGN(4);
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */*fsl_clock.c.*
        */*clock_config.c.*
        */*fsl_qspi.c.*
        */*fsl_flashc.c.*
        */*fsl_power.c.*
        */*mflash_drv.c.*
    ) .text)                 /* .text sections (code) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */*fsl_clock.c.*
        */*clock_config.c.*
        */*fsl_qspi.c.*
        */*fsl_flashc.c.*
        */*fsl_power.c.*
        */*mflash_drv.c.*
    ) .text*)                /* .text* sections (code) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */*fsl_clock.c.*
        */*clock_config.c.*
        */*fsl_qspi.c.*
        */*fsl_flashc.c.*
        */*fsl_power.c.*
        */*mflash_drv.c.*
    ) .rodata)               /* .rodata sections (constants, strings, etc.) */
    *(EXCLUDE_FILE(
        /* Exclude flash and frequently executed functions from XIP */
        */*fsl_clock.c.*
        */*clock_config.c.*
        */*fsl_qspi.c.*
        */*fsl_flashc.c.*
        */*fsl_power.c.*
        */*mflash_drv.c.*
    ) .rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
  } > m_text

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > m_text

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > m_text

 .ctors :
  {
    __CTOR_LIST__ = .;
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __CTOR_END__ = .;
  } > m_text

  .dtors :
  {
    __DTOR_LIST__ = .;
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_END__ = .;
  } > m_text

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } > m_text

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } > m_text

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } > m_text

  __etext = .;    /* define a global symbol at end of code */
  __DATA_ROM = .; /* Symbol is used by startup for data initialization */

  .interrupts_ram :
  {
    . = ALIGN(4);
    __VECTOR_RAM__ = .;
    __interrupts_ram_start__ = .; /* Create a global symbol at data start */
    *(.m_interrupts_ram)     /* This is a user defined section */
    . += M_VECTOR_RAM_SIZE;
    . = ALIGN(4);
    __interrupts_ram_end__ = .; /* Define a global symbol at data end */
  } > m_data0

  __VECTOR_RAM = DEFINED(__ram_vector_table__) ? __VECTOR_RAM__ : ORIGIN(m_interrupts);
  __RAM_VECTOR_TABLE_SIZE_BYTES = DEFINED(__ram_vector_table__) ? (__interrupts_ram_end__ - __interrupts_ram_start__) : 0x0;

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __DATA_RAM = .;
    __data_start__ = .;      /* create a global symbol at data start */
    *(CodeQuickAccess)       /* CodeQuickAccess sections */
    *(DataQuickAccess)       /* DataQuickAccess sections */
    /* Explicit placement of flash and frequently executed functions in RAM */
    */*fsl_clock.c.*(.text .text* .rodata .rodata*)
    */*clock_config.c.*(.text .text* .rodata .rodata*)
    */*fsl_qspi.c.*(.text .text* .rodata .rodata*)
    */*fsl_flashc.c.*(.text .text* .rodata .rodata*)
    */*fsl_power.c.*(.text .text* .rodata .rodata*)
    */*mflash_drv.c.*(.text .text* .rodata .rodata*)
    *(.data)                 /* .data sections */
    *(.data*)                /* .data* sections */
    KEEP(*(.jcr*))
    . = ALIGN(4);
    __data_end__ = .;        /* define a global symbol at data end */
  } > m_data0

  __DATA_END = __DATA_ROM + (__data_end__ - __data_start__);

  text_end = ORIGIN(m_text) + LENGTH(m_text);
  ASSERT(__DATA_END <= text_end, "region m_text overflowed with text and data")

  /* Uninitialized data section */
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    . = ALIGN(4);
    __START_BSS = .;
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
    __END_BSS = .;
  } > m_data0

  .heap :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    __HeapBase = .;
    . += HEAP_SIZE;
    __HeapLimit = .;
    __heap_limit = .; /* Add for _sbrk */
  } > m_data1

  .stack :
  {
    . = ALIGN(8);
    . += STACK_SIZE;
  } > m_data1

  .nvram :
  {
    . = ALIGN(4);
    __nvram_start__ = .;     /* create a global symbol at nvram start */
    *(.nvram*)               /* .nvram* sections */
    *(.nvram_uninit*)        /* .nvram_uninit* sections */
    . = ALIGN(4);
    __nvram_end__ = .;       /* define a global symbol at nvram end */
  } > m_nvram

  /* Initializes stack on the end of block */
  __StackTop   = ORIGIN(m_data1) + LENGTH(m_data1);
  __StackLimit = __StackTop - STACK_SIZE;
  PROVIDE(__stack = __StackTop);

  .ARM.attributes 0 : { *(.ARM.attributes) }

  ASSERT(__StackLimit >= __HeapLimit, "region m_data1 overflowed with stack and heap")
}


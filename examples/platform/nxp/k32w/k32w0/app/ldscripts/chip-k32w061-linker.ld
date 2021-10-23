/*
 *  Copyright (c) 2019, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   GCC linker script for K32W061.
 */

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/*
 * stack size for the boot rom during warm boot and application
 * 256 is sufficient (pwrm_test) but keep it large to 1024
 */
BOOT_RESUME_STACK_SIZE = 1024;

STACK_SIZE             = (4096);

MEM_RAM0_BASE          = 0x4000400;
MEM_RAM0_SIZE          = 0x0015c00;

MEM_RAM1_BASE          = 0x4020000;
MEM_RAM1_SIZE          = 0x10000;

MEMORY
{
  /* Define each memory region. RAM0 definition leaves the first 1kB for the
     boot code */
  Flash640 (rx) : ORIGIN = 0, LENGTH = 0x00a0000            /* 640K bytes (alias Flash) */
  RAM0 (rwx)    : ORIGIN = 0x4000400,  LENGTH = 0x0015c00   /* 87K bytes (alias RAM) */
  RAM1 (rwx)    : ORIGIN = 0x4020000,  LENGTH = 0x10000     /* 64K bytes (alias RAM2) */
}

/* Define a symbol for the top of each memory region */
__top_RAM1      = MEM_RAM1_BASE + MEM_RAM1_SIZE; /* 64K bytes */

/* To be improved. At this moment the second RAM bank is dedicated entirely to heap + stack. */
HEAP_SIZE = DEFINED(HEAP_SIZE) ? HEAP_SIZE : 0xF000;

/* set external flash properties - external flash is present on the DK6 board */
m_ext_flash_size          = 0x00100000;
m_ext_flash_base          = 0x00000000;
m_ext_flash_sector_size   = 4096;

NVMSectorCountLink        = 63;

NV_STORAGE_SIZE           = NVMSectorCountLink * m_ext_flash_sector_size;
NV_STORAGE_MAX_SECTORS    = NVMSectorCountLink;
NV_STORAGE_SECTOR_SIZE    = m_ext_flash_sector_size;
NV_STORAGE_START_ADDRESS  = m_ext_flash_size - 1;
NV_STORAGE_END_ADDRESS    = NV_STORAGE_START_ADDRESS - NV_STORAGE_SIZE + 1;

m_app_size                = 0x00a0000; /* 640K */

__ram_vector_table__      = 1;
vector_table_size         = 0x120;
M_VECTOR_RAM_SIZE         = DEFINED(__ram_vector_table__) ? vector_table_size : 0x0;

__base_RAM0               = 0x4000400;

ENTRY(ResetISR)

SECTIONS
{
    /* MAIN TEXT SECTION */
    .header : ALIGN(4)
    {
        _flash_start = ABSOLUTE(.);
        _flash_beg = ABSOLUTE(.);

        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        __VECTOR_TABLE = .;
        __Vectors = .;
        KEEP(*(.isr_vector))
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */

        FILL(0xff)
        . = ALIGN (0x10);
    } >Flash640

    .ro_nonce : ALIGN(0x10)
    {
        _FlsNonceStart = ABSOLUTE(.);
        *(.ro_nonce) /* nonce value is 16 bytes.*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    .ro_ota_header : ALIGN(0x10)
    {
        _enc_start = ABSOLUTE(.);
        _enc_offset = (_enc_start & 0x0000000F);
        _FlsOtaHeader = ABSOLUTE(.);
        *(.ro_ota_header) /* Ota Header 69 bytes*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    .ro_se_lnkKey (ALIGN((. - _enc_offset), 16) + _enc_offset):
    {
        _FlsLinkKey = ABSOLUTE(.);
        *(.ro_se_lnkKey)  /* Link Key 16 bytes*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > Flash640

    .filler :
    {
        BYTE(0xff);
        FILL(0xff);
        . = ALIGN(0x40);
    } > Flash640

    .text : ALIGN(0x40)
    {
        FILL(0xff)

       *(.after_vectors*)
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

        *(.rodata .rodata.* .constdata .constdata.*)

        . = ALIGN(4);
    } > Flash640
    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
       FILL(0xff)
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > Flash640
    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
       FILL(0xff)
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > Flash640
    __exidx_end = .;

    _etext = .;

    .heap (COPY):
    {
        __HeapBase = .;
         _heap = .;
        KEEP(*(.heap*))
        PROVIDE(end = .);
         . = ALIGN(4);
        __end__ = .;
         _end_heap = .;
        __HeapLimit = .;
    } > RAM1

    .interrupts_ram : ALIGN(0x200)
    {
        . = ALIGN(4);
        __VECTOR_RAM__ = .;
        __interrupts_ram_start__ = .;   /* Create a global symbol at data start */
        *(.m_interrupts_ram)            /* This is a user defined section */
        . += M_VECTOR_RAM_SIZE;
        . = ALIGN(4);
        __interrupts_ram_end__ = .;     /* Define a global symbol at data end */
    } > RAM0

    /* MAIN DATA SECTION */
    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RAM0

    /* Main DATA section (RAM0) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       *(vtable)
       *(.ramfunc*)
       *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        __init_array_start = . ;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = . ;

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))

       . = ALIGN(4) ;
       _edata = . ;
    } > RAM0 AT>Flash640

    __VECTOR_RAM = __VECTOR_RAM__;
    __RAM_VECTOR_TABLE_SIZE_BYTES = DEFINED(__ram_vector_table__) ? (__interrupts_ram_end__ - __interrupts_ram_start__) : 0x0;

    /* MAIN BSS SECTION */
    .bss (NOLOAD) : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        *(g_u32NwkFrameCounter)
        . = ALIGN(4) ;
        _ebss = .;

        PROVIDE(end = .);
    } > RAM0

    /* BSS section for MAC buffers */
    .bss_MAC (NOLOAD) : ALIGN(4)
    {
       /* MAC buffer section: must be within 128kB block. __mac_buffer_base is
          defined further down to be on 128kB alignment */
        __mac_buffer_start = .;
       *(.mac_buffer)

        . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
    } > RAM0

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*)
        . = ALIGN(4) ;
        _end_noinit = .;
    } > RAM0

    /* end of firmware RAM to be retained in power down mode */
    _end_fw_retention = .;

    /* non retained RAM section */
    /* stack for rom boot during warm resume */
    .boot_resume_stack (NOLOAD): ALIGN(4)
    {
        _boot_resume_stack = .;
        *(.boot_resume_stack*)
        . += BOOT_RESUME_STACK_SIZE;
        . = ALIGN(4) ;
        _end_boot_resume_stack = .;
    } > RAM0

    __nv_storage_end_address = NV_STORAGE_END_ADDRESS;
    __nv_storage_start_address = NV_STORAGE_START_ADDRESS;

    PROVIDE(_vStackTop = __top_RAM1);
    PROVIDE(__mac_buffer_base = (__mac_buffer_start & 0xfffe0000));
    PROVIDE(BOOT_GetStartPowerMode = 0x03000e9d);
    PROVIDE(ROM_GetFlash = 0x03000e0d);
    PROVIDE(pmc_reset_get_cause = 0x030046e9);
    PROVIDE(psector_ReadIeee802_15_4_MacId1 = 0x030053b1);
    PROVIDE(Chip_LOWPOWER_ChipSoftwareReset = 0x03003fa1);
    PROVIDE(_pvHeapStart = _heap);
    PROVIDE(_pvHeapLimit = _pvHeapStart + (HEAP_SIZE));

    __StackLimit = _vStackTop - STACK_SIZE;
   ASSERT (LENGTH(Flash640) >= (_etext + SIZEOF(.data)), "Application does not fit into flash!")
}

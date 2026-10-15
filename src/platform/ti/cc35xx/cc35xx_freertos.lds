/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

INCLUDE "ti_build_linker.lds.toolbox"
STACKSIZE = 0x2FF0;
FLASH_INT_VEC_SIZE = 0x2400; /* Including padding */


MEMORY
{   
    /* Flash regions */
    FLASH_INT_VEC         (RX) : ORIGIN = 0x14000000, LENGTH = FLASH_INT_VEC_SIZE
    FLASH_NON_SECURE      (RX) : ORIGIN = 0x14000000 + FLASH_INT_VEC_SIZE, LENGTH = build_linker_toolbox_FLASH_SIZE - FLASH_INT_VEC_SIZE
    
    /* RAM regions */
    TCM_CRAM_NON_SECURE   (RWX) : ORIGIN = 0x00000000, LENGTH = 0x00007FFF  /* 32KB */
    CRAM_NON_SECURE       (RWX) : ORIGIN = 0x08000000, LENGTH = 0x0000FFFF  /* 64KB */
    TCM_DRAM_NON_SECURE   (RWX) : ORIGIN = 0x20000000, LENGTH = (build_linker_toolbox_PSRAM_SIZE == 0 ? 0x1FFFF : 0x0FFFF)  /* 64KB for PSRAM */  /* 128KB for NO-PSRAM */
    CONNECTIVITY_SHARED_MEM (RWX) : ORIGIN = 0x28000000, LENGTH = 0x000000FF
    BOOT_REPORT_SHARED_MEM  (RWX) : ORIGIN = 0x28000100, LENGTH = 0x00000CAF
    DRAM_NON_SECURE       (RWX) : ORIGIN = 0x28000DB0, LENGTH = 0x0007F24F  /* ~509KB */
    PS_RAM                (RW)  : ORIGIN = 0x60000000, LENGTH = (build_linker_toolbox_PSRAM_SIZE > 0 ? build_linker_toolbox_PSRAM_SIZE : 0x1)    /* 8MB or 2MB or 0 MB Configure by sysconfig */ 

    /* Logging data regions */
    LOG_DATA (R) : ORIGIN = 0x90000000, LENGTH = 0x40000        /* 256 KB */
    LOG_PTR  (R) : ORIGIN = 0x94000008, LENGTH = 0x40000        /* 256 KB */
    
}

SECTIONS
{
    /* Reserved for bootloader metadata - application starts at 0x2000 */
    .reserved (0x14000000) (NOLOAD) : {        
        . += 0x2000;
    } > FLASH_INT_VEC
    
    /* Vector table in flash */
    .resetVecs : {
        KEEP (*(.resetVecs))
    } > FLASH_INT_VEC

    /* Code sections */
    .text : {
        CREATE_OBJECT_SYMBOLS
        *(.text)
        *(.text.*)
        . = ALIGN(0x4);
        KEEP (*(.ctors))
        . = ALIGN(0x4);
        KEEP (*(.dtors))
        . = ALIGN(0x4);
        __init_array_start = .;
        KEEP (*(.init_array*))
        __init_array_end = .;
        *(.init)
        *(.fini*)
    } > FLASH_NON_SECURE
    
    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);

    /* Data section */
    .data : ALIGN(4) {
        __data_load__ = LOADADDR(.data);
        __data_start__ = .;
        *(.data)
        *(.data.*)
        /* SPAKE2p constants from CHIPCryptoPAL.h are placed in RAM (DMA limitation) */
        *(.rodata._ZN4chip6CryptoL14spake2p_M_p256E)
        *(.rodata._ZN4chip6CryptoL14spake2p_N_p256E)
        . = ALIGN(4);
        __data_end__ = .;
    } > TCM_DRAM_NON_SECURE AT> FLASH_NON_SECURE    
    
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.rodata_*)
        KEEP (*(.timestampPFormat))
        . = ALIGN(4);
    } > FLASH_NON_SECURE
    
    .cinit : {
        *(.cinit)
    } > FLASH_NON_SECURE
    
    .binit : {
        *(.binit)
    } > FLASH_NON_SECURE
    
    /* TI RAM functions - code that runs from RAM */
    .TI.ramfunc : ALIGN(4) {
        __ramfunc_load__ = LOADADDR(.TI.ramfunc);
        __ramfunc_start__ = .;
        *(.TI.ramfunc)
        __ramfunc_end__ = .;
    } > TCM_CRAM_NON_SECURE AT> FLASH_NON_SECURE 

    /* Exception handling sections */
    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH_NON_SECURE
    
    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH_NON_SECURE

    
    /* System memory section */
    .sysmem : {
        __SYSMEM_START__ = .;
        *(.sysmem)
        . = ALIGN(4);
        __SYSMEM_END__ = .;
    } > DRAM_NON_SECURE
    
    /* BSS sections */
    .bss : {
        __bss_start__ = .;
        *(.shbss)
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > DRAM_NON_SECURE  
    
    
    /* Stack section */
    .stack (NOLOAD) : ALIGN(8) {
        _stack = .;
        __stack = .;
        KEEP(*(.stack))
        . += STACKSIZE;
        _stack_end = .;
        __stack_end = .;
    } > TCM_DRAM_NON_SECURE
    
    /* Connectivity shared status section */
    .connectivity_shared_status_section (NOLOAD) : ALIGN(4) {
        KEEP (*(.connectivity_shared_status_section))
    } > CONNECTIVITY_SHARED_MEM
    
    /* boot report shared section */
    .boot_report_shared_section (NOLOAD) : ALIGN(4) {
        _Boot_report_address = .;
    } > BOOT_REPORT_SHARED_MEM
    
    
    /* Logging sections */
    .log_data (COPY) : ALIGN(4) {
        KEEP (*(.log_data))
    } > LOG_DATA
    
    .log_ptr (COPY) : ALIGN(4) {
        KEEP (*(.log_ptr*))
    } > LOG_PTR
}

ENTRY(resetISR)

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
   FLASH      (rx)  : ORIGIN = 0x1008000, LENGTH = 0x389000
   RAM        (rwx) : ORIGIN = 0x20000100, LENGTH = 0x7ff00
   RAM_ALIAS  (x)   : ORIGIN = 0x800100, LENGTH = 0x7ff00
   BOOTLOADER_RESET_REGION (rwx) : ORIGIN = 0x20000000, LENGTH = 0x100
   /* Dummy region is used for the dummy segment, which in turn never gets data
    * loaded, but is only used to calculate the size of objects placed in it. */
   DUMMY      (!ax) : ORIGIN = 0xffc77000, LENGTH = 0x389000
 }
__flash_size__ = 0x391000;
__flash_page_size__ = 0x1000;
__mspu_region_size__ = 0x8000;

/*******************************************************************************
 * Due to an issue in GNU LD, which results in wrongly applying the writeable
 * flag to a text section (thereby triggering an error since the memory block is
 * declared rx), we need to define our segments manually.
 *
 * Typical Silicon Labs program layout:
 *
 * ----- START OF APPLICATION FLASH -----
 * R X - TrustZone Secure-to-Nonsecure trampoline app.
 * R   - Vector table
 * R X - RAM functions (copied to RAM at startup)
 * R   - Read-only data
 * R X - Code
 * R X - Secure Gateways (Trustzone S only)
 * R X - extab
 * R X - exidx
 * R   - startup copy table
 * R   - startup zeroization table
 * RW  - Initialised RAM data (copied to RAM at startup)
 *     - [unallocated space]
 *     - NVM (defined using linker symbols only, not an ELF section)
 * ----- END OF APPLICATION FLASH -----
 *
 * ----- START OF APPLICATION RAM -----
 * RW  - Bootloader reset reason (if present)
 * RW  - stack
 * RW  - stack sealing
 * RW  - BSS (zero-init RAM)
 * RW  - uninitialised RAM
 * R X - RAM functions
 * RW  - Initialised RAM
 * RW  - Heap (automatically expanded to end of RAM)
 * ----- END OF APPLICATION RAM -----
 *
 * ----- START OF DUMMY REGION -----
 * RW  - NVM sections (not loadable, only collected such that the linker can
 *         calculate the size of the symbols placed inside it, to subtract that
 *         size from the end of the available flash space)
 * ----- END OF DUMMY REGION -----
 *
 * For the purpose of defining segments, we need a new segment whenever the
 * data loaded changes type. That means e.g. we need three segments for all RAM
 * content: the initial RW segment, the RX segment, and the final RW segment.
 ******************************************************************************/

PHDRS
{
  text_s PT_LOAD FLAGS(5);        /* Executable S code (if present) */
  vectors PT_LOAD FLAGS(4);       /* Flash content before RAM functions is R */
  readonly PT_LOAD FLAGS(4);      /* Readonly content after RAM functions */
  text PT_LOAD FLAGS(5);          /* Executable flash content after readonly */
  tables PT_LOAD FLAGS(4);        /* Tables supporting startup copying are R */
  empty PT_LOAD FLAGS(4);         /* Empty area is R */

  ram_pre_code PT_LOAD FLAGS(6);  /* All RAM content before RAMfuncs is RW */
  ram_code PT_LOAD FLAGS(5);      /* RAMfuncs need to be RX */
  ram_data PT_LOAD FLAGS(6);      /* Relocatable RAM data needs own segment */
  ram_post_data PT_LOAD FLAGS(6); /* All RAM content after data is RW */
  btl_data PT_LOAD FLAGS(6);      /* Bootloader data is in a separate region */

  dummy_seg PT_NULL FLAGS(6);     /* Dummy segment for dummy sections */
}

ENTRY(Reset_Handler)

SECTIONS
{
  .secure_app :
  {
    __Secure_Vectors = .;
    KEEP(*(.secure_vectors))
    KEEP(*(.secure_config_data))
    __Secure_Reset_Handler = .;
    KEEP(*(.secure_reset_handler))
    KEEP(*(.secure_fault_handler))
    . = ALIGN(__mspu_region_size__);
  } > FLASH :text_s

  .vectors :
  {
    linker_vectors_begin = .;
    KEEP(*(.vectors))
    linker_vectors_end = .;

    __Vectors_End = .;
    __Vectors_Size = __Vectors_End - __Vectors;
  } > FLASH :vectors
  text_ram :
  {
    . = ALIGN(32);
    __vma_ramfuncs_start__ = .;

    /* text_application_ram */
    *(text_application_ram)

    . = ALIGN(4);

    /* text_<component>_ram */
    __text_system_ram_start__ = .;
    *(text_system_*timecritical*)

    . = ALIGN(4);
    __text_system_ram_end__ = .;
    __text_aes_ccm_ram_start__ = .;
    *(text_aes_ccm_*security*)
    *(text_aes_ccm_*timecritical*)

    . = ALIGN(4);
    __text_aes_ccm_ram_end__ = .;
    __text_aes_common_ram_start__ = .;
    *(text_aes_common_*security*)
    *(text_aes_common_*timecritical*)

    . = ALIGN(4);
    __text_aes_common_ram_end__ = .;
    __text_clock_manager_ram_start__ = .;
    *(text_clock_manager_*timecritical*)

    . = ALIGN(4);
    __text_clock_manager_ram_end__ = .;
    __text_device_peripheral_ram_start__ = .;
    *(text_device_peripheral_*timecritical*)

    . = ALIGN(4);
    __text_device_peripheral_ram_end__ = .;
    __text_dma_channel_ram_start__ = .;
    *(text_dma_channel_*timecritical*)

    . = ALIGN(4);
    __text_dma_channel_ram_end__ = .;
    __text_dma_manager_ram_start__ = .;
    *(text_dma_manager_*timecritical*)

    . = ALIGN(4);
    __text_dma_manager_ram_end__ = .;
    __text_dmadrv_ram_start__ = .;
    *(text_dmadrv_*timecritical*)

    . = ALIGN(4);
    __text_dmadrv_ram_end__ = .;
    __text_gpio_ram_start__ = .;
    *(text_gpio_*timecritical*)

    . = ALIGN(4);
    __text_gpio_ram_end__ = .;
    __text_hal_common_ram_start__ = .;
    *(text_hal_common_*timecritical*)

    . = ALIGN(4);
    __text_hal_common_ram_end__ = .;
    __text_hal_gpio_ram_start__ = .;
    *(text_hal_gpio_*timecritical*)

    . = ALIGN(4);
    __text_hal_gpio_ram_end__ = .;
    __text_hal_ldma_ram_start__ = .;
    *(text_hal_ldma_*timecritical*)

    . = ALIGN(4);
    __text_hal_ldma_ram_end__ = .;
    __text_hal_sysrtc_ram_start__ = .;
    *(text_hal_sysrtc_*timecritical*)

    . = ALIGN(4);
    __text_hal_sysrtc_ram_end__ = .;
    __text_interrupt_manager_ram_start__ = .;
    *(text_interrupt_manager_*timecritical*)

    . = ALIGN(4);
    __text_interrupt_manager_ram_end__ = .;
    __text_memlcd_ram_start__ = .;
    *(text_memlcd_*timecritical*)

    . = ALIGN(4);
    __text_memlcd_ram_end__ = .;
    __text_memory_manager_ram_start__ = .;
    *(text_memory_manager_*timecritical*)

    . = ALIGN(4);
    __text_memory_manager_ram_end__ = .;
    __text_ot_platform_abstraction_ram_start__ = .;
    *(text_ot_platform_abstraction_*timecritical*)

    . = ALIGN(4);
    __text_ot_platform_abstraction_ram_end__ = .;
    __text_power_manager_ram_start__ = .;
    *(text_power_manager_*timecritical*)

    . = ALIGN(4);
    __text_power_manager_ram_end__ = .;
    __text_rail_lib_ram_start__ = .;
    *(text_rail_lib_*timecritical*)

    . = ALIGN(4);
    __text_rail_lib_ram_end__ = .;
    __text_se_manager_ram_start__ = .;
    *(text_se_manager_*timecritical*)

    . = ALIGN(4);
    __text_se_manager_ram_end__ = .;
    __text_core_ram_start__ = .;
    *(text_core_*timecritical*)

    . = ALIGN(4);
    __text_core_ram_end__ = .;
    __text_sleeptimer_ram_start__ = .;
    *(text_sleeptimer_*timecritical*)

    . = ALIGN(4);
    __text_sleeptimer_ram_end__ = .;
    __text_sli_crypto_ram_start__ = .;
    *(text_sli_crypto_*timecritical*)

    . = ALIGN(4);
    __text_sli_crypto_ram_end__ = .;
    __text_psec_osal_ram_start__ = .;
    *(text_psec_osal_*timecritical*)

    . = ALIGN(4);
    __text_psec_osal_ram_end__ = .;
    __text_sxsymcrypt_ram_start__ = .;
    *(text_sxsymcrypt_*timecritical*)

    . = ALIGN(4);
    __text_sxsymcrypt_ram_end__ = .;
    __text_uartdrv_ram_start__ = .;
    *(text_uartdrv_*timecritical*)

    . = ALIGN(4);
    __text_uartdrv_ram_end__ = .;
    *libgcc.a:_*.*(.text*)
    *libgcc.a:_*.*(.rodata*)
    *memset.*(.text*)
    *memset.*(.rodata*)
    */link_raw_api.*(.text*)
    */link_raw_api.*(.rodata*)
    */sub_mac.*(.text*)
    */sub_mac.*(.rodata*)
    */factory_diags.*(.text*)
    */factory_diags.*(.rodata*)
    */link_metrics.*(.text*)
    */link_metrics.*(.rodata*)
    */mac.*(.text*)
    */mac.*(.rodata*)
    *(text_freertos_kernel_*timecritical*)

    . = ALIGN(32);
    __vma_ramfuncs_end__ = .;
  } > RAM_ALIAS AT > FLASH :ram_code

  __lma_ramfuncs_start__ = LOADADDR(text_ram);
  __lma_ramfuncs_end__ = LOADADDR(text_ram) + SIZEOF(text_ram);

  .rodata :
  {
  } > FLASH :readonly

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
  } > FLASH :text

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH :text

  __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > FLASH :text
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
  } > FLASH :tables

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
  } > FLASH :tables
  .bootloader_reset_section (NOLOAD):
  {
    __ResetReasonStart__ = .;
    KEEP(*(.bootloader_reset_section))
  } > BOOTLOADER_RESET_REGION :btl_data
  text_ram_vma (NOLOAD):
  {
    /* Offset the location counter to account for RAM code that was placed in
       the aliased RAM region.
     */
    . += (__vma_ramfuncs_end__ - __vma_ramfuncs_start__);
  } > RAM :ram_pre_code

  .stack (NOLOAD):
  {
    . = ALIGN(8);
    __stack_limit = .;
    __StackLimit = .;
    KEEP(*(.stack*))
    . = ALIGN(4);
    __stack = .;
    __StackTop = .;
  } > RAM :ram_pre_code

  .noinit (NOLOAD):
  {
    __noinit_start__ = .;
    *(.noinit*);
    __noinit_end__ = .;
  } > RAM :ram_pre_code
  vector_table_ram (NOLOAD):
  {
    linker_vectors_ram_begin = .;
    KEEP(*(.vector_table_ram))
    linker_vectors_ram_end = .;
  } > RAM :ram_pre_code

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(SORT_BY_ALIGNMENT(.bss*))
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM :ram_pre_code

  .data :
  {
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
    /* Shareable data sections here */
    __start_shareable = .;
    *(.rail_shareable)
    /* Add each additional shareable sections here */
    __stop_shareable = .;
    . = ALIGN(4);
    *(SEGGER_RTT)

    . = ALIGN(4);
    /* All data end */

  } > RAM AT > FLASH :ram_data

  /* Values to support copying initialised values at startup */
  __etext = LOADADDR(.data);
  __data_start__ = ADDR(.data);
  __data_end__ = ADDR(.data) + SIZEOF(.data);

  ASSERT((__etext % 4) == 0, "Data section needs to be word-aligned in LMA")
  ASSERT((__data_start__ % 4) == 0, "Data section needs to be word-aligned in VMA")
  ASSERT((__data_end__ % 4) == 0, "Data section needs to be word-multiple sized")
.code_flash_end : {
  code_flash_end = .;
} > FLASH :empty

/* Calculate heap size based on RAM limits. */
heap_limit = ORIGIN(RAM) + LENGTH(RAM);  /* End of RAM */
heap_size = heap_limit - __HeapBase;
  .memory_manager_heap (NOLOAD):
  {
    . = ALIGN(8);
    __HeapBase = .;
    . += heap_size;
    __end__ = .;
    _end = __end__;
    KEEP(*(.memory_manager_heap*))
    __HeapLimit = ORIGIN(RAM) + LENGTH(RAM);
  } > RAM :ram_post_data

  __ram_end__ = 0x20000100 + 0x7ff00;
  __main_flash_end__ = 0x1008000 + 0x389000;

   /* This is where we handle flash storage blocks. We use dummy sections for finding the configured
   * block sizes and then "place" them at the end of flash when the size is known. */
  .internal_storage (NOLOAD) : {
    KEEP(*(.internal_storage*))
  } > DUMMY :dummy_seg


  .nvm (NOLOAD) : {
    KEEP(*(.simee*))
  } > DUMMY :dummy_seg
  __ramfuncs_start__ = ORIGIN(RAM) + (__vma_ramfuncs_start__ - ORIGIN(RAM_ALIAS));
  __ramfuncs_end__ = ORIGIN(RAM) + (__vma_ramfuncs_end__ - ORIGIN(RAM_ALIAS));

  /* Static secure tokens reserve flash memory at the end of the flash */
  linker_static_secure_tokens_end = __main_flash_end__;
  linker_static_secure_tokens_begin = linker_static_secure_tokens_end - 0x2000;

  linker_nvm_end = linker_static_secure_tokens_begin;
  linker_nvm_begin = linker_nvm_end - SIZEOF(.nvm);
  linker_storage_end = linker_nvm_begin;
  __nvm3Base = linker_nvm_begin;

  linker_storage_begin = linker_storage_end - SIZEOF(.internal_storage);
  ASSERT((linker_storage_begin >= (__etext + SIZEOF(.data))), "FLASH memory overflowed !")

  HIDDEN(app_flash_end = 0x1008000 + 0x389000);
  ASSERT( (linker_nvm_begin + SIZEOF(.nvm)) <= linker_static_secure_tokens_begin, "NVM3 is excessing the flash size !")
  ASSERT( (linker_static_secure_tokens_begin + 0x2000) <= app_flash_end, "Static secure storage is exceeding the flash size !")
  ASSERT( code_flash_end - 0x1008000 <= 0x1f0000, "Code size exceeds code region 1 capacity. Flash partitions can be reconfigured: https://docs.silabs.com/gecko-platform/latest/flash-partitions-series3/")

}


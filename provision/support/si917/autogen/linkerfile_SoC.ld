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
   rom   (rx)  : ORIGIN = 0x8202000, LENGTH = 0x1fe000
   ram   (rwx) : ORIGIN = 0xc, LENGTH = 0x2fc00
 }
 MEMORY
 {
   udma0   (rwx)  : ORIGIN = 0x2fc00, LENGTH = 0x400
   udma1   (rwx)  : ORIGIN = 0x24061c00, LENGTH = 0x400
 }

ENTRY(Reset_Handler)
 
SECTIONS
{

	.text :
	{
		KEEP(*(.isr_vector))
     	  KEEP(*(.reset_handler))
			*(EXCLUDE_FILE(*sl_si91x_bus.o *sl_si91x_driver.o *sli_si91x_multithreaded.o *rsi_deepsleep_soc.o *rsi_hal_mcu_m4_ram.o *croutine.o *event_groups.o *list.o *queue.o *stream_buffer.o *tasks.o *timers.o *cmsis_os2.o *freertos_umm_malloc_host.o *malloc_buffers.o *sl_rsi_utility.o *port.o *heap_*.o) .text*)
		
    

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
		
		KEEP(*(.eh_fram e*))			
	} > rom 
		
	
	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)			
	} > rom 
	
	
	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)			
	} > rom 
	 
	__exidx_end = .;
	__etext = .;
	
	


	
	/* _sidata is used in code startup code */
	_sidata = __etext;
	
	.data :
	
	{
		__data_start__ = .;
		
		/* _sdata is used in startup code */
		_sdata = __data_start__;
		KEEP(*(.ramVector))	
		KEEP(*(.init))
		KEEP(*(.fini))
		*(.rodata*)
		*(.data*)
		
		*rsi_hal_mcu_m4_ram.o(.text*)
		*sl_si91x_driver.o(.text*)
		*sl_si91x_bus.o(.text*)
		*sli_si91x_multithreaded.o(.text*)
		*rsi_deepsleep_soc.o(.text*)
		*croutine.o(.text*)
		*event_groups.o(.text*)
		*list.o(.text*)
		*queue.o(.text*)
		*cmsis_os2.o(.text*)
		*stream_buffer.o(.text*)
		*tasks.o(.text*)
		*timers.o(.text*)
		*freertos_umm_malloc_host.o(.text*)
		*malloc_buffers.o(.text*)
		*sl_rsi_utility.o(.text*)
		*port.o(.text*)
		*heap_*.o(.text*)
		

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
		
		/* _edata is used in startup code */
		_edata = __data_end__; 
	} > ram AT> rom
		
	
	.bss (NOLOAD) :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .; 
	} > ram 
	

	/* Modify the below value to change the stack size */
	StackSize = 0x3000;
	.stack (NOLOAD):
	{
		__StackLimit = .;
		KEEP(*(.stack*))
		. = ALIGN(4);
		. += StackSize;
		__StackTop = .;
		PROVIDE(__stack = __StackTop);			
	} > ram
			
  	.heap (COPY):
  	{
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		_end = __end__;
		KEEP(*(.heap*))	
		. = ORIGIN(ram) + LENGTH(ram);
  		
		__HeapLimit = .;			
  	} > ram
  
	__heap_size = __HeapLimit - __HeapBase;
	.udma_addr0 :
	{
		*(.udma_addr0*)
	} > udma0 AT> rom
	

	.udma_addr1 :
	{
		*(.udma_addr1*)		
	} > udma1 AT> rom
  __ram_end__ = 0xc + 0x2fc00;
  __main_flash_end__ = 0x8202000 + 0x1fe000;
  
	   /* This is where we handle flash storage blocks. We use dummy sections for finding the configured
   * block sizes and then "place" them at the end of flash when the size is known. */
  .internal_storage (DSECT) : {
    KEEP(*(.internal_storage*))
  } > rom
  
  .nvm (DSECT) : {
    KEEP(*(.simee*))
  } > rom

  linker_nvm_end = __main_flash_end__;
  linker_nvm_begin = linker_nvm_end - SIZEOF(.nvm);
  linker_nvm_size = SIZEOF(.nvm);
  linker_storage_end = linker_nvm_begin;
  __nvm3Base = linker_nvm_begin;	
	
  linker_storage_begin = linker_storage_end - SIZEOF(.internal_storage);
  linker_storage_size = SIZEOF(.internal_storage);
  ASSERT((linker_storage_begin >= (__etext + SIZEOF(.data))), "FLASH memory overflowed !")   
}
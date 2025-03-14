OUTPUT_ARCH( "riscv" )

ENTRY( bl702_start )

__EM_SIZE = DEFINED(btble_controller_init) ? 8K : 0K;
__CACHE_SIZE = 16K;
__stack_size = DEFINED(__stack_size) ? __stack_size : 2K;

MEMORY
{
  flash  (rxai!w) : ORIGIN = 0x23000000, LENGTH = (2M)
  tcm_ocram (wxa) : ORIGIN = 0x4201C000 + __CACHE_SIZE, LENGTH = (16K - __CACHE_SIZE + 80K - __EM_SIZE - __stack_size)
  sys_stack (wxa) : ORIGIN = 0x42034000 - __EM_SIZE - __stack_size, LENGTH = __stack_size
  hbnram    (wxa) : ORIGIN = 0x40010000, LENGTH = (4K)
  psram     (wxa) : ORIGIN = 0x26000000, LENGTH = (2M)
}

SECTIONS
{
  .init :
  {
    KEEP (*(SORT_NONE(.init)))
  } >flash

  .ver :
  {
    . = ORIGIN(flash) + 0xC00;
    LONG(0x42464c42); /* BLFB */
    LONG(0x46524556); /* VERF */
    KEEP(*(.blverinf))
  } >flash

  .text :
  {
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
    *(.gnu.linkonce.t.*)
  } >flash

  .rodata :
  {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.sdata2.*)
    *(.gcc_except_table.*)

    /* static cli cmds */
    . = ALIGN(4);
    _bl_static_cli_cmds_start = .;
    KEEP(*(.static_cli_cmds))
    *(.static_cli_cmds)
    _bl_static_cli_cmds_end = .;

    /* static blog code1 */
    . = ALIGN(4);
    _bl_static_blogcomponent_code_start = .;
    KEEP(SORT(*)(.static_blogcomponent_code*))
    *(.static_blogcomponent_code*)
    _bl_static_blogcomponent_code_end = .;

    /* static blog code2 */
    . = ALIGN(4);
    _bl_static_blogfile_code_start = .;
    KEEP(SORT(*)(.static_blogfile_code*))
    *(.static_blogfile_code*)
    _bl_static_blogfile_code_end = .;

    /* static blog code3 */
    . = ALIGN(4);
    _bl_static_blogpri_code_start = .;
    KEEP(SORT(*)(.static_blogpri_code*))
    *(.static_blogpri_code*)
    _bl_static_blogpri_code_end = .;

    /* section information for components's version */
    . = ALIGN(4);
    _version_info_section_start = .;
    KEEP(*(.version.*))
    _version_info_section_end = .;

    *(.gnu.linkonce.r.*)
  } >flash

  .init_array :
  {
    __init_array_start = .;
    KEEP(*(.init_array))
    __init_array_end = .;
  } >flash

  .bleromro :
  {
    KEEP(*(.bleromro))
  } >flash

  .bleromrw (NOLOAD) :
  {
    KEEP(*(.bleromrw))
  } >tcm_ocram

  .hbn : ALIGN(4)
  {
    PROVIDE( _hbn_load = LOADADDR(.hbn) );
    PROVIDE( _hbn_run = ADDR(.hbn) );
    PROVIDE( _hbn_run_end = ADDR(.hbn) + SIZEOF(.hbn) );

    *(.hbn_code.*)
    *(.hbn_data)
    *(.retention)
  } >hbnram AT >flash

  .hbn_noinit (NOLOAD) :
  {
    *(.hbn_noinit)
    *(.retention_noinit)
  } >hbnram

  .tcmdata (NOLOAD) :
  {
    *(.tcm_data)
  } >tcm_ocram

  .tcmcode : ALIGN(4)
  {
    PROVIDE( _tcm_load = LOADADDR(.tcmcode) );
    PROVIDE( _tcm_run = ADDR(.tcmcode) );
    PROVIDE( _tcm_run_end = ADDR(.tcmcode) + SIZEOF(.tcmcode) );

    *(.tcm_code.*)
    *(.tcm_const.*)
    *(.sclock_rlt_code.*)
    *(.sclock_rlt_const.*)
  } >tcm_ocram AT >flash

  .data : ALIGN(4)
  {
    PROVIDE( _data_load = LOADADDR(.data) );
    PROVIDE( _data_run = ADDR(.data) );
    PROVIDE( _data_run_end = ADDR(.data) + SIZEOF(.data) );

    *(.pds_code.*)
    *(.data .data.*)
    *(.gnu.linkonce.d.*)

    . = ALIGN(8);
    /*always put freertos under global_pointer with the following order. No change!*/
    PROVIDE( __global_pointer$ = . + 0x7F0 );
    KEEP(*libbl702l_freertos.a:*(.sdata.*))
    KEEP(*libbl702l_freertos.a:*(.sbss.*))
    KEEP(*libbl702l_freertos.a:*(.bss.*))
    KEEP(*libbl702l_freertos.a:*(COMMON))
    *(.sdata .sdata.*)
    *(.gnu.linkonce.s.*)

    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
    
    . = ALIGN(8);
    *(._k_queue.static.*)
    *(._k_sem.static.*)
    *(._k_mutex.static.*)
    _bt_gatt_service_static_list_start = .;
    KEEP(*(SORT_BY_NAME("._bt_gatt_service_static.static.*")))
    _bt_gatt_service_static_list_end = .;
    _bt_l2cap_fixed_chan_list_start = .;
    KEEP(*(SORT_BY_NAME("._bt_l2cap_fixed_chan.static.*")))
    _bt_l2cap_fixed_chan_list_end = .;

    . = ALIGN(8);
    KEEP(*libCHIP.a:*(.bss.*PlatformManagerImp*))
  } >tcm_ocram AT >flash

  .boot2 (NOLOAD) :
  {
    PROVIDE ( __boot2_pt_addr_start = . );
    *(.bss.boot2_partition_table)
    PROVIDE ( __boot2_pt_addr_end   = . );

    PROVIDE ( __boot2_flashCfg_start = . );
    *(.bss.boot2_flashCfg)
    PROVIDE ( __boot2_flashCfg_end = . );
  } >hbnram

  .bss_psram (NOLOAD) :
  {
    PROVIDE( __bss_psram_start = ADDR(.bss_psram) );
    PROVIDE( __bss_psram_end = ADDR(.bss_psram) + SIZEOF(.bss_psram) );

    PROVIDE ( __psram_bss_init_start = . );
    /** put the bss data in psram between __psram_bss_init_start and __psram_bss_init_end will bet reset to 0 after psram init*/
    . = ALIGN(16);
    KEEP(*libCHIP.a:*(.sbss.*))
    KEEP(*libCHIP.a:*(.bss.*))

    . = ALIGN(16);
    KEEP(*libCHIPAppServer.a:*(.sbss.*))
    KEEP(*libCHIPAppServer.a:*(.bss.*))

    . = ALIGN(16);
    KEEP(*libMatterDeviceInfoProviderExample.a:*(.sbss.*))
    KEEP(*libMatterDeviceInfoProviderExample.a:*(.bss.*))

    . = ALIGN(16);
    KEEP(*libopenthread-ftd.a:*(.sbss.*))
    KEEP(*libopenthread-ftd.a:*(.bss.*))
    KEEP(*libopenthread-mtd.a:*(.sbss.*))
    KEEP(*libopenthread-mtd.a:*(.bss.*))

    . = ALIGN(16);
    KEEP(*src/app/clusters/*/bouffalolab*.cpp.o(.sbss.*))
    KEEP(*src/app/clusters/*/bouffalolab*.cpp.o(.bss.*))

    . = ALIGN(16);
    KEEP(*examples/platform/bouffalolab/common/plat/chip*.c.o(.sbss.*))
    KEEP(*examples/platform/bouffalolab/common/plat/chip*.c.o(.sbss.*))
    KEEP(*examples/platform/bouffalolab/common/plat/chip*.cpp.o(.bss.*))
    KEEP(*examples/platform/bouffalolab/common/plat/chip*.cpp.o(.bss.*))

    . = ALIGN(16);
    KEEP(*examples/*/bouffalolab/common/chip*.c.o(.sbss.*))
    KEEP(*examples/*/bouffalolab/common/chip*.c.o(.sbss.*))
    KEEP(*examples/*/bouffalolab/common/chip*.cpp.o(.bss.*))
    KEEP(*examples/*/bouffalolab/common/chip*.cpp.o(.bss.*))

    . = ALIGN(16);
    KEEP(*sdk_lwip.*.c.o(.sbss.*))
    KEEP(*sdk_lwip.*.c.o(.bss.*))
    KEEP(*lwip.*.c.o(.bss.*))

    . = ALIGN(16);
    KEEP(*stack_main.o(.bss.*))
    
    . = ALIGN(16);
    *(.wifi_ram*)
    PROVIDE ( __psram_bss_init_end = . );
  } > psram

  .bss (NOLOAD) :
  {
    PROVIDE( __bss_start = ADDR(.bss) );
    PROVIDE( __bss_end = ADDR(.bss) + SIZEOF(.bss) );

    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
  } >tcm_ocram

  .sha_ocram (NOLOAD) :
  {
    *(.sha_ocram*)
  } >tcm_ocram

  . = ALIGN(8);
  PROVIDE( _heap_start = . );
  PROVIDE( _heap_size = ORIGIN(tcm_ocram) + LENGTH(tcm_ocram) - _heap_start );

  PROVIDE( _heap2_start = 0 );
  PROVIDE( _heap2_size = 0 );

  .stack (NOLOAD) : ALIGN(8)
  {
    PROVIDE( _sp_base = . );
    . = . + __stack_size;
    PROVIDE( _sp_main = . );
    __freertos_irq_stack_top = .;
  } >sys_stack

  .psram (NOLOAD) :
  {
    . = ALIGN(16);
    PROVIDE( _psram_start = . );
    *(.wifi_ram*)
    . = ALIGN(16);
    PROVIDE( _psram_end = . );
  } >psram

  . = ALIGN(8);
  PROVIDE( _heap3_start = . );
  PROVIDE( _heap3_size = ORIGIN(psram) + LENGTH(psram) - _heap3_start );


  /* cli */
  PROVIDE( _ld_bl_static_cli_cmds_start = _bl_static_cli_cmds_start );
  PROVIDE( _ld_bl_static_cli_cmds_end   = _bl_static_cli_cmds_end );

  /* blog */
  PROVIDE( _ld_bl_static_blogcomponent_code_start = _bl_static_blogcomponent_code_start );
  PROVIDE( _ld_bl_static_blogcomponent_code_end   = _bl_static_blogcomponent_code_end );
  PROVIDE( _ld_bl_static_blogfile_code_start = _bl_static_blogfile_code_start );
  PROVIDE( _ld_bl_static_blogfile_code_end   = _bl_static_blogfile_code_end );
  PROVIDE( _ld_bl_static_blogpri_code_start = _bl_static_blogpri_code_start );
  PROVIDE( _ld_bl_static_blogpri_code_end   = _bl_static_blogpri_code_end );

  /* ram information */
  PROVIDE( _ld_ram_size0 = LENGTH(flash) );
  PROVIDE( _ld_ram_addr0 = ORIGIN(flash) );
  PROVIDE( _ld_ram_size1 = LENGTH(tcm_ocram) );
  PROVIDE( _ld_ram_addr1 = ORIGIN(tcm_ocram) );
  PROVIDE( _ld_ram_size2 = LENGTH(hbnram) );
  PROVIDE( _ld_ram_addr2 = ORIGIN(hbnram) );
  PROVIDE( _ld_ram_size3 = SIZEOF(.stack) );
  PROVIDE( _ld_ram_addr3 = ADDR(.stack) );
  PROVIDE( _ld_psram_size = LENGTH(psram) );
  PROVIDE( _ld_psram_addr = ORIGIN(psram) );

  /* EM size */
  PROVIDE( __LD_CONFIG_EM_SEL = __EM_SIZE );

  /* BLE_CORE_REG size */
  PROVIDE( __LD_BLE_CORE_REG_SIZE = __EM_SIZE ? 0x214 : 0 );

  /* rom driver */
  INCLUDE ../../third_party/bouffalolab/repo/components/platform/soc/bl702l/bl702l/evb/ld/romdriver.ld
}

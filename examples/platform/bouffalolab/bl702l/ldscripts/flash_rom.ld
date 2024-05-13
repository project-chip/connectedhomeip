OUTPUT_ARCH( "riscv" )

ENTRY( bl702_start )

__EM_SIZE = DEFINED(btble_controller_init) ? 16K : 0K;
__CACHE_SIZE = DEFINED(__CACHE_SIZE) ? __CACHE_SIZE : 16K;

MEMORY
{
  flash  (rxai!w) : ORIGIN = 0x23000000, LENGTH = (2M)
  tcm_ocram (wxa) : ORIGIN = 0x4201C000 + __CACHE_SIZE, LENGTH = (16K - __CACHE_SIZE + 80K - __EM_SIZE)
  hbnram    (wxa) : ORIGIN = 0x40010000, LENGTH = (4K)
}

SECTIONS
{
  __stack_size = DEFINED(__stack_size) ? __stack_size : 2K;
  BOOT2_PT_ADDR = 0x4202e094;

  .init :
  {
    KEEP (*(SORT_NONE(.init)))
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

    *(.gnu.linkonce.r.*)
  } >flash

  .init_array :
  {
    __init_array_start = .;
    KEEP(*(.init_array))
    __init_array_end = .;
  } >flash

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

  .rsvd (NOLOAD) :
  {
    *(.rsvd_data)
    *(.ble_rsvd_mem)
  } >hbnram

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

  .romdata : ALIGN(16)
  {
    PROVIDE( _rom_data_run = . );
    PROVIDE( __global_pointer$ = . + 0x800 );
    . = . + 0xc98;
  } >tcm_ocram

  .data : ALIGN(4)
  {
    PROVIDE( _data_load = LOADADDR(.data) );
    PROVIDE( _data_run = ADDR(.data) );
    PROVIDE( _data_run_end = ADDR(.data) + SIZEOF(.data) );

    *(.pds_code.*)
    *(.data .data.*)
    *(.gnu.linkonce.d.*)

    . = ALIGN(8);
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
  } >tcm_ocram AT >flash

  .boot2 (NOLOAD) :
  {
    PROVIDE ( __boot2_pt_addr_start = . );
    *(.bss.boot2_partition_table)
    PROVIDE ( __boot2_pt_addr_end   = . );

    PROVIDE ( __boot2_flashCfg_start = . );
    *(.bss.boot2_flashCfg)
    PROVIDE ( __boot2_flashCfg_end = . );
  } >tcm_ocram

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

  .stack (NOLOAD) : ALIGN(8)
  {
    PROVIDE ( _sp_base = . );
    . = . + __stack_size;
    PROVIDE( _sp_main = . );
    __freertos_irq_stack_top = .;
  } >tcm_ocram

  . = ALIGN(8);
  PROVIDE( _heap_start = . );
  PROVIDE( _heap_size = ORIGIN(tcm_ocram) + LENGTH(tcm_ocram) - _heap_start );

  PROVIDE( _heap2_start = 0 );
  PROVIDE( _heap2_size = 0 );


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

  /* BOOT2 parameter */
  PROVIDE ( __boot2_pt_addr_src = BOOT2_PT_ADDR );

  /* EM size */
  PROVIDE(__LD_CONFIG_EM_SEL = __EM_SIZE);

  /* rom driver */
  INCLUDE ../../third_party/bouffalolab/repo/components/platform/soc/bl702l/bl702l/evb/ld/romdriver.ld

  /* rom data & code */
  INCLUDE ../..//third_party/bouffalolab/repo/components/platform/soc/bl702l/bl702l/evb/ld/rom_map.ld
}

/*
 * (c) 2019-2020, Qorvo
 */

/* Memory Spaces Definitions */
MEMORY
{
  SYSRAM (rxw) : ORIGIN = 0x20008000, LENGTH = 0x8000
  /* Do not use last 32 bytes of retained UCRAM as these are used for the ROM bootloader stack if sleep is enabled */
  UCRAM  (rxw) : ORIGIN = 0x20010000, LENGTH = 0xffe0
  FLASH (rx) : ORIGIN = 0x4000000, LENGTH = 0x80000
  ROM    (rx)  : ORIGIN = 0x04800000, LENGTH = 0x8000
}

SECTIONS
{
    flash_end = 0x4080000;

/* 
 * User license is first - only for FLASH programs.
 *
 * BLE/MAC address entries (+0x10) in the user license skipped to
 * allow injection of custom values by programmer tool.
 */
    .user_license   0x4000000 + 0x10 : { KEEP(*(.user_license)); } > FLASH
    .isr_vector : ALIGN(0x100) { KEEP(*(.isr_vector)); } > FLASH

    .text :   { . = ALIGN(4); *(.text)   *(.text.*);  } > FLASH
    .rodata : { . = ALIGN(4); *(.rodata) *(.rodata.*);} > FLASH
    .rom :    { . = ALIGN(4); *(.rom)   *(.rom.*);    } > ROM

    /* Start retained memory in sleep */
    _slower_retain = 0x20008000;

    /* Fixed sections */
    .mw_crc 0x20008000 : { . += 0x4; } > SYSRAM
    .crc    0x20008004 : { . += 0x4; } > SYSRAM
    .ret_hw 0x20008008 : { . += 0x1B4; } > SYSRAM
    .ret_sw 0x200081BC : { . += 0x7B; }  > SYSRAM
    ASSERT(. >= 0x200081c0, "gpmicro areas must fall into retention areas")

    /*      0x20008370-0x2000837F : unused space
     * .events and .pbm_options too big for this space */

    /* PUF RAM Area */
    .pkey   0x20008380 : { . += 0x80;  }  > SYSRAM
    .pufr   0x20008400 : { . += 0x400; }  > SYSRAM

    /* Windowed sections */
    /* ram_regmap, events, pbm options and pbm data are taken from buffers in .lower_ram_retain */

    .lower_ram_retain_gpmicro_accessible (NOLOAD) :  { . = ALIGN(4); *(.lower_ram_retain_gpmicro_accessible) *(.lower_ram_retain_gpmicro_accessible.*); } > SYSRAM
    .lower_ram_retain (NOLOAD) : { . = ALIGN(4); *(.lower_ram_retain) *(.lower_ram_retain.*); } > SYSRAM

    .data : { . = ALIGN(4); *(.data) *(.data.*);          } > SYSRAM AT > FLASH
    .bss :  { . = ALIGN(4); *(.bss)  *(.bss.*) *(COMMON); } > UCRAM

    _elower_retain = . ; /* End memory to be retained */
    .lower_ram_noretain (NOLOAD) : { . = ALIGN(4); *(.lower_ram_noretain) *(.lower_ram_noretain.*); } > SYSRAM
    _elower_ram = . ;


    /* make sure this goes into ucram */
    . = ORIGIN(UCRAM);

    PROVIDE(_shigher_retain = .);
    .higher_ram_retain (NOLOAD) : { . = ALIGN(4); *(.higher_ram_retain) *(.higher_ram_retain.*); } > UCRAM
    .heap   (NOLOAD) :            { . = ALIGN(4); . += 0x8000; } > UCRAM

    PROVIDE(_ehigher_retain = .);
    .higher_ram_noretain (NOLOAD) : { . = ALIGN(4); *(.higher_ram_noretain) *(.higher_ram_noretain.*); } > UCRAM
    _ehigher_ram = . ;

    /* remove the debugging information from the standard libraries */
    /DISCARD/ : {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .gpNvm flash_end - 0x4000:
    {
        gpNvm_Start = . ;
        . = 0x4000 ;
        gpNvm_End = . ;
    } > FLASH

    /* Linker Symbols */
    _vpp    = ADDR(.isr_vector) >> 8;
    _etext  = ADDR(.text) + SIZEOF(.text);
    _sidata = LOADADDR(.data);
    _sdata  = ADDR(.data);
    _edata  = ADDR(.data) + ALIGN(SIZEOF(.data), 4);
    _ldata  = _edata - _sdata;
    _sbss   = ADDR(.bss);
    _ebss   = ADDR(.bss)  + ALIGN(SIZEOF(.bss),  4);
    _lbss   = _ebss - _sbss;
    _sheap  = ADDR(.heap);
    _eheap  = ADDR(.heap)  + ALIGN(SIZEOF(.heap),  4);
    _lheap  = _eheap - _sheap;

    /* using remaining UCRAM for stack */
    _sstack = _eheap;
    _estack = ORIGIN(UCRAM) + LENGTH(UCRAM);

    /* check minimum stack size is still available */
    min_stack_size = 0x200;
    stack_size     = _estack - _sstack;
    ASSERT(stack_size >= min_stack_size, "STACK too small")

    /* needed for ram retention configuration */
    __lowerram_retain_size     = _elower_retain - _slower_retain;
    __higherram_retain_size     = SIZEOF(.higher_ram_retain);

    sw_retention_begin = ADDR(.ret_sw);
    sw_retention_end = ADDR(.ret_sw) + SIZEOF(.ret_sw);
    sw_retention_size = SIZEOF(.ret_sw);
}

ENTRY(reset_handler)

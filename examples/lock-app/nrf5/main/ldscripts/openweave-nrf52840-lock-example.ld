SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

/* Total size of device FLASH in bytes */
TOTAL_FLASH_SIZE = 0x100000;

/* Total size of device RAM in bytes */
TOTAL_RAM_SIZE = 0x40000;

/* Size of an individual FLASH page in bytes */
FLASH_PAGE_SIZE = 4096;

/* Number of FLASH pages reserved for Nordic FDS.  NOTE: This MUST correspond
 * to the value specified for FDS_VIRTUAL_PAGES in app_config.h */
FDS_FLASH_PAGES = 2;

/* Number of FLASH pages reserved for OpenThread data storage. */
OT_DATA_FLASH_PAGES = 4;

MEMORY
{
    /* FLASH region occupied by the Nordic SoftDevice */
    SD_FLASH (rx) : ORIGIN = 0, LENGTH = 0x26000
    
    /* RAM region used by the Nordic SoftDevice */
    SD_RAM (rw) : ORIGIN = 0x20000000, LENGTH = 0x5800

    /* FLASH region used for Nordic FDS value storage. */
    FDS_FLASH (rw) : ORIGIN = TOTAL_FLASH_SIZE - (FLASH_PAGE_SIZE * FDS_FLASH_PAGES), LENGTH = (FLASH_PAGE_SIZE * FDS_FLASH_PAGES)

    /* FLASH region used for OpenThread data storage. */ 
    OT_DATA_FLASH (rw) : ORIGIN = ORIGIN(FDS_FLASH) - (FLASH_PAGE_SIZE * OT_DATA_FLASH_PAGES), LENGTH = (FLASH_PAGE_SIZE * OT_DATA_FLASH_PAGES)
    
    /* FLASH region used for application code and read-only data. */
    FLASH (rx) : ORIGIN = ORIGIN(SD_FLASH) + LENGTH(SD_FLASH), LENGTH = ORIGIN(OT_DATA_FLASH) - ORIGIN(FLASH)
    
    /* RAM region used for application dynamic data. */
    RAM (rw) : ORIGIN = ORIGIN(SD_RAM) + LENGTH(SD_RAM), LENGTH = TOTAL_RAM_SIZE - LENGTH(SD_RAM)
}

SECTIONS
{
    . = ALIGN(4);
  
    .log_dynamic_data :
    {
        PROVIDE(__start_log_dynamic_data = .);
        KEEP(*(SORT(.log_dynamic_data*)))
        PROVIDE(__stop_log_dynamic_data = .);
    } > RAM
    
    .log_filter_data :
    {
        PROVIDE(__start_log_filter_data = .);
        KEEP(*(SORT(.log_filter_data*)))
        PROVIDE(__stop_log_filter_data = .);
    } > RAM
} 
INSERT AFTER .data;

SECTIONS
{
    .sdh_soc_observers :
    {
        PROVIDE(__start_sdh_soc_observers = .);
        KEEP(*(SORT(.sdh_soc_observers*)))
        PROVIDE(__stop_sdh_soc_observers = .);
    } > FLASH
    
    .sdh_ble_observers :
    {
        PROVIDE(__start_sdh_ble_observers = .);
        KEEP(*(SORT(.sdh_ble_observers*)))
        PROVIDE(__stop_sdh_ble_observers = .);
    } > FLASH
    
    .sdh_stack_observers :
    {
        PROVIDE(__start_sdh_stack_observers = .);
        KEEP(*(SORT(.sdh_stack_observers*)))
        PROVIDE(__stop_sdh_stack_observers = .);
    } > FLASH
    
    .sdh_req_observers :
    {
        PROVIDE(__start_sdh_req_observers = .);
        KEEP(*(SORT(.sdh_req_observers*)))
        PROVIDE(__stop_sdh_req_observers = .);
    } > FLASH
    
    .sdh_state_observers :
    {
        PROVIDE(__start_sdh_state_observers = .);
        KEEP(*(SORT(.sdh_state_observers*)))
        PROVIDE(__stop_sdh_state_observers = .);
    } > FLASH
    
    .crypto_data :
    {
        PROVIDE(__start_crypto_data = .);
        KEEP(*(SORT(.crypto_data*)))
        PROVIDE(__stop_crypto_data = .);
    } > FLASH
    
    .log_const_data :
    {
        PROVIDE(__start_log_const_data = .);
        KEEP(*(SORT(.log_const_data*)))
        PROVIDE(__stop_log_const_data = .);
    } > FLASH
    
    .log_backends :
    {
        PROVIDE(__start_log_backends = .);
        KEEP(*(SORT(.log_backends*)))
        PROVIDE(__stop_log_backends = .);
    } > FLASH
    
    .nrf_balloc :
    {
        PROVIDE(__start_nrf_balloc = .);
        KEEP(*(.nrf_balloc))
        PROVIDE(__stop_nrf_balloc = .);
    } > FLASH

    __start_ot_flash_data = ORIGIN(OT_DATA_FLASH);
    __stop_ot_flash_data = (ORIGIN(OT_DATA_FLASH) + LENGTH(OT_DATA_FLASH));
}
INSERT AFTER .text

INCLUDE "nrf_common.ld"

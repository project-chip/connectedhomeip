#ifdef __cplusplus
extern "C" {
#endif
#include "sl_spidrv_exp_config.h"
#include "sl_mx25_flash_shutdown_usart_config.h"
#include "sl_wfx_host_api.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define LCD_BIT_RATE                 1100000
#define EXP_HDR_BIT_RATE             16000000
#define SPI_FLASH_BIT_RATE           16000000

typedef enum PERIPHERAL_TYPE {
  EXP_HDR =0,
  LCD    ,
  EXT_SPIFLASH ,
}peripheraltype_t;

extern SemaphoreHandle_t spi_sem_sync_hdl;
extern peripheraltype_t pr_type;

void spi_drv_reinit(uint32_t);
void set_spi_baudrate(peripheraltype_t);

void spiflash_cs_assert(void);
void spiflash_cs_deassert(void);

void pre_bootloader_spi_transfer(void);
void post_bootloader_spi_transfer(void);

void pre_lcd_spi_transfer(void);
void post_lcd_spi_transfer(void);

#ifdef __cplusplus
}
#endif

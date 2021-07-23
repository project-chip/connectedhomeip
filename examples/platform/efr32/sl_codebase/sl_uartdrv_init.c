#include "uartdrv.h"
#include "sl_uartdrv_instances.h"
#include <stddef.h>

#include "sl_uartdrv_usart_vcom_config.h"

UARTDRV_HandleData_t sl_uartdrv_usart_vcom_handle_data;
UARTDRV_Handle_t sl_uartdrv_usart_vcom_handle = &sl_uartdrv_usart_vcom_handle_data;

static UARTDRV_Handle_t sli_uartdrv_default_handle = NULL;

/* If CTS and RTS not defined, define a default value to avoid errors */
#ifndef SL_UARTDRV_USART_VCOM_CTS_PORT
#define SL_UARTDRV_USART_VCOM_CTS_PORT  gpioPortA
#define SL_UARTDRV_USART_VCOM_CTS_PIN   0
#if defined(_USART_ROUTELOC1_MASK)
#define SL_UARTDRV_USART_VCOM_CTS_LOC   0
#endif
#endif

#ifndef SL_UARTDRV_USART_VCOM_RTS_PORT
#define SL_UARTDRV_USART_VCOM_RTS_PORT  gpioPortA
#define SL_UARTDRV_USART_VCOM_RTS_PIN   0
#if defined(_USART_ROUTELOC1_MASK)
#define SL_UARTDRV_USART_VCOM_RTS_LOC   0
#endif
#endif


/* Define RX and TX buffer queues */
DEFINE_BUF_QUEUE(SL_UARTDRV_USART_VCOM_RX_BUFFER_SIZE, sl_uartdrv_usart_vcom_rx_buffer);
DEFINE_BUF_QUEUE(SL_UARTDRV_USART_VCOM_TX_BUFFER_SIZE, sl_uartdrv_usart_vcom_tx_buffer);


/* Create uartdrv initialization structs */
UARTDRV_InitUart_t sl_uartdrv_usart_init_vcom = { 
  .port = SL_UARTDRV_USART_VCOM_PERIPHERAL,
  .baudRate = SL_UARTDRV_USART_VCOM_BAUDRATE,
#if defined(_USART_ROUTELOC0_MASK)
  .portLocationTx = SL_UARTDRV_USART_VCOM_TX_LOC,
  .portLocationRx = SL_UARTDRV_USART_VCOM_RX_LOC,
#elif defined(_USART_ROUTE_MASK)
  .portLocation = SL_UARTDRV_USART_VCOM_ROUTE_LOC,
#elif defined(_GPIO_USART_ROUTEEN_MASK)
  .txPort = SL_UARTDRV_USART_VCOM_TX_PORT,
  .rxPort = SL_UARTDRV_USART_VCOM_RX_PORT,
  .txPin = SL_UARTDRV_USART_VCOM_TX_PIN,
  .rxPin = SL_UARTDRV_USART_VCOM_RX_PIN,
  .uartNum = SL_UARTDRV_USART_VCOM_PERIPHERAL_NO,
#endif
  .stopBits = SL_UARTDRV_USART_VCOM_STOP_BITS,
  .parity = SL_UARTDRV_USART_VCOM_PARITY,
  .oversampling = SL_UARTDRV_USART_VCOM_OVERSAMPLING,
#if defined(USART_CTRL_MVDIS)
  .mvdis = SL_UARTDRV_USART_VCOM_MVDIS,
#endif
  .fcType = SL_UARTDRV_USART_VCOM_FLOW_CONTROL_TYPE,
  .ctsPort = SL_UARTDRV_USART_VCOM_CTS_PORT,
  .rtsPort = SL_UARTDRV_USART_VCOM_RTS_PORT,
  .ctsPin = SL_UARTDRV_USART_VCOM_CTS_PIN,
  .rtsPin = SL_UARTDRV_USART_VCOM_RTS_PIN,
  .rxQueue = (UARTDRV_Buffer_FifoQueue_t *)&sl_uartdrv_usart_vcom_rx_buffer,
  .txQueue = (UARTDRV_Buffer_FifoQueue_t *)&sl_uartdrv_usart_vcom_tx_buffer,
#if defined(_USART_ROUTELOC1_MASK)
  .portLocationCts = SL_UARTDRV_USART_VCOM_CTS_LOC,
  .portLocationRts = SL_UARTDRV_USART_VCOM_RTS_LOC,
#endif
};


void sl_uartdrv_init_instances(void){
  UARTDRV_InitUart(sl_uartdrv_usart_vcom_handle, &sl_uartdrv_usart_init_vcom);
  sl_uartdrv_set_default(sl_uartdrv_usart_vcom_handle);
}

sl_status_t sl_uartdrv_set_default(UARTDRV_Handle_t handle)
{
  sl_status_t status = SL_STATUS_INVALID_HANDLE;

  if (handle != NULL) {
    sli_uartdrv_default_handle = handle;
    status = SL_STATUS_OK;
  }

  return status;
}

UARTDRV_Handle_t sl_uartdrv_get_default(void)
{
  return sli_uartdrv_default_handle;
}

#ifndef __UART_IF_H__
#define __UART_IF_H__

#ifdef __cplusplus
extern "C"
{
#endif

// TI-Driver includes
#include <ti/drivers/UART2.h>
#include "ti_drivers_config.h"

//Defines

#define UART_PRINT Report
#define DBG_PRINT  Report
#define ERR_PRINT(x) Report("Error [%d] at line [%d] in function [%s]  \n\r",\
                                                                x, __LINE__, \
                                                                 __FUNCTION__)

/* API */

UART2_Handle InitUart(void);
int32_t UartGetJsonStruct(char *pBuffer, int32_t BufLen);


char getch(void);

#ifdef __cplusplus
}
#endif

#endif // __UART_IF_H__

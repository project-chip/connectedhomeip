/**
  ******************************************************************************
  * @file    dbg_trace.h
  * @author  MCD Application Team
  * @brief   Header for dbg_trace.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DBG_TRACE_H
#define DBG_TRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported types ------------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#if ( ( CFG_DEBUG_TRACE_FULL != 0 ) || ( CFG_DEBUG_TRACE_LIGHT != 0 ) )
#define PRINT_LOG_BUFF_DBG(...) DbgTraceBuffer(__VA_ARGS__)
#if ( CFG_DEBUG_TRACE_FULL != 0 )
#define PRINT_MESG_DBG(...)     do{printf("\r\n [%s][%s][%d] ", DbgTraceGetFileName(__FILE__),__FUNCTION__,__LINE__);printf(__VA_ARGS__);}while(0);
#else
#define PRINT_MESG_DBG          printf
#endif
#else
#define PRINT_LOG_BUFF_DBG(...)
#define PRINT_MESG_DBG(...)
#endif

#define PRINT_NO_MESG(...)

/* Exported functions ------------------------------------------------------- */

  /**
   * @brief Request the user to initialize the peripheral to output traces
   *
   * @param  None
   * @retval None
   */
extern void DbgOutputInit( void );

/**
 * @brief Request the user to sent the traces on the output peripheral
 *
 * @param  p_data:  Address of the buffer to be sent
 * @param  size:    Size of the data to be sent
 * @param  cb:      Function to be called when the data has been sent
 * @retval None
 */
extern void DbgOutputTraces(  uint8_t *p_data, uint16_t size, void (*cb)(void) );

/**
 * @brief DbgTraceInit Initialize Logging feature.
 *
 * @param:  None
 * @retval: None
 */
void DbgTraceInit( void );

/**********************************************************************************************************************/
/** This function outputs into the log the buffer (in hex) and the provided format string and arguments.
 ***********************************************************************************************************************
 *
 * @param pBuffer Buffer to be output into the logs.
 * @param u32Length Length of the buffer, in bytes.
 * @param strFormat The format string in printf() style.
 * @param ... Arguments of the format string.
 *
 **********************************************************************************************************************/
void DbgTraceBuffer( const void *pBuffer , uint32_t u32Length , const char *strFormat , ... );

const char *DbgTraceGetFileName( const char *fullpath );

/**
 * @brief Override the standard lib function to redirect printf to USART.
 * @param handle output handle (STDIO, STDERR...)
 * @param buf buffer to write
 * @param bufsize buffer size
 * @retval Number of elements written
 */
size_t DbgTraceWrite(int handle, const unsigned char * buf, size_t bufSize);

#ifdef __cplusplus
}
#endif

#endif /* DBG_TRACE_H */


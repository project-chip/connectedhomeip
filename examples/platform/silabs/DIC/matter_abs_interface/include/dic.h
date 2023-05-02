/**
 * @file
 * @brief Matter abstraction layer for Direct Internet Connectivity.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#ifndef __DIC_H
#define __DIC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"

typedef enum {
	DIC_OK = 0,
	DIC_ERR_INVAL,
	DIC_ERR_MEM,
	DIC_ERR_FAIL,
} dic_err_t;

typedef struct {
	uint8_t *dataP;
	uint16_t dataLen;
} dic_buff_t;

dic_err_t DIC_Init(void);

dic_err_t DIC_SendMsg(const char *subject, const char *content);

#ifdef __cplusplus
}
#endif
#endif //__DIC_H

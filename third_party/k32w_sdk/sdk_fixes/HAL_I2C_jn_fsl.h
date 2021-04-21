/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef _HAL_I2C_JN_FSL_H_
#define _HAL_I2C_JN_FSL_H_

#include "fsl_i2c.h"
#include "ntag_defines.h"

#define HAL_I2C_HANDLE_T I2C_Type *
#define HAL_I2C_INIT_PARAM_T uint32_t
#define HAL_I2C_INIT_DEFAULT 400000U
#define HAL_I2C_INVALID_HANDLE NULL
#define HAL_I2C_STATUS_T uint16_t
#define HAL_I2C_OK 0
#define HAL_I2C_InitDevice(bitrate, input_clock, instance) I2C_InitDevice(bitrate, input_clock, instance)
#define HAL_I2C_SendBytes(instance, address, bytes, len) TransmitPoll(instance, address, bytes, len)
#define HAL_I2C_RecvBytes(instance, address, bytes, len) ReceivePoll(instance, address, bytes, len)
#define HAL_I2C_CloseDevice(instance) I2C_CloseDevice(instance)
#define HAL_I2C_RX_RESERVED_BYTES 0
#define HAL_I2C_TX_RESERVED_BYTES 0

#ifdef __cplusplus
extern "C" {
#endif

int16_t I2C_InitDevice(uint32_t bitrate, clock_name_t input_clock, I2C_Type *instance);
int16_t I2C_CloseDevice(I2C_Type *instance);

int16_t ReceivePoll(I2C_Type *instance, uint8_t address, uint8_t *bytes, size_t len);
int16_t TransmitPoll(I2C_Type *instance, uint8_t address, uint8_t *bytes, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_I2C_JN_FSL_H_ */

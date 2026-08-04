/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    bleplat.c
 * @author  MCD Application Team
 * @brief   This file implements the platform functions for BLE stack library.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#include "bleplat.h"
#include "app_ble.h"
#include "app_common.h"
#include "baes.h"
#include "ble_timer.h"
#include "blestack.h"
#include "host_stack_if.h"
#include "pka_ctrl.h"

#include "ble_wrap.c"

/*****************************************************************************/

void BLEPLAT_Init(void)
{

    return;
}

/*****************************************************************************/

void BLEPLAT_NvmStore(const uint64_t * ptr, uint16_t size)
{
    UNUSED(ptr);
    UNUSED(size);

    APP_BLE_HostNvmStore();
    return;
}

/*****************************************************************************/

void BLEPLAT_RngGet(uint8_t n, uint32_t * val)
{
    /* Read 32-bit random values from HW driver */
    HW_RNG_Get(n, val);
}

/*****************************************************************************/

void BLEPLAT_AesEcbEncrypt(const uint8_t * key, const uint8_t * input, uint8_t * output)
{
    BAES_EcbCrypt(key, input, output, 1);
}

/*****************************************************************************/

void BLEPLAT_AesCmacSetKey(const uint8_t * key)
{
    BAES_CmacSetKey(key);
}

/*****************************************************************************/

void BLEPLAT_AesCmacCompute(const uint8_t * input, uint32_t input_length, uint8_t * output_tag)
{
    BAES_CmacCompute(input, input_length, output_tag);
}

/*****************************************************************************/

int BLEPLAT_AesCcmCrypt(uint8_t mode, const uint8_t * key, uint8_t iv_length, const uint8_t * iv, uint16_t add_length,
                        const uint8_t * add, uint32_t input_length, const uint8_t * input, uint8_t tag_length, uint8_t * tag,
                        uint8_t * output)
{
    return BAES_CcmCrypt(mode, key, iv_length, iv, add_length, add, input_length, input, tag_length, tag, output);
}

/*****************************************************************************/

int BLEPLAT_PkaStartP256Key(const uint32_t * local_private_key)
{
    return PKACTRL_StartP256Key(local_private_key);
}

/*****************************************************************************/

void BLEPLAT_PkaReadP256Key(uint32_t * local_public_key)
{
    PKACTRL_ReadP256Key(local_public_key);
}

/*****************************************************************************/

int BLEPLAT_PkaStartDhKey(const uint32_t * local_private_key, const uint32_t * remote_public_key)
{
    return PKACTRL_StartDhKey(local_private_key, remote_public_key);
}

/*****************************************************************************/

int BLEPLAT_PkaReadDhKey(uint32_t * dh_key)
{
    return PKACTRL_ReadDhKey(dh_key);
}

/*****************************************************************************/

void PKACTRL_CB_Complete(void)
{
    BLEPLATCB_PkaComplete();

    BleStackCB_Process();
}

/*****************************************************************************/

uint8_t BLEPLAT_TimerStart(uint16_t id, uint32_t timeout)
{
    return BLE_TIMER_Start(id, timeout);
}

/*****************************************************************************/

void BLEPLAT_TimerStop(uint16_t id)
{
    BLE_TIMER_Stop(id);
}
/*****************************************************************************/

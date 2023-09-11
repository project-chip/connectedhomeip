/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Implementation of a key-value store using mtb_kvstore lib
 *
 */

#pragma once

#include "cy_result.h"
#include "cyhal_flash.h"
#include "mtb_kvstore.h"

cy_rslt_t mtb_key_value_store_init(mtb_kvstore_t * kvstore_obj);
uint32_t bd_read_size(void * context, uint32_t addr);
uint32_t bd_program_size(void * context, uint32_t addr);
uint32_t bd_erase_size(void * context, uint32_t addr);
cy_rslt_t bd_read(void * context, uint32_t addr, uint32_t length, uint8_t * buf);
cy_rslt_t bd_program(void * context, uint32_t addr, uint32_t length, const uint8_t * buf);
cy_rslt_t bd_erase(void * context, uint32_t addr, uint32_t length);

/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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

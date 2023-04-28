/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#pragma once
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "AppTask.h"
#ifdef CFG_PLF_RV32
#include "asr_adc.h"
#define duet_adc_dev_t asr_adc_dev_t
#else
#include "duet_adc.h"
#endif

#define MAX_BRIGHTNESS 254
#define MAX_VOLTAGE    1200

class DCP_Sensor
{
public:
    void Init(void);
    int32_t Get(void);
    void Report(int32_t iVol);

    int32_t iVoltage;

private:
    duet_adc_dev_t adc_config_struct;
};
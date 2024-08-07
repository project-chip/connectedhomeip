/*******************************************************************************
 * File Name: cycfg_routing.c
 *
 * Description:
 * Establishes all necessary connections between hardware elements.
 * This file was automatically generated and should not be modified.
 * Configurator Backend 3.10.0
 * device-db 4.100.0.4304
 * mtb-pdl-cat1 3.9.0.29592
 *
 ********************************************************************************
 * Copyright 2024 Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ********************************************************************************/

#include "cycfg_routing.h"
#include "cy_device_headers.h"
#include "cy_trigmux.h"
#include "stdbool.h"

void init_cycfg_routing(void)
{
    HSIOM->AMUX_SPLIT_CTL[2] = HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_AA_SL_Msk | HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_AA_SR_Msk |
        HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_BB_SL_Msk | HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_BB_SR_Msk;
    HSIOM->AMUX_SPLIT_CTL[4] = HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_AA_SL_Msk | HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_AA_SR_Msk |
        HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_BB_SL_Msk | HSIOM_V2_AMUX_SPLIT_CTL_SWITCH_BB_SR_Msk;
}

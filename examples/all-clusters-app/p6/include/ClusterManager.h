/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
 * @file ClusterManager.h
 *
 * Implementations for the DeviceManager callbacks for this application
 *
 **/

#pragma once

#include "LEDWidget.h"
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceLayer.h>

extern LEDWidget sStatusLED;
extern LEDWidget sLightLED;
extern LEDWidget sClusterLED;

class ClusterManager
{
public:
    void OnOnOffPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnLevelControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnColorControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnIdentifyPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);

private:
    friend ClusterManager & ClusterMgr(void);
    bool mEndpointOnOffState[2];
    static ClusterManager sCluster;
};

inline ClusterManager & ClusterMgr(void)
{
    return ClusterManager::sCluster;
}

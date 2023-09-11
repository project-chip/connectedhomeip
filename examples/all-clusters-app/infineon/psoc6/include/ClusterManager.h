/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2021 Cypress Semiconductor Corporation (an Infineon company)
 *
 * SPDX-License-Identifier: Apache-2.0
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

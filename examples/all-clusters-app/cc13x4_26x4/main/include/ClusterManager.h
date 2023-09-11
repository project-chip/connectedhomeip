/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file ClusterManager.h
 *
 * Declarations for the ClusterManager callbacks for this application
 *
 **/

#pragma once

#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceLayer.h>

class ClusterManager
{
public:
    void OnOnOffPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnLevelControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnColorControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnIdentifyPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint16_t size,
                                               uint8_t * value);

private:
    friend ClusterManager & ClusterMgr(void);
    bool mEndpointOnOffState[2];
    static ClusterManager sCluster;
};

inline ClusterManager & ClusterMgr(void)
{
    return ClusterManager::sCluster;
}

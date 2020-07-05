/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file contains definitions for standard CHIP Clusters
 *
 */

#ifndef CHIPSTANDARDCLUSTERS_H_
#define CHIPSTANDARDCLUSTERS_H_

#include <lib/core/CHIPBaseCluster.h>
#include <lib/core/CHIPStandardAttributes.h>

namespace chip {
namespace DataModel {

/* Base Cluster */
    static inline CHIPBaseCluster *CHIPClusterBasicNew(uint8_t ZCLVersion,
                                                       uint8_t applicationVersion,
                                                       uint8_t stackVersion,
                                                       uint8_t HWVersion)
{
    CHIPBaseCluster *cluster = new CHIPBaseCluster(0x0000);
    cluster->AddAttribute(CHIPAttributeZCLVersionNew(ZCLVersion));
    cluster->AddAttribute(CHIPAttributeApplicationVersionNew(applicationVersion));
    cluster->AddAttribute(CHIPAttributeStackVersionNew(stackVersion));
    cluster->AddAttribute(CHIPAttributeHWVersionNew(HWVersion));
    return cluster;
}

/* On/Off Cluster */
static inline CHIPBaseCluster *CHIPClusterOnOffNew(void)
{
    CHIPBaseCluster *cluster = new CHIPBaseCluster(0x0006);
    cluster->AddAttribute(CHIPAttributeOnOffNew());
    cluster->AddAttribute(CHIPAttributeGlobalSceneControlNew());
    cluster->AddAttribute(CHIPAttributeOnTimeNew());
    cluster->AddAttribute(CHIPAttributeOffWaitTimeNew());
    return cluster;
}

} // namespace DataModel
} // namespace chip

#endif /* CHIPSTANDARDCLUSTERS_H_ */

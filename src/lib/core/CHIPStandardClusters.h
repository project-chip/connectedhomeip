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

static const uint16_t kClusterIdBase = 0x0000;
class CHIPClusterBasic : public CHIPBaseCluster
{
public:
    CHIPClusterBasic(uint8_t ZCLVersion,
                     uint8_t applicationVersion,
                     uint8_t stackVersion,
                     uint8_t HWVersion) :
        CHIPBaseCluster(kClusterIdBase)
    {
        AddAttribute(CHIPAttributeZCLVersionNew(ZCLVersion));
        AddAttribute(CHIPAttributeApplicationVersionNew(applicationVersion));
        AddAttribute(CHIPAttributeStackVersionNew(stackVersion));
        AddAttribute(CHIPAttributeHWVersionNew(HWVersion));
    }
};

static const uint16_t kClusterIdOnOff = 0x0006;
class CHIPClusterOnOff : public CHIPBaseCluster
{
public:
    CHIPClusterOnOff() :
        CHIPBaseCluster(kClusterIdOnOff)
    {
        AddAttribute(CHIPAttributeOnOffNew());
        AddAttribute(CHIPAttributeGlobalSceneControlNew());
        AddAttribute(CHIPAttributeOnTimeNew());
        AddAttribute(CHIPAttributeOffWaitTimeNew());
    }
};


} // namespace DataModel
} // namespace chip

#endif /* CHIPSTANDARDCLUSTERS_H_ */

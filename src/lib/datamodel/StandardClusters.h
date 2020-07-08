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

#include <datamodel/Cluster.h>
#include <datamodel/StandardAttributes.h>

namespace chip {
namespace DataModel {

static const uint16_t kClusterIdBase = 0x0000;
class ClusterBasic : public Cluster
{
public:
    ClusterBasic() : Cluster(kClusterIdBase) {}

    int Init(uint8_t ZCLVersion, uint8_t applicationVersion, uint8_t stackVersion, uint8_t HWVersion)
    {
        if (mAttrs[0] == nullptr)
        {
            AddAttribute(CHIPAttributeZCLVersionNew(ZCLVersion));
            AddAttribute(CHIPAttributeApplicationVersionNew(applicationVersion));
            AddAttribute(CHIPAttributeStackVersionNew(stackVersion));
            AddAttribute(CHIPAttributeHWVersionNew(HWVersion));
        }
        return SUCCESS;
    }

    ClusterBasic(uint8_t ZCLVersion, uint8_t applicationVersion, uint8_t stackVersion, uint8_t HWVersion) :
        Cluster(kClusterIdBase)
    {
        Init(ZCLVersion, applicationVersion, stackVersion, HWVersion);
    }
};

static const uint16_t kClusterIdOnOff = 0x0006;
class ClusterOnOff : public Cluster
{
public:
    ClusterOnOff() : Cluster(kClusterIdOnOff)
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

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
 *      This file contains definitions for CHIP Cluster OnOff
 *
 */

#ifndef CHIPCLUSTERONOFF_H_
#define CHIPCLUSTERONOFF_H_

#include <datamodel/Cluster.h>

namespace chip {
namespace DataModel {

/* Cluster ID */
static const ClusterId_t kClusterIdOnOff = 0x0006;

/* Attribute IDs */
static const AttributeId_t kAttributeIdOnOff              = 0x0000;
static const AttributeId_t kAttributeIdGlobalSceneControl = 0x4000;
static const AttributeId_t kAttributeIdOnTime             = 0x4001;
static const AttributeId_t kAttributeIdOffWaitTime        = 0x4002;

/**
 * @brief
 *   This class implements the OnOff cluster as defined in the CHIP specification.
 */
class ClusterOnOff : public Cluster
{
    // TODO: these should
private:
    AttributeSimple<bool> mOnOff;
    AttributeSimple<bool> mGlobalSceneControl;
    AttributeSimple<uint16_t> mOnTime;
    AttributeSimple<uint16_t> mOffWaitTime;

public:
    ClusterOnOff() :
        Cluster(kClusterIdOnOff), mOnOff(kAttributeIdOnOff), mGlobalSceneControl(kAttributeIdGlobalSceneControl),
        mOnTime(kAttributeIdOnTime), mOffWaitTime(kAttributeIdOffWaitTime)
    {
        AddAttribute(&mOnOff);
        AddAttribute(&mGlobalSceneControl);
        AddAttribute(&mOnTime);
        AddAttribute(&mOffWaitTime);
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTERONOFF_H_ */

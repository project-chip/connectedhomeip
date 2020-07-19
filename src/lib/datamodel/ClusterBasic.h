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
 *      This file contains definitions for CHIP Cluster Basic
 *
 */

#ifndef CHIPCLUSTERBASIC_H_
#define CHIPCLUSTERBASIC_H_

#include <datamodel/Cluster.h>

namespace chip {
namespace DataModel {

/* Cluster ID */
static const ClusterId_t kClusterIdBase = 0x0000;

/* Attribute IDs */
static const AttributeId_t kAttributeIdZCLVersion         = 0x0000;
static const AttributeId_t kAttributeIdApplicationVersion = 0x0001;
static const AttributeId_t kAttributeIdStackVersion       = 0x0002;
static const AttributeId_t kAttributeIdHWVersion          = 0x0003;

/**
 * @brief
 *   This class implements the Base cluster as defined in the CHIP specification.
 */
class ClusterBasic : public Cluster
{
private:
    Attribute mZCLVersion;
    Attribute mApplicationVersion;
    Attribute mStackVersion;
    Attribute mHWVersion;

public:
    ClusterBasic(uint8_t ZCLVersion, uint8_t applicationVersion, uint8_t stackVersion, uint8_t HWVersion) :
        Cluster(kClusterIdBase),
        /* Attributes */
        mZCLVersion(kAttributeIdZCLVersion, ValueUInt8(ZCLVersion)),
        mApplicationVersion(kAttributeIdApplicationVersion, ValueUInt8(applicationVersion)),
        mStackVersion(kAttributeIdStackVersion, ValueUInt8(stackVersion)), mHWVersion(kAttributeIdHWVersion, ValueUInt8(HWVersion))
    {
        AddAttribute(&mZCLVersion);
        AddAttribute(&mApplicationVersion);
        AddAttribute(&mStackVersion);
        AddAttribute(&mHWVersion);
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTERBASIC_H_ */

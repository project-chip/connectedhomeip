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

/* Command IDs */
static const uint16_t kOnOffCmdIdOff                  = 0x00;
static const uint16_t kOnOffCmdIdOn                   = 0x01;
static const uint16_t kOnOffCmdIdToggle               = 0x02;
static const uint16_t kOnOffCmdIdOffWithEffect        = 0x40;
static const uint16_t kOnOffCmdIdOffWithRecall        = 0x41;
static const uint16_t kOnOffCmdIdOnWithTimedOff       = 0x42;

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

    virtual CHIP_ERROR HandleCommandOff(const Command & cmd)
    {
        return Set(kAttributeIdOnOff, ValueBool(false));
    }
    
    virtual CHIP_ERROR HandleCommandOn(const Command & cmd)
    {
        return Set(kAttributeIdOnOff, ValueBool(true));
    }
    
    virtual CHIP_ERROR HandleCommandToggle(const Command & cmd)
    {
        Value currentVal;
        Get(kAttributeIdOnOff, currentVal);
        return Set(kAttributeIdOnOff, ValueBool( ! ValueToBool(currentVal)));
    }

    virtual CHIP_ERROR HandleCommands(const Command & cmd)
    {
        switch (cmd.mId)
        {
        case kOnOffCmdIdOff:
            HandleCommandOff(cmd);
            break;
        case kOnOffCmdIdOn:
            HandleCommandOn(cmd);
            break;
        case kOnOffCmdIdToggle:
            HandleCommandToggle(cmd);
            break;
        default:
            /* Unsupported */
            return CHIP_ERROR_INTERNAL;
            break;
        }
        return CHIP_ERROR_INTERNAL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTERONOFF_H_ */

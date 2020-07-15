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
static const uint16_t kOnOffCmdIdOff            = 0x00;
static const uint16_t kOnOffCmdIdOn             = 0x01;
static const uint16_t kOnOffCmdIdToggle         = 0x02;
static const uint16_t kOnOffCmdIdOffWithEffect  = 0x40;
static const uint16_t kOnOffCmdIdOffWithRecall  = 0x41;
static const uint16_t kOnOffCmdIdOnWithTimedOff = 0x42;

/**
 * @brief
 *   This class implements the OnOff cluster as defined in the CHIP specification.
 */
class ClusterOnOff : public Cluster
{
    // TODO: these should
private:
    Attribute mOnOff;
    Attribute mGlobalSceneControl;
    Attribute mOnTime;
    Attribute mOffWaitTime;

public:
    ClusterOnOff() :
        Cluster(kClusterIdOnOff), mOnOff(kAttributeIdOnOff, kCHIPValueType_Bool),
        mGlobalSceneControl(kAttributeIdGlobalSceneControl, kCHIPValueType_Bool),
        mOnTime(kAttributeIdOnTime, kCHIPValueType_UInt16), mOffWaitTime(kAttributeIdOffWaitTime, kCHIPValueType_UInt16)
    {
        AddAttribute(&mOnOff);
        AddAttribute(&mGlobalSceneControl);
        AddAttribute(&mOnTime);
        AddAttribute(&mOffWaitTime);
    }

    /**
     * @brief
     *   Handle the off command. This command is already handled in the ClusterOnOff class, and the
     *   Cluster's Set() method will be called with the appropriate value. Applications may choose
     *   to override this handling if required.
     *
     * @param cmd the command to handle
     *
     * @return CHIP_NO_ERROR on success or a failure-specific error code otherwise
     */
    virtual CHIP_ERROR HandleCommandOff(const Command & cmd) { return Set(kAttributeIdOnOff, ValueBool(false)); }

    /**
     * @brief
     *   Handle the on command. This command is already handled in the ClusterOnOff class, and the
     *   Cluster's Set() method will be called with the appropriate value. Applications may choose
     *   to override this handling if required.
     *
     * @param cmd the command to handle
     *
     * @return CHIP_NO_ERROR on success or a failure-specific error code otherwise
     */
    virtual CHIP_ERROR HandleCommandOn(const Command & cmd) { return Set(kAttributeIdOnOff, ValueBool(true)); }

    /**
     * @brief
     *   Handle the toggle command. This command is already handled in the ClusterOnOff class, and
     *   the Cluster's Set() method will be called with the appropriate value. Applications may
     *   choose to override this handling if required.
     *
     * @param cmd the command to handle
     *
     * @return CHIP_NO_ERROR on success or a failure-specific error code otherwise
     */
    virtual CHIP_ERROR HandleCommandToggle(const Command & cmd)
    {
        Value currentVal;
        Get(kAttributeIdOnOff, currentVal);
        return Set(kAttributeIdOnOff, ValueBool(!ValueToBool(currentVal)));
    }

    /**
     * @brief
     *   Handle commands for the Cluster OnOff. This is already handled in the ClusterOnOff
     *   class. Applications may choose to override this handling if required.
     *
     * @param cmd the command to handle
     *
     * @return CHIP_NO_ERROR on success or a failure-specific error code otherwise
     */
    virtual CHIP_ERROR HandleCommand(const Command & cmd)
    {
        switch (cmd.mId)
        {
        case kOnOffCmdIdOff:
            return HandleCommandOff(cmd);
        case kOnOffCmdIdOn:
            return HandleCommandOn(cmd);
        case kOnOffCmdIdToggle:
            return HandleCommandToggle(cmd);
        default:
            /* Unsupported */
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_ERROR_INTERNAL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPCLUSTERONOFF_H_ */

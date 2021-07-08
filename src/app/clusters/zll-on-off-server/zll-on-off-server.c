/**
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
 *
 *    Copyright (c) 2020 Silicon Labs
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
 * @file
 * @brief Routines for the ZLL On Off Server plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc.
 * www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 * Laboratories Inc. Your use of this software is
 * governed by the terms of Silicon Labs Master
 * Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software is distributed to you in Source Code
 * format and is governed by the sections of the MSLA
 * applicable to Source Code.
 *
 ******************************************************************************/

#include "zll-on-off-server.h"
#include "../../include/af.h"
#include <app/CommandHandler.h>

#define ZLL_ON_OFF_CLUSTER_ON_OFF_CONTROL_ACCEPT_ONLY_WHEN_ON_MASK EMBER_BIT(0)

#define readOnOff(endpoint, onOff) readBoolean((endpoint), ZCL_ON_OFF_ATTRIBUTE_ID, "on/off", (onOff))
#define writeOnOff(endpoint, onOff) writeBoolean((endpoint), ZCL_ON_OFF_ATTRIBUTE_ID, "on/off", (onOff))
#define readGlobalSceneControl(endpoint, globalSceneControl)                                                                       \
    readBoolean((endpoint), ZCL_GLOBAL_SCENE_CONTROL_ATTRIBUTE_ID, "global scene control", (globalSceneControl))
#define writeGlobalSceneControl(endpoint, globalSceneControl)                                                                      \
    writeBoolean((endpoint), ZCL_GLOBAL_SCENE_CONTROL_ATTRIBUTE_ID, "global scene control", (globalSceneControl))
#define readOnTime(endpoint, onTime) readInt16u((endpoint), ZCL_ON_TIME_ATTRIBUTE_ID, "on time", (onTime))
#define writeOnTime(endpoint, onTime) writeInt16u((endpoint), ZCL_ON_TIME_ATTRIBUTE_ID, "on time", (onTime))
#define readOffWaitTime(endpoint, offWaitTime)                                                                                     \
    readInt16u((endpoint), ZCL_OFF_WAIT_TIME_ATTRIBUTE_ID, "off wait time", (offWaitTime))
#define writeOffWaitTime(endpoint, offWaitTime)                                                                                    \
    writeInt16u((endpoint), ZCL_OFF_WAIT_TIME_ATTRIBUTE_ID, "off wait time", (offWaitTime))

static EmberAfStatus readBoolean(uint8_t endpoint, EmberAfAttributeId attribute, const char * name, bool * value)
{
    EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, attribute, (uint8_t *) value, sizeof(bool));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfOnOffClusterPrintln("ERR: %ping %p %x", "read", name, status);
    }
    return status;
}

static EmberAfStatus writeBoolean(uint8_t endpoint, EmberAfAttributeId attribute, const char * name, bool value)
{
    EmberAfStatus status =
        emberAfWriteServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, attribute, (uint8_t *) &value, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfOnOffClusterPrintln("ERR: %ping %p %x", "writ", name, status);
    }
    return status;
}

static EmberAfStatus readInt16u(uint8_t endpoint, EmberAfAttributeId attribute, const char * name, uint16_t * value)
{
    EmberAfStatus status =
        emberAfReadServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, attribute, (uint8_t *) value, sizeof(uint16_t));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfOnOffClusterPrintln("ERR: %ping %p %x", "read", name, status);
    }
    return status;
}

static EmberAfStatus writeInt16u(uint8_t endpoint, EmberAfAttributeId attribute, const char * name, uint16_t value)
{
    EmberAfStatus status =
        emberAfWriteServerAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, attribute, (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfOnOffClusterPrintln("ERR: %ping %p %x", "writ", name, status);
    }
    return status;
}

void emberAfOnOffClusterServerTickCallback(uint8_t endpoint)
{
    uint16_t onTime, offWaitTime;
    bool onOff;

    if (readOnOff(endpoint, &onOff) != EMBER_ZCL_STATUS_SUCCESS || readOnTime(endpoint, &onTime) != EMBER_ZCL_STATUS_SUCCESS ||
        readOffWaitTime(endpoint, &offWaitTime) != EMBER_ZCL_STATUS_SUCCESS)
    {
        return;
    }

    // If the values of the OnTime and OffWaitTime attributes are both less than
    // 0xFFFF, the device shall then update the device every 1/10th second until
    // both the OnTime and OffWaitTime attributes are equal to 0x0000, as
    // follows:
    //
    //   If the value of the OnOff attribute is equal to 0x01 (on) and the value
    //   of the OnTime attribute is greater than zero, the device shall decrement
    //   the value of the OnTime attribute. If the value of the OnTime attribute
    //   reaches 0x0000, the device shall set the OffWaitTime and OnOff
    //   attributes to 0x0000 and 0x00, respectively.
    //
    //   If the value of the OnOff attribute is equal to 0x00 (off) and the value
    //   of the OffWaitTime attribute is greater than zero, the device shall
    //   decrement the value of the OffWaitTime attribute. If the value of the
    //   OffWaitTime attribute reaches 0x0000, the device shall terminate the
    //   update.
    if (onOff && 0x0000 < onTime)
    {
        onTime--;
        writeOnTime(endpoint, onTime);
        if (onTime == 0x0000)
        {
            offWaitTime = 0x0000;
            writeOffWaitTime(endpoint, offWaitTime);
            onOff = false;
            writeOnOff(endpoint, onOff);
            return;
        }
    }
    else if (!onOff && 0x0000 < offWaitTime)
    {
        offWaitTime--;
        writeOffWaitTime(endpoint, offWaitTime);
        if (offWaitTime == 0x0000)
        {
            return;
        }
    }

    emberAfScheduleServerTick(endpoint, ZCL_ON_OFF_CLUSTER_ID, MILLISECOND_TICKS_PER_SECOND / 10);
}

bool emberAfOnOffClusterOffWithEffectCallback(chip::app::CommandHandler * commandObj, uint8_t effectId, uint8_t effectVariant)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_INVALID_VALUE;
    bool globalSceneControl;
    uint8_t endpoint = emberAfCurrentEndpoint();

    // Ensure parameters have values withing proper range.
    if (effectId > EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DYING_LIGHT ||
        effectVariant >
            EMBER_ZCL_ON_OFF_DELAYED_ALL_OFF_EFFECT_VARIANT_50_PERCENT_DIM_DOWN_IN_0P8_SECONDS_THEN_FADE_TO_OFF_IN_12_SECONDS ||
        (effectId == EMBER_ZCL_ON_OFF_EFFECT_IDENTIFIER_DYING_LIGHT &&
         effectVariant >
             EMBER_ZCL_ON_OFF_DYING_LIGHT_EFFECT_VARIANT_20_PERCENTER_DIM_UP_IN_0P5_SECONDS_THEN_FADE_TO_OFF_IN_1_SECOND))
    {
        goto kickout;
    }

    // If the GlobalSceneControl attribute is equal to true, the application on
    // the associated endpoint shall store its settings in its global scene then
    // set the GlobalSceneControl attribute to false.
    status = readGlobalSceneControl(endpoint, &globalSceneControl);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        goto kickout;
    }
    else if (globalSceneControl)
    {
        status = emberAfScenesClusterStoreCurrentSceneCallback(endpoint, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID,
                                                               ZCL_SCENES_GLOBAL_SCENE_SCENE_ID);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: %ping %p %x", "stor", "global scene", status);
            goto kickout;
        }
        globalSceneControl = false;
        status             = writeGlobalSceneControl(endpoint, globalSceneControl);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            goto kickout;
        }
    }

    // The application will handle the actual effect and variant.
    status = emberAfPluginZllOnOffServerOffWithEffectCallback(endpoint, effectId, effectVariant);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // If the application handled the effect, the endpoint shall enter its
        // "off" state, update the OnOff attribute accordingly, and set the OnTime
        // attribute to 0x0000.
        status = emberAfOnOffClusterSetValueCallback(endpoint, ZCL_OFF_COMMAND_ID, false);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            status = writeOnTime(endpoint, 0x0000);
        }
    }

kickout:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOnOffClusterOnWithRecallGlobalSceneCallback(chip::app::CommandHandler * commandObj)
{
    EmberAfStatus status;
    bool globalSceneControl;
    uint8_t endpoint = emberAfCurrentEndpoint();

    status = readGlobalSceneControl(endpoint, &globalSceneControl);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        goto kickout;
    }
    else if (!globalSceneControl)
    {
        uint16_t onTime;
        status = emberAfScenesClusterRecallSavedSceneCallback(endpoint, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID,
                                                              ZCL_SCENES_GLOBAL_SCENE_SCENE_ID);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: %ping %p %x", "recall", "global scene", status);
            goto kickout;
        }
        globalSceneControl = true;
        status             = writeGlobalSceneControl(endpoint, globalSceneControl);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            goto kickout;
        }
        status = readOnTime(endpoint, &onTime);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            goto kickout;
        }
        if (onTime == 0x0000)
        {
            status = writeOffWaitTime(endpoint, 0x0000);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                goto kickout;
            }
        }
    }
    else
    {
        goto kickout;
    }

kickout:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOnOffClusterOnWithTimedOffCallback(chip::app::CommandHandler * commandObj, uint8_t onOffControl, uint16_t onTime,
                                               uint16_t offWaitTime)
{
    EmberAfStatus status;
    uint16_t onTimeAttribute, offWaitTimeAttribute;
    bool onOffAttribute;
    uint8_t endpoint = emberAfCurrentEndpoint();

    // The valid range of the OnTime and OffWaitTime fields is 0x0000 to 0xFFFF.
    if (onTime == 0xFFFF || offWaitTime == 0xFFFF)
    {
        status = EMBER_ZCL_STATUS_INVALID_FIELD;
        goto kickout;
    }

    // On receipt of this command, if the accept only when on sub-field of the
    // on/off control field is set to 1 and the value of the OnOff attribute is
    // equal to 0x00 (off), the command shall be discarded.
    status = readOnOff(endpoint, &onOffAttribute);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        goto kickout;
    }
    else if ((onOffControl & ZLL_ON_OFF_CLUSTER_ON_OFF_CONTROL_ACCEPT_ONLY_WHEN_ON_MASK) && !onOffAttribute)
    {
        goto kickout;
    }

    status = readOnTime(endpoint, &onTimeAttribute);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        goto kickout;
    }
    status = readOffWaitTime(endpoint, &offWaitTimeAttribute);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        goto kickout;
    }

    // If the value of the OffWaitTime attribute is greater than zero and the
    // value of the OnOff attribute is equal to 0x00, then the device shall set
    // the OffWaitTime attribute to the minimum of the OffWaitTime attribute and
    // the value specified in the off wait time field.  In all other cases, the
    // device shall set the OnTime attribute to the maximum of the OnTime
    // attribute and the value specified in the on time field, set the
    // OffWaitTime attribute to the value specified in the off wait time field
    // and set the OnOff attribute to 0x01 (on).
    if (0x0000 < offWaitTimeAttribute && !onOffAttribute)
    {
        if (offWaitTime < offWaitTimeAttribute)
        {
            offWaitTimeAttribute = offWaitTime;
        }
        status = writeOffWaitTime(endpoint, offWaitTimeAttribute);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            goto kickout;
        }
    }
    else
    {
        if (onTimeAttribute < onTime)
        {
            onTimeAttribute = onTime;
        }
        status = writeOnTime(endpoint, onTimeAttribute);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            goto kickout;
        }
        offWaitTimeAttribute = offWaitTime;
        status               = writeOffWaitTime(endpoint, offWaitTimeAttribute);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            goto kickout;
        }
        onOffAttribute = true;
        status         = writeOnOff(endpoint, onOffAttribute);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            goto kickout;
        }
    }

    // If the values of the OnTime and OffWaitTime attributes are both less than
    // 0xFFFF, the device shall then update the device every 1/10th second until
    // both the OnTime and OffWaitTime attributes are equal to 0x0000.
    if (onTimeAttribute < 0xFFFF && offWaitTimeAttribute < 0xFFFF)
    {
        emberAfScheduleServerTick(endpoint, ZCL_ON_OFF_CLUSTER_ID, MILLISECOND_TICKS_PER_SECOND / 10);
    }
    else
    {
        emberAfDeactivateServerTick(endpoint, ZCL_ON_OFF_CLUSTER_ID);
    }

kickout:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

EmberAfStatus emberAfPluginZllOnOffServerOffZllExtensions(const EmberAfClusterCommand * cmd)
{
    // On receipt of the off command, the OnTime attribute shall be set to
    // 0x0000.
    return writeOnTime(cmd->apsFrame->destinationEndpoint, 0x0000);
}

EmberAfStatus emberAfPluginZllOnOffServerOnZllExtensions(const EmberAfClusterCommand * cmd)
{
    // On receipt of the on command, if the value of the OnTime attribute is
    // equal to 0x0000, the device shall set the OffWaitTime attribute to 0x0000.
    uint16_t onTime;
    uint8_t endpoint     = cmd->apsFrame->destinationEndpoint;
    EmberAfStatus status = readOnTime(endpoint, &onTime);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }
    else if (onTime == 0x0000)
    {
        status = writeOffWaitTime(endpoint, 0x0000);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return status;
        }
    }

    // The GlobalSceneControl attribute shall be set to true after the reception
    // of a standard ZCL on command.
    return writeGlobalSceneControl(endpoint, true);
}

EmberAfStatus emberAfPluginZllOnOffServerToggleZllExtensions(const EmberAfClusterCommand * cmd)
{
    // On receipt of the toggle command, if the value of the OnOff attribute is
    // equal to 0x00 and if the value of the OnTime attribute is equal to 0x0000,
    // the device shall set the OffWaitTime attribute to 0x0000.  If the value of
    // the OnOff attribute is equal to 0x01, the OnTime attribute shall be set to
    // 0x0000.  When this function is called, the OnOff attribute has already
    // been toggled, so the logic is reversed.
    //
    // The GlobalSceneControl attribute shall be set to true after the
    // receiption of a command which causes the OnOff attribute to be set to
    // true, e.g. ZCL toggle command
    bool onOff;
    uint8_t endpoint     = cmd->apsFrame->destinationEndpoint;
    EmberAfStatus status = readOnOff(endpoint, &onOff);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }
    else if (onOff)
    {
        uint16_t onTime;
        status = readOnTime(endpoint, &onTime);
        if (status == EMBER_ZCL_STATUS_SUCCESS && onTime == 0x0000)
        {
            status = writeOffWaitTime(endpoint, 0x0000);
        }

        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            status = writeGlobalSceneControl(endpoint, true);
        }

        return status;
    }
    else
    {
        return writeOnTime(endpoint, 0x0000);
    }
}

EmberAfStatus emberAfPluginZllOnOffServerLevelControlZllExtensions(uint8_t endpoint)
{
    // On receipt of a level control cluster command that causes the OnOff
    // attribute to be set to 0x00, the OnTime attribute shall be set to 0x0000.
    // On receipt of a level control cluster command that causes the OnOff
    // attribute to be set to 0x01, if the value of the OnTime attribute is equal
    // to 0x0000, the device shall set the OffWaitTime attribute to 0x0000.
    bool onOff;
    EmberAfStatus status = readOnOff(endpoint, &onOff);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return status;
    }
    else if (!onOff)
    {
        return writeOnTime(endpoint, 0x0000);
    }
    else
    {
        uint16_t onTime;
        status = readOnTime(endpoint, &onTime);
        if (status == EMBER_ZCL_STATUS_SUCCESS && onTime == 0x0000)
        {
            status = writeOffWaitTime(endpoint, 0x0000);
        }
        return status;
    }
}

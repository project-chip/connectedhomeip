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
/***************************************************************************/
/**
 * @file
 * @brief Routines for the On-Off plugin, which
 *implements the On-Off server cluster.
 *******************************************************************************
 ******************************************************************************/
#include "on-off-server.h"

#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>

#include <app/CommandHandler.h>
#include <app/reporting/reporting.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
#include "../zll-on-off-server/zll-on-off-server.h"
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
#include "../zll-level-control-server/zll-level-control-server.h"
#endif

using namespace chip;

#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
static bool areStartUpOnOffServerAttributesTokenized(EndpointId endpoint);
#endif

EmberAfStatus emberAfOnOffClusterSetValueCallback(EndpointId endpoint, uint8_t command, bool initiatedByLevelChange)
{
    EmberAfStatus status;
    bool currentValue, newValue;

    emberAfOnOffClusterPrintln("On/Off set value: %x %x", endpoint, command);

    // read current on/off value
    status = emberAfReadAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                  (uint8_t *) &currentValue, sizeof(currentValue),
                                  NULL); // data type
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfOnOffClusterPrintln("ERR: reading on/off %x", status);
        return status;
    }

    // if the value is already what we want to set it to then do nothing
    if ((!currentValue && command == ZCL_OFF_COMMAND_ID) || (currentValue && command == ZCL_ON_COMMAND_ID))
    {
        emberAfOnOffClusterPrintln("On/off already set to new value");
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    // we either got a toggle, or an on when off, or an off when on,
    // so we need to swap the value
    newValue = !currentValue;
    emberAfOnOffClusterPrintln("Toggle on/off from %x to %x", currentValue, newValue);

    // the sequence of updating on/off attribute and kick off level change effect should
    // be depend on whether we are turning on or off. If we are turning on the light, we
    // should update the on/off attribute before kicking off level change, if we are
    // turning off the light, we should do the opposite, that is kick off level change
    // before updating the on/off attribute.
    if (newValue)
    {
        // write the new on/off value
        status = emberAfWriteAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                       (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
            return status;
        }

#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
        // If initiatedByLevelChange is false, then we assume that the level change
        // ZCL stuff has not happened and we do it here
        if (!initiatedByLevelChange && emberAfContainsServer(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID))
        {
            emberAfOnOffClusterLevelControlEffectCallback(endpoint, newValue);
        }
#endif // EMBER_AF_PLUGIN_LEVEL_CONTROL
    }
    else
    {
#ifdef EMBER_AF_PLUGIN_LEVEL_CONTROL
        // If initiatedByLevelChange is false, then we assume that the level change
        // ZCL stuff has not happened and we do it here
        if (!initiatedByLevelChange && emberAfContainsServer(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID))
        {
            emberAfOnOffClusterLevelControlEffectCallback(endpoint, newValue);
        }
#endif // EMBER_AF_PLUGIN_LEVEL_CONTROL

        // write the new on/off value
        status = emberAfWriteAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                       (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
            return status;
        }
    }

#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (initiatedByLevelChange)
    {
        emberAfPluginZllOnOffServerLevelControlZllExtensions(endpoint);
    }
#endif

#ifdef EMBER_AF_PLUGIN_SCENES
    // the scene has been changed (the value of on/off has changed) so
    // the current scene as descibed in the attribute table is invalid,
    // so mark it as invalid (just writes the valid/invalid attribute)
    if (emberAfContainsServer(endpoint, ZCL_SCENES_CLUSTER_ID))
    {
        emberAfScenesClusterMakeInvalidCallback(endpoint);
    }
#endif // EMBER_AF_PLUGIN_SCENES

    // The returned status is based solely on the On/Off cluster.  Errors in the
    // Level Control and/or Scenes cluster are ignored.
    return EMBER_ZCL_STATUS_SUCCESS;
}

bool emberAfOnOffClusterOffCallback(chip::app::CommandHandler * commandObj)
{
    EmberAfStatus status = emberAfOnOffClusterSetValueCallback(emberAfCurrentEndpoint(), ZCL_OFF_COMMAND_ID, false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllOnOffServerOffZllExtensions(emberAfCurrentCommand());
    }
#endif
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOnOffClusterOnCallback(chip::app::CommandHandler * commandObj)
{
    EmberAfStatus status = emberAfOnOffClusterSetValueCallback(emberAfCurrentEndpoint(), ZCL_ON_COMMAND_ID, false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllOnOffServerOnZllExtensions(emberAfCurrentCommand());
    }
#endif
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOnOffClusterToggleCallback(chip::app::CommandHandler * commandObj)
{
    EmberAfStatus status = emberAfOnOffClusterSetValueCallback(emberAfCurrentEndpoint(), ZCL_TOGGLE_COMMAND_ID, false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPluginZllOnOffServerToggleZllExtensions(emberAfCurrentCommand());
    }
#endif
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void emberAfOnOffClusterServerInitCallback(EndpointId endpoint)
{
#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
    // StartUp behavior relies on OnOff and StartUpOnOff attributes being tokenized.
    if (areStartUpOnOffServerAttributesTokenized(endpoint))
    {
        // Read the StartUpOnOff attribute and set the OnOff attribute as per
        // following from zcl 7 14-0127-20i-zcl-ch-3-general.doc.
        // 3.8.2.2.5	StartUpOnOff Attribute
        // The StartUpOnOff attribute SHALL define the desired startup behavior of a
        // lamp device when it is supplied with power and this state SHALL be
        // reflected in the OnOff attribute.  The values of the StartUpOnOff
        // attribute are listed below.
        // Table 3 46. Values of the StartUpOnOff Attribute
        // Value      Action on power up
        // 0x00       Set the OnOff attribute to 0 (off).
        // 0x01       Set the OnOff attribute to 1 (on).
        // 0x02       If the previous value of the OnOff attribute is equal to 0,
        //            set the OnOff attribute to 1.If the previous value of the OnOff
        //            attribute is equal to 1, set the OnOff attribute to 0 (toggle).
        // 0x03-0xfe  These values are reserved.  No action.
        // 0xff       Set the OnOff attribute to its previous value.

        // Initialize startUpOnOff to No action value 0xFE
        uint8_t startUpOnOff = 0xFE;
        EmberAfStatus status = emberAfReadAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_START_UP_ON_OFF_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER, (uint8_t *) &startUpOnOff, sizeof(startUpOnOff), NULL);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            // Initialise updated value to 0
            bool updatedOnOff = 0;
            status            = emberAfReadAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                          (uint8_t *) &updatedOnOff, sizeof(updatedOnOff), NULL);
            if (status == EMBER_ZCL_STATUS_SUCCESS)
            {
                switch (startUpOnOff)
                {
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_OFF:
                    updatedOnOff = 0; // Off
                    break;
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_ON:
                    updatedOnOff = 1; // On
                    break;
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_TOGGLE:
                    updatedOnOff = !updatedOnOff;
                    break;
                case EMBER_ZCL_START_UP_ON_OFF_VALUE_SET_TO_PREVIOUS:
                default:
                    // All other values 0x03- 0xFE are reserved - no action.
                    // When value is 0xFF - update with last value - that is as good as
                    // no action.
                    break;
                }
                status = emberAfWriteAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                               (uint8_t *) &updatedOnOff, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
            }
        }
    }
#endif
    emberAfPluginOnOffClusterServerPostInitCallback(endpoint);
}

#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
static bool areStartUpOnOffServerAttributesTokenized(EndpointId endpoint)
{
    EmberAfAttributeMetadata * metadata;

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                              EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    metadata = emberAfLocateAttributeMetadata(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_START_UP_ON_OFF_ATTRIBUTE_ID,
                                              CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
    if (!emberAfAttributeIsTokenized(metadata))
    {
        return false;
    }

    return true;
}
#endif

void emberAfPluginOnOffClusterServerPostInitCallback(EndpointId endpoint) {}

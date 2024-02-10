/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/support/logging/CHIPLogging.h>

#include "AppTask.h"
#include "LightingManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af-types.h>
#include <assert.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    EndpointId endpoint     = attributePath.mEndpointId;
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION, 0, size, value);
    }
    else if (clusterId == LevelControl::Id && attributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        if (size == 1)
        {
            ChipLogProgress(Zcl, "New level: %u", *value);
            LightingMgr().InitiateAction(LightingManager::LEVEL_ACTION, 0, size, value);
        }
        else
        {
            ChipLogError(Zcl, "wrong length for level: %d", size);
        }
    }
    else if (clusterId == ColorControl::Id)
    {
        /* ignore several attributes that are currently not processed */
        if ((attributeId == ColorControl::Attributes::RemainingTime::Id) ||
            (attributeId == ColorControl::Attributes::EnhancedColorMode::Id) ||
            (attributeId == ColorControl::Attributes::ColorMode::Id))
        {
            return;
        }

        /* XY color space */
        if (attributeId == ColorControl::Attributes::CurrentX::Id || attributeId == ColorControl::Attributes::CurrentY::Id)
        {
            if (size != sizeof(uint16_t))
            {
                ChipLogError(Zcl, "Wrong length for ColorControl value: %d", size);
                return;
            }
            XyColor_t xy;
            if (attributeId == ColorControl::Attributes::CurrentX::Id)
            {
                xy.x = *reinterpret_cast<uint16_t *>(value);
                // get Y from cluster value storage
                Protocols::InteractionModel::Status status = ColorControl::Attributes::CurrentY::Get(endpoint, &xy.y);
                assert(status == Protocols::InteractionModel::Status::Success);
            }
            if (attributeId == ColorControl::Attributes::CurrentY::Id)
            {
                xy.y = *reinterpret_cast<uint16_t *>(value);
                // get X from cluster value storage
                Protocols::InteractionModel::Status status = ColorControl::Attributes::CurrentX::Get(endpoint, &xy.x);
                assert(status == Protocols::InteractionModel::Status::Success);
            }

            ChipLogProgress(Zcl, "New XY color: %u|%u", xy.x, xy.y);
            LightingMgr().InitiateAction(LightingManager::COLOR_ACTION_XY, 0, sizeof(xy), (uint8_t *) &xy);
        }
        /* HSV color space */
        else if (attributeId == ColorControl::Attributes::CurrentHue::Id ||
                 attributeId == ColorControl::Attributes::CurrentSaturation::Id ||
                 attributeId == ColorControl::Attributes::EnhancedCurrentHue::Id)
        {
            if (size != sizeof(uint8_t))
            {
                ChipLogError(Zcl, "Wrong length for ColorControl value: %d", size);
                return;
            }
            HsvColor_t hsv;
            if (attributeId == ColorControl::Attributes::EnhancedCurrentHue::Id)
            {
                // We only support 8-bit hue. Assuming hue is linear, normalize 16-bit to 8-bit.
                hsv.h = (uint8_t) ((*reinterpret_cast<uint16_t *>(value)) >> 8);
                // get saturation from cluster value storage
                Protocols::InteractionModel::Status status = ColorControl::Attributes::CurrentSaturation::Get(endpoint, &hsv.s);
                assert(status == Protocols::InteractionModel::Status::Success);
            }
            else if (attributeId == ColorControl::Attributes::CurrentHue::Id)
            {
                hsv.h = *value;
                // get saturation from cluster value storage
                Protocols::InteractionModel::Status status = ColorControl::Attributes::CurrentSaturation::Get(endpoint, &hsv.s);
                assert(status == Protocols::InteractionModel::Status::Success);
            }
            else if (attributeId == ColorControl::Attributes::CurrentSaturation::Id)
            {
                hsv.s = *value;
                // get hue from cluster value storage
                Protocols::InteractionModel::Status status = ColorControl::Attributes::CurrentHue::Get(endpoint, &hsv.h);
                assert(status == Protocols::InteractionModel::Status::Success);
            }
            ChipLogProgress(Zcl, "New HSV color: %u|%u", hsv.h, hsv.s);
            LightingMgr().InitiateAction(LightingManager::COLOR_ACTION_HSV, 0, sizeof(hsv), (uint8_t *) &hsv);
        }
        else if (attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id)
        {
            CtColor_t ct;
            ct.ctMireds = *reinterpret_cast<uint16_t *>(value);
            ChipLogProgress(Zcl, "New CT color: %u", ct.ctMireds);
            LightingMgr().InitiateAction(LightingManager::COLOR_ACTION_CT, 0, sizeof(ct), (uint8_t *) &ct.ctMireds);
        }
        else
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
            return;
        }
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    uint8_t levelValue;
    XyColor_t xy;
    bool onOffValue = false;
    app::DataModel::Nullable<uint8_t> currentLevel;
    Protocols::InteractionModel::Status status;

    status = OnOff::Attributes::OnOff::Get(1, &onOffValue);

    if (status == Protocols::InteractionModel::Status::Success)
    {
        LightingMgr().InitiateAction(onOffValue ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION, 0, 1,
                                     (uint8_t *) onOffValue);
    }

    /* restore values saved by DeferredAttributePersistenceProvider */
    status = LevelControl::Attributes::CurrentLevel::Get(endpoint, currentLevel);
    if (status != Protocols::InteractionModel::Status::Success || currentLevel.IsNull())
    {
        return;
    }

    levelValue = currentLevel.Value();

    status = ColorControl::Attributes::CurrentY::Get(endpoint, &xy.y);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        return;
    }
    status = ColorControl::Attributes::CurrentX::Get(endpoint, &xy.x);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        return;
    }
    ChipLogProgress(Zcl, "restore level: %u", levelValue);
    LightingMgr().InitiateAction(LightingManager::LEVEL_ACTION, 0, 1, &levelValue);
    ChipLogProgress(Zcl, "restore XY color: %u|%u", xy.x, xy.y);
    LightingMgr().InitiateAction(LightingManager::COLOR_ACTION_XY, 0, sizeof(xy), (uint8_t *) &xy);
}

/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ColorControlManager.h"
#include "DoorLockManager.h"
#include "OnOffManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace ::chip::app::Clusters;

static void OnOffClusterAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint16_t size, uint8_t * value)
{
    if (attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        bool onoff = static_cast<bool>(*value);

        ChipLogProgress(Zcl, "Received on/off command endpoint %d value = %d", static_cast<int>(attributePath.mEndpointId), onoff);

        OnOffManager().PostOnOffChanged(attributePath.mEndpointId, onoff);
    }
}

static void ColorControlClusterAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint16_t size,
                                                       uint8_t * value)
{
    if (attributePath.mAttributeId == ColorControl::Attributes::CurrentHue::Id)
    {
        uint8_t currentHue = static_cast<uint8_t>(*value);

        ChipLogProgress(Zcl, "Received CurrentHue command endpoint %d value = %d", static_cast<int>(attributePath.mEndpointId),
                        currentHue);

        ColorControlManager().PostCurrentHueChanged(attributePath.mEndpointId, currentHue);
    }
    else if (attributePath.mAttributeId == ColorControl::Attributes::CurrentSaturation::Id)
    {
        uint8_t currentSaturation = static_cast<uint8_t>(*value);

        ChipLogProgress(Zcl, "Received CurrentSaturation command endpoint %d value = %d",
                        static_cast<int>(attributePath.mEndpointId), currentSaturation);

        ColorControlManager().PostCurrentSaturationChanged(attributePath.mEndpointId, currentSaturation);
    }
    else if (attributePath.mAttributeId == ColorControl::Attributes::ColorTemperatureMireds::Id)
    {
        int16_t colorTemperatureMireds = static_cast<int16_t>(*value);

        ChipLogProgress(Zcl, "Received ColorTemperatureMireds command endpoint %d value = %d",
                        static_cast<int>(attributePath.mEndpointId), colorTemperatureMireds);

        ColorControlManager().PostColorTemperatureChanged(attributePath.mEndpointId, colorTemperatureMireds);
    }
    else if (attributePath.mAttributeId == ColorControl::Attributes::ColorMode::Id)
    {
        uint8_t colorMode = static_cast<uint8_t>(*value);

        ChipLogProgress(Zcl, "Received ColorMode command endpoint %d value = %d", static_cast<int>(attributePath.mEndpointId),
                        colorMode);

        ColorControlManager().PostColorModeChanged(attributePath.mEndpointId, colorMode);
    }
    else if (attributePath.mAttributeId == ColorControl::Attributes::EnhancedColorMode::Id)
    {
        uint8_t enhancedColorMode = static_cast<uint8_t>(*value);

        ChipLogProgress(Zcl, "Received EnhancedColorMode command endpoint %d value = %d",
                        static_cast<int>(attributePath.mEndpointId), enhancedColorMode);

        ColorControlManager().PostEnhancedColorModeChanged(attributePath.mEndpointId, enhancedColorMode);
    }
}

static void DoorLockClusterAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint16_t size, uint8_t * value)
{
    if (attributePath.mAttributeId == DoorLock::Attributes::LockState::Id)
    {
        uint8_t lockState = *value;

        ChipLogProgress(Zcl, "Received lock state command endpoint %d value = %d", static_cast<int>(attributePath.mEndpointId),
                        lockState);

        DoorLockManager().PostLockStateChanged(attributePath.mEndpointId, lockState);
    }
}

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ChipLogProgress(Zcl, "[Device] clusterId:%d,attributeId:%d,endpoint:%d", attributePath.mClusterId, attributePath.mAttributeId,
                    static_cast<int>(attributePath.mEndpointId));

    switch (attributePath.mClusterId)
    {
    case OnOff::Id:
        OnOffClusterAttributeChangeCallback(attributePath, size, value);
        break;
    case ColorControl::Id:
        ColorControlClusterAttributeChangeCallback(attributePath, size, value);
        break;
    case DoorLock::Id:
        DoorLockClusterAttributeChangeCallback(attributePath, size, value);
        break;

    default:
        break;
    }
}

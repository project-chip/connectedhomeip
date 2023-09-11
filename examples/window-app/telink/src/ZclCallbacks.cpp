/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"
#include "PWMDevice.h"
#include "WindowCovering.h"
#include <AppConfig.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/window-covering-server/window-covering-delegate.h>
#include <lib/support/logging/CHIPLogging.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app::Clusters::WindowCovering;

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    switch (attributePath.mClusterId)
    {
    case app::Clusters::Identify::Id:
        ChipLogProgress(Zcl, "Identify cluster ID: " ChipLogFormatMEI " Type: %u Value: %u, length: %u",
                        ChipLogValueMEI(attributePath.mAttributeId), type, *value, size);
        break;
    case app::Clusters::WindowCovering::Id:
        ChipLogProgress(Zcl, "Window covering cluster ID: " ChipLogFormatMEI " Type: %u Value: %u, length: %u",
                        ChipLogValueMEI(attributePath.mAttributeId), type, *value, size);
        break;
    default:
        break;
    }
}

/* Forwards all attributes changes */
void MatterWindowCoveringClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    if (attributePath.mEndpointId == WindowCovering::Endpoint())
    {
        switch (attributePath.mAttributeId)
        {
        case Attributes::TargetPositionLiftPercent100ths::Id:
            WindowCovering::Instance().StartMove(WindowCoveringType::Lift);
            break;
        case Attributes::TargetPositionTiltPercent100ths::Id:
            WindowCovering::Instance().StartMove(WindowCoveringType::Tilt);
            break;
        case Attributes::CurrentPositionLiftPercent100ths::Id:
            WindowCovering::Instance().PositionLEDUpdate(WindowCoveringType::Lift);
            break;
        case Attributes::CurrentPositionTiltPercent100ths::Id:
            WindowCovering::Instance().PositionLEDUpdate(WindowCoveringType::Tilt);
            break;
        default:
            WindowCovering::Instance().SchedulePostAttributeChange(attributePath.mEndpointId, attributePath.mAttributeId);
            break;
        };
    }
}

/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : ZclCallback.c
 * Description        : Cluster output source file for Matter.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#include "AppTask.h"
#include "LightingManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af-types.h>
#include <assert.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (value == NULL)
    {
        return;
    }
    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION, 0, size, value);
    }
    else if (clusterId == LevelControl::Id && attributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        LightingMgr().InitiateAction(LightingManager::LEVEL_ACTION, 0, size, value);
    }
}

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    bool onOffValue = false;
    app::DataModel::Nullable<uint8_t> currentLevel;
    uint8_t Level = 0;
    Protocols::InteractionModel::Status status;

    /* restore last values saved in persistence */
    status = OnOff::Attributes::OnOff::Get(endpoint, &onOffValue);

    if (status == Protocols::InteractionModel::Status::Success)
    {
        uint8_t onOffByte = onOffValue ? 1 : 0;
        LightingMgr().InitiateAction(onOffValue ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION, 0, 1, &onOffByte);
    }

    status = LevelControl::Attributes::CurrentLevel::Get(endpoint, currentLevel);

    if (status == Protocols::InteractionModel::Status::Success && !currentLevel.IsNull())
    {
        Level = currentLevel.Value();
        LightingMgr().InitiateAction(LightingManager::LEVEL_ACTION, 0, 1, &Level);
    }
}

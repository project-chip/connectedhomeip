/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "lib/core/CHIPError.h"

CHIP_ERROR InitBindingHandler();
void SwitchWorkerFunction(intptr_t context);
void BindingWorkerFunction(intptr_t context);

struct BindingCommandData
{
    chip::AttributeId attributeId;
    chip::EndpointId localEndpointId = 1;
    chip::CommandId commandId;
    chip::ClusterId clusterId;
    bool isGroup         = false;
    bool isReadAttribute = false;
    uint32_t args[7];
};

/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "lib/core/CHIPError.h"

CHIP_ERROR InitBindingHandler();
bool IsGroupBound();
void SwitchWorkerFunction(intptr_t context);
void BindingWorkerFunction(intptr_t context);

struct BindingCommandData
{
    chip::EndpointId localEndpointId = 1;
    chip::CommandId commandId;
    chip::ClusterId clusterId;
    bool isGroup = false;
};

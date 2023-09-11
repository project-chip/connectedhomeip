/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <controller/InvokeInteraction.h>
#include <platform/CHIPDeviceLayer.h>

class BindingHandler
{
public:
    struct BindingData
    {
        chip::EndpointId EndpointId;
        chip::CommandId CommandId;
        chip::ClusterId ClusterId;
        uint8_t Value;
        bool IsGroup{ false };
    };

    void Init();
    void PrintBindingTable();
    bool IsGroupBound();

    static void SwitchWorkerHandler(intptr_t);
    static void OnInvokeCommandFailure(BindingData & aBindingData, CHIP_ERROR aError);

    static BindingHandler & GetInstance()
    {
        static BindingHandler sBindingHandler;
        return sBindingHandler;
    }

private:
    static void OnOffProcessCommand(chip::CommandId, const EmberBindingTableEntry &, chip::OperationalDeviceProxy *, void *);
    static void LevelControlProcessCommand(chip::CommandId, const EmberBindingTableEntry &, chip::OperationalDeviceProxy *, void *);
    static void LightSwitchChangedHandler(const EmberBindingTableEntry &, chip::OperationalDeviceProxy *, void *);
    static void LightSwitchContextReleaseHandler(void * context);
    static void InitInternal(intptr_t);

    bool mCaseSessionRecovered = false;
};

/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023-2024 NXP
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

#include "AppTask.h"
#include "CHIPDeviceManager.h"
#include "ICDUtil.h"
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <static-supported-temperature-levels.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceRefrigerator = 0x41;
// Refrigerator Namespace: 0x41, tag 0x00 (Refrigerator)
constexpr const uint8_t kTagRefrigerator = 0x00;
// Refrigerator Namespace: 0x41, tag 0x01 (Freezer)
constexpr const uint8_t kTagFreezer                                                = 0x01;
const Clusters::Descriptor::Structs::SemanticTagStruct::Type refrigeratorTagList[] = { { .namespaceID = kNamespaceRefrigerator,
                                                                                         .tag         = kTagRefrigerator } };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type freezerTagList[]      = { { .namespaceID = kNamespaceRefrigerator,
                                                                                         .tag         = kTagFreezer } };
} // namespace

void RefrigeratorApp::AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP Refrigerator Demo App");
}

void RefrigeratorApp::AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

void RefrigeratorApp::AppTask::PostInitMatterServerInstance()
{
// set Parent Endpoint and Composition Type for an Endpoint
    EndpointId kRefEndpointId           = 1;
    EndpointId kColdCabinetEndpointId   = 2;
    EndpointId kFreezeCabinetEndpointId = 3;
    SetTreeCompositionForEndpoint(kRefEndpointId);
    SetParentEndpointForEndpoint(kColdCabinetEndpointId, kRefEndpointId);
    SetParentEndpointForEndpoint(kFreezeCabinetEndpointId, kRefEndpointId);
    // set TagList
    SetTagList(kColdCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(refrigeratorTagList));
    SetTagList(kFreezeCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(freezerTagList));

    app::Clusters::TemperatureControl::SetInstance(&sAppSupportedTemperatureLevelsDelegate);
}

// This returns an instance of this class.
RefrigeratorApp::AppTask & RefrigeratorApp::AppTask::GetDefaultInstance()
{
    static RefrigeratorApp::AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return RefrigeratorApp::AppTask::GetDefaultInstance();
}

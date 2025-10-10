/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "Identify.h"
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip::app;

namespace {

DefaultTimerDelegate sTimerDelegate;
IdentifyDelegateImpl sIdentifyDelegate;
constexpr chip::EndpointId kEndpointId = 1;

RegisteredServerCluster<Clusters::IdentifyCluster>
    gIdentifyCluster1(Clusters::IdentifyCluster::Config(kEndpointId, sTimerDelegate)
                          .WithIdentifyType(Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator)
                          .WithDelegate(&sIdentifyDelegate));

} // namespace

CHIP_ERROR IdentifyInit()
{
    if (CodegenDataModelProvider::Instance().Registry().Get({ kEndpointId, Clusters::Identify::Id }) != nullptr)
    {
        // Already registered
        return CHIP_NO_ERROR;
    }

    return CodegenDataModelProvider::Instance().Registry().Register(gIdentifyCluster1.Registration());
}

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

#include "CodegenIntegration.h"
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanState;
using namespace chip::app::Clusters::BooleanState::Attributes;

namespace {

LazyRegisteredServerCluster<BooleanStateCluster> gServer;

constexpr EndpointId kEndpointWithBooleanState = 1;

bool ValidEndpointForBooleanState(EndpointId endpoint)
{
    if (endpoint != kEndpointWithBooleanState)
    {
        ChipLogError(AppServer, "BooleanState cluster invalid endpoint");
        return false;
    }
    return true;
}

} // namespace

void emberAfBooleanStateClusterServerInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(ValidEndpointForBooleanState(endpoint));

    gServer.Create(endpoint);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "BooleanState cluster error registration");
    }
}

void MatterBooleanStateClusterServerShutdownCallback(EndpointId endpoint)
{
    VerifyOrReturn(ValidEndpointForBooleanState(endpoint));

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "BooleanState unregister error");
    }

    gServer.Destroy();
}

namespace chip::app::Clusters::BooleanState {

CHIP_ERROR
SetStateValue(const StateValue::TypeInfo::Type & stateValue)
{
    return gServer.Cluster().SetStateValue(stateValue);
}

Attributes::StateValue::TypeInfo::Type GetStateValue()
{
    return gServer.Cluster().GetStateValue();
}

} // namespace chip::app::Clusters::BooleanState

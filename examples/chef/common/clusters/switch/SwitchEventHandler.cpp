/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#ifdef MATTER_DM_PLUGIN_SWITCH_SERVER
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <platform/PlatformManager.h>

#include "SwitchEventHandler.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::DeviceLayer;

void SwitchEventHandler::OnSwitchLatched(EndpointId endpointId, uint8_t newPosition)
{
    ChipLogDetail(NotSpecified, "%s: endpointId=%d, newPosition=%d", __func__, endpointId, newPosition);

    Clusters::SwitchServer::Instance().OnSwitchLatch(endpointId, newPosition);
}

void SwitchEventHandler::OnInitialPress(EndpointId endpointId, uint8_t newPosition)
{
    ChipLogDetail(NotSpecified, "%s: endpointId=%d, newPosition=%d", __func__, endpointId, newPosition);

    Clusters::SwitchServer::Instance().OnInitialPress(endpointId, newPosition);
}

void SwitchEventHandler::OnLongPress(EndpointId endpointId, uint8_t newPosition)
{
    ChipLogDetail(NotSpecified, "%s: endpointId=%d, newPosition=%d", __func__, endpointId, newPosition);

    Clusters::SwitchServer::Instance().OnLongPress(endpointId, newPosition);
}

void SwitchEventHandler::OnShortRelease(EndpointId endpointId, uint8_t previousPosition)
{
    ChipLogDetail(NotSpecified, "%s: endpointId=%d, previousPosition=%d", __func__, endpointId, previousPosition);

    Clusters::SwitchServer::Instance().OnShortRelease(endpointId, previousPosition);
}

void SwitchEventHandler::OnLongRelease(EndpointId endpointId, uint8_t previousPosition)
{
    ChipLogDetail(NotSpecified, "%s: endpointId=%d, previousPosition=%d", __func__, endpointId, previousPosition);

    Clusters::SwitchServer::Instance().OnLongRelease(endpointId, previousPosition);
}

void SwitchEventHandler::OnMultiPressOngoing(EndpointId endpointId, uint8_t newPosition, uint8_t count)
{
    ChipLogDetail(NotSpecified, "%s: endpointId=%d, newPosition=%d, count=%d", __func__, endpointId, newPosition, count);

    Clusters::SwitchServer::Instance().OnMultiPressOngoing(endpointId, newPosition, count);
}

void SwitchEventHandler::OnMultiPressComplete(EndpointId endpointId, uint8_t previousPosition, uint8_t count)
{
    ChipLogDetail(NotSpecified, "%s: endpointId=%d, previousPosition=%d, count=%d", __func__, endpointId, previousPosition, count);

    Clusters::SwitchServer::Instance().OnMultiPressComplete(endpointId, previousPosition, count);
}
#endif // MATTER_DM_PLUGIN_SWITCH_SERVER

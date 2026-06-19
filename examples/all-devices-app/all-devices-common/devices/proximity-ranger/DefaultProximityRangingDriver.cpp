/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "DefaultProximityRangingDriver.h"

#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

CHIP_ERROR DefaultProximityRangingDriver::Init(Callback & callback)
{
    mCallback = &callback;
    return mController.AddListener(this);
}

void DefaultProximityRangingDriver::Shutdown()
{
    mController.RemoveListener(this);
    mCallback = nullptr;
}

ResultCodeEnum DefaultProximityRangingDriver::HandleStartRanging(uint8_t sessionId,
                                                                 const Commands::StartRangingRequest::DecodableType & request)
{
    return mController.StartSession(sessionId, request);
}

CHIP_ERROR DefaultProximityRangingDriver::HandleStopRanging(uint8_t sessionId)
{
    return mController.StopSession(sessionId);
}

CHIP_ERROR DefaultProximityRangingDriver::GetRangingCapabilities(AttributeValueEncoder & encoder)
{
    return mController.GetRangingCapabilities(encoder);
}

std::optional<BleRbcConfig> DefaultProximityRangingDriver::GetBleRbcConfig()
{
    return BleRbcConfig{ mController.GetBleDeviceId() };
}

CHIP_ERROR DefaultProximityRangingDriver::GetActiveSessionIds(Span<uint8_t> & sessionIds)
{
    return mController.GetActiveSessionIds(sessionIds);
}

void DefaultProximityRangingDriver::OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status)
{
    VerifyOrReturn(mCallback != nullptr);
    mCallback->OnSessionStopped(sessionId, status);
}

void DefaultProximityRangingDriver::OnSessionMeasurement(uint8_t sessionId,
                                                         const Structs::RangingMeasurementDataStruct::Type & measurement)
{
    VerifyOrReturn(mCallback != nullptr);
    mCallback->OnMeasurementData(sessionId, measurement);
}

void DefaultProximityRangingDriver::OnAttributeChanged(AttributeId attributeId)
{
    VerifyOrReturn(mCallback != nullptr);
    mCallback->OnAttributeChanged(attributeId);
}

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip

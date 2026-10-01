/*
 *
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

#include <LockEndpoint.h>
#include <LockManager.h>
#include <app/clusters/door-lock-server/CodegenIntegration.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#include "DoorLockClusterBridge.h"

#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;
using chip::Optional;
using chip::app::DataModel::Nullable;

namespace LockApp {

LockEndpoint * LockAppDoorLockDelegate::Resolve() const
{
    return LockManager::Instance().GetLockEndpoint(mEndpointId);
}

bool LockAppDoorLockDelegate::HandleDoorLockCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                                    const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                                    OperationErrorEnum & err)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->Lock(fabricIdx, nodeId, pinCode, err, OperationSourceEnum::kRemote);
    }
    ChipLogError(Zcl, "Door Lock App: no endpoint available for LockDoor [endpointId=%d]", endpointId);
    return false;
}

bool LockAppDoorLockDelegate::HandleDoorUnlockCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                                      const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                                      OperationErrorEnum & err)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->Unlock(fabricIdx, nodeId, pinCode, err, OperationSourceEnum::kRemote);
    }
    ChipLogError(Zcl, "Door Lock App: no endpoint available for UnlockDoor [endpointId=%d]", endpointId);
    return false;
}

bool LockAppDoorLockDelegate::HandleDoorUnboltCommand(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx,
                                                      const Nullable<NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                                      OperationErrorEnum & err)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->Unbolt(fabricIdx, nodeId, pinCode, err, OperationSourceEnum::kRemote);
    }
    ChipLogError(Zcl, "Door Lock App: no endpoint available for UnboltDoor [endpointId=%d]", endpointId);
    return false;
}

CHIP_ERROR LockAppDoorLockDelegate::GetAliroReaderVerificationKey(MutableByteSpan & verificationKey)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetAliroReaderVerificationKey(verificationKey);
    }
    verificationKey.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LockAppDoorLockDelegate::GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetAliroReaderGroupIdentifier(groupIdentifier);
    }
    groupIdentifier.reduce_size(0);
    return CHIP_NO_ERROR;
}

// Not nullable: forward the stub (all-zero) subidentifier when no endpoint exists.
CHIP_ERROR LockAppDoorLockDelegate::GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetAliroReaderGroupSubIdentifier(groupSubIdentifier);
    }
    memset(groupSubIdentifier.data(), 0, groupSubIdentifier.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR LockAppDoorLockDelegate::GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                                                MutableByteSpan & protocolVersion)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(index, protocolVersion);
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR LockAppDoorLockDelegate::GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetAliroGroupResolvingKey(groupResolvingKey);
    }
    groupResolvingKey.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LockAppDoorLockDelegate::GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetAliroSupportedBLEUWBProtocolVersionAtIndex(index, protocolVersion);
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

uint8_t LockAppDoorLockDelegate::GetAliroBLEAdvertisingVersion()
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetAliroBLEAdvertisingVersion();
    }
    return 0;
}

uint16_t LockAppDoorLockDelegate::GetNumberOfAliroCredentialIssuerKeysSupported()
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetNumberOfAliroCredentialIssuerKeysSupported();
    }
    return 0;
}

uint16_t LockAppDoorLockDelegate::GetNumberOfAliroEndpointKeysSupported()
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->GetNumberOfAliroEndpointKeysSupported();
    }
    return 0;
}

CHIP_ERROR LockAppDoorLockDelegate::SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                                         const ByteSpan & groupIdentifier,
                                                         const Optional<ByteSpan> & groupResolvingKey)
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->SetAliroReaderConfig(signingKey, verificationKey, groupIdentifier, groupResolvingKey);
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR LockAppDoorLockDelegate::ClearAliroReaderConfig()
{
    if (auto * endpoint = Resolve())
    {
        return endpoint->ClearAliroReaderConfig();
    }
    return CHIP_NO_ERROR;
}

} // namespace LockApp

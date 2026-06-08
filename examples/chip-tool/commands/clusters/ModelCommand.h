/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#ifdef CONFIG_USE_LOCAL_STORAGE
#include <controller/ExamplePersistentStorage.h>
#endif // CONFIG_USE_LOCAL_STORAGE

#include "../common/CHIPCommand.h"
#include <lib/core/CHIPEncoding.h>

class ModelCommand : public CHIPCommand
{
public:
    ModelCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig, bool supportsMultipleEndpoints = false,
                 const char * helpText = nullptr) :
        CHIPCommand(commandName, credsIssuerConfig, helpText),
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mSupportsMultipleEndpoints(supportsMultipleEndpoints)
    {}

    void AddArguments(bool skipEndpoints = false)
    {
        AddArgument(
            "destination-id", 0, UINT64_MAX, &mDestinationId,
            "64-bit node or group identifier.\n  Group identifiers are detected by being in the 0xFFFF'FFFF'FFFF'xxxx range.");
        if (skipEndpoints == false)
        {
            if (mSupportsMultipleEndpoints)
            {
                AddArgument("endpoint-ids", 0, UINT16_MAX, &mEndPointId,
                            "Comma-separated list of endpoint ids (e.g. \"1\" or \"1,2,3\").\n  Allowed to be 0xFFFF to indicate a "
                            "wildcard endpoint.");
            }
            else
            {
                AddArgument("endpoint-id-ignored-for-group-commands", 0, UINT16_MAX, &mEndPointId,
                            "Endpoint the command is targeted at.");
            }
        }
        AddArgument(
            "lit-icd-peer", 0, 1, &mIsPeerLIT,
            "Whether to treat the peer as a LIT ICD. false: Always no, true: Always yes, (not set): Yes if the peer is registered "
            "to this controller.");
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
        AddArgument("allow-large-payload", 0, 1, &mAllowLargePayload,
                    "If true, indicates that the session should allow large application payloads (which requires a TCP connection)."
                    "Defaults to false, which uses a UDP+MRP session.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(mTimeout.ValueOr(20)); }

    virtual CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endPointIds) = 0;

    virtual CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) { return CHIP_ERROR_BAD_REQUEST; };

    void ClearICDEntry(const chip::ScopedNodeId & nodeId);
    void StoreICDEntryWithKey(chip::app::ICDClientInfo & clientinfo, chip::ByteSpan key);
    void Shutdown() override;

protected:
    bool IsPeerLIT();

    chip::NodeId GetDestinationId() const { return mDestinationId; }

    chip::Optional<uint16_t> mTimeout;

private:
    chip::NodeId mDestinationId;
    std::vector<chip::EndpointId> mEndPointId;
    chip::Optional<bool> mIsPeerLIT;
    chip::Optional<bool> mAllowLargePayload;

    void CheckPeerICDType();

    bool AllowLargePayload();

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    const bool mSupportsMultipleEndpoints;
};

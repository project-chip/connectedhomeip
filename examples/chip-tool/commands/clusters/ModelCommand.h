/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    ModelCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig, bool supportsMultipleEndpoints = false) :
        CHIPCommand(commandName, credsIssuerConfig), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
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
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(mTimeout.ValueOr(20)); }

    virtual CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endPointIds) = 0;

    virtual CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) { return CHIP_ERROR_BAD_REQUEST; };

    void Shutdown() override;

protected:
    chip::Optional<uint16_t> mTimeout;

private:
    chip::NodeId mDestinationId;
    std::vector<chip::EndpointId> mEndPointId;

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    const bool mSupportsMultipleEndpoints;
};

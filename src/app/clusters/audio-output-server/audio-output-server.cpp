/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Routines for the Audio Output plugin, the
 *server implementation of the Audio Output cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/audio-output-server/audio-output-delegate.h>
#include <app/clusters/audio-output-server/audio-output-server.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters::AudioOutput;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::AudioOutput::Delegate;

namespace {

Delegate * gDelegateTable[EMBER_AF_AUDIO_OUTPUT_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::AudioOutput::Id);
    return (ep == 0xFFFF ? NULL : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Audio Output has no delegate set for endpoint:%" PRIu16, endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace AudioOutput {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::AudioOutput::Id);
    if (ep != 0xFFFF)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace AudioOutput
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class AudioOutputAttrAccess : public app::AttributeAccessInterface
{
public:
    AudioOutputAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::AudioOutput::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadOutputListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadCurrentOutputAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

AudioOutputAttrAccess gAudioOutputAttrAccess;

CHIP_ERROR AudioOutputAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::AudioOutput::Attributes::AudioOutputList::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadOutputListAttribute(aEncoder, delegate);
    }
    case app::Clusters::AudioOutput::Attributes::CurrentAudioOutput::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadCurrentOutputAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioOutputAttrAccess::ReadOutputListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetOutputList(aEncoder);
}

CHIP_ERROR AudioOutputAttrAccess::ReadCurrentOutputAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint8_t currentOutput = delegate->HandleGetCurrentOutput();
    return aEncoder.Encode(currentOutput);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfAudioOutputClusterRenameOutputRequestCallback(app::CommandHandler * command,
                                                          const app::ConcreteCommandPath & commandPath,
                                                          const Commands::RenameOutputRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & index        = commandData.index;
    auto & name         = commandData.name;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAudioOutputClusterRenameOutputRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    bool success         = delegate->HandleRenameOutput(index, name);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfAudioOutputClusterSelectOutputRequestCallback(app::CommandHandler * command,
                                                          const app::ConcreteCommandPath & commandPath,
                                                          const Commands::SelectOutputRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & index        = commandData.index;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAudioOutputClusterSelectOutputRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    bool success         = delegate->HandleSelectOutput(index);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterAudioOutputPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAudioOutputAttrAccess);
}

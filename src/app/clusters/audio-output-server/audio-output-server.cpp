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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app::Clusters::AudioOutput;

static constexpr size_t kAudioOutputDelegateTableSize =
    EMBER_AF_AUDIO_OUTPUT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::AudioOutput::Delegate;

namespace {

Delegate * gDelegateTable[kAudioOutputDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::AudioOutput::Id);
    return (ep == 0xFFFF ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Audio Output has no delegate set for endpoint:%u", endpoint);
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

bool HasFeature(chip::EndpointId endpoint, AudioOutputFeature feature)
{
    bool hasFeature     = false;
    uint32_t featureMap = 0;

    EmberAfStatus status = Attributes::FeatureMap::Get(endpoint, &featureMap);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        hasFeature = (featureMap & chip::to_underlying(feature));
    }

    return hasFeature;
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
    case app::Clusters::AudioOutput::Attributes::OutputList::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadOutputListAttribute(aEncoder, delegate);
    }
    case app::Clusters::AudioOutput::Attributes::CurrentOutput::Id: {
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

bool emberAfAudioOutputClusterRenameOutputCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::RenameOutput::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & index        = commandData.index;
    auto & name         = commandData.name;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!HasFeature(endpoint, AudioOutputFeature::kNameUpdates))
    {
        ChipLogError(Zcl, "AudioOutput no name updates feature");
        err = CHIP_ERROR_INCORRECT_STATE;
        ExitNow();
    }

    Protocols::InteractionModel::Status status;

    if (delegate->HandleRenameOutput(index, name))
    {
        status = Protocols::InteractionModel::Status::Success;
    }
    else
    {
        status = Protocols::InteractionModel::Status::Failure;
    }
    command->AddStatus(commandPath, status);

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAudioOutputClusterRenameOutputCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Protocols::InteractionModel::Status::Failure);
    }

    return true;
}

bool emberAfAudioOutputClusterSelectOutputCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::SelectOutput::DecodableType & commandData)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    EndpointId endpoint  = commandPath.mEndpointId;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    auto & index         = commandData.index;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleSelectOutput(index))
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfAudioOutputClusterSelectOutputCallback error: %s", err.AsString());
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterAudioOutputPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAudioOutputAttrAccess);
}

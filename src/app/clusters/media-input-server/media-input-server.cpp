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
 * @brief Routines for the Media Input plugin, the
 *server implementation of the Media Input cluster.
 *******************************************************************************
 ******************************************************************************/

#include "media-input-server.h"
#include "media-input-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaInput;
using Protocols::InteractionModel::Status;

static constexpr size_t kMediaInputDelegateTableSize =
    MATTER_DM_MEDIA_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kMediaInputDelegateTableSize <= kEmberInvalidEndpointIndex, "MediaInput Delegate tablle size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::MediaInput::Delegate;

namespace {

Delegate * gDelegateTable[kMediaInputDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, MediaInput::Id, MATTER_DM_MEDIA_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kMediaInputDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Media Input has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MediaInput {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, MediaInput::Id, MATTER_DM_MEDIA_INPUT_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kMediaInputDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

bool HasFeature(chip::EndpointId endpoint, Feature feature)
{
    bool hasFeature     = false;
    uint32_t featureMap = 0;

    Status status = Attributes::FeatureMap::Get(endpoint, &featureMap);
    if (Status::Success == status)
    {
        hasFeature = (featureMap & chip::to_underlying(feature));
    }

    return hasFeature;
}

} // namespace MediaInput
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class MediaInputAttrAccess : public app::AttributeAccessInterface
{
public:
    MediaInputAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::MediaInput::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadInputListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadCurrentInputAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

MediaInputAttrAccess gMediaInputAttrAccess;

CHIP_ERROR MediaInputAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::MediaInput::Attributes::InputList::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadInputListAttribute(aEncoder, delegate);
    }
    case app::Clusters::MediaInput::Attributes::CurrentInput::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadCurrentInputAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaInputAttrAccess::ReadInputListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetInputList(aEncoder);
}

CHIP_ERROR MediaInputAttrAccess::ReadCurrentInputAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint8_t currentInput = delegate->HandleGetCurrentInput();
    return aEncoder.Encode(currentInput);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfMediaInputClusterSelectInputCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::SelectInput::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & input = commandData.index;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleSelectInput(input))
    {
        status = Status::Failure;
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaInputClusterSelectInputCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

bool emberAfMediaInputClusterShowInputStatusCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::ShowInputStatus::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleShowInputStatus())
    {
        status = Status::Failure;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaInputClusterShowInputStatusCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

bool emberAfMediaInputClusterHideInputStatusCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::HideInputStatus::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleHideInputStatus())
    {
        status = Status::Failure;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaInputClusterHideInputStatusCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

bool emberAfMediaInputClusterRenameInputCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::RenameInput::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & index        = commandData.index;
    auto & name         = commandData.name;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    if (!HasFeature(endpoint, Feature::kNameUpdates))
    {
        ChipLogError(Zcl, "MediaInput no name updates feature");
        err = CHIP_ERROR_INCORRECT_STATE;
        ExitNow();
    }

    Protocols::InteractionModel::Status status;

    if (delegate->HandleRenameInput(index, name))
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
        ChipLogError(Zcl, "emberAfMediaInputClusterRenameInputCallback error: %" CHIP_ERROR_FORMAT, err.Format());
        command->AddStatus(commandPath, Protocols::InteractionModel::Status::Failure);
    }

    return true;
}

void MatterMediaInputPluginServerInitCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Register(&gMediaInputAttrAccess);
}

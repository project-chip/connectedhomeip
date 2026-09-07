/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/messages-server/messages-delegate.h>
#include <app/clusters/messages-server/messages-server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Messages;
using chip::app::LogEvent;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kMessagesDelegateTableSize =
    MATTER_DM_MESSAGES_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kMessagesDelegateTableSize <= kEmberInvalidEndpointIndex, "Messages Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

namespace {

Delegate * gDelegateTable[kMessagesDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "MessagesCluster NOT returning delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Messages::Id, MATTER_DM_MESSAGES_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kMessagesDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Message Cluster has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Messages {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Messages::Id, MATTER_DM_MESSAGES_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kMessagesDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

bool Delegate::HasFeature(chip::EndpointId endpoint, Feature feature)
{
    uint32_t featureMap = GetFeatureMap(endpoint);
    return (featureMap & chip::to_underlying(feature));
}

CHIP_ERROR LogMessageNotPresentedEvent(chip::EndpointId endpoint, const ByteSpan & messageId, bool removedFromQueue)
{
    Events::MessageNotPresented::Type event{ .messageID = messageId, .removedFromQueue = removedFromQueue };

    EventNumber eventNumber;
    CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "LogMessageNotPresentedEvent: unable to send event: %s [endpointId=%d]", err.AsString(), endpoint);
    }
    return err;
}

} // namespace Messages
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class MessagesAttrAccess : public app::AttributeAccessInterface
{
public:
    MessagesAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), Messages::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadMessages(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadActiveMessageIds(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSupportedLanguageCodes(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSupportedMimeTypes(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

MessagesAttrAccess gMessagesAttrAccess;

CHIP_ERROR MessagesAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::Messages::Attributes::Messages::Id:
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadMessages(aEncoder, delegate);
    case app::Clusters::Messages::Attributes::ActiveMessageIDs::Id:
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadActiveMessageIds(aEncoder, delegate);
    case app::Clusters::Messages::Attributes::SupportedLanguageCodes::Id:
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadSupportedLanguageCodes(aEncoder, delegate);
    case app::Clusters::Messages::Attributes::SupportedMimeTypes::Id:
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadSupportedMimeTypes(aEncoder, delegate);
    case app::Clusters::Messages::Attributes::FeatureMap::Id:
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadFeatureFlagAttribute(endpoint, aEncoder, delegate);
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MessagesAttrAccess::ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder,
                                                        Delegate * delegate)
{
    uint32_t featureFlag = delegate->GetFeatureMap(endpoint);
    return aEncoder.Encode(featureFlag);
}

CHIP_ERROR MessagesAttrAccess::ReadMessages(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetMessages(aEncoder);
}

CHIP_ERROR MessagesAttrAccess::ReadActiveMessageIds(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetActiveMessageIds(aEncoder);
}

CHIP_ERROR MessagesAttrAccess::ReadSupportedLanguageCodes(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetSupportedLanguageCodes(aEncoder);
}

CHIP_ERROR MessagesAttrAccess::ReadSupportedMimeTypes(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetSupportedMimeTypes(aEncoder);
}

} // anonymous namespace

bool emberAfMessagesClusterPresentMessagesRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Messages::Commands::PresentMessagesRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & messageId      = commandData.messageID;
    auto & priority       = commandData.priority;
    auto & messageControl = commandData.messageControl;
    auto & startTime      = commandData.startTime;
    auto & duration       = commandData.duration;
    auto & messageText    = commandData.messageText;
    auto & responses      = commandData.responses;
    auto & languageCode   = commandData.languageCode;
    auto & messageUri     = commandData.messageURI;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, status = Status::NotFound);

    VerifyOrExit(messageId.size() == kMessageIdLength,
                 ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback invalid message id length");
                 status = Status::ConstraintError);

    VerifyOrExit(messageText.size() <= kMessageTextLengthMax,
                 ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback invalid message text length");
                 status = Status::ConstraintError);

    VerifyOrExit(!messageControl.Has(MessageControlBitmap::kSpokenMessage) ||
                     delegate->HasFeature(endpoint, Feature::kSpokenMessages),
                 ChipLogProgress(Zcl,
                                 "emberAfMessagesClusterPresentMessagesRequestCallback SpokenMessage bit set but "
                                 "SpokenMessages feature not supported");
                 status = Status::InvalidCommand);

    VerifyOrExit(!messageControl.Has(MessageControlBitmap::kAudioMessage) ||
                     delegate->HasFeature(endpoint, Feature::kAudioMessages),
                 ChipLogProgress(Zcl,
                                 "emberAfMessagesClusterPresentMessagesRequestCallback AudioMessage bit set but "
                                 "AudioMessages feature not supported");
                 status = Status::InvalidCommand);

    VerifyOrExit(
        !(messageControl.Has(MessageControlBitmap::kSpokenMessage) && messageControl.Has(MessageControlBitmap::kAudioMessage)) ||
            delegate->HasFeature(endpoint, Feature::kMultiModalMessages),
        ChipLogProgress(Zcl,
                        "emberAfMessagesClusterPresentMessagesRequestCallback SpokenMessage and AudioMessage "
                        "bits both set but MultiModalMessages feature not supported");
        status = Status::InvalidCommand);

    VerifyOrExit(
        !messageControl.Has(MessageControlBitmap::kSpokenMessage) || languageCode.HasValue(),
        ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback SpokenMessage bit set but LanguageCode missing");
        status = Status::ConstraintError);

    VerifyOrExit(
        !messageControl.Has(MessageControlBitmap::kAudioMessage) || messageUri.HasValue(),
        ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback AudioMessage bit set but MessageURI missing");
        status = Status::ConstraintError);

    if (languageCode.HasValue())
    {
        VerifyOrExit(delegate->HasFeature(endpoint, Feature::kSpokenMessages),
                     ChipLogProgress(Zcl,
                                     "emberAfMessagesClusterPresentMessagesRequestCallback LanguageCode sent but "
                                     "SpokenMessages feature not supported");
                     status = Status::InvalidCommand);

        VerifyOrExit(languageCode.Value().size() > 0 && languageCode.Value().size() <= kLanguageCodeLengthMax,
                     ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback invalid LanguageCode length");
                     status = Status::ConstraintError);
    }

    if (messageUri.HasValue())
    {
        VerifyOrExit(delegate->HasFeature(endpoint, Feature::kAudioMessages),
                     ChipLogProgress(Zcl,
                                     "emberAfMessagesClusterPresentMessagesRequestCallback MessageURI sent but "
                                     "AudioMessages feature not supported");
                     status = Status::InvalidCommand);

        VerifyOrExit(messageUri.Value().size() > 0 && messageUri.Value().size() <= kMessageUriLengthMax,
                     ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback invalid MessageURI length");
                     status = Status::ConstraintError);
    }

    if (responses.HasValue())
    {
        size_t size = 0;
        err         = responses.Value().ComputeSize(&size);
        VerifyOrExit(err == CHIP_NO_ERROR,
                     ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback size check failed");
                     status = Status::ConstraintError);

        VerifyOrExit(
            delegate->HasFeature(endpoint, Feature::kConfirmationResponse),
            ChipLogProgress(
                Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback responses sent but response feature not supported");
            status = Status::InvalidCommand);

        VerifyOrExit(size <= kMessageMaxOptionCount,
                     ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback too many options");
                     status = Status::ConstraintError);

        auto iter = responses.Value().begin();
        while (iter.Next())
        {
            auto & response = iter.GetValue();

            // response feature is checked above
            VerifyOrExit(response.messageResponseID.HasValue() && response.label.HasValue(),
                         ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback missing response id or label");
                         status = Status::InvalidCommand);

            VerifyOrExit(response.messageResponseID.Value() >= kMessageResponseIdMin,
                         ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback responseID value check failed");
                         status = Status::ConstraintError);

            VerifyOrExit(response.label.Value().size() <= kMessageResponseLabelMaxLength,
                         ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback label length check failed");
                         status = Status::ConstraintError);
        }
        VerifyOrExit(iter.GetStatus() == CHIP_NO_ERROR,
                     ChipLogProgress(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback TLV parsing error");
                     status = Status::InvalidAction);
    }

    err = delegate->HandlePresentMessagesRequest(messageId, priority, messageControl, startTime, duration, messageText, responses,
                                                 languageCode, messageUri);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMessagesClusterPresentMessagesRequestCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Messages Cluster CancelMessagesRequest Command callback (from client)
 */

bool emberAfMessagesClusterCancelMessagesRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Messages::Commands::CancelMessagesRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & messageIds = commandData.messageIDs;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        auto iter = messageIds.begin();
        while (iter.Next())
        {
            auto & id = iter.GetValue();
            VerifyOrExit(id.size() >= kMessageIdLength,
                         ChipLogProgress(Zcl, "emberAfMessagesClusterCancelMessagesRequestCallback message id size check failed");
                         status = Status::ConstraintError);
        }
        VerifyOrExit(iter.GetStatus() == CHIP_NO_ERROR,
                     ChipLogProgress(Zcl, "emberAfMessagesClusterCancelMessagesRequestCallback TLV parsing error");
                     status = Status::InvalidAction);
    }

    err = delegate->HandleCancelMessagesRequest(messageIds);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMessagesClusterCancelMessagesRequestCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

void MatterMessagesPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gMessagesAttrAccess);
}

void MatterMessagesPluginServerShutdownCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(&gMessagesAttrAccess);
}

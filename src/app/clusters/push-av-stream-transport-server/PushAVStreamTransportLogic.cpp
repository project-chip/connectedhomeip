/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *
 */

#include "constants.h"
#include <algorithm>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportLogic.h>
#include <app/data-model/WrappedStructEncoder.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/util/util.h>
#include <array>
#include <assert.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <protocols/interaction_model/StatusCode.h>
#include <set>
#include <uriparser/Uri.h>
#include <vector>

static constexpr uint16_t kMaxConnectionId = 65535; // This is also invalid connectionID
static constexpr uint16_t kMaxEndpointId   = 65534;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using namespace chip::app::Clusters::PushAvStreamTransport::Structs;
using namespace chip::app::Clusters::PushAvStreamTransport::Attributes;
using namespace Protocols::InteractionModel;
using chip::Protocols::InteractionModel::Status;

namespace {

bool HasDuplicateLargeMotionZoneIds(
    const DataModel::DecodableList<chip::app::Clusters::PushAvStreamTransport::Structs::TransportZoneOptionsStruct::DecodableType> &
        zones,
    uint32_t zoneGroups, Span<uint16_t> zoneBits)
{
    for (uint16_t group = 0; group < 16; ++group)
    {
        if ((zoneGroups & (uint32_t{ 1 } << group)) == 0)
        {
            continue;
        }
        std::fill(zoneBits.begin(), zoneBits.end(), 0);
        auto iter = zones.begin();
        while (iter.Next())
        {
            const auto & zone = iter.GetValue().zone;
            if (zone.IsNull() || (zone.Value() >> 12) != group)
            {
                continue;
            }
            const uint16_t zoneId = zone.Value();
            auto & word           = zoneBits[(zoneId & 0x0fff) >> 4];
            const uint16_t bit    = static_cast<uint16_t>(uint16_t{ 1 } << (zoneId & 15));
            if ((word & bit) != 0)
            {
                return true;
            }
            word |= bit;
        }
    }
    return false;
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {

namespace Internal {

class RequiredZoneFieldReader : public DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType>
{
public:
    explicit RequiredZoneFieldReader(const DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> & zones) :
        DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType>(zones)
    {}

    CHIP_ERROR Validate() const
    {
        if (mReader.GetContainerType() == TLV::kTLVType_NotSpecified)
        {
            return CHIP_NO_ERROR;
        }
        TLV::TLVReader reader;
        reader.Init(mReader);
        CHIP_ERROR status;
        while ((status = reader.Next()) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
            TLV::TLVType containerType;
            ReturnErrorOnFailure(reader.EnterContainer(containerType));
            bool zonePresent = false;
            while ((status = reader.Next()) == CHIP_NO_ERROR)
            {
                if (reader.GetTag() == TLV::ContextTag(to_underlying(Structs::TransportZoneOptionsStruct::Fields::kZone)))
                {
                    zonePresent = true;
                }
            }
            VerifyOrReturnError(status == CHIP_END_OF_TLV, status);
            ReturnErrorOnFailure(reader.ExitContainer(containerType));
            VerifyOrReturnError(zonePresent, CHIP_ERROR_MISSING_TLV_ELEMENT);
        }
        return status == CHIP_END_OF_TLV ? CHIP_NO_ERROR : status;
    }
};

std::string extractTextRange(const UriTextRangeA & range)
{
    if (range.first == nullptr || range.afterLast == nullptr || range.first >= range.afterLast)
    {
        return "";
    }
    return std::string(range.first, range.afterLast);
}

std::string extractPath(const UriPathSegmentA * pathHead)
{
    std::string path;
    const UriPathSegmentA * segment = pathHead;

    while (segment != nullptr)
    {
        path += "/";

        if (segment->text.first != nullptr && segment->text.afterLast != nullptr)
        {
            path += std::string(segment->text.first, segment->text.afterLast);
        }
        segment = segment->next;
    }

    return path;
}

} // namespace Internal

PushAvStreamTransportServerLogic::PushAvStreamTransportServerLogic(EndpointId aEndpoint, BitFlags<Feature> aFeatures) :
    mEndpointId(aEndpoint), mFeatures(aFeatures),
    mSupportedFormats{ PushAvStreamTransport::SupportedFormatStruct{ PushAvStreamTransport::ContainerFormatEnum::kCmaf,
                                                                     PushAvStreamTransport::IngestMethodsEnum::kCMAFIngest } }
{}

PushAvStreamTransportServerLogic::~PushAvStreamTransportServerLogic() {}

CHIP_ERROR PushAvStreamTransportServerLogic::Init(AttributePersistenceProvider & attributeStorage)
{
    VerifyOrDie(mCluster != nullptr);

    if (!mCurrentConnections.empty())
    {
        VerifyOrReturnError(!IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
        mAttributeStorage = nullptr;
        ReturnErrorOnFailure(CleanupFailedRestoration());
    }
    mAttributeStorage = &attributeStorage;
    CHIP_ERROR err    = LoadPersistentAttributes();
    if (err != CHIP_NO_ERROR)
    {
        mAttributeStorage = nullptr;
    }
    return err;
}

void PushAvStreamTransportServerLogic::Shutdown()
{
    for (const auto & timerContext : mTimerContexts)
    {
        DeviceLayer::SystemLayer().CancelTimer(PushAVStreamTransportDeallocateCallback, static_cast<void *>(timerContext.get()));
    }
    mAttributeStorage = nullptr;
}

bool PushAvStreamTransportServerLogic::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

bool PushAvStreamTransportServerLogic::IsNullDelegateWithLogging(EndpointId endpointIdForLogging)
{

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "No PushAvStreamTransportDelegate set for ep:%u", endpointIdForLogging);
        return true;
    }
    return false;
}

PushAvStreamTransportServerLogic::UpsertResultEnum
PushAvStreamTransportServerLogic::UpsertStreamTransportConnection(const TransportConfigurationStorage & transportConfiguration)
{
    UpsertResultEnum result;
    auto it =
        std::find_if(mCurrentConnections.begin(), mCurrentConnections.end(),
                     [id = transportConfiguration.connectionID](const auto & existing) { return existing.connectionID == id; });

    if (it != mCurrentConnections.end())
    {
        TransportConfigurationStorage oldValue = *it;
        *it                                    = transportConfiguration;
        result                                 = UpsertResultEnum::kUpdated;
        CHIP_ERROR err                         = StoreCurrentConnections();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to store updated connection, reverting: %" CHIP_ERROR_FORMAT, err.Format());
            *it = oldValue; // Revert
            return UpsertResultEnum::kFailed;
        }
    }
    else
    {
        mCurrentConnections.push_back(transportConfiguration);
        result         = UpsertResultEnum::kInserted;
        CHIP_ERROR err = StoreCurrentConnections();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to store new connection, reverting: %" CHIP_ERROR_FORMAT, err.Format());
            mCurrentConnections.pop_back(); // Revert
            return UpsertResultEnum::kFailed;
        }
    }

    mCluster->ReportAttributeChange(PushAvStreamTransport::Attributes::CurrentConnections::Id);

    return result;
}

CHIP_ERROR PushAvStreamTransportServerLogic::RemoveStreamTransportConnection(const uint16_t transportConnectionId)
{
    auto it = std::find_if(
        mCurrentConnections.begin(), mCurrentConnections.end(),
        [transportConnectionId](const TransportConfigurationStorage & s) { return s.connectionID == transportConnectionId; });

    if (it != mCurrentConnections.end())
    {
        TransportConfigurationStorage removedValue = *it;
        mCurrentConnections.erase(it);

        CHIP_ERROR err = StoreCurrentConnections();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to store after removal, reverting: %" CHIP_ERROR_FORMAT, err.Format());
            mCurrentConnections.push_back(removedValue); // Revert
            return err;
        }

        // Notify the stack that the CurrentConnections attribute has changed.
        mCluster->ReportAttributeChange(PushAvStreamTransport::Attributes::CurrentConnections::Id);
    }
    return CHIP_NO_ERROR;
}

void PushAvStreamTransportServerLogic::RemoveTimerAppState(const uint16_t connectionID)
{
    // Erase-Remove idiom
    auto it = std::remove_if(mTimerContexts.begin(), mTimerContexts.end(),
                             [connectionID](const std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext> & s) {
                                 if (s->connectionID == connectionID)
                                 {
                                     DeviceLayer::SystemLayer().CancelTimer(PushAVStreamTransportDeallocateCallback,
                                                                            static_cast<void *>(s.get()));
                                     return true; // Remove from vector
                                 }
                                 return false; // Keep in vector
                             });

    mTimerContexts.erase(it, mTimerContexts.end());
}

CHIP_ERROR PushAvStreamTransportServerLogic::CleanupFailedRestoration()
{
    auto connection = mCurrentConnections.begin();
    while (connection != mCurrentConnections.end())
    {
        const auto status = mDelegate->DeallocatePushTransport(connection->connectionID);
        if (status == Status::Success || status == Status::NotFound)
        {
            RemoveTimerAppState(connection->connectionID);
            connection = mCurrentConnections.erase(connection);
        }
        else
        {
            ChipLogError(Zcl, "Failed to roll back restored connection %u: status %u", connection->connectionID,
                         to_underlying(status));
            ++connection;
        }
    }
    return mCurrentConnections.empty() ? CHIP_NO_ERROR : CHIP_ERROR_BUSY;
}

CHIP_ERROR PushAvStreamTransportServerLogic::LoadPersistentAttributes()
{
    // Load currentConnections
    ReturnErrorOnFailure(LoadCurrentConnections());

    // Signal delegate that all persistent configuration attributes have been loaded.
    CHIP_ERROR err = mDelegate->PersistentAttributesLoadedCallback();
    if (err != CHIP_NO_ERROR)
    {
        LogErrorOnFailure(CleanupFailedRestoration());
    }
    return err;
}

TransportConfigurationStorage * PushAvStreamTransportServerLogic::FindStreamTransportConnection(const uint16_t connectionID)
{
    for (auto & transportConnection : mCurrentConnections)
    {
        if (transportConnection.connectionID == connectionID)
        {
            return &transportConnection;
        }
    }
    return nullptr;
}

TransportConfigurationStorage *
PushAvStreamTransportServerLogic::FindStreamTransportConnectionWithinFabric(const uint16_t connectionID, FabricIndex fabricIndex)
{
    for (auto & transportConnection : mCurrentConnections)
    {
        if (transportConnection.connectionID == connectionID)
        {
            if (transportConnection.GetFabricIndex() == fabricIndex)
            {
                return &transportConnection;
            }
        }
    }
    return nullptr;
}

uint16_t PushAvStreamTransportServerLogic::GenerateConnectionID()
{
    static uint16_t lastID = 0;

    for (uint16_t i = 0; i < kMaxConnectionId; ++i)
    {
        uint16_t candidateID = static_cast<uint16_t>((lastID + i + 1) % kMaxConnectionId); // Wrap from 0 to 65534
        if (FindStreamTransportConnection(candidateID) == nullptr)
        {
            lastID = candidateID;
            return candidateID;
        }
    }

    return kMaxConnectionId; // All 0 to 65534 IDs are in use
}

void PushAvStreamTransportServerLogic::PushAVStreamTransportDeallocateCallback(System::Layer *, void * callbackContext)
{
    PushAVStreamTransportDeallocateCallbackContext * transportDeallocateContext =
        static_cast<PushAVStreamTransportDeallocateCallbackContext *>((callbackContext));

    uint16_t connectionID = transportDeallocateContext->connectionID;

    // Call the delegate
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
        transportDeallocateContext->instance->mDelegate->DeallocatePushTransport(connectionID));

    if (delegateStatus.IsSuccess())
    {
        ChipLogProgress(Zcl, "Push AV Stream Transport Deallocate timer expired. %s", "Deallocating");

        // Remove connection from CurrentConnections
        CHIP_ERROR err = transportDeallocateContext->instance->RemoveStreamTransportConnection(connectionID);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to remove transport connection: %" CHIP_ERROR_FORMAT, err.Format());
        }
        transportDeallocateContext->instance->RemoveTimerAppState(connectionID);
    }
    else
    {
        ChipLogError(Zcl, "Push AV Stream Transport Deallocate timer expired. %s", "Deallocation Failed");
    }
}

bool PushAvStreamTransportServerLogic::ValidateUrl(const std::string & url)
{
    UriUriA uri;
    const char * errorPos;
    int result = uriParseSingleUriA(&uri, url.c_str(), &errorPos);

    // Check if URI parsing failed
    if (result != URI_SUCCESS)
    {
        // No need to call uriFreeUriMembersA on failure
        ChipLogError(Zcl, "PushAVStreamTransport URL : %s is not RFC 3986 compliant", url.c_str());
        return false;
    }

    // Extract components
    std::string scheme = Internal::extractTextRange(uri.scheme);
    std::string host   = Internal::extractTextRange(uri.hostText);
    std::string path   = Internal::extractPath(uri.pathHead);

    // Free URI structure
    uriFreeUriMembersA(&uri);

    // Convert scheme to lowercase for case-insensitive comparison
    for (char & c : scheme)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (scheme != "https")
    {
        ChipLogError(Camera, "URL does not have a https scheme");
        return false;
    }

    if (host.empty())
    {
        ChipLogError(Camera, "URL does not contain a valid host.");
        return false;
    }

    bool pathEndsWithSlash = !path.empty() && path.back() == '/';
    if (!pathEndsWithSlash)
    {
        ChipLogError(Camera, "URL does not end with '/'");
        return false;
    }

    bool noQuery = url.find('?') == std::string::npos;
    if (!noQuery)
    {
        ChipLogError(Camera, "URL contains query character '?'");
        return false;
    }

    bool noFragment = url.find('#') == std::string::npos;
    if (!noFragment)
    {
        ChipLogError(Camera, "URL contains fragment character '#'");
        return false;
    }

    return true;
}

CHIP_ERROR PushAvStreamTransportServerLogic::ScheduleTransportDeallocate(uint16_t connectionID, uint32_t timeoutSec)
{
    uint32_t timeoutMs = timeoutSec * MILLISECOND_TICKS_PER_SECOND;

    std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext> transportDeallocateContext{ new (
        std::nothrow) PushAVStreamTransportDeallocateCallbackContext{ this, connectionID } };

    if (transportDeallocateContext == nullptr)
    {
        ChipLogError(Zcl, "Failed to allocate memory for deallocate context");
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(timeoutMs),
                                                           PushAVStreamTransportDeallocateCallback,
                                                           static_cast<void *>(transportDeallocateContext.get()));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to schedule deallocate: timeout=%" PRIu32 ", status=%" CHIP_ERROR_FORMAT, timeoutSec,
                     err.Format());
    }
    else
    {
        mTimerContexts.push_back(transportDeallocateContext);
    }

    return err;
}

Status
PushAvStreamTransportServerLogic::ValidateZoneSensitivity(const Structs::TransportZoneOptionsStruct::DecodableType & zone) const
{
    if (mFeatures.Has(Feature::kPerZoneSensitivity))
    {
        VerifyOrReturnValue(zone.sensitivity.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl, "Transport zone missing sensitivity on endpoint %u", mEndpointId));
        VerifyOrReturnValue(zone.sensitivity.Value() >= 1 && zone.sensitivity.Value() <= 10, Status::ConstraintError,
                            ChipLogError(Zcl, "Transport zone sensitivity out of range on endpoint %u", mEndpointId));
    }
    else
    {
        VerifyOrReturnValue(!zone.sensitivity.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl, "Transport zone sensitivity is not supported on endpoint %u", mEndpointId));
    }
    return Status::Success;
}

Status PushAvStreamTransportServerLogic::ValidateIncomingTransportOptions(
    const Structs::TransportOptionsStruct::DecodableType & transportOptions)
{
    // Contraints check on incoming transport Options
    VerifyOrReturnValue(
        transportOptions.streamUsage != StreamUsageEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid streamUsage ", mEndpointId));

    // Check for video stream list count constraint (spec: max 16)
    if (transportOptions.videoStreams.HasValue())
    {
        size_t vsCount   = 0;
        CHIP_ERROR vsErr = transportOptions.videoStreams.Value().ComputeSize(&vsCount);
        VerifyOrReturnValue(vsErr == CHIP_NO_ERROR, Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: VideoStreams list TLV "
                                         "validation failed: %" CHIP_ERROR_FORMAT,
                                         mEndpointId, vsErr.Format()));
        VerifyOrReturnValue(vsCount <= kMaxVideoStreams, Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: VideoStreams count %u "
                                         "exceeds max %u",
                                         mEndpointId, static_cast<unsigned>(vsCount), static_cast<unsigned>(kMaxVideoStreams)));

        for (auto iter = transportOptions.videoStreams.Value().begin(); iter.Next();)
        {
            auto streamName = iter.GetValue().videoStreamName;
            VerifyOrReturnValue(streamName.size() >= kMinStreamNameLength && streamName.size() <= kMaxStreamNameLength,
                                Status::ConstraintError,
                                ChipLogError(Zcl,
                                             "Transport Options verification from command data[ep=%d]: Video Stream Name "
                                             "Constraint Error",
                                             mEndpointId));
        }
    }

    // Check for audio stream list count constraint (spec: max 16)
    if (transportOptions.audioStreams.HasValue())
    {
        size_t asCount   = 0;
        CHIP_ERROR asErr = transportOptions.audioStreams.Value().ComputeSize(&asCount);
        VerifyOrReturnValue(asErr == CHIP_NO_ERROR, Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: AudioStreams list TLV "
                                         "validation failed: %" CHIP_ERROR_FORMAT,
                                         mEndpointId, asErr.Format()));
        VerifyOrReturnValue(asCount <= kMaxAudioStreams, Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: AudioStreams count %u "
                                         "exceeds max %u",
                                         mEndpointId, static_cast<unsigned>(asCount), static_cast<unsigned>(kMaxAudioStreams)));

        for (auto iter = transportOptions.audioStreams.Value().begin(); iter.Next();)
        {
            auto streamName = iter.GetValue().audioStreamName;
            VerifyOrReturnValue(streamName.size() >= kMinStreamNameLength && streamName.size() <= kMaxStreamNameLength,
                                Status::ConstraintError,
                                ChipLogError(Zcl,
                                             "Transport Options verification from command data[ep=%d]: Audio Stream Name "
                                             "Constraint Error",
                                             mEndpointId));
        }
    }

    VerifyOrReturnValue(transportOptions.TLSEndpointID <= kMaxEndpointId, Status::ConstraintError,
                        ChipLogError(Zcl,
                                     "Transport Options verification from command data[ep=%d]: EndpointID field Constraint Error",
                                     mEndpointId));

    VerifyOrReturnValue(transportOptions.url.size() >= kMinUrlLength && transportOptions.url.size() <= kMaxUrlLength,
                        Status::ConstraintError,
                        ChipLogError(Zcl,
                                     "Transport Options verification from command data[ep=%d]: URL length: %" PRIu32
                                     " not in allowed length range of 13 to 2000",
                                     mEndpointId, static_cast<uint32_t>(transportOptions.url.size())));

    auto & triggerOptions = transportOptions.triggerOptions;
    if ((triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion ||
         triggerOptions.triggerType == TransportTriggerTypeEnum::kAmbientContext) &&
        triggerOptions.motionZones.HasValue() && !triggerOptions.motionZones.Value().IsNull())
    {
        VerifyOrReturnValue(Internal::RequiredZoneFieldReader(triggerOptions.motionZones.Value().Value()).Validate() ==
                                CHIP_NO_ERROR,
                            Status::InvalidCommand);
    }

    VerifyOrReturnValue(
        triggerOptions.triggerType != TransportTriggerTypeEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid triggerType ", mEndpointId));

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion ||
        triggerOptions.triggerType == TransportTriggerTypeEnum::kAmbientContext)
    {
        VerifyOrReturnValue(
            triggerOptions.motionZones.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Missing motion zones ", mEndpointId));
        if (!triggerOptions.motionZones.Value().IsNull())
        {
            auto & motionZonesList = triggerOptions.motionZones;
            auto iter              = motionZonesList.Value().Value().begin();

            while (iter.Next())
            {
                const auto status = ValidateZoneSensitivity(iter.GetValue());
                VerifyOrReturnValue(status == Status::Success, status);
            }

            if (iter.GetStatus() != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Motion Zones TLV Validation failed",
                             mEndpointId);
                return Status::InvalidCommand;
            }
        }
    }

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion)
    {
        VerifyOrReturnValue(triggerOptions.motionTimeControl.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: Missing Motion Time Control ",
                                         mEndpointId));

        VerifyOrReturnValue(
            triggerOptions.motionTimeControl.Value().initialDuration >= 1, Status::ConstraintError,
            ChipLogError(
                Zcl,
                "Transport Options verification from command data[ep=%d]: Motion Time Control (InitialDuration) Constraint Error",
                mEndpointId));

        VerifyOrReturnValue(
            triggerOptions.motionTimeControl.Value().maxDuration >= 1, Status::ConstraintError,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Motion Time Control (MaxDuration) Constraint Error",
                mEndpointId));
    }
    else if (triggerOptions.triggerType == TransportTriggerTypeEnum::kAmbientContext)
    {
        VerifyOrReturnValue(
            !triggerOptions.motionTimeControl.HasValue(), Status::InvalidCommand,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Found Motion Time Control which is not expected ",
                mEndpointId));
    }
    else
    {
        VerifyOrReturnValue(
            !triggerOptions.motionZones.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Found motion zones which is not expected",
                         mEndpointId));

        VerifyOrReturnValue(
            !triggerOptions.motionTimeControl.HasValue(), Status::InvalidCommand,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Found Motion Time Control which is not expected ",
                mEndpointId));
    }

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion && !mFeatures.Has(Feature::kPerZoneSensitivity))
    {
        VerifyOrReturnValue(
            triggerOptions.motionSensitivity.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Missing Motion Sensitivity ", mEndpointId));

        if (!triggerOptions.motionSensitivity.Value().IsNull())
        {
            VerifyOrReturnValue(
                triggerOptions.motionSensitivity.Value().Value() >= 1 && triggerOptions.motionSensitivity.Value().Value() <= 10,
                Status::ConstraintError,
                ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Motion Sensitivity Constraint Error",
                             mEndpointId));
        }
    }
    else
    {
        VerifyOrReturnValue(
            !triggerOptions.motionSensitivity.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl,
                         "Transport Options verification from command data[ep=%d]: Found Motion Sensitivity which is not expected ",
                         mEndpointId));
    }

    if (triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion ||
        triggerOptions.triggerType == TransportTriggerTypeEnum::kAmbientContext ||
        triggerOptions.triggerType == TransportTriggerTypeEnum::kCommand)
    {
        VerifyOrReturnValue(triggerOptions.maxPreRollLen.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: Missing Max Pre Roll Len field ",
                                         mEndpointId));
    }
    else
    {
        VerifyOrReturnValue(
            !triggerOptions.maxPreRollLen.HasValue(), Status::InvalidCommand,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Found Max Pre Roll Len field which is not expected.",
                mEndpointId));
    }

    IngestMethodsEnum ingestMethod = transportOptions.ingestMethod;

    VerifyOrReturnValue(
        ingestMethod != IngestMethodsEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid Ingest Method ", mEndpointId));

    const Structs::ContainerOptionsStruct::Type & containerOptions = transportOptions.containerOptions;

    VerifyOrReturnValue(
        containerOptions.containerType != ContainerFormatEnum::kUnknownEnumValue, Status::ConstraintError,
        ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid Container Format ", mEndpointId));

    if (containerOptions.containerType == ContainerFormatEnum::kCmaf)
    {
        VerifyOrReturnValue(containerOptions.CMAFContainerOptions.HasValue(), Status::InvalidCommand,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: Missing CMAF Container Options ",
                                         mEndpointId));

        CMAFInterfaceEnum cmafInterface = containerOptions.CMAFContainerOptions.Value().CMAFInterface;

        VerifyOrReturnValue(
            cmafInterface == CMAFInterfaceEnum::kInterface1 || cmafInterface == CMAFInterfaceEnum::kInterface2DASH ||
                cmafInterface == CMAFInterfaceEnum::kInterface2HLS,
            Status::ConstraintError,
            ChipLogError(Zcl,
                         "Transport Options verification from command data[ep=%d]: CMAF Container Options CMAFInterface not valid",
                         mEndpointId));

        uint16_t segmentDuration = containerOptions.CMAFContainerOptions.Value().segmentDuration;

        VerifyOrReturnValue(segmentDuration >= kMinSegmentDuration && segmentDuration <= kMaxSegmentDuration,
                            Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: CMAF Container Options Segment "
                                         "Duration field not within allowed range",
                                         mEndpointId));

        uint16_t chunkDuration = containerOptions.CMAFContainerOptions.Value().chunkDuration;

        VerifyOrReturnValue(chunkDuration <= segmentDuration / 2, Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "Transport Options verification from command data[ep=%d]: CMAF Container Options Chunk "
                                         "Duration field not within allowed range",
                                         mEndpointId));

        if (!mFeatures.Has(Feature::kMetadata))
        {
            VerifyOrReturnValue(!containerOptions.CMAFContainerOptions.Value().metadataEnabled.HasValue(), Status::InvalidCommand,
                                ChipLogError(Zcl,
                                             "Transport Options verification from command data[ep=%d]: Found CMAF Container "
                                             "Options MetadataEnabled which is not expected.",
                                             mEndpointId));
        }
    }
    else
    {
        VerifyOrReturnValue(
            !containerOptions.CMAFContainerOptions.HasValue(), Status::InvalidCommand,
            ChipLogError(
                Zcl, "Transport Options verification from command data[ep=%d]: Found CMAF Container Options which is not expected ",
                mEndpointId));
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> PushAvStreamTransportServerLogic::ValidateStreamParameters(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const PushAvStreamTransport::Structs::TransportOptionsStruct::DecodableType & transportOptions,
    const std::shared_ptr<PushAvStreamTransport::TransportOptionsStorage> transportOptionsPtr)
{

    // Check mutual exclusivity and presence of stream fields
    auto listHasElements = [](const auto & list) {
        size_t s = 0;
        return list.ComputeSize(&s) == CHIP_NO_ERROR && s > 0;
    };
    bool hasNewStreamFields =
        (transportOptions.videoStreams.HasValue() && listHasElements(transportOptions.videoStreams.Value())) ||
        (transportOptions.audioStreams.HasValue() && listHasElements(transportOptions.audioStreams.Value()));
    bool hasOldStreamFields = transportOptions.videoStreamID.HasValue() || transportOptions.audioStreamID.HasValue();

    VerifyOrReturnValue(!(hasNewStreamFields && hasOldStreamFields), Status::InvalidCommand,
                        ChipLogError(Zcl, "Transport Options[ep=%d]: Both new and old stream fields are present", mEndpointId));

    VerifyOrReturnValue(hasNewStreamFields || hasOldStreamFields, Status::InvalidCommand,
                        ChipLogError(Zcl, "Transport Options[ep=%d]: Missing stream fields", mEndpointId));

    // Validate new multistream fields
    if (hasNewStreamFields)
    {
        std::set<std::string> videoStreamNames;

        // Validate videoStreams
        uint16_t videoStreamsCount = 0;
        if (transportOptions.videoStreams.HasValue())
        {
            std::set<uint16_t> videoStreamIDs;

            for (auto iter = transportOptions.videoStreams.Value().begin(); iter.Next();)
            {
                auto & videoStream = iter.GetValue();

                videoStreamsCount++;
                videoStreamNames.insert(std::string(videoStream.videoStreamName.data(), videoStream.videoStreamName.size()));
                videoStreamIDs.insert(videoStream.videoStreamID);
            }

            // Check for duplicates within video streams
            if ((videoStreamNames.size() != videoStreamsCount) || (videoStreamIDs.size() != videoStreamsCount))
            {
                auto status = to_underlying(StatusCodeEnum::kDuplicateStreamValues);
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Duplicate videoStreamName or videoStreamID found",
                             mEndpointId);
                TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
                return std::nullopt;
            }

            for (auto iter = transportOptions.videoStreams.Value().begin(); iter.Next();)
            {
                auto & videoStream = iter.GetValue();
                // Validate videoStreamID
                auto status = Protocols::InteractionModel::ClusterStatusCode(mDelegate->SetVideoStream(videoStream.videoStreamID));
                VerifyOrReturnValue(status.IsSuccess(), Status::InvalidCommand,
                                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Video Stream ID", mEndpointId));
            }
        }

        // Validate audioStreams
        uint16_t audioStreamsCount = 0;
        if (transportOptions.audioStreams.HasValue())
        {
            std::set<std::string> audioStreamNames;
            std::set<uint16_t> audioStreamIDs;

            // Check against video stream names
            for (auto iter = transportOptions.audioStreams.Value().begin(); iter.Next();)
            {
                auto & audioStream = iter.GetValue();

                audioStreamsCount++;
                audioStreamNames.insert(std::string(audioStream.audioStreamName.data(), audioStream.audioStreamName.size()));
                audioStreamIDs.insert(audioStream.audioStreamID);
            }

            // Check for duplicates within audio streams
            if ((audioStreamNames.size() != audioStreamsCount) || (audioStreamIDs.size() != audioStreamsCount))
            {
                auto status = to_underlying(StatusCodeEnum::kDuplicateStreamValues);
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Duplicate audioStreamName or audioStreamID found",
                             mEndpointId);
                TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
                return std::nullopt;
            }

            for (auto iter = transportOptions.audioStreams.Value().begin(); iter.Next();)
            {
                auto & audioStream = iter.GetValue();
                // Check for duplicate with video streams
                if (videoStreamNames.find(std::string(audioStream.audioStreamName.data(), audioStream.audioStreamName.size())) !=
                    videoStreamNames.end())
                {
                    auto status = to_underlying(StatusCodeEnum::kInvalidStream);
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: AudioStreamName conflicts with video stream",
                                 mEndpointId);
                    TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
                    return std::nullopt;
                }

                // Validate audioStreamID
                auto status = Protocols::InteractionModel::ClusterStatusCode(mDelegate->SetAudioStream(audioStream.audioStreamID));
                VerifyOrReturnValue(status.IsSuccess(), Status::InvalidCommand,
                                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Audio Stream ID", mEndpointId));
            }
        }

        // Check if both videoStreams and audioStreams are empty
        if (videoStreamsCount == 0 && audioStreamsCount == 0)
        {
            auto status = to_underlying(StatusCodeEnum::kInvalidStream);
            ChipLogError(Zcl, "Transport Options[ep=%d]: Both VideoStreams and AudioStreams empty", mEndpointId);
            TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
            return std::nullopt;
        }
    }

    // Validate old format single stream fields
    if (hasOldStreamFields)
    {

        if (transportOptions.videoStreamID.HasValue())
        {
            uint16_t finalVideoStreamID;

            if (transportOptions.videoStreamID.Value().IsNull())
            {
                auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                    mDelegate->SelectVideoStream(transportOptions.streamUsage, finalVideoStreamID));
                if (!delegateStatus.IsSuccess())
                {
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: AllocatedVideoStreamsAttribute is empty", mEndpointId);
                    handler.AddStatus(commandPath, Status::InvalidInState);
                    return std::nullopt;
                }

                transportOptionsPtr->videoStreamID.SetValue(finalVideoStreamID);
            }
            else
            {
                auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                    mDelegate->SetVideoStream(transportOptions.videoStreamID.Value().Value()));

                if (!delegateStatus.IsSuccess())
                {
                    auto cluster_status = to_underlying(StatusCodeEnum::kInvalidStream);
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Video Stream ", mEndpointId);
                    TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, cluster_status);
                    return std::nullopt;
                }

                finalVideoStreamID = transportOptions.videoStreamID.Value().Value();
            }

            // Create new video stream entry with name 'video' and the provided or selected VideoStreamID i.e; finalVideoStreamID
            Structs::VideoStreamStruct::Type newVideoStream;
            newVideoStream.videoStreamName = "video"_span;
            newVideoStream.videoStreamID   = finalVideoStreamID;

            // Add to storage and update the list
            VerifyOrReturnValue(transportOptionsPtr->AddVideoStream(newVideoStream) == CHIP_NO_ERROR, Status::Failure,
                                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Failed to add video stream", mEndpointId));
        }

        if (transportOptions.audioStreamID.HasValue())
        {
            uint16_t finalAudioStreamID;

            if (transportOptions.audioStreamID.Value().IsNull())
            {
                auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                    mDelegate->SelectAudioStream(transportOptions.streamUsage, finalAudioStreamID));
                if (!delegateStatus.IsSuccess())
                {
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: AllocatedAudioStreamsAttribute is empty", mEndpointId);
                    handler.AddStatus(commandPath, Status::InvalidInState);
                    return std::nullopt;
                }

                transportOptionsPtr->audioStreamID.SetValue(finalAudioStreamID);
            }
            else
            {
                auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(
                    mDelegate->SetAudioStream(transportOptions.audioStreamID.Value().Value()));

                if (!delegateStatus.IsSuccess())
                {
                    auto cluster_status = to_underlying(StatusCodeEnum::kInvalidStream);
                    ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Audio Stream ", mEndpointId);
                    TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, cluster_status);
                    return std::nullopt;
                }

                finalAudioStreamID = transportOptions.audioStreamID.Value().Value();
            }

            // Create new audio stream entry with name 'audio' and the provided or selected AudioStreamID i.e; finalAudioStreamID
            Structs::AudioStreamStruct::Type newAudioStream;
            newAudioStream.audioStreamName = "audio"_span;
            newAudioStream.audioStreamID   = finalAudioStreamID;

            // Add to storage and update the list
            VerifyOrReturnValue(transportOptionsPtr->AddAudioStream(newAudioStream) == CHIP_NO_ERROR, Status::Failure,
                                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Failed to add audio stream", mEndpointId));
        }
    }

    return Status::Success;
}

DataModel::ActionReturnStatus
PushAvStreamTransportServerLogic::ValidateMotionZones(const Structs::TransportTriggerOptionsStruct::DecodableType & triggerOptions)
{
    if ((triggerOptions.triggerType == TransportTriggerTypeEnum::kMotion ||
         triggerOptions.triggerType == TransportTriggerTypeEnum::kAmbientContext) &&
        triggerOptions.motionZones.HasValue() && !triggerOptions.motionZones.Value().IsNull())
    {
        const auto & zones  = triggerOptions.motionZones.Value().Value();
        size_t zoneListSize = 0;
        VerifyOrReturnValue(zones.ComputeSize(&zoneListSize) == CHIP_NO_ERROR, Status::InvalidCommand);

        bool nullFound     = false;
        bool duplicateNull = false;
        std::array<uint16_t, 256> zoneIds{};
        size_t zoneIdCount  = 0;
        uint32_t zoneGroups = 0;
        auto decodeIter     = zones.begin();
        while (decodeIter.Next())
        {
            const auto & zone = decodeIter.GetValue().zone;
            if (zone.IsNull())
            {
                duplicateNull = duplicateNull || nullFound;
                nullFound     = true;
            }
            else
            {
                if (zoneIdCount < zoneIds.size())
                {
                    zoneIds[zoneIdCount] = zone.Value();
                }
                ++zoneIdCount;
                zoneGroups |= uint32_t{ 1 } << (zone.Value() >> 12);
            }
        }
        VerifyOrReturnValue(decodeIter.GetStatus() == CHIP_NO_ERROR, Status::InvalidCommand);
        VerifyOrReturnValue(!duplicateNull, Status::AlreadyExists);

        if (zoneIdCount <= zoneIds.size())
        {
            auto end = zoneIds.data() + zoneIdCount;
            std::sort(zoneIds.data(), end);
            VerifyOrReturnValue(std::adjacent_find(zoneIds.data(), end) == end, Status::AlreadyExists);
        }
        else
        {
            VerifyOrReturnValue(!HasDuplicateLargeMotionZoneIds(zones, zoneGroups, Span<uint16_t>(zoneIds.data(), zoneIds.size())),
                                Status::AlreadyExists);
        }

        VerifyOrReturnValue(
            mDelegate->ValidateMotionZoneListSize(zoneListSize), Status::DynamicConstraintError,
            ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid Motion Zone Size ", mEndpointId));

        auto iter = zones.begin();
        while (iter.Next())
        {
            const auto & transportZoneOption = iter.GetValue();
            if (!transportZoneOption.zone.IsNull() &&
                mDelegate->ValidateZoneId(transportZoneOption.zone.Value()) != Status::Success)
            {
                auto status = to_underlying(StatusCodeEnum::kInvalidZone);
                ChipLogError(Zcl, "Transport Options verification from command data[ep=%d]: Invalid ZoneId", mEndpointId);
                return ClusterStatusCode::ClusterSpecificFailure(status);
            }
        }
        VerifyOrReturnValue(iter.GetStatus() == CHIP_NO_ERROR, Status::InvalidCommand);
    }
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleAllocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                              const Commands::AllocatePushTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    // Enforce CHIP_CONFIG_MAX_NUM_PUSH_TRANSPORTS as a per-fabric limit.
    size_t numConnectionsForFabric = 0;
    for (const auto & entry : mCurrentConnections)
    {
        // Each entry is keyed by connection ID; the value stores the fabric index.
        if (entry.GetFabricIndex() == handler.GetAccessingFabricIndex())
        {
            ++numConnectionsForFabric;
        }
    }

    if (numConnectionsForFabric >= CHIP_CONFIG_MAX_NUM_PUSH_TRANSPORTS)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Maximum number of connections reached for fabric", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    Commands::AllocatePushTransportResponse::Type response;
    auto & transportOptions = commandData.transportOptions;

    // Handle constraint checks against the provided command fields
    Status transportOptionsValidityStatus = ValidateIncomingTransportOptions(transportOptions);

    VerifyOrDo(transportOptionsValidityStatus == Status::Success, {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: TransportOptions of command data is not Valid", mEndpointId);
        handler.AddStatus(commandPath, transportOptionsValidityStatus);
        return std::nullopt;
    });

    // Validate the TLS Endpoint
    if (mTLSClientManagementDelegate != nullptr)
    {
        // Note: TLS clusters are always on EP0, per spec
        //
        CHIP_ERROR tlsEndpointValidityStatus = mTLSClientManagementDelegate->FindProvisionedEndpointByID(
            kRootEndpointId, handler.GetAccessingFabricIndex(), commandData.transportOptions.TLSEndpointID,
            [&](auto & TLSEndpoint) -> CHIP_ERROR {
                // The spec requires a push transport's TLS endpoint to carry a non-null ccdid; a null one means no
                // client certificate is associated with the endpoint, which the caller reports as an invalid TLS endpoint.
                VerifyOrReturnError(!TLSEndpoint.ccdid.IsNull(), CHIP_ERROR_INVALID_ARGUMENT,
                                    ChipLogError(Zcl,
                                                 "HandleAllocatePushTransport[ep=%d]: TLS endpoint %u has no client certificate",
                                                 mEndpointId, commandData.transportOptions.TLSEndpointID));

                // Use heap allocation for large certificate buffers to reduce stack usage
                auto rootCertBuffer   = std::make_unique<PersistenceBuffer<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES>>();
                auto clientCertBuffer = std::make_unique<PersistenceBuffer<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES>>();

                Tls::CertificateTable::BufferedClientCert clientCertEntry(*clientCertBuffer);
                Tls::CertificateTable::BufferedRootCert rootCertEntry(*rootCertBuffer);

                if (mTLSCertificateManagementDelegate != nullptr)
                {
                    auto & table = mTLSCertificateManagementDelegate->GetCertificateTable();
                    ReturnErrorOnFailure(table.GetClientCertificateEntry(handler.GetAccessingFabricIndex(),
                                                                         TLSEndpoint.ccdid.Value(), clientCertEntry));
                    ReturnErrorOnFailure(
                        table.GetRootCertificateEntry(handler.GetAccessingFabricIndex(), TLSEndpoint.caid, rootCertEntry));
                    mDelegate->SetTLSCerts(clientCertEntry, rootCertEntry);
                }
                else
                {
                    // For tests, create empty cert entries
                    mDelegate->SetTLSCerts(clientCertEntry, rootCertEntry);
                }
                return CHIP_NO_ERROR;
            });

        VerifyOrDo(tlsEndpointValidityStatus == CHIP_NO_ERROR, {
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: TLSEndpointID of command data is not valid/Provisioned",
                         mEndpointId);
            auto status = to_underlying(StatusCodeEnum::kInvalidTLSEndpoint);
            TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
            return std::nullopt;
        });
    }
    else
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: TLS Client Management Delegate is not set", mEndpointId);
        auto status = to_underlying(StatusCodeEnum::kInvalidTLSEndpoint);
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    // IngestMethod has alread been validated against the constraints above, this is handling the subsequent required spec logic
    // to cross-reference use against the ContainerType
    IngestMethodsEnum ingestMethod = commandData.transportOptions.ingestMethod;

    bool isFormatSupported = false;

    for (auto & supportsFormat : mSupportedFormats)
    {
        if ((supportsFormat.ingestMethod == ingestMethod) &&
            (supportsFormat.containerFormat == commandData.transportOptions.containerOptions.containerType))
        {
            isFormatSupported = true;
            break;
        }
    }

    if (isFormatSupported == false)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidCombination);
        ChipLogError(Zcl,
                     "HandleAllocatePushTransport[ep=%d]: Invalid Ingest Method and Container Format Combination : (Ingest Method: "
                     "%02X and Container Format: %02X)",
                     mEndpointId, to_underlying(ingestMethod),
                     to_underlying(commandData.transportOptions.containerOptions.containerType));
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    if (transportOptions.containerOptions.containerType == ContainerFormatEnum::kCmaf &&
        transportOptions.containerOptions.CMAFContainerOptions.HasValue())
    {
        bool isValidUrl = ValidateUrl(std::string(transportOptions.url.data(), transportOptions.url.size()));

        if (isValidUrl == false)
        {
            auto status = to_underlying(StatusCodeEnum::kInvalidURL);
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Invalid Url", mEndpointId);
            TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
            return std::nullopt;
        }
    }

    bool isValidStreamUsage = mDelegate->ValidateStreamUsage(transportOptions.streamUsage);
    if (isValidStreamUsage == false)
    {
        auto status = to_underlying(StatusCodeEnum::kInvalidStreamUsage);
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: StreamUsage not present in the StreamUsagePriorities list",
                     mEndpointId);
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, status);
        return std::nullopt;
    }

    auto zoneStatus = ValidateMotionZones(transportOptions.triggerOptions);
    if (zoneStatus != Status::Success)
    {
        return zoneStatus;
    }

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    if (transportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Memory Allocation failed for transportOptions", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    auto validateStreamStatus = ValidateStreamParameters(handler, commandPath, transportOptions, transportOptionsPtr);

    if (validateStreamStatus != Status::Success)
    {
        ChipLogError(Camera, "HandleAllocatePushTransport[ep=%d]: Stream validation failed", mEndpointId);
        return validateStreamStatus;
    }

    if (transportOptions.containerOptions.containerType == ContainerFormatEnum::kCmaf &&
        transportOptions.containerOptions.CMAFContainerOptions.HasValue())
    {
        // SegmentDuration validation is restricted to video streams because SegmentDuration must be a multiple of
        // KeyFrameInterval. See spec issue: https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/12322
        if (transportOptionsPtr->videoStreamID.HasValue())
        {
            bool isValidSegmentDuration = mDelegate->ValidateSegmentDuration(
                transportOptions.containerOptions.CMAFContainerOptions.Value().segmentDuration, transportOptionsPtr->videoStreamID);
            if (isValidSegmentDuration == false)
            {
                auto segmentDurationStatus = to_underlying(StatusCodeEnum::kInvalidOptions);
                ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Segment Duration not within allowed range", mEndpointId);
                TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, segmentDurationStatus);
                return std::nullopt;
            }
        }
    }

    // Validate Bandwidth Requirement
    Optional<DataModel::Nullable<uint16_t>> firstVideoStreamID;
    Optional<DataModel::Nullable<uint16_t>> firstAudioStreamID;

    if (transportOptions.videoStreams.HasValue())
    {
        auto videoIter = transportOptions.videoStreams.Value().begin();
        if (videoIter.Next())
        {
            firstVideoStreamID.SetValue(videoIter.GetValue().videoStreamID);
            transportOptionsPtr->videoStreamID = firstVideoStreamID;
        }
    }
    if (transportOptions.audioStreams.HasValue())
    {
        auto audioIter = transportOptions.audioStreams.Value().begin();
        if (audioIter.Next())
        {
            firstAudioStreamID.SetValue(audioIter.GetValue().audioStreamID);
            transportOptionsPtr->audioStreamID = firstAudioStreamID;
        }
    }

    Status retStatus = mDelegate->ValidateBandwidthLimit(transportOptions.streamUsage, firstVideoStreamID, firstAudioStreamID);

    if (retStatus != Status::Success)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Resource Exhausted", mEndpointId);
        handler.AddStatus(commandPath, retStatus);
        return std::nullopt;
    }

    // Validate MaxPreRollLength constraint
    if (transportOptionsPtr->videoStreamID.HasValue() && transportOptions.triggerOptions.maxPreRollLen.HasValue())
    {
        uint16_t maxPreRollLength = transportOptions.triggerOptions.maxPreRollLen.Value();
        if (maxPreRollLength != 0 &&
            !mDelegate->ValidateMaxPreRollLength(maxPreRollLength, transportOptionsPtr->videoStreamID.Value()))
        {
            auto maxPreRollLengthStatus = to_underlying(StatusCodeEnum::kInvalidPreRollLength);
            ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: MaxPreRollLength (%u) validation failed", mEndpointId,
                         maxPreRollLength);
            TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, maxPreRollLengthStatus);
            return std::nullopt;
        }
    }

    uint16_t connectionID = GenerateConnectionID();

    if (connectionID == kMaxConnectionId)
    {
        ChipLogError(Zcl, "HandleAllocatePushTransport[ep=%d]: Max Connections Exhausted", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    FabricIndex accessingFabricIndex = handler.GetAccessingFabricIndex();

    retStatus = mDelegate->AllocatePushTransport(*transportOptionsPtr, connectionID, accessingFabricIndex);

    if (retStatus == Status::Success)
    {
        // add connection to CurrentConnections
        TransportConfigurationStorage outTransportConfiguration(connectionID, transportOptionsPtr);

        outTransportConfiguration.transportStatus = TransportStatusEnum::kInactive;

        outTransportConfiguration.SetFabricIndex(accessingFabricIndex);

        if (UpsertStreamTransportConnection(outTransportConfiguration) == UpsertResultEnum::kFailed)
        {
            handler.AddStatus(commandPath, Status::Failure);
            return std::nullopt;
        }

        response.transportConfiguration = outTransportConfiguration;

        // ExpiryTime Handling
        if (transportOptions.expiryTime.HasValue())
        {
            TEMPORARY_RETURN_IGNORED ScheduleTransportDeallocate(connectionID, transportOptions.expiryTime.Value());
        }

        handler.AddResponse(commandPath, response);
    }
    else
    {
        handler.AddStatus(commandPath, retStatus);
    }

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> PushAvStreamTransportServerLogic::HandleDeallocatePushTransport(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const Commands::DeallocatePushTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    uint16_t connectionID                                  = commandData.connectionID;
    FabricIndex FabricIndex                                = handler.GetAccessingFabricIndex();
    TransportConfigurationStorage * transportConfiguration = FindStreamTransportConnectionWithinFabric(connectionID, FabricIndex);
    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport[ep=%d]: ConnectionID Not Found.", mEndpointId);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    if (mDelegate->GetTransportBusyStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleDeallocatePushTransport[ep=%d]: Connection is Busy", mEndpointId);
        handler.AddStatus(commandPath, Status::Busy);
        return std::nullopt;
    }

    // Call the delegate
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(mDelegate->DeallocatePushTransport(connectionID));

    if (delegateStatus.IsSuccess())
    {
        // Remove connection from CurrentConnections
        CHIP_ERROR err = RemoveStreamTransportConnection(connectionID);
        if (err != CHIP_NO_ERROR)
        {
            handler.AddStatus(commandPath, Status::Failure);
            return std::nullopt;
        }
        RemoveTimerAppState(connectionID);
    }

    handler.AddStatus(commandPath, delegateStatus);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleModifyPushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                            const Commands::ModifyPushTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & transportOptions = commandData.transportOptions;

    // Contraints check on incoming transport Options
    Status transportOptionsValidityStatus = ValidateIncomingTransportOptions(transportOptions);

    VerifyOrDo(transportOptionsValidityStatus == Status::Success, {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: TransportOptions of command data is not Valid", mEndpointId);
        handler.AddStatus(commandPath, transportOptionsValidityStatus);
        return std::nullopt;
    });

    FabricIndex fabricIndex = handler.GetAccessingFabricIndex();

    TransportConfigurationStorage * transportConfiguration = FindStreamTransportConnectionWithinFabric(connectionID, fabricIndex);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: ConnectionID Not Found.", mEndpointId);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    if (mDelegate->GetTransportBusyStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: Connection is Busy", mEndpointId);
        handler.AddStatus(commandPath, Status::Busy);
        return std::nullopt;
    }

    auto zoneStatus = ValidateMotionZones(transportOptions.triggerOptions);
    if (zoneStatus != Status::Success)
    {
        return zoneStatus;
    }

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    if (transportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleModifyPushTransport[ep=%d]: Memory Allocation failed for transportOptions", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    auto validateStreamStatus = ValidateStreamParameters(handler, commandPath, transportOptions, transportOptionsPtr);

    if (validateStreamStatus != Status::Success)
    {
        ChipLogError(Camera, "HandleAllocatePushTransport[ep=%d]: Stream validation failed", mEndpointId);
        return validateStreamStatus;
    }

    // Call the delegate
    status = mDelegate->ModifyPushTransport(connectionID, *transportOptionsPtr);

    if (status == Status::Success)
    {
        auto oldTransportOptionsPtr = transportConfiguration->GetTransportOptionsPtr();
        transportConfiguration->SetTransportOptionsPtr(transportOptionsPtr);

        CHIP_ERROR err = StoreCurrentConnections();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to store modified connection, reverting: %" CHIP_ERROR_FORMAT, err.Format());
            transportConfiguration->SetTransportOptionsPtr(oldTransportOptionsPtr); // Revert
            status = Status::Failure;                                               // Reflect storage failure
        }
        else
        {
            mCluster->ReportAttributeChange(PushAvStreamTransport::Attributes::CurrentConnections::Id);
        }
    }

    handler.AddStatus(commandPath, status);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleSetTransportStatus(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                           const Commands::SetTransportStatus::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Status status                              = Status::Success;
    DataModel::Nullable<uint16_t> connectionID = commandData.connectionID;
    auto & transportStatus                     = commandData.transportStatus;

    VerifyOrDo(transportStatus != TransportStatusEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleSetTransportStatus[ep=%d]: Invalid Transport Status in command data", mEndpointId);
        handler.AddStatus(commandPath, Status::ConstraintError);
        return std::nullopt;
    });

    std::vector<uint16_t> connectionIDList;

    if (connectionID.IsNull())
    {
        for (auto & transportConnection : mCurrentConnections)
        {
            if (transportConnection.fabricIndex == handler.GetAccessingFabricIndex())
            {
                connectionIDList.push_back(transportConnection.connectionID);
            }
        }
    }
    else
    {
        FabricIndex fabricIndex = handler.GetAccessingFabricIndex();
        TransportConfigurationStorage * transportConfiguration =
            FindStreamTransportConnectionWithinFabric(connectionID.Value(), fabricIndex);
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleSetTransportStatus[ep=%d]: ConnectionID Not Found.", mEndpointId);
            handler.AddStatus(commandPath, Status::NotFound);
            return std::nullopt;
        }
        connectionIDList.push_back(connectionID.Value());
    }

    // Call the delegate
    status = mDelegate->SetTransportStatus(connectionIDList, transportStatus);
    if (status == Status::Success)
    {
        auto originalConnections = mCurrentConnections; // Keep a copy for potential revert

        for (auto & connID : connectionIDList)
        {
            for (auto & transportConnection : mCurrentConnections)
            {
                if (transportConnection.connectionID == connID)
                {
                    transportConnection.transportStatus = transportStatus;
                }
            }
        }

        CHIP_ERROR err = StoreCurrentConnections();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to store status update, reverting: %" CHIP_ERROR_FORMAT, err.Format());
            mCurrentConnections = originalConnections; // Revert
            status              = Status::Failure;     // Reflect storage failure
        }
        else
        {
            mCluster->ReportAttributeChange(PushAvStreamTransport::Attributes::CurrentConnections::Id);
        }
    }
    handler.AddStatus(commandPath, status);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> PushAvStreamTransportServerLogic::HandleManuallyTriggerTransport(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const Commands::ManuallyTriggerTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Status status           = Status::Success;
    uint16_t connectionID   = commandData.connectionID;
    auto & activationReason = commandData.activationReason;

    VerifyOrDo(activationReason != TriggerActivationReasonEnum::kUnknownEnumValue, {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Activation Reason ", mEndpointId);
        handler.AddStatus(commandPath, Status::ConstraintError);
        return std::nullopt;
    });

    Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> timeControl = commandData.timeControl;

    if (timeControl.HasValue())
    {
        VerifyOrDo(timeControl.Value().initialDuration >= 1, {
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Motion Time Control (InitialDuration) Constraint Error",
                         mEndpointId);
            handler.AddStatus(commandPath, Status::ConstraintError);
            return std::nullopt;
        });

        VerifyOrDo(timeControl.Value().maxDuration >= 1, {
            ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Motion Time Control (MaxDuration) Constraint Error",
                         mEndpointId);
            handler.AddStatus(commandPath, Status::ConstraintError);
            return std::nullopt;
        });
    }

    FabricIndex fabricIndex                                = handler.GetAccessingFabricIndex();
    TransportConfigurationStorage * transportConfiguration = FindStreamTransportConnectionWithinFabric(connectionID, fabricIndex);

    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: ConnectionID Not Found.", mEndpointId);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    // Note, we will always have Transport Options as they're mandatory in the initial allocate
    status = CheckPrivacyModes(transportConfiguration->transportOptions.Value().streamUsage);
    if (status != Status::Success)
    {
        handler.AddStatus(commandPath, status);
        return std::nullopt;
    }

    if (mDelegate->GetTransportBusyStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Connection is Busy", mEndpointId);
        handler.AddStatus(commandPath, Status::Busy);
        return std::nullopt;
    }

    if (transportConfiguration->transportStatus == TransportStatusEnum::kInactive)
    {
        auto clusterStatus = to_underlying(StatusCodeEnum::kInvalidTransportStatus);
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Transport status", mEndpointId);
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, clusterStatus);
        return std::nullopt;
    }

    if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType == TransportTriggerTypeEnum::kContinuous)
    {
        auto clusterStatus = to_underlying(StatusCodeEnum::kInvalidTriggerType);
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Invalid Trigger type", mEndpointId);
        TEMPORARY_RETURN_IGNORED handler.AddClusterSpecificFailure(commandPath, clusterStatus);
        return std::nullopt;
    }

    if (transportConfiguration->transportOptions.Value().triggerOptions.triggerType == TransportTriggerTypeEnum::kCommand &&
        !timeControl.HasValue())
    {
        ChipLogError(Zcl, "HandleManuallyTriggerTransport[ep=%d]: Time control field not present", mEndpointId);
        handler.AddStatus(commandPath, Status::DynamicConstraintError);
        return std::nullopt;
    }

    // When trigger type is motion in the allocated transport but triggering it manually
    if (!timeControl.HasValue())
    {
        timeControl = transportConfiguration->transportOptions.Value().triggerOptions.motionTimeControl;
    }

    // Call the delegate
    status = mDelegate->ManuallyTriggerTransport(connectionID, activationReason, timeControl);

    handler.AddStatus(commandPath, status);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
PushAvStreamTransportServerLogic::HandleFindTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                      const Commands::FindTransport::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    Commands::FindTransportResponse::Type response;

    DataModel::Nullable<uint16_t> connectionID = commandData.connectionID;

    std::vector<Structs::TransportConfigurationStruct::Type> transportConfigurations;

    if (connectionID.IsNull())
    {
        if (mCurrentConnections.size() == 0)
        {
            ChipLogError(Zcl, "HandleFindTransport[ep=%d]: ConnectionID not found", mEndpointId);
            handler.AddStatus(commandPath, Status::NotFound);
            return std::nullopt;
        }

        for (auto & connection : mCurrentConnections)
        {
            if (connection.fabricIndex == handler.GetAccessingFabricIndex())
            {
                transportConfigurations.push_back(connection);
            }
        }
    }
    else
    {
        FabricIndex fabricIndex = handler.GetAccessingFabricIndex();
        TransportConfigurationStorage * transportConfiguration =
            FindStreamTransportConnectionWithinFabric(connectionID.Value(), fabricIndex);
        if (transportConfiguration == nullptr)
        {
            ChipLogError(Zcl, "HandleFindTransport[ep=%d]: ConnectionID not found", mEndpointId);
            handler.AddStatus(commandPath, Status::NotFound);
            return std::nullopt;
        }

        transportConfigurations.push_back(*transportConfiguration);
    }

    if (transportConfigurations.size() == 0)
    {
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    response.transportConfigurations = DataModel::List<const Structs::TransportConfigurationStruct::Type>(
        transportConfigurations.data(), transportConfigurations.size());

    handler.AddResponse(commandPath, response);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> PushAvStreamTransportServerLogic::HandleUpdateMotionZoneOptions(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const PushAvStreamTransport::Commands::UpdateMotionZoneOptions::DecodableType & commandData)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        handler.AddStatus(commandPath, Status::UnsupportedCommand);
        return std::nullopt;
    }

    uint16_t connectionID   = commandData.connectionID;
    FabricIndex fabricIndex = handler.GetAccessingFabricIndex();

    TransportConfigurationStorage * transportConfiguration = FindStreamTransportConnectionWithinFabric(connectionID, fabricIndex);
    if (transportConfiguration == nullptr)
    {
        ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: ConnectionID (%u) Not Found for fabric %u", mEndpointId,
                     connectionID, fabricIndex);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    if (mDelegate->GetTransportBusyStatus(connectionID) == PushAvStreamTransportStatusEnum::kBusy)
    {
        ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: Connection is Busy", mEndpointId);
        handler.AddStatus(commandPath, Status::Busy);
        return std::nullopt;
    }

    auto transportOptionsPtr = transportConfiguration->GetTransportOptionsPtr();
    if (transportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: Transport options missing for connection %u", mEndpointId,
                     connectionID);
        handler.AddStatus(commandPath, Status::NotFound);
        return std::nullopt;
    }

    const auto triggerType = transportOptionsPtr->triggerOptions.triggerType;
    if (triggerType != TransportTriggerTypeEnum::kMotion && triggerType != TransportTriggerTypeEnum::kAmbientContext)
    {
        ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: Transport trigger type does not use zones", mEndpointId);
        handler.AddStatus(commandPath, Status::InvalidCommand);
        return std::nullopt;
    }

    // Validate MotionSensitivity
    if (commandData.motionSensitivity.HasValue())
    {
        if (triggerType != TransportTriggerTypeEnum::kMotion || mFeatures.Has(Feature::kPerZoneSensitivity))
        {
            ChipLogError(Zcl,
                         "HandleUpdateMotionZoneOptions[ep=%d]: MotionSensitivity does not apply to this transport or feature set",
                         mEndpointId);
            handler.AddStatus(commandPath, Status::InvalidCommand);
            return std::nullopt;
        }

        if (!commandData.motionSensitivity.Value().IsNull())
        {
            uint8_t sensitivity = commandData.motionSensitivity.Value().Value();
            if (sensitivity < 1 || sensitivity > 10)
            {
                ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: MotionSensitivity out of range: %u", mEndpointId,
                             sensitivity);
                handler.AddStatus(commandPath, Status::ConstraintError);
                return std::nullopt;
            }
        }
    }

    // Validate MotionZones
    if (commandData.motionZones.HasValue() && !commandData.motionZones.Value().IsNull())
    {
        const auto & zones = commandData.motionZones.Value().Value();
        if (Internal::RequiredZoneFieldReader(zones).Validate() != CHIP_NO_ERROR)
        {
            handler.AddStatus(commandPath, Status::InvalidCommand);
            return std::nullopt;
        }
        auto zoneIterator = zones.begin();
        while (zoneIterator.Next())
        {
            const auto sensitivityStatus = ValidateZoneSensitivity(zoneIterator.GetValue());
            if (sensitivityStatus != Status::Success)
            {
                handler.AddStatus(commandPath, sensitivityStatus);
                return std::nullopt;
            }
        }
        if (zoneIterator.GetStatus() != CHIP_NO_ERROR)
        {
            handler.AddStatus(commandPath, Status::InvalidCommand);
            return std::nullopt;
        }
        Structs::TransportTriggerOptionsStruct::DecodableType zoneOptions;
        zoneOptions.triggerType = triggerType;
        zoneOptions.motionZones = commandData.motionZones;
        const auto zoneStatus   = ValidateMotionZones(zoneOptions);
        if (zoneStatus != Status::Success)
        {
            handler.AddStatus(commandPath, zoneStatus.GetStatusCode());
            return std::nullopt;
        }
    }

    std::shared_ptr<TransportOptionsStorage> updatedTransportOptionsPtr{ new (std::nothrow)
                                                                             TransportOptionsStorage(*transportOptionsPtr) };
    if (updatedTransportOptionsPtr == nullptr)
    {
        ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: Memory Allocation failed for transportOptions", mEndpointId);
        handler.AddStatus(commandPath, Status::ResourceExhausted);
        return std::nullopt;
    }

    if (commandData.motionSensitivity.HasValue())
    {
        updatedTransportOptionsPtr->UpdateMotionSensitivity(commandData.motionSensitivity);
    }

    if (commandData.motionZones.HasValue())
    {
        CHIP_ERROR updateErr = updatedTransportOptionsPtr->UpdateMotionZones(commandData.motionZones);
        if (updateErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: Failed to update motion zones: %" CHIP_ERROR_FORMAT,
                         mEndpointId, updateErr.Format());
            handler.AddStatus(commandPath, Status::InvalidCommand);
            return std::nullopt;
        }
    }

    Status status = mDelegate->UpdateMotionZoneOptions(connectionID, *updatedTransportOptionsPtr);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: Delegate UpdateMotionZoneOptions failed: %u", mEndpointId,
                     to_underlying(status));
        handler.AddStatus(commandPath, status);
        return std::nullopt;
    }

    transportConfiguration->SetTransportOptionsPtr(updatedTransportOptionsPtr);

    CHIP_ERROR err = StoreCurrentConnections();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl,
                     "HandleUpdateMotionZoneOptions[ep=%d]: Failed to store modified connection, reverting: %" CHIP_ERROR_FORMAT,
                     mEndpointId, err.Format());
        Status rollbackStatus = mDelegate->UpdateMotionZoneOptions(connectionID, *transportOptionsPtr);
        if (rollbackStatus != Status::Success)
        {
            ChipLogError(Zcl, "HandleUpdateMotionZoneOptions[ep=%d]: Delegate rollback failed: %u", mEndpointId,
                         to_underlying(rollbackStatus));
        }
        transportConfiguration->SetTransportOptionsPtr(transportOptionsPtr);
        handler.AddStatus(commandPath, Status::Failure);
        return std::nullopt;
    }

    mCluster->ReportAttributeChange(PushAvStreamTransport::Attributes::CurrentConnections::Id);
    handler.AddStatus(commandPath, Status::Success);
    return std::nullopt;
}

Status PushAvStreamTransportServerLogic::CheckPrivacyModes(StreamUsageEnum streamUsage)
{
    bool hardPrivacyModeActive = false;

    CHIP_ERROR err = mDelegate->IsHardPrivacyModeActive(hardPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PushAvStreamTransport:CheckPrivacyModes: Failed to check Hard Privacy mode: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return Status::Failure;
    }

    if (hardPrivacyModeActive)
    {
        ChipLogError(Zcl, "PushAvStreamTransport:CheckPrivacyModes: Hard Privacy mode is enabled");
        return Status::InvalidInState;
    }

    bool softLivestreamPrivacyModeActive = false;
    err                                  = mDelegate->IsSoftLivestreamPrivacyModeActive(softLivestreamPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Failed to check Soft LivestreamPrivacy mode: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return Status::Failure;
    }

    if (softLivestreamPrivacyModeActive && streamUsage == StreamUsageEnum::kLiveView)
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Soft LivestreamPrivacy mode is enabled and StreamUsage is LiveView");
        return Status::InvalidInState;
    }

    bool softRecordingPrivacyModeActive = false;
    err                                 = mDelegate->IsSoftRecordingPrivacyModeActive(softRecordingPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Failed to check SoftRecordingPrivacyModeActive: %" CHIP_ERROR_FORMAT,
                     err.Format());
        return Status::Failure;
    }

    if (softRecordingPrivacyModeActive && (streamUsage == StreamUsageEnum::kRecording || streamUsage == StreamUsageEnum::kAnalysis))
    {
        ChipLogError(Zcl,
                     "PushAvStreamTransport:CheckPrivacyModes: Soft RecordingPrivacy mode is enabled and StreamUsage is Recording "
                     "or Analysis");
        return Status::InvalidInState;
    }

    return Status::Success;
}

Status PushAvStreamTransportServerLogic::GeneratePushTransportBeginEvent(
    const uint16_t connectionID, const TransportTriggerTypeEnum triggerType,
    const Optional<TriggerActivationReasonEnum> activationReason, const ContainerFormatEnum containerType,
    const Optional<uint64_t> cmafSessionNumber)
{
    Events::PushTransportBegin::Type event;
    EventNumber eventNumber;

    event.connectionID     = connectionID;
    event.triggerType      = triggerType;
    event.activationReason = activationReason;
    event.containerType.SetValue(containerType);
    event.CMAFSessionNumber = cmafSessionNumber;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate PushAVTransportBegin event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status PushAvStreamTransportServerLogic::GeneratePushTransportEndEvent(const uint16_t connectionID)
{
    Events::PushTransportEnd::Type event;
    EventNumber eventNumber;

    event.connectionID = connectionID;

    // Get container type from transport configuration to include in the event
    TransportConfigurationStorage * transportConfig = FindStreamTransportConnection(connectionID);
    if (transportConfig != nullptr)
    {
        event.containerType.SetValue(transportConfig->transportOptions.Value().containerOptions.containerType);

        // For CMAF container type, we need to provide CMAF session number
        if (event.containerType == ContainerFormatEnum::kCmaf)
        {
            uint64_t sessionNumber = 0;
            if (mDelegate != nullptr && mDelegate->GetCMAFSessionNumber(connectionID, sessionNumber))
            {
                event.CMAFSessionNumber = MakeOptional<uint64_t>(sessionNumber);
            }
            else
            {
                ChipLogError(Zcl, "GeneratePushTransportEndEvent: Unable to get CMAF session number for connection %u",
                             connectionID);
                // Don't include the session number if we can't get it
                event.CMAFSessionNumber = Optional<uint64_t>();
            }
        }
        else
        {
            event.CMAFSessionNumber = Optional<uint64_t>();
        }
    }
    else
    {
        // Fallback values if transport config not found
        event.containerType.SetValue(ContainerFormatEnum::kCmaf); // Default fallback
        event.CMAFSessionNumber = Optional<uint64_t>();
    }

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate PushAVTransportEnd event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status PushAvStreamTransportServerLogic::NotifyTransportStarted(uint16_t connectionID, TransportTriggerTypeEnum triggerType,
                                                                Optional<TriggerActivationReasonEnum> activationReason)
{
    ChipLogProgress(Zcl, "NotifyTransportStarted called for connectionID %u with triggerType %u", connectionID,
                    to_underlying(triggerType));

    // Validate that the connection exists
    TransportConfigurationStorage * transportConfig = FindStreamTransportConnection(connectionID);
    if (transportConfig == nullptr)
    {
        ChipLogError(Zcl, "NotifyTransportStarted: ConnectionID %u not found", connectionID);
        return Status::NotFound;
    }

    // Get container type from transport configuration
    ContainerFormatEnum containerType = transportConfig->transportOptions.Value().containerOptions.containerType;

    // For CMAF container type, we need to provide CMAF session number
    Optional<uint64_t> cmafSessionNumber;
    if (containerType == ContainerFormatEnum::kCmaf)
    {
        uint64_t sessionNumber = 0;
        if (mDelegate != nullptr && mDelegate->GetCMAFSessionNumber(connectionID, sessionNumber))
        {
            cmafSessionNumber = MakeOptional<uint64_t>(sessionNumber);
        }
        else
        {
            ChipLogError(Zcl, "NotifyTransportStarted: Unable to get CMAF session number for connection %u", connectionID);
            // Don't include the session number if we can't get it
            cmafSessionNumber = Optional<uint64_t>();
        }
    }

    // Generate the PushTransportBegin event
    return GeneratePushTransportBeginEvent(connectionID, triggerType, activationReason, containerType, cmafSessionNumber);
}

Status PushAvStreamTransportServerLogic::NotifyTransportStopped(uint16_t connectionID, TransportTriggerTypeEnum triggerType)
{
    ChipLogProgress(Zcl, "NotifyTransportStopped called for connectionID %u with triggerType %u", connectionID,
                    to_underlying(triggerType));

    // Validate that the connection exists
    TransportConfigurationStorage * transportConfig = FindStreamTransportConnection(connectionID);
    if (transportConfig == nullptr)
    {
        ChipLogError(Zcl, "NotifyTransportStopped: ConnectionID %u not found", connectionID);
        return Status::NotFound;
    }

    // Generate the PushTransportEnd event
    return GeneratePushTransportEndEvent(connectionID);
}

CHIP_ERROR PushAvStreamTransportServerLogic::StoreCurrentConnections()
{
    VerifyOrReturnError(mAttributeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Platform::ScopedMemoryBuffer<uint8_t> currentConns;
    MutableByteSpan bufferSpan;
    size_t maxBufferSize = static_cast<size_t>(kMaxCurrentConnectionsSerializedSize);

    if (!currentConns.Alloc(maxBufferSize))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    bufferSpan = MutableByteSpan{ currentConns.Get(), maxBufferSize };
    TLV::TLVWriter writer;
    writer.Init(bufferSpan);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (const auto & connection : mCurrentConnections)
    {
        // Use the EncodeForRead() API to also TLV-encode the fabric index for storage.
        ReturnErrorOnFailure(connection.EncodeForRead(writer, TLV::AnonymousTag(), connection.GetFabricIndex()));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    bufferSpan.reduce_size(writer.GetLengthWritten());

    auto path = ConcreteAttributePath(mEndpointId, PushAvStreamTransport::Id, CurrentConnections::Id);
    ReturnErrorOnFailure(mAttributeStorage->WriteValue(path, bufferSpan));

    ChipLogProgress(Zcl, "Saved %u CurrentConnections", static_cast<unsigned int>(mCurrentConnections.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportServerLogic::LoadCurrentConnections()
{
    Platform::ScopedMemoryBuffer<uint8_t> currentConns;
    MutableByteSpan bufferSpan;
    size_t maxBufferSize = static_cast<size_t>(kMaxCurrentConnectionsSerializedSize);

    if (!currentConns.Alloc(maxBufferSize))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    bufferSpan = MutableByteSpan{ currentConns.Get(), maxBufferSize };

    // Clear the list before attempting to load
    mCurrentConnections.clear();

    auto path = ConcreteAttributePath(mEndpointId, PushAvStreamTransport::Id, CurrentConnections::Id);

    CHIP_ERROR err = mAttributeStorage->ReadValue(path, bufferSpan);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogProgress(Zcl, "No persisted CurrentConnections found.");
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    TLV::TLVReader reader;
    reader.Init(bufferSpan);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    TLV::TLVReader validationReader = reader;
    while ((err = validationReader.Next()) == CHIP_NO_ERROR)
    {
        Structs::TransportConfigurationStruct::DecodableType decoded;
        ReturnErrorOnFailure(decoded.Decode(validationReader));
        VerifyOrReturnError(decoded.transportOptions.HasValue(), CHIP_ERROR_MISSING_TLV_ELEMENT);
        const auto & options       = decoded.transportOptions.Value();
        const auto validateStreams = [](const auto & streams, size_t capacity) -> CHIP_ERROR {
            if (streams.HasValue())
            {
                auto iter    = streams.Value().begin();
                size_t count = 0;
                while (iter.Next())
                {
                    ++count;
                }
                ReturnErrorOnFailure(iter.GetStatus());
                VerifyOrReturnError(count <= capacity, CHIP_ERROR_BUFFER_TOO_SMALL);
            }
            return CHIP_NO_ERROR;
        };
        ReturnErrorOnFailure(validateStreams(options.videoStreams, kMaxVideoStreams));
        ReturnErrorOnFailure(validateStreams(options.audioStreams, kMaxAudioStreams));
        const auto & zones = options.triggerOptions.motionZones;
        if (zones.HasValue() && !zones.Value().IsNull())
        {
            ReturnErrorOnFailure(Internal::RequiredZoneFieldReader(zones.Value().Value()).Validate());
            auto zoneIter          = zones.Value().Value().begin();
            size_t zoneCount       = 0;
            auto sensitivityStatus = Status::Success;
            while (zoneIter.Next())
            {
                ++zoneCount;
                if (sensitivityStatus == Status::Success)
                {
                    sensitivityStatus = ValidateZoneSensitivity(zoneIter.GetValue());
                }
            }
            ReturnErrorOnFailure(zoneIter.GetStatus());
            VerifyOrReturnError(sensitivityStatus == Status::Success, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(mDelegate->ValidateMotionZoneListSize(zoneCount), CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(ValidateMotionZones(options.triggerOptions).IsSuccess(), CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
    VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);
    ReturnErrorOnFailure(validationReader.ExitContainer(arrayType));
    ReturnErrorOnFailure(validationReader.VerifyEndOfContainer());

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        // Decode into a temporary DecodableType first.
        Structs::TransportConfigurationStruct::DecodableType decodedTransportConfig;
        ReturnErrorOnFailure(decodedTransportConfig.Decode(reader));

        // Now, create the storage version which will perform a deep copy of the data.
        std::shared_ptr<TransportOptionsStorage> transportOptionsStorage;
        if (decodedTransportConfig.transportOptions.HasValue())
        {
            auto & trigger = decodedTransportConfig.transportOptions.Value().triggerOptions;
            if (trigger.triggerType == TransportTriggerTypeEnum::kAmbientContext && !trigger.motionZones.HasValue())
            {
                trigger.motionZones.Emplace().SetNull();
            }
            transportOptionsStorage = std::make_shared<TransportOptionsStorage>(decodedTransportConfig.transportOptions.Value());
            if (!transportOptionsStorage)
            {
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        TransportConfigurationStorage connection(decodedTransportConfig.connectionID, transportOptionsStorage);
        connection.transportStatus = decodedTransportConfig.transportStatus;
        connection.fabricIndex     = decodedTransportConfig.fabricIndex;
        mCurrentConnections.push_back(connection);
    }

    VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));

    ChipLogProgress(Zcl, "Loaded %u CurrentConnections", static_cast<unsigned int>(mCurrentConnections.size()));

    return reader.VerifyEndOfContainer();
}

} // namespace Clusters
} // namespace app
} // namespace chip

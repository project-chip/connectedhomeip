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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/odd-sized-integers.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>
#include <tracing/macros.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/scenes-server.h>
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

#ifdef MATTER_DM_PLUGIN_ON_OFF
#include <app/clusters/on-off-server/on-off-server.h>
#endif // MATTER_DM_PLUGIN_ON_OFF

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::Protocols;
using chip::Protocols::InteractionModel::Status;

using BootReasonType = GeneralDiagnostics::BootReasonEnum;

static InteractionModel::Status verifyModeValue(const EndpointId endpointId, const uint8_t newMode);

static ModeSelect::SupportedModesManager * sSupportedModesManager = nullptr;

const SupportedModesManager * ModeSelect::getSupportedModesManager()
{
    return sSupportedModesManager;
}

void ModeSelect::setSupportedModesManager(ModeSelect::SupportedModesManager * aSupportedModesManager)
{
    sSupportedModesManager = aSupportedModesManager;
}
namespace {

inline bool areStartUpModeAndCurrentModeNonVolatile(EndpointId endpoint);

class ModeSelectAttrAccess : public AttributeAccessInterface
{
public:
    ModeSelectAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), ModeSelect::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

ModeSelectAttrAccess gModeSelectAttrAccess;

CHIP_ERROR ModeSelectAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == ModeSelect::Id);

    const ModeSelect::SupportedModesManager * gSupportedModeManager = ModeSelect::getSupportedModesManager();

    if (ModeSelect::Attributes::SupportedModes::Id == aPath.mAttributeId)
    {
        if (gSupportedModeManager == nullptr)
        {
            ChipLogError(Zcl, "ModeSelect: SupportedModesManager is NULL");
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }
        const ModeSelect::SupportedModesManager::ModeOptionsProvider modeOptionsProvider =
            gSupportedModeManager->getModeOptionsProvider(aPath.mEndpointId);
        if (modeOptionsProvider.begin() == nullptr)
        {
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR err;
        err = aEncoder.EncodeList([modeOptionsProvider](const auto & encoder) -> CHIP_ERROR {
            const auto * end = modeOptionsProvider.end();
            for (auto * it = modeOptionsProvider.begin(); it != end; ++it)
            {
                auto & modeOption = *it;
                ReturnErrorOnFailure(encoder.Encode(modeOption));
            }
            return CHIP_NO_ERROR;
        });
        ReturnErrorOnFailure(err);
    }
    return CHIP_NO_ERROR;
}

Status ChangeToMode(EndpointId endpointId, uint8_t newMode)
{
    MATTER_TRACE_SCOPE("ChangeToMode", "ModeSelect");
    ChipLogProgress(Zcl, "ModeSelect: Entering emberAfModeSelectClusterChangeToModeCallback");

    // Check that the newMode matches one of the supported options
    const ModeSelect::Structs::ModeOptionStruct::Type * modeOptionPtr;
    const ModeSelect::SupportedModesManager * gSupportedModeManager = ModeSelect::getSupportedModesManager();
    if (gSupportedModeManager == nullptr)
    {
        ChipLogError(Zcl, "ModeSelect: SupportedModesManager is NULL");
        return Status::Failure;
    }
    Status checkSupportedModeStatus = gSupportedModeManager->getModeOptionByMode(endpointId, newMode, &modeOptionPtr);
    if (Status::Success != checkSupportedModeStatus)
    {
        ChipLogProgress(Zcl, "ModeSelect: Failed to find the option with mode %u", newMode);
        return checkSupportedModeStatus;
    }
    ModeSelect::Attributes::CurrentMode::Set(endpointId, newMode);

    return Status::Success;
}

} // anonymous namespace

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
static constexpr size_t kModeSelectMaxEnpointCount =
    MATTER_DM_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static void timerCallback(System::Layer *, void * callbackContext);
static void sceneModeSelectCallback(EndpointId endpoint);
using ModeSelectEndPointPair = scenes::DefaultSceneHandlerImpl::EndpointStatePair<uint8_t>;
using ModeSelectTransitionTimeInterface =
    scenes::DefaultSceneHandlerImpl::TransitionTimeInterface<kModeSelectMaxEnpointCount,
                                                             MATTER_DM_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT>;

class DefaultModeSelectSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    DefaultSceneHandlerImpl::StatePairBuffer<uint8_t, kModeSelectMaxEnpointCount> mSceneEndpointStatePairs;
    // As per spec, 1 attribute is scenable in the mode select cluster
    static constexpr uint8_t kScenableAttributeCount = 1;

    DefaultModeSelectSceneHandler() = default;
    ~DefaultModeSelectSceneHandler() override {}

    // Default function for the mode select cluster, only puts the mode select cluster ID in the span if supported on the given
    // endpoint
    virtual void GetSupportedClusters(EndpointId endpoint, Span<ClusterId> & clusterBuffer) override
    {
        if (emberAfContainsServer(endpoint, ModeSelect::Id) && clusterBuffer.size() >= 1)
        {
            clusterBuffer[0] = ModeSelect::Id;
            clusterBuffer.reduce_size(1);
        }
        else
        {
            clusterBuffer.reduce_size(0);
        }
    }

    // Default function for mode select cluster, only checks if mode select is enabled on the endpoint
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override
    {
        return (cluster == ModeSelect::Id) && (emberAfContainsServer(endpoint, ModeSelect::Id));
    }

    /// @brief Serialize the Cluster's EFS value
    /// @param [in] endpoint target endpoint
    /// @param [in] cluster  target cluster
    /// @param [out] serializedBytes data to serialize into EFS
    /// @return CHIP_NO_ERROR if successfully serialized the data, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override
    {
        using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

        uint8_t currentMode;
        // read CurrentMode value
        Status status = Attributes::CurrentMode::Get(endpoint, &currentMode);
        if (status != Status::Success)
        {
            ChipLogError(Zcl, "ERR: reading CurrentMode 0x%02x", to_underlying(status));
            return CHIP_ERROR_READ_FAILED;
        }

        AttributeValuePair pairs[kScenableAttributeCount];

        pairs[0].attributeID = Attributes::CurrentMode::Id;
        pairs[0].valueUnsigned8.SetValue(currentMode);

        app::DataModel::List<AttributeValuePair> attributeValueList(pairs);

        return EncodeAttributeValueList(attributeValueList, serializedBytes);
    }

    /// @brief Default EFS interaction when applying scene to the ModeSelect Cluster
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serializedBytes Data from nvm
    /// @param timeMs transition time in ms
    /// @return CHIP_NO_ERROR if value as expected, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override
    {
        app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;

        VerifyOrReturnError(cluster == ModeSelect::Id, CHIP_ERROR_INVALID_ARGUMENT);

        ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

        size_t attributeCount = 0;
        ReturnErrorOnFailure(attributeValueList.ComputeSize(&attributeCount));
        VerifyOrReturnError(attributeCount <= kScenableAttributeCount, CHIP_ERROR_BUFFER_TOO_SMALL);

        auto pair_iterator = attributeValueList.begin();
        while (pair_iterator.Next())
        {
            auto & decodePair = pair_iterator.GetValue();
            VerifyOrReturnError(decodePair.attributeID == Attributes::CurrentMode::Id, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(mSceneEndpointStatePairs.InsertPair(
                ModeSelectEndPointPair(endpoint, static_cast<uint8_t>(decodePair.valueUnsigned8.HasValue()))));
        }
        // Verify that the EFS was completely read
        CHIP_ERROR err = pair_iterator.GetStatus();
        if (CHIP_NO_ERROR != err)
        {
            mSceneEndpointStatePairs.RemovePair(endpoint);
            return err;
        }

        VerifyOrReturnError(mTransitionTimeInterface.sceneEventControl(endpoint) != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(timeMs), timerCallback,
                                              mTransitionTimeInterface.sceneEventControl(endpoint));

        return CHIP_NO_ERROR;
    }

private:
    ModeSelectTransitionTimeInterface mTransitionTimeInterface =
        ModeSelectTransitionTimeInterface(ModeSelect::Id, sceneModeSelectCallback);
};
static DefaultModeSelectSceneHandler sModeSelectSceneHandler;

static void timerCallback(System::Layer *, void * callbackContext)
{
    auto control = static_cast<EmberEventControl *>(callbackContext);
    (control->callback)(control->endpoint);
}

/**
 * @brief This function is a callback to apply the mode that was saved when the ApplyScene was called with a transition time greater
 * than 0.
 *
 * @param endpoint The endpoint ID that the scene mode selection is associated with.
 *
 */
static void sceneModeSelectCallback(EndpointId endpoint)
{
    ModeSelectEndPointPair savedState;
    ReturnOnFailure(sModeSelectSceneHandler.mSceneEndpointStatePairs.GetPair(endpoint, savedState));
    ChangeToMode(endpoint, savedState.mValue);
    sModeSelectSceneHandler.mSceneEndpointStatePairs.RemovePair(endpoint);
}

#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

bool emberAfModeSelectClusterChangeToModeCallback(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
                                                  const ModeSelect::Commands::ChangeToMode::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "ModeSelect: Entering emberAfModeSelectClusterChangeToModeCallback");

    uint8_t currentMode = 0;
    ModeSelect::Attributes::CurrentMode::Get(commandPath.mEndpointId, &currentMode);
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    if (currentMode != commandData.newMode)
    {
        //  the scene has been changed (the value of CurrentMode has changed) so
        //  the current scene as described in the scene table is invalid
        ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(commandPath.mEndpointId);
    }
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

    Status status = ChangeToMode(commandPath.mEndpointId, commandData.newMode);

    if (Status::Success != status)
    {
        commandHandler->AddStatus(commandPath, status);
        return true;
    }

    ChipLogProgress(Zcl, "ModeSelect: ChangeToMode successful");
    commandHandler->AddStatus(commandPath, status);
    return true;
}

/**
 * Callback for Mode Select Cluster Server Initialization.
 * Enabled in src/app/zap-templates/templates/app/helper.js
 * @param endpointId    id of the endpoint that is being initialized
 */
void emberAfModeSelectClusterServerInitCallback(EndpointId endpointId)
{
#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
    ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpointId, &sModeSelectSceneHandler);
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

    // StartUp behavior relies on CurrentMode StartUpMode attributes being non-volatile.
    if (areStartUpModeAndCurrentModeNonVolatile(endpointId))
    {
        // Read the StartUpMode attribute and set the CurrentMode attribute
        // The StartUpMode attribute SHALL define the desired startup behavior of a
        // device when it is supplied with power and this state SHALL be
        // reflected in the CurrentMode attribute.  The values of the StartUpMode
        // attribute are listed below.

        DataModel::Nullable<uint8_t> startUpMode;
        Status status = Attributes::StartUpMode::Get(endpointId, startUpMode);
        if (status == Status::Success && !startUpMode.IsNull())
        {

#ifdef MATTER_DM_PLUGIN_ON_OFF
            // OnMode with Power Up
            // If the On/Off feature is supported and the On/Off cluster attribute StartUpOnOff is present, with a
            // value of On (turn on at power up), then the CurrentMode attribute SHALL be set to the OnMode attribute
            // value when the server is supplied with power, except if the OnMode attribute is null.
            if (emberAfContainsServer(endpointId, OnOff::Id) &&
                emberAfContainsAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                emberAfContainsAttribute(endpointId, ModeSelect::Id, ModeSelect::Attributes::OnMode::Id))
            {
                Attributes::OnMode::TypeInfo::Type onMode;
                bool onOffValueForStartUp = false;
                if (Attributes::OnMode::Get(endpointId, onMode) == Status::Success &&
                    !emberAfIsKnownVolatileAttribute(endpointId, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id) &&
                    OnOffServer::Instance().getOnOffValueForStartUp(endpointId, onOffValueForStartUp) == Status::Success)
                {
                    if (onOffValueForStartUp && !onMode.IsNull())
                    {
                        ChipLogProgress(Zcl, "ModeSelect: CurrentMode is overwritten by OnMode");
                        return;
                    }
                }
            }
#endif // MATTER_DM_PLUGIN_ON_OFF

            BootReasonType bootReason = BootReasonType::kUnspecified;
            CHIP_ERROR error          = DeviceLayer::GetDiagnosticDataProvider().GetBootReason(bootReason);

            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Unable to retrieve boot reason: %" CHIP_ERROR_FORMAT, error.Format());
                // We really only care whether the boot reason is OTA.  Assume it's not.
                bootReason = BootReasonType::kUnspecified;
            }
            if (bootReason == BootReasonType::kSoftwareUpdateCompleted)
            {
                ChipLogProgress(Zcl, "ModeSelect: CurrentMode is ignored for OTA reboot");
                return;
            }

            // Initialise currentMode to 0
            uint8_t currentMode = 0;
            status              = Attributes::CurrentMode::Get(endpointId, &currentMode);

            if ((status == Status::Success) && (startUpMode.Value() != currentMode))
            {
                status = Attributes::CurrentMode::Set(endpointId, startUpMode.Value());
                if (status != Status::Success)
                {
                    ChipLogError(Zcl, "ModeSelect: Error initializing CurrentMode, Status code 0x%02x", to_underlying(status));
                }
                else
                {
                    ChipLogProgress(Zcl, "ModeSelect: Successfully initialized CurrentMode to %u", startUpMode.Value());
                }
            }
        }
    }
    else
    {
        ChipLogProgress(Zcl, "ModeSelect: Skipped initializing CurrentMode by StartUpMode because one of them is volatile");
    }
}

namespace {

/**
 * Checks if StartUpMode and CurrentMode are non-volatile.
 * @param endpointId    id of the endpoint to check
 * @return true if both attributes are non-volatile; false otherwise.
 */
inline bool areStartUpModeAndCurrentModeNonVolatile(EndpointId endpointId)
{
    return !emberAfIsKnownVolatileAttribute(endpointId, ModeSelect::Id, Attributes::CurrentMode::Id) &&
        !emberAfIsKnownVolatileAttribute(endpointId, ModeSelect::Id, Attributes::StartUpMode::Id);
}

} // namespace

void MatterModeSelectPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gModeSelectAttrAccess);
}

/**
 * Callback for Mode Select Cluster Server Pre Attribute Changed
 * Enabled in src/app/zap-templates/templates/app/helper.js
 * @param attributePath Concrete attribute path to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
InteractionModel::Status MatterModeSelectClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                                                  EmberAfAttributeType attributeType, uint16_t size,
                                                                                  uint8_t * value)
{
    const EndpointId endpointId = attributePath.mEndpointId;
    InteractionModel::Status result;

    switch (attributePath.mAttributeId)
    {
    case ModeSelect::Attributes::StartUpMode::Id:
        result = verifyModeValue(endpointId, *value);
        break;
    case ModeSelect::Attributes::OnMode::Id:
        result = verifyModeValue(endpointId, *value);
        break;
    default:
        result = InteractionModel::Status::Success;
    }

    return result;
}

/**
 * Checks the new mode against the endpoint's supported modes.
 * @param endpointId    endpointId of the endpoint
 * @param newMode       value of the new mode
 * @return              Success status if the value is valid; InvalidValue otherwise.
 */
static InteractionModel::Status verifyModeValue(const EndpointId endpointId, const uint8_t newMode)
{
    if (NumericAttributeTraits<uint8_t>::IsNullValue(newMode)) // This indicates that the new mode is null.
    {
        return InteractionModel::Status::Success;
    }
    const ModeSelect::Structs::ModeOptionStruct::Type * modeOptionPtr;
    const ModeSelect::SupportedModesManager * gSupportedModeManager = ModeSelect::getSupportedModesManager();
    if (gSupportedModeManager == nullptr)
    {
        ChipLogError(Zcl, "ModeSelect: SupportedModesManager is NULL");
        return Status::Failure;
    }
    return gSupportedModeManager->getModeOptionByMode(endpointId, newMode, &modeOptionPtr);
}

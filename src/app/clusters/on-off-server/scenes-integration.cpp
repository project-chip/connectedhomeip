/**
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
#include <app/clusters/on-off-server/scenes-integration.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/clusters/scenes-server/scenes-server.h> // nogncheck

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;

using chip::Protocols::InteractionModel::Status;

static constexpr size_t kOnOffMaxEndpointCount =
    MATTER_DM_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static void sceneOnOffCallback(EndpointId endpoint);
using OnOffEndPointPair = scenes::DefaultSceneHandlerImpl::EndpointStatePair<bool>;
using OnOffTransitionTimeInterface =
    scenes::DefaultSceneHandlerImpl::TransitionTimeInterface<kOnOffMaxEndpointCount,
                                                             MATTER_DM_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT>;

#if CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

class DefaultOnOffSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    DefaultSceneHandlerImpl::StatePairBuffer<bool, kOnOffMaxEndpointCount> mSceneEndpointStatePairs;
    // As per spec, 1 attribute is scenable in the on off cluster
    static constexpr uint8_t scenableAttributeCount = 1;

    DefaultOnOffSceneHandler()           = default;
    ~DefaultOnOffSceneHandler() override = default;

    // Default function for OnOff cluster, only puts the OnOff cluster ID in the span if supported on the given endpoint
    void GetSupportedClusters(EndpointId endpoint, Span<ClusterId> & clusterBuffer) override
    {
        ClusterId * buffer = clusterBuffer.data();
        if (emberAfContainsServer(endpoint, OnOff::Id) && clusterBuffer.size() >= 1)
        {
            buffer[0] = OnOff::Id;
            clusterBuffer.reduce_size(1);
        }
        else
        {
            clusterBuffer.reduce_size(0);
        }
    }

    // Default function for OnOff cluster, only checks if OnOff is enabled on the endpoint
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override
    {
        return (cluster == OnOff::Id) && (emberAfContainsServer(endpoint, OnOff::Id));
    }

    /// @brief Serialize the Cluster's EFS value
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serializedBytes data to serialize into EFS
    /// @return CHIP_NO_ERROR if successfully serialized the data, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override
    {
        using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

        bool currentValue;
        // read current on/off value
        Status status = Attributes::OnOff::Get(endpoint, &currentValue);
        if (status != Status::Success)
        {
            ChipLogError(Zcl, "ERR: reading on/off %x", to_underlying(status));
            return CHIP_ERROR_READ_FAILED;
        }

        AttributeValuePair pairs[scenableAttributeCount];

        pairs[0].attributeID = Attributes::OnOff::Id;
        pairs[0].valueUnsigned8.SetValue(currentValue);

        app::DataModel::List<AttributeValuePair> attributeValueList(pairs);

        return EncodeAttributeValueList(attributeValueList, serializedBytes);
    }

    /// @brief Default EFS interaction when applying scene to the OnOff Cluster
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serializedBytes Data from nvm
    /// @param timeMs transition time in ms
    /// @return CHIP_NO_ERROR if value as expected, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override
    {
        app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;

        VerifyOrReturnError(cluster == OnOff::Id, CHIP_ERROR_INVALID_ARGUMENT);

        ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

        size_t attributeCount = 0;
        ReturnErrorOnFailure(attributeValueList.ComputeSize(&attributeCount));
        VerifyOrReturnError(attributeCount <= scenableAttributeCount, CHIP_ERROR_BUFFER_TOO_SMALL);

        auto pair_iterator = attributeValueList.begin();
        while (pair_iterator.Next())
        {
            auto & decodePair = pair_iterator.GetValue();
            VerifyOrReturnError(decodePair.attributeID == Attributes::OnOff::Id, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(mSceneEndpointStatePairs.InsertPair(
                OnOffEndPointPair(endpoint, static_cast<bool>(decodePair.valueUnsigned8.Value()))));
        }
        // Verify that the EFS was completely read
        CHIP_ERROR err = pair_iterator.GetStatus();
        if (CHIP_NO_ERROR != err)
        {
            TEMPORARY_RETURN_IGNORED mSceneEndpointStatePairs.RemovePair(endpoint);
            return err;
        }

        VerifyOrReturnError(mTransitionTimeInterface.sceneEventControl(endpoint) != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        OnOffServer::Instance().scheduleTimerCallbackMs(mTransitionTimeInterface.sceneEventControl(endpoint), timeMs);

        return CHIP_NO_ERROR;
    }

private:
    OnOffTransitionTimeInterface mTransitionTimeInterface = OnOffTransitionTimeInterface(OnOff::Id, sceneOnOffCallback);
};
static DefaultOnOffSceneHandler sOnOffSceneHandler;

static void sceneOnOffCallback(EndpointId endpoint)
{
    OnOffEndPointPair savedState;
    ReturnOnFailure(sOnOffSceneHandler.mSceneEndpointStatePairs.GetPair(endpoint, savedState));
    CommandId command = (savedState.mValue) ? Commands::On::Id : Commands::Off::Id;
    OnOffServer::Instance().setOnOffValue(endpoint, command, false);
    ReturnOnFailure(sOnOffSceneHandler.mSceneEndpointStatePairs.RemovePair(endpoint));
}

namespace chip::app::Clusters::OnOff::Internal::Scenes {
chip::scenes::SceneHandler * GlobalHandler()
{
    return &sOnOffSceneHandler;
}

void RegisterGlobalHandler(chip::EndpointId endpoint)
{
    app::Clusters::ScenesManagement::ScenesServer::Instance().RegisterSceneHandler(endpoint, GlobalHandler());
}

} // namespace chip::app::Clusters::OnOff::Internal::Scenes

#endif // CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

namespace chip::app::Clusters::OnOff::Internal::Scenes {

void Store(FabricIndex fabricIndex, EndpointId endpoint)
{
    ScenesManagement::ScenesServer::Instance().StoreCurrentScene(
        fabricIndex, endpoint, ScenesManagement::ScenesServer::kGlobalSceneGroupId, ScenesManagement::ScenesServer::kGlobalSceneId);
}

void Recall(FabricIndex fabricIndex, EndpointId endpoint)
{
    ScenesManagement::ScenesServer::Instance().RecallScene(
        fabricIndex, endpoint, ScenesManagement::ScenesServer::kGlobalSceneGroupId, ScenesManagement::ScenesServer::kGlobalSceneId);
}

void MarkInvalid(EndpointId endpoint)
{
    ScenesManagement::ScenesServer::Instance().MakeSceneInvalidForAllFabrics(endpoint);
}

} // namespace chip::app::Clusters::OnOff::Internal::Scenes

#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <app/clusters/scenes-server/SceneHandlerImpl.h>
#include <app/data-model/Nullable.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ModeSelect/AttributeIds.h>
#include <clusters/ModeSelect/Enums.h>
#include <clusters/ModeSelect/Structs.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

class ModeSelectCluster : public DefaultServerCluster, public scenes::DefaultSceneHandlerImpl
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<ModeSelect::Attributes::StartUpMode::Id>;

    class Delegate
    {
    public:
        virtual ~Delegate()                                                                        = default;
        virtual Span<const ModeSelect::Structs::ModeOptionStruct::Type> GetSupportedModes() const = 0;
    };

    struct Config
    {
        BitMask<ModeSelect::Feature> featureMap;
        OptionalAttributeSet optionalAttributeSet;
        bool onOffValueForStartUp = false;
        DataModel::Nullable<uint8_t> initialStartUpMode; // ZAP default; fallback when no persisted value
        DataModel::Nullable<uint8_t> initialOnMode;      // ZAP default; fallback when no persisted value
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    ModeSelectCluster(EndpointId endpointId, Delegate & delegate, const Config & config);

    // ServerClusterInterface overrides
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    // Attribute accessors
    uint8_t GetCurrentMode() const { return mCurrentMode; }
    DataModel::Nullable<uint8_t> GetStartUpMode() const { return mStartUpMode; }
    DataModel::Nullable<uint8_t> GetOnMode() const { return mOnMode; }

    // Setters: persist to NVM and notify attribute changes
    Protocols::InteractionModel::Status UpdateCurrentMode(uint8_t newMode);
    Protocols::InteractionModel::Status UpdateStartUpMode(DataModel::Nullable<uint8_t> newStartUpMode);
    Protocols::InteractionModel::Status UpdateOnMode(DataModel::Nullable<uint8_t> newOnMode);

    bool IsSupportedMode(uint8_t mode) const;

    // Called by setSupportedModesManager() when modes become available after Startup().
    void ApplyStartupModeLogic();

    // scenes::SceneHandler overrides
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override;
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override;
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override;

private:
    void LoadPersistentAttributes(AttributePersistenceProvider & provider);

    Delegate & mDelegate;
    const BitMask<ModeSelect::Feature> mFeatureMap;
    const OptionalAttributeSet mOptionalAttributeSet;
    const bool mOnOffValueForStartUp;
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;

    uint8_t mCurrentMode = 0;
    DataModel::Nullable<uint8_t> mStartUpMode{};
    DataModel::Nullable<uint8_t> mOnMode{};
};

namespace ModeSelect {
/// Returns the cluster instance registered on the given endpoint, or nullptr if not found.
/// Only valid when code-driven cluster integration is active (all-clusters apps).
ModeSelectCluster * FindClusterOnEndpoint(EndpointId endpointId);
} // namespace ModeSelect

} // namespace Clusters
} // namespace app
} // namespace chip

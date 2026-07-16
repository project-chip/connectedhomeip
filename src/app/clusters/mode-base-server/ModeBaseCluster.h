/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/mode-base-server/AppDelegate.h>
#include <app/clusters/mode-base-server/mode-base-cluster-objects.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app::Clusters {

class ModeBaseCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<ModeBase::Attributes::StartUpMode::Id>;

    struct Config
    {
        BitMask<ModeBase::Feature> feature;
        OptionalAttributeSet optionalAttributeSet;
        ModeBase::AppDelegate & appDelegate;
        bool onOffValueForStartUp = false;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        uint32_t clusterRevision = 0;
    };

    ModeBaseCluster(EndpointId endpointId, ClusterId aClusterId, const Config & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    // Attribute setters
    /**
     * Sets the Current-Mode attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param aNewMode The value to which the Current-Mode mode is to be set.
     * @return Returns a ConstraintError if the aNewMode value is not valid. Returns Success otherwise.
     */
    Protocols::InteractionModel::Status UpdateCurrentMode(uint8_t aNewMode);

    /**
     * Sets the Start-Up attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param aNewStartUpMode The value to which the Start-Up mode is to be set.
     * @return Returns a ConstraintError if the aNewStartUpMode value is not valid. Returns Success otherwise.
     */
    Protocols::InteractionModel::Status UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode);

    /**
     * Sets the On-Mode attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param aNewOnMode The value to which the On-Mode mode is to be set.
     * @return Returns a ConstraintError if the aNewOnMode value is not valid. Returns Success otherwise.
     */
    Protocols::InteractionModel::Status UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode);

    // Attribute getters.
    /**
     * @return The Current mode.
     */
    uint8_t GetCurrentMode() const { return mCurrentMode; }

    /**
     * @return The Start-Up mode.
     */
    DataModel::Nullable<uint8_t> GetStartUpMode() const { return mStartUpMode; }

    /**
     * @return The On mode.
     */
    DataModel::Nullable<uint8_t> GetOnMode() const { return mOnMode; }

    /**
     * Reports that the contents of the supported modes attribute have changed.
     * The device SHALL call this method whenever it changes the list of supported modes.
     */
    void ReportSupportedModesChange();

    /**
     * This function returns true if the mode value given matches one of the supported modes, otherwise it returns false.
     * @param mode
     */
    bool IsSupportedMode(uint8_t mode);

    // Get mode value by mode tag
    CHIP_ERROR GetModeValueByModeTag(uint16_t modeTag, uint8_t & value);

private:
    const BitMask<ModeBase::Feature> mFeature;
    const OptionalAttributeSet mOptionalAttributeSet;
    ModeBase::AppDelegate & mAppDelegate;
    bool mOnOffValueForStartUp = false;
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
    uint32_t mClusterRevision = 0;

    // Attribute data store
    uint8_t mCurrentMode = 0; // This is a temporary value and may not be valid. We will change this to the value of the first
                              // mode in the list at the start of the Init function to ensure that it represents a valid mode.
    DataModel::Nullable<uint8_t> mStartUpMode;
    DataModel::Nullable<uint8_t> mOnMode;

    /**
     * Internal change-to-mode command handler function.
     */
    std::optional<DataModel::ActionReturnStatus>
    HandleChangeToMode(CommandHandler & commandObj, const ConcreteCommandPath & commandPath,
                       const ModeBase::Commands::ChangeToMode::DecodableType & commandData);

    /**
     * Helper function that loads all the persistent attributes from the KVS. These attributes are CurrentMode,
     * StartUpMode and OnMode.
     */
    void LoadPersistentAttributes();

    /**
     * Helper function that logs the status of a scalar value from the KVS.
     * @param status The status of the operation.
     * @param value The value of the attribute.
     * @param attributeName The name of the attribute.
     */
    void LogStatus(Protocols::InteractionModel::Status status, const uint8_t & value, const char * attributeName);

    /**
     * Helper function that logs the status of a nullable scalar value from the KVS.
     * @param status The status of the operation.
     * @param value The value of the attribute.
     * @param attributeName The name of the attribute.
     */
    void LogStatus(Protocols::InteractionModel::Status status, const DataModel::Nullable<uint8_t> & value,
                   const char * attributeName);

    /**
     * Helper function that encodes the supported modes.
     * @param encoder The encoder to encode the supported modes into.
     */
    CHIP_ERROR EncodeSupportedModes(const AttributeValueEncoder::ListEncodeHelper & encoder);
};

} // namespace chip::app::Clusters

/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/mode-base-server/Delegate.h>
#include <app/clusters/mode-base-server/mode-base-cluster-objects.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <lib/support/IntrusiveList.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

class Instance : public CommandHandlerInterface, public AttributeAccessInterface, public IntrusiveListNodeBase<>
{
public:
    /**
     * Creates a mode base cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase derived cluster to be instantiated.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature);

    ~Instance() override;

    /**
     * Initialise the ModeBase server instance.
     * @return Returns an error if the given endpoint and cluster ID have not been enabled in zap, if the
     * CommandHandler or AttributeHandler registration fails or if the Delegate::Init() returns an error.
     */
    CHIP_ERROR Init();

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
    uint8_t GetCurrentMode() const;

    /**
     * @return The Start-Up mode.
     */
    DataModel::Nullable<uint8_t> GetStartUpMode() const;

    /**
     * @return The On mode.
     */
    DataModel::Nullable<uint8_t> GetOnMode() const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() const { return mEndpointId; }

    // Cluster constants, from the spec.
    static constexpr uint8_t kMaxModeLabelSize = 64;
    static constexpr uint8_t kMaxNumOfModeTags = 8;

    // List change reporting
    /**
     * Reports that the contents of the supported modes attribute have changed.
     * The device SHALL call this method whenever it changes the list of supported modes.
     */
    void ReportSupportedModesChange();

    /**
     * Returns true if the feature is supported.
     * @param feature the feature to check.
     */
    bool HasFeature(Feature feature) const;

    /**
     * This function returns true if the mode value given matches one of the supported modes, otherwise it returns false.
     * @param mode
     */
    bool IsSupportedMode(uint8_t mode);

    // Unregisters this instance if already registered.
    void Shutdown();

    // Get mode value by mode tag
    CHIP_ERROR GetModeValueByModeTag(uint16_t modeTag, uint8_t & value);

    bool GetFailTransition() { return failTransition; }
    void ToggleFailTransition() { failTransition = !failTransition; }

private:
    Delegate * mDelegate;

    EndpointId mEndpointId;
    ClusterId mClusterId;

    // Attribute data store
    uint8_t mCurrentMode = 0; // This is a temporary value and may not be valid. We will change this to the value of the first
                              // mode in the list at the start of the Init function to ensure that it represents a valid mode.
    DataModel::Nullable<uint8_t> mStartUpMode;
    DataModel::Nullable<uint8_t> mOnMode;
    uint32_t mFeature;

    bool failTransition = false;

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    /**
     * Register this ModeBase instance in gModeBaseAliasesInstances.
     */
    void RegisterThisInstance();

    /**
     * Unregister this ModeBase instance in gModeBaseAliasesInstances.
     */
    void UnregisterThisInstance();

    /**
     * Internal change-to-mode command handler function.
     */
    void HandleChangeToMode(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & req);

    /**
     * Helper function that loads all the persistent attributes from the KVS. These attributes are CurrentMode,
     * StartUpMode and OnMode.
     */
    void LoadPersistentAttributes();

    /**
     * Helper function that encodes the supported modes.
     * @param encoder The encoder to encode the supported modes into.
     */
    CHIP_ERROR EncodeSupportedModes(const AttributeValueEncoder::ListEncodeHelper & encoder);
};

// This does not return a reference to const IntrusiveList, because the caller might need
// to change the state of the instances in the list and const IntrusiveList only allows
// access to const Instance.
IntrusiveList<Instance> & GetModeBaseInstanceList();

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip

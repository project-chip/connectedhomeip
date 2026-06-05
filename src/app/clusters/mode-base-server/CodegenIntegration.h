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

#include <app/clusters/mode-base-server/Delegate.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/mode-base-server/mode-base-cluster-objects.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/support/IntrusiveList.h>

namespace chip::app::Clusters::ModeBase {

class Instance : public IntrusiveListNodeBase<>
{
public:
    /**
     * Creates a ModeBase cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aDelegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase derived cluster to be instantiated.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature);

    /**
     * @brief Destroys a ModeBase cluster instance. The Deinit() function needs to be called for this instance
     * to be unregistered and stopped from being called by the interaction model.
     */
    ~Instance();

    /**
     * Initialise the ModeBase cluster instance.
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
    EndpointId GetEndpointId() const { return mClusterPath.mEndpointId; }

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
    bool HasFeature(ModeBase::Feature feature) const { return mFeature.Has(feature); }

    /**
     * This function returns true if the mode value given matches one of the supported modes, otherwise it returns false.
     * @param mode
     */
    bool IsSupportedMode(uint8_t mode);

    // Unregisters this instance if already registered.
    void Shutdown();

    // Get mode value by mode tag
    CHIP_ERROR GetModeValueByModeTag(uint16_t modeTag, uint8_t & value);

    bool GetFailTransition() const { return mFailTransition; }
    void ToggleFailTransition() { mFailTransition = !mFailTransition; }

private:
    Delegate * mDelegate{};
    ConcreteClusterPath mClusterPath{};
    BitMask<ModeBase::Feature> mFeature{};
    ModeBaseCluster::OptionalAttributeSet mOptionalAttributeSet{};
    bool mFailTransition = false;

    // The Code Driven ModeBase cluster instance (lazy-initialized)
    chip::app::LazyRegisteredServerCluster<ModeBaseCluster> mCluster;

    void RegisterThisInstance();
    void UnregisterThisInstance();
};

// This does not return a reference to const IntrusiveList, because the caller might need
// to change the state of the instances in the list and const IntrusiveList only allows
// access to const Instance.
IntrusiveList<Instance> & GetModeBaseInstanceList();

} // namespace chip::app::Clusters::ModeBase

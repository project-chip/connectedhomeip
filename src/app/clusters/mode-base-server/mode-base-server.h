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

#include "mode-base-cluster-objects.h"
#include <app/AttributeAccessInterface.h>
#include <app/AttributePersistenceProvider.h>
#include <app/CommandHandlerInterface.h>
#include <lib/support/IntrusiveList.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

class Delegate;

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

class Delegate
{
protected:
    Instance * mInstance = nullptr;

public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of an Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     */
    void SetInstance(Instance * aInstance) { mInstance = aInstance; }

    // The following functions should be overridden by the SDK user to implement the business logic of their application.
    /**
     * This init function will be called during the ModeBase server initialization after the Instance information has been
     * validated and the Instance has been registered. This can be used to initialise app logic.
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * Get the mode label of the Nth mode in the list of modes.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param label A reference to the mutable char span which will be mutated to receive the label on success. Use
     * CopyCharSpanToMutableCharSpan to copy into the MutableCharSpan.
     * @return Returns a CHIP_NO_ERROR if there was no error and the label was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available labels.
     *
     * Note: This is used by the SDK to populate the supported modes attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportSupportedModesChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) = 0;

    /**
     * Get the mode value of the Nth mode in the list of modes.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param value a reference to the uint8_t variable that is to contain the mode value.
     * @return Returns a CHIP_NO_ERROR if there was no error and the value was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available values.
     *
     * Note: This is used by the SDK to populate the supported modes attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportSupportedModesChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) = 0;

    /**
     * Get the mode tags of the Nth mode in the list of modes.
     * The caller will make sure the List points to an existing buffer of sufficient size to hold the spec-required number
     * of tags, and the size of the List is the size of the buffer.
     *
     * The implementation must place its desired ModeTagStructType instances in that buffer and call tags.reduce_size
     * on the list to indicate how many entries were initialized.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param tags a reference to an existing and initialised buffer that is to contain the mode tags. std::copy can be used
     * to copy into the buffer.
     * @return Returns a CHIP_NO_ERROR if there was no error and the mode tags were returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available mode tags.
     *
     * Note: This is used by the SDK to populate the supported modes attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportSupportedModesChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) = 0;

    /**
     * When a ChangeToMode command is received, if the NewMode value is a supported mode, this method is called to 1) decide if
     * we should go ahead with transitioning to this mode and 2) formulate the ChangeToModeResponse that will be sent back to the
     * client. If this function returns a response.status of StatusCode::kSuccess, the change request is accepted
     * and the CurrentMode is set to the NewMode. Else, the CurrentMode is left untouched. The response is sent as a
     * ChangeToModeResponse command.
     *
     * This function is to be overridden by a user implemented function that makes this decision based on the application logic.
     * @param NewMode The new made that the device is requested to transition to.
     * @param response A reference to a response that will be sent to the client. The contents of which con be modified by the
     * application.
     *
     */
    virtual void HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response) = 0;
};

// A set of pointers to all initialised ModeBase instances. It provides a way to access all ModeBase derived clusters.
// todo change once there is a clear public interface for the OnOff cluster data dependencies (#27508)
static IntrusiveList<Instance> gModeBaseAliasesInstances;

// This does not return a reference to const IntrusiveList, because the caller might need
// to change the state of the instances in the list and const IntrusiveList only allows
// access to const Instance.
IntrusiveList<Instance> & GetModeBaseInstanceList();

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip

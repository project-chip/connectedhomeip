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
#include <app/util/af.h>
#include <map>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    /**
     * This is a helper function to build a mode option structure. It takes the label/name of the mode,
     * the value of the mode and a list of mode tags that apply to this mode.
     *
     * **NOTE** The caller must ensure that the lifetime of the label and modeTags is as long as the
     * returned structure.
     */
    static chip::app::Clusters::detail::Structs::ModeOptionStruct::Type
    BuildModeOptionStruct(const char * label, uint8_t mode,
                          const DataModel::List<const chip::app::Clusters::detail::Structs::ModeTagStruct::Type> modeTags)
    {
        chip::app::Clusters::detail::Structs::ModeOptionStruct::Type option;
        option.label    = CharSpan::fromCharString(label);
        option.mode     = mode;
        option.modeTags = modeTags;
        return option;
    }

    /**
     * Initialise the ModeBase server instance.
     * @return Returns an error if the cluster ID given is not af a valid ModeBase cluster, if the
     * given endpoint and cluster ID have not been enabled in zap, if the CommandHandler or
     * AttributeHandler registration fails or if the AppInit() returns an error.
     */
    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;
    CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Attribute setters
    chip::Protocols::InteractionModel::Status UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode);
    chip::Protocols::InteractionModel::Status UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode);
    chip::Protocols::InteractionModel::Status UpdateCurrentMode(uint8_t aNewMode);

    // Attribute getters
    DataModel::Nullable<uint8_t> GetStartUpMode() const;
    DataModel::Nullable<uint8_t> GetOnMode() const;
    uint8_t GetCurrentMode() const;
    EndpointId GetEndpointId() const { return mEndpointId; }

    // Cluster constants
    static constexpr uint8_t kMaxModeLabelSize = 64;
    static constexpr uint8_t kMaxNumOfModeTags = 8;

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

private:
    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    // Attribute data store
    uint8_t mCurrentMode;
    DataModel::Nullable<uint8_t> mStartUpMode;
    DataModel::Nullable<uint8_t> mOnMode;

    uint32_t mFeature;

    /**
     * This checks to see if this clusters instance is a valid ModeBase aliased cluster based on the AliasedClusters list.
     * @return true if the clusterId of this instance is a valid ModeBase cluster.
     */
    bool isDerivedCluster() const;

    /**
     * Internal change-to-mode command handler function.
     */
    void handleChangeToMode(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & req);

    /**
     * Helper function that loads all the persistent attributes from the KVS. These attributes are CurrentMode,
     * StartUpMode and OnMode.
     */
    void loadPersistentAttributes();

    /**
     * Helper function that encodes the supported modes.
     * @param encoder The encoder to encode the supported modes into.
     */
    CHIP_ERROR encodeSupportedModes(const AttributeValueEncoder::ListEncodeHelper &encoder);

public:
    /**
     * Creates a mode base cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase aliased cluster to be instantiated.
     * @param aFeature The bitmask value that identifies which features are supported by this instance.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mEndpointId(aEndpointId), mClusterId(aClusterId),
        mFeature(aFeature)
    {
        mCurrentMode = 0; // This is a temporary value and may not be valid. We will change this to the value of the first
                          // mode in the list at the start of the Init function to ensure that it represents a valid mode.
        mStartUpMode = DataModel::Nullable<uint8_t>(); // Initialised to null
        mOnMode      = DataModel::Nullable<uint8_t>(); // Initialised to null
    }

    ~Instance() override;

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    // The following functions should be overridden by the SDK user to implement the business logic of their application.

    /**
     * This init function will be called during the ModeBase server initialization after the Instance information has been
     * validated and the Instance has been registered. This can be used to initialise app logic.
     */
    virtual CHIP_ERROR AppInit() = 0;

    /**
     * Returns the number of modes managed by this instance.
     */
    virtual uint8_t NumberOfModes() = 0;

    /**
     * Get the mode label of the Nth mode in the list of modes.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param label A reference to the mutable char span which will be mutated to receive the label on success. Use
     * CopyCharSpanToMutableCharSpan to copy into the MutableCharSpan.
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label) = 0;

    /**
     * Get the mode value of the Nth mode in the list of modes.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param value a reference to the uint8_t variable that is to contain the mode value.
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) = 0;

    /**
     * Get the mode tags of the Nth mode in the list of modes.
     * The caller must make sure the List points to an existing buffer of sufficient size to hold the spec-required number
     * of tags, and the size of the List is the size of the buffer.
     *
     * The implementation must place its desired ModeTagStructType instances in that buffer and call tags.reduce_size
     * on the list to indicate how many entries were initialized.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param tags a reference to an existing and initialised buffer that is to contain the mode tags. std::copy can be used
     * to copy into the buffer.
     * @return Returns a CHIP_NO_ERROR if there was no error.
     */
    virtual CHIP_ERROR
    GetModeTagsByIndex(uint8_t modeIndex,
                       DataModel::List<chip::app::Clusters::detail::Structs::ModeTagStruct::Type> & modeTags) = 0;

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

// This map holds pointers to all initialised ModeBase instances. It provides a way to access all ModeBase clusters.
// todo change once there is a clear public interface for the OnOff cluster data dependencies (#27508)
static std::map<uint32_t, Instance *> ModeBaseAliasesInstanceMap;

std::map<uint32_t, Instance *> GetModeBaseInstances();

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip

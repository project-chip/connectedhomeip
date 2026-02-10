/*
 *
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/Chime/Attributes.h>
#include <clusters/Chime/Commands.h>
#include <clusters/Chime/Structs.h>

namespace chip {
namespace app {
namespace Clusters {

class ChimeDelegate;

class ChimeCluster : public DefaultServerCluster
{
public:
    /**
     * Creates a Chime Cluster instance.
     * @param aEndpointId The endpoint on which this cluster exists.
     * @param aDelegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    ChimeCluster(EndpointId endpointId, ChimeDelegate & delegate);
    ~ChimeCluster();

    // Attribute Setters
    /**
     * Sets the SelectedChime attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param chimeSoundID The value to which the SelectedChime  is to be set.
     * @return Returns a ConstraintError if the chimeSoundID value is not valid. Returns Success otherwise.
     */
    Protocols::InteractionModel::Status SetSelectedChime(uint8_t chimeSoundID);

    /**
     * Sets the Enabled attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param Enabled The value to which the Enabled  is to be set.
     */
    Protocols::InteractionModel::Status SetEnabled(bool enabled);

    // Attribute Getters
    /**
     * @return The Current SelectedChime.
     */
    uint8_t GetSelectedChime() const;

    /**
     * @return The Enabled attribute.
     */
    bool GetEnabled() const;

    // Cluster constants from the spec
    static constexpr uint8_t kMaxChimeSoundNameSize = 48;

    /**
     * @brief ServerClusterInterface methods.
     */
    CHIP_ERROR Startup(ServerClusterContext & context) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

private:
    ChimeDelegate & mDelegate;

    // Attribute local storage
    uint8_t mSelectedChime = 0;
    bool mEnabled          = true;
    // Helpers
    // Loads all the persistent attributes from the KVS.
    void LoadPersistentAttributes();

    // Checks if the chimeID is supported by the delegate
    bool IsSupportedChimeID(uint8_t chimeID);

    // Encodes all Installed Chime Sounds
    CHIP_ERROR EncodeSupportedChimeSounds(const AttributeValueEncoder::ListEncodeHelper & encoder);

    DataModel::ActionReturnStatus HandlePlayChimeSound(CommandHandler & aHandler, const ConcreteCommandPath & aPath,
                                                       const Chime::Commands::PlayChimeSound::DecodableType & commandData);

    void GenerateChimeStartedPlayingEvent(const uint8_t chimeID);
};

/** @brief
 *  Defines methods for implementing application-specific logic for the Chime Cluster.
 */
class ChimeDelegate
{
public:
    ChimeDelegate() = default;

    virtual ~ChimeDelegate() = default;

    /**
     * Get the installed  chime sounds.
     * @param index The index of the chime sound to be returned. It is assumed that chime sounds are indexable from 0 and with no
     * gaps.
     * @param chimeID a reference to the uint8_t variable that is to contain the ChimeID value.
     * @param name  A reference to the mutable char span which will be mutated to receive the chime sound name on success. Use
     * CopyCharSpanToMutableCharSpan to copy into the MutableCharSpan.
     * @return Returns a CHIP_NO_ERROR if there was no error and the chime sound details were returned successfully,
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED. if the index is beyond the list of available chime sounds.
     *
     * Note: This is used by the SDK to populate the InstalledChimeSounds attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportInstalledChimeSoundsChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, MutableCharSpan & name) = 0;

    /**
     * Get the installed  chime sound IDs
     * @param index The index of the chime ID to be returned. It is assumed that chime sounds are indexable from 0 and with no
     * gaps.
     * @param chimeID a reference to the uint8_t variable that is to contain the ChimeID value.
     * @return Returns a CHIP_NO_ERROR if there was no error and the ChimeID was returned successfully,
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is beyond the list of available chime sounds.
     *
     * Note: This is used by the SDK to help populate the InstalledChimeSounds attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportInstalledChimeSoundsChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) = 0;

    // Commands
    /**
     * @brief Delegate should implement a handler to play the currently active chime sound.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     * @param chimeID the identifier for the chime to be played
     */
    virtual Protocols::InteractionModel::Status PlayChimeSound(uint8_t chimeID) = 0;

protected:
    friend class ChimeCluster;

    // This is named mChimeServer instead of mChimeCluster to preserve backwards compatibility with legacy usage.
    ChimeCluster * mChimeServer = nullptr;

    void SetChimeCluster(ChimeCluster * chimeCluster) { mChimeServer = chimeCluster; }
    ChimeCluster * GetChimeCluster() const { return mChimeServer; }
};

} // namespace Clusters
} // namespace app
} // namespace chip

/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class ChimeDelegate;

class ChimeServer : private AttributeAccessInterface, private CommandHandlerInterface
{
public:
    /**
     * Creates a chime server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aDelegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    ChimeServer(EndpointId endpointId, ChimeDelegate & delegate);
    ~ChimeServer();

    /**
     * Initialise the chime server instance.
     * @return Returns an error  if the CommandHandler or AttributeHandler registration fails.
     */
    CHIP_ERROR Init();

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
    Protocols::InteractionModel::Status SetEnabled(bool Enabled);

    // Attribute Getters
    /**
     * @return The Current SelectedChime.
     */
    uint8_t GetSelectedChime() const;

    /**
     * @return The Enabled attribute..
     */
    bool GetEnabled() const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

    // Cluster constants from the spec
    static constexpr uint8_t kMaxChimeSoundNameSize = 48;

    // List Change Reporting
    /**
     * Reports that the contents of the InstalledChimeSounds attribute have changed.
     * The device SHALL call this method whenever it changes the list of installed chime sounds.
     */
    void ReportInstalledChimeSoundsChange();

private:
    ChimeDelegate & mDelegate;

    // Attribute local storage
    uint8_t mSelectedChime;
    bool mEnabled;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    void HandlePlayChimeSound(HandlerContext & ctx, const Chime::Commands::PlayChimeSound::DecodableType & req);

    // Helpers
    // Loads all the persistent attributes from the KVS.
    void LoadPersistentAttributes();

    // Checks if the chimeID is supported by the delegate
    bool IsSupportedChimeID(uint8_t chimeID);

    // Encodes all Installed Chime Sounds
    CHIP_ERROR EncodeSupportedChimeSounds(const AttributeValueEncoder::ListEncodeHelper & encoder);
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
     * CHIP_ERROR_NOT_FOUND if the index in beyond the list of available chime sounds.
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
     * CHIP_ERROR_NOT_FOUND if the index in beyond the list of available chime sounds.
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
     */
    virtual Protocols::InteractionModel::Status PlayChimeSound() = 0;

protected:
    friend class ChimeServer;

    ChimeServer * mChimeServer = nullptr;

    // sets the Chime Server pointer
    void SetChimeServer(ChimeServer * chimeServer) { mChimeServer = chimeServer; }
    ChimeServer * GetChimeServer() const { return mChimeServer; }
};

} // namespace Clusters
} // namespace app
} // namespace chip

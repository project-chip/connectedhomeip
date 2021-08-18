/*
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

#pragma once

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <app/util/basic-types.h>
#include <controller/data_model/zap-generated/CHIPClientCallbacks.h>

namespace chip {
namespace Controller {

// The interface for the underlying connection of Interaction Peer
class InteractionPeerDelegate
{
public:
    /**
     * @brief Setup the peer connection if needed.
     *
     */
    virtual CHIP_ERROR PreparePeer() = 0;

    /**
     * @brief Returns the peer's device ID.
     *
     */
    virtual NodeId GetDeviceId() const = 0;

    /**
     * @brief Returns the peer's fabric index.
     *
     */
    virtual FabricIndex GetFabricIndex() const = 0;

    /**
     * @brief Returns the underlying session handle
     *
     */
    virtual SessionHandle * GetSessionHandle() = 0;

    /**
     * @brief Sends a raw message to the peer.
     *
     */
    virtual CHIP_ERROR SendMessage(Protocols::Id protocolId, uint8_t msgType, Messaging::SendFlags sendFlags,
                                   System::PacketBufferHandle && message) = 0;

    virtual ~InteractionPeerDelegate() {}
};

// The interface for generated cluster control client to interact with a remote peer
class InteractionPeer
{
public:
    InteractionPeer(InteractionPeerDelegate * delegate) : mDelegate(delegate) {}

    /**
     * @brief Sends a raw message to the peer.
     *
     */
    CHIP_ERROR SendMessage(Protocols::Id protocolId, uint8_t msgType, Messaging::SendFlags sendFlags,
                           System::PacketBufferHandle && message);

    /**
     * @brief Sends a raw message to the peer, type-safe version.
     *
     */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    CHIP_ERROR SendMessage(MessageType msgType, Messaging::SendFlags sendFlags, System::PacketBufferHandle && message)
    {
        return SendMessage(Protocols::MessageTypeTraits<MessageType>::ProtocolId(), to_underlying(msgType), sendFlags,
                           std::move(message));
    }

    /**
     * @brief Sends read attribute request to peer.
     *
     */
    CHIP_ERROR SendReadAttributeRequest(app::AttributePathParams aPath, Callback::Cancelable * onSuccessCallback,
                                        Callback::Cancelable * onFailureCallback, app::TLVDataFilter aTlvDataFilter);

    /**
     * @brief Sends write attribute request to peer.
     *
     */
    CHIP_ERROR SendWriteAttributeRequest(app::WriteClientHandle aHandle, Callback::Cancelable * onSuccessCallback,
                                         Callback::Cancelable * onFailureCallback);

    /**
     * @brief Returns a new sequence number for interaction.
     *
     */
    uint8_t GetNextSequenceNumber() { return mSequenceNumber++; };

    // TODO: Add docstring @erjiaqing
    void AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                            app::TLVDataFilter tlvDataFilter = nullptr);

    // TODO: Add docstring @erjiaqing
    void CancelResponseHandler(uint8_t seqNum);

    // TODO: Add docstring @erjiaqing
    void AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute, Callback::Cancelable * onReportCallback);

    // This two functions are pretty tricky, it is used to bridge the response, we need to implement interaction model delegate on
    // the app side instead of register callbacks here. The IM delegate can provide more infomation then callback and it is
    // type-safe.
    // TODO: Implement interaction model delegate in the application.
    // TODO: Add docstring @erjiaqing
    void AddIMResponseHandler(app::CommandSender * commandObj, Callback::Cancelable * onSuccessCallback,
                              Callback::Cancelable * onFailureCallback);

    // TODO: Add docstring @erjiaqing
    void CancelIMResponseHandler(app::CommandSender * commandObj);

    /**
     * @brief Send the command in internal command sender.
     *
     */
    CHIP_ERROR SendCommands(app::CommandSender * commandObj);

    /**
     * @brief Returns the device ID of the peer.
     *
     */
    NodeId GetDeviceId() { return mDelegate->GetDeviceId(); }

private:
    InteractionPeerDelegate * mDelegate;
    uint8_t mSequenceNumber                     = 0;
    app::CHIPDeviceCallbacksMgr & mCallbacksMgr = app::CHIPDeviceCallbacksMgr::GetInstance();
};

} // namespace Controller
} // namespace chip

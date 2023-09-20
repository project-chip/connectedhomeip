/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"
#include "sl_status.h"
#include <functional>

#ifndef MPC_SENDABLE_COMMAND
#define MPC_SENDABLE_COMMAND

#define MPC_MAX_COMMAND_RETRY   1

namespace mpc {
using namespace chip;

/**
 * @brief Helper class for sending a chip command.
 *
 * This class implements the boilerpladte code for etablishing a CASE sesison towards a node and
 * sending a command to the node.
 *
 * This object holds the command data and information about the final destination of the command.
 *
 * Example use:
 *   SendableCommand<Clusters::LevelControl::Commands::MoveToLevel::Type> move_to_level;
 *   move_to_level.Data().level = 42;
 *   move_to_level.Send(chip::ScopedNodeId(2,1), 1);
 *
 *
 * @tparam T Must be a command Type, ie Clusters::OnOff::Commands::On::Type or similar.
 */
template <typename T>
class SendableCommand
{
public:
    /**
     * @brief Callbakc which is called when the send operation is compleeted.
     *
     * first argument is the transmission status
     * second argument is the Nodeid to which the transmission was attempted
     */
    using SendDoneCallback = std::function<void(CHIP_ERROR, ScopedNodeId)>;

    /**
     * @brief Construct a new Sendable Command object
     *
     */
    SendableCommand() : mOnConnectedCallback(onConnected, this), mOnConnectionFailureCallback(onConnectFailure, this) {}

    /**
     * @brief Send the command to the to an endpoint on a node. This function will spawn a copy of this object which
     * is which is used durring the whole session. The copy will be dellocated when the session finishes.
     *
     * @param node_id Destination node
     * @param endpoint_id Destination endpoint
     * @param callback Optional callback to notify when the tranmission is done
     */
    void Send(ScopedNodeId node_id, EndpointId endpoint_id, Optional<SendDoneCallback> callback = NullOptionalType())
    {
        SendableCommand * temp_cmd = Platform::New<SendableCommand>();
        temp_cmd->m_endpoint_id    = endpoint_id;
        temp_cmd->mCommand         = mCommand;
        temp_cmd->mSendDone        = callback;
        Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(node_id, &temp_cmd->mOnConnectedCallback,
                                                                              &temp_cmd->mOnConnectionFailureCallback);
    };

    /**
     * @brief Accessor for the command data
     *
     * @return T&
     */
    T & Data() { return mCommand; }

private:
    static void onConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
    {
        SendableCommand<T> * ctx = static_cast<SendableCommand<T> *>(context);
        auto onSuccess           = [ctx, &sessionHandle](const app::ConcreteCommandPath & commandPath, const app::StatusIB & status,
                                               const auto & dataResponse) {
            ChipLogProgress(NotSpecified, "command succeeds");

            if (ctx->mSendDone.HasValue())
            {
                ctx->mSendDone.Value()(status.ToChipError(), sessionHandle->GetPeer());
            }
            Platform::Delete(ctx);
        };
        auto onFailure = [ctx, &sessionHandle](CHIP_ERROR error) {
            ChipLogError(NotSpecified, "command failed: %" CHIP_ERROR_FORMAT, error.Format());
            if (ctx->mSendDone.HasValue())
            {
                ctx->mSendDone.Value()(error, sessionHandle->GetPeer());
            }
            Platform::Delete(ctx);
        };
        auto err = Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, ctx->m_endpoint_id, ctx->mCommand, 
                                                    onSuccess, onFailure);
        // Retry immediately in-case of synchronous send failure (possibly internal failure such as stale session)
        if (err != CHIP_NO_ERROR && ctx->mRetryCount++ < MPC_MAX_COMMAND_RETRY) {
            Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(sessionHandle->GetPeer(), 
                                                    &ctx->mOnConnectedCallback, &ctx->mOnConnectionFailureCallback);
        }
    };

    static void onConnectFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
    {
        SendableCommand<T> * ctx = static_cast<SendableCommand<T> *>(context);
        ChipLogError(NotSpecified, "Connection Failed: %" CHIP_ERROR_FORMAT, error.Format());
        
        if (ctx->mRetryCount++ < MPC_MAX_COMMAND_RETRY) {
            Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(peerId, 
                                                    &ctx->mOnConnectedCallback, &ctx->mOnConnectionFailureCallback);
            return;
        }
        if (ctx->mSendDone.HasValue())
        {
            ctx->mSendDone.Value()(error, peerId);
        }
        Platform::Delete(ctx);
    };

    T mCommand;
    EndpointId m_endpoint_id;
    uint8_t mRetryCount = 0;
    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
    Optional<SendDoneCallback> mSendDone;
};

} // namespace mpc

#endif

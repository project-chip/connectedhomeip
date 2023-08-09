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
#include "mpc_command_sender.hpp"
#include "sl_log.h"

#include "app/server/Server.h"

#define LOG_TAG "mpc_command_sender"
static SessionManagerProvider defaultSessionProvider;
SessionManagerProvider * AttributeReadRequest::caseSessProvider = &defaultSessionProvider;

void AttributeReadRequest::on_device_connected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                               const SessionHandle & sessionHandle)
{
    AttributeReadRequest * handle = reinterpret_cast<AttributeReadRequest *>(context);
    if (!handle)
        return;
    if (SL_STATUS_OK != handle->Send(exchangeMgr, sessionHandle))
    {
        sl_log_error(LOG_TAG, "Sending command failed");
        // Not tearing down session here, as it can be re-used for immediate next send else it gets destroyed on certain timeout
        // anyways
    }
}

void AttributeReadRequest::on_device_connection_failure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    sl_log_error(LOG_TAG, "on_device_connection_failure Some Problem: %s ", error.AsString());
    AttributeReadRequest * handle = reinterpret_cast<AttributeReadRequest *>(context);
    if (handle && handle->mCallbacks)
        handle->mCallbacks->OnError(error);
}

sl_status_t AttributeReadRequest::Send(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    CHIP_ERROR err;
    ReadPrepareParams params(sessionHandle);
    params.mpAttributePathParamsList    = mPath.data();
    params.mAttributePathParamsListSize = mPath.size();
    mBufferedReadAdapter                = Platform::MakeShared<BufferedReadCallback>(*mCallbacks);

    client = Platform::MakeUnique<ReadClient>(InteractionModelEngine::GetInstance(), &exchangeMgr, *mBufferedReadAdapter,
                                              ReadClient::InteractionType::Read);
    if (CHIP_NO_ERROR != (err = client->SendRequest(params)))
    {
        sl_log_error(LOG_TAG, "Some Problem");
        mCallbacks->OnError(err);
        return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
}

sl_status_t AttributeReadRequest::SendCommand()
{
    ScopedNodeId nodeId(mDest, 1); // TODO: Maybe we need to take fabricIndex as a parameter as well?
    caseSessProvider->FindOrEstablishSession(nodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    return SL_STATUS_OK;
}

sl_status_t SubscribeRequest::Send(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    CHIP_ERROR err;

    ReadPrepareParams params(sessionHandle);
    params.mpAttributePathParamsList    = mPath.data();
    params.mAttributePathParamsListSize = mPath.size();
    params.mMinIntervalFloorSeconds     = mMinInterval;
    params.mMaxIntervalCeilingSeconds   = mMaxInterval;
    params.mKeepSubscriptions           = mKeepSubs;

    mBufferedReadAdapter = Platform::MakeShared<BufferedReadCallback>(*mCallbacks);

    client = Platform::MakeUnique<ReadClient>(InteractionModelEngine::GetInstance(), &exchangeMgr, *mBufferedReadAdapter,
                                              ReadClient::InteractionType::Subscribe);
    if (CHIP_NO_ERROR != (err = client->SendRequest(params)))
    {
        sl_log_error(LOG_TAG, "Some Problem");
        mCallbacks->OnError(err);
        return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
}

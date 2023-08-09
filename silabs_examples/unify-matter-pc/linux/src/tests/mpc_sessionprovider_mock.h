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

/**
 * @defgroup mpc_sessionprovider_mock
 * @ingroup mpc_components
 * @brief Implements session provider mock class for unit test
 *
 * @{
 */

#ifndef MPC_SESSIONPROVIDER_MOCK_H
#define MPC_SESSIONPROVIDER_MOCK_H

class TestSessionProvider : public SessionManagerProvider
{
public:
    TestSessionProvider(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle, bool isFail) :
        mExchangeMgr(exchangeMgr), mSessionHandle(sessionHandle), doSuccess(!isFail)
    {
        bkpSessionPvdr                         = AttributeReadRequest::caseSessProvider;
        AttributeReadRequest::caseSessProvider = this;
    }
    ~TestSessionProvider() { AttributeReadRequest::caseSessProvider = bkpSessionPvdr; }
    void FindOrEstablishSession(ScopedNodeId nodeId, Callback::Callback<OnDeviceConnected> * OnConnectedCallback,
                                Callback::Callback<OnDeviceConnectionFailure> * OnConnectionFailureCallback) override
    {
        if (OnConnectedCallback && doSuccess)
            OnConnectedCallback->mCall(OnConnectedCallback->mContext, mExchangeMgr, mSessionHandle);
        else if (OnConnectionFailureCallback)
            OnConnectionFailureCallback->mCall(OnConnectionFailureCallback->mContext, nodeId, CHIP_ERROR_CONNECTION_ABORTED);
    }

private:
    Messaging::ExchangeManager & mExchangeMgr;
    const SessionHandle & mSessionHandle;
    SessionManagerProvider * bkpSessionPvdr = nullptr;
    bool doSuccess;
};

#endif // MPC_SESSIONPROVIDER_MOCK_H
/** @} end mpc_sessionprovider_mock */

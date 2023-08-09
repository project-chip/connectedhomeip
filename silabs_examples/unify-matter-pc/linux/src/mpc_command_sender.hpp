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

/**
 * @defgroup mpc_command_sender MPC Command Sender
 * @ingroup mpc_components
 * @brief mpc_command_sender implements reusable classes to send various matter commands
 *
 * @{
 */

#ifndef MPC_COMMAND_SENDER_HPP
#define MPC_COMMAND_SENDER_HPP

#include "app/BufferedReadCallback.h"
#include "app/ConcreteAttributePath.h"
#include "app/InteractionModelEngine.h"
#include "app/server/Server.h"

#include "sl_status.h"

using namespace chip;
using namespace chip::app;

class SessionManagerProvider
{
public:
    virtual ~SessionManagerProvider() = default;
    virtual void FindOrEstablishSession(ScopedNodeId nodeId, Callback::Callback<OnDeviceConnected> * OnConnectedCallback,
                                        Callback::Callback<OnDeviceConnectionFailure> * OnConnectionFailureCallback)
    {
        Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(nodeId, OnConnectedCallback,
                                                                              OnConnectionFailureCallback);
    }
};
/**
 * @brief Class to send ReadAttribute matter command from MPC
 */
class AttributeReadRequest
{
public:
    virtual ~AttributeReadRequest() = default;
    /**
     * @brief Construct a new AttributeReadRequest object
     *
     * @param dest destination matter node id for which attribute needs to be read
     * @param epID endpointID in destination to which the cluster and attribute to be read belong
     * @param clustID cluster ID to which attribute to be read belong
     * @param attrID attribute ID for the attribute is to be read
     */
    AttributeReadRequest(NodeId dest, EndpointId epID, ClusterId clustID, AttributeId attrID) :
        mDest(dest), mPath(1, AttributePathParams(epID, clustID, attrID)), mOnConnectedCallback(on_device_connected, this),
        mOnConnectionFailureCallback(on_device_connection_failure, this)
    {}

    /**
     * @brief Construct a new AttributeReadRequest object
     *
     * @param dest destination matter node id for which attribute needs to be read
     * @param path vector of AttributePathParams containing the path of attributes that are to be read
     */
    AttributeReadRequest(NodeId dest, std::vector<AttributePathParams> & path) :
        mDest(dest), mPath(path), mOnConnectedCallback(on_device_connected, this),
        mOnConnectionFailureCallback(on_device_connection_failure, this)
    {}

    /**
     * @brief Registers a callback delegate to be invoke upon completion of read attribute
     *
     * @param callbacks class object implementing the callbacks to be invoked
     */
    void SetCallbacks(ReadClient::Callback * callbacks) { mCallbacks = callbacks; };

    /**
     * @brief Establishes a case session if doesn't already exist and then sends the command
     */
    sl_status_t SendCommand();

    /**
     * @brief Sends the command by re-using already available session
     *
     * @param exchangeMgr exchange manager linked to available session
     * @param sessionHandle session handle to available session
     */
    virtual sl_status_t Send(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

private:
    friend class chip::app::TestReadInteraction;
    friend class SubscribeRequest;
    friend class TestSessionProvider;

    Platform::UniquePtr<ReadClient> client;
    Platform::SharedPtr<BufferedReadCallback> mBufferedReadAdapter;
    ReadClient::Callback * mCallbacks;
    NodeId mDest;
    std::vector<AttributePathParams> mPath;
    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
    static SessionManagerProvider * caseSessProvider;

    static void on_device_connection_failure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);
    static void on_device_connected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
};

/**
 * @brief Structure holds parameter for Subscribe matter command
 */
typedef struct
{

    /// @brief minimum expected interval between consecutive subcription reports
    /// (should be a floored 16-bit integer value )
    uint16_t minInterval;
    /// @brief maximum expected interval between consecutive subcription reports
    /// (should be a ceiled 16-bit integer value )
    uint16_t maxInterval;
    /// @brief indicates if the existing subscription from MPC is to be retained or overwritten
    bool keepSubscription;
} SubscribeRequestParams;

/**
 * @brief Class to send Subscribe matter command from MPC
 */
class SubscribeRequest : public AttributeReadRequest
{
public:
    /**
     * @brief Construct a new SubscribeRequest object
     *
     * @param dest destination matter node id for which attribute needs to be read
     * @param epID endpointID in destination to which the cluster and attribute to be read belong
     * @param clustID cluster ID to which attribute to be read belong
     * @param attrID attribute ID for the attribute is to be read
     * @param params subscription parameters @ref SubscribeRequestParams
     */
    SubscribeRequest(NodeId dest, EndpointId epID, ClusterId clustID, AttributeId attrID, SubscribeRequestParams params) :
        AttributeReadRequest(dest, epID, clustID, attrID), mMinInterval(params.minInterval), mMaxInterval(params.maxInterval),
        mKeepSubs(params.keepSubscription)
    {}

    /**
     * @brief Construct a new SubscribeRequest object
     *
     * @param dest destination matter node id for which attribute needs to be read
     * @param path vector of AttributePathParams containing the path of attributes that are to be read
     * @param params subscription parameters @ref SubscribeRequestParams
     */
    SubscribeRequest(NodeId dest, std::vector<AttributePathParams> & path, SubscribeRequestParams params) :
        AttributeReadRequest(dest, path), mMinInterval(params.minInterval), mMaxInterval(params.maxInterval),
        mKeepSubs(params.keepSubscription)
    {}

    /**
     * @brief Sends the command by re-using already available session
     *
     * @param exchangeMgr exchange manager linked to available session
     * @param sessionHandle session handle to available session
     */
    sl_status_t Send(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle) override;

private:
    uint16_t mMinInterval;
    uint16_t mMaxInterval;
    bool mKeepSubs;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // MPC_COMMAND_SENDER_HPP
/** @} end mpc_command_sender */

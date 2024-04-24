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

#include "core/BaseCluster.h"
#include "core/Endpoint.h"

#include "lib/support/logging/CHIPLogging.h"
namespace matter {
namespace casting {
namespace core {

template <typename TypeInfo>
using ReadResponseSuccessCallbackFn  = std::function<void(void * context, chip::Optional<typename TypeInfo::DecodableType> before,
                                                         typename TypeInfo::DecodableArgType after)>;
using ReadResponseFailureCallbackFn  = std::function<void(void * context, CHIP_ERROR err)>;
using WriteResponseSuccessCallbackFn = std::function<void(void * context)>;
using WriteResponseFailureCallbackFn = std::function<void(void * context, CHIP_ERROR err)>;

template <typename TypeInfo>
struct ReadAttributeContext;
template <typename TypeInfo>
struct WriteAttributeContext;
template <typename TypeInfo>
struct SubscribeAttributeContext;

template <typename TypeInfo>
class Attribute
{
private:
    bool hasValue = false;
    typename TypeInfo::DecodableType value;

protected:
    memory::Weak<core::Endpoint> GetEndpoint() const { return mEndpoint.lock(); }
    memory::Weak<core::Endpoint> mEndpoint;

public:
    Attribute(memory::Weak<core::Endpoint> endpoint) { this->mEndpoint = endpoint; }

    ~Attribute() {}

    Attribute()                       = delete;
    Attribute(Attribute & other)      = delete;
    void operator=(const Attribute &) = delete;

    chip::Optional<typename TypeInfo::DecodableType> GetValue()
    {
        return hasValue ? chip::MakeOptional(value) : chip::NullOptional;
    }

    /**
     * @brief Reads the value of the Attribute that belongs to the associated Endpoint and corresponding Cluster
     * @param context current context passed back in successCb/FailureCb
     * @param successCb Called when the Attribute is read successfully, with the value of the attribute after reading, as well as
     * before (if the Attribute had been previously read)
     * @param failureCb Called when there is a failure in reading the Attribute
     */
    void Read(void * context, ReadResponseSuccessCallbackFn<TypeInfo> successCb, ReadResponseFailureCallbackFn failureCb)
    {
        memory::Strong<core::Endpoint> endpoint = this->GetEndpoint().lock();
        if (endpoint)
        {
            ReadAttributeContext<TypeInfo> * attributeContext =
                new ReadAttributeContext<TypeInfo>(this, endpoint, context, successCb, failureCb);

            endpoint->GetCastingPlayer()->FindOrEstablishSession(
                attributeContext,
                // FindOrEstablishSession success handler
                [](void * _context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle) {
                    ReadAttributeContext<TypeInfo> * _attributeContext = static_cast<ReadAttributeContext<TypeInfo> *>(_context);
                    ChipLogProgress(AppServer, "<Attribute>::Read() Found or established session");

                    // Read attribute
                    MediaClusterBase mediaClusterBase(exchangeMgr, sessionHandle, _attributeContext->mEndpoint->GetId());
                    CHIP_ERROR err = mediaClusterBase.template ReadAttribute<TypeInfo>(
                        _attributeContext,
                        // Read success handler
                        [](void * __context, typename TypeInfo::DecodableArgType response) {
                            ReadAttributeContext<TypeInfo> * __attributeContext =
                                static_cast<ReadAttributeContext<TypeInfo> *>(__context);
                            ChipLogProgress(AppServer, "<Attribute>::Read() success");
                            Attribute<TypeInfo> * __attr = static_cast<Attribute<TypeInfo> *>(__attributeContext->mAttribute);
                            if (__attr->hasValue)
                            {
                                typename TypeInfo::DecodableType prevValue = __attr->value;
                                __attr->value                              = response;
                                __attributeContext->mSuccessCb(__attributeContext->mClientContext, chip::MakeOptional(prevValue),
                                                               __attr->value);
                            }
                            else
                            {
                                __attr->hasValue = true;
                                __attr->value    = response;
                                __attributeContext->mSuccessCb(__attributeContext->mClientContext, chip::NullOptional,
                                                               __attr->value);
                            }
                            delete __attributeContext;
                        },
                        // Read failure handler
                        [](void * __context, CHIP_ERROR error) {
                            ReadAttributeContext<TypeInfo> * __attributeContext =
                                static_cast<ReadAttributeContext<TypeInfo> *>(__context);
                            ChipLogError(AppServer,
                                         "<Attribute>::Read() failure response on EndpointId: %d with error: "
                                         "%" CHIP_ERROR_FORMAT,
                                         __attributeContext->mEndpoint->GetId(), error.Format());
                            __attributeContext->mFailureCb(__attributeContext->mClientContext, error);
                            delete __attributeContext;
                        });

                    // error in reading the attribute
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(AppServer,
                                     "<Attribute>::Read() failure in reading attribute on EndpointId: %d with error: "
                                     "%" CHIP_ERROR_FORMAT,
                                     _attributeContext->mEndpoint->GetId(), err.Format());
                        _attributeContext->mFailureCb(_attributeContext->mClientContext, err);
                        delete _attributeContext;
                    }
                },
                // FindOrEstablishSession failure handler
                [](void * _context, const chip::ScopedNodeId & peerId, CHIP_ERROR error) {
                    ReadAttributeContext<TypeInfo> * _attributeContext = static_cast<ReadAttributeContext<TypeInfo> *>(_context);
                    ChipLogError(AppServer,
                                 "<Attribute>::Read() failure in retrieving session info for peerId.nodeId: "
                                 "0x" ChipLogFormatX64 ", peer.fabricIndex: %d with error: %" CHIP_ERROR_FORMAT,
                                 ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex(), error.Format());
                    _attributeContext->mFailureCb(_attributeContext->mClientContext, error);
                    delete _attributeContext;
                });
        }
        else
        {
            ChipLogError(AppServer, "<Attribute>::Read() failure in retrieving Endpoint");
            failureCb(context, CHIP_ERROR_INCORRECT_STATE);
        }
    }

    /**
     * @brief Writes the value of the Attribute to an associated Endpoint and corresponding Cluster
     *
     * @param requestData value of the Attribute to be written
     * @param context current context passed back in successCb/FailureCb
     * @param successCb Called when the Attribute is written successfully
     * @param failureCb Called when there is a failure in writing the Attribute
     * @param aTimedWriteTimeoutMs write timeout
     */
    void Write(const typename TypeInfo::Type & requestData, void * context, WriteResponseSuccessCallbackFn successCb,
               WriteResponseFailureCallbackFn failureCb, const chip::Optional<uint16_t> & aTimedWriteTimeoutMs)
    {
        memory::Strong<core::Endpoint> endpoint = this->GetEndpoint().lock();
        if (endpoint)
        {
            WriteAttributeContext<typename TypeInfo::Type> * attributeContext = new WriteAttributeContext<typename TypeInfo::Type>(
                this, endpoint, requestData, context, successCb, failureCb, aTimedWriteTimeoutMs);

            endpoint->GetCastingPlayer()->FindOrEstablishSession(
                attributeContext,
                // FindOrEstablishSession success handler
                [](void * _context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle) {
                    WriteAttributeContext<typename TypeInfo::Type> * _attributeContext =
                        static_cast<WriteAttributeContext<typename TypeInfo::Type> *>(_context);
                    ChipLogProgress(AppServer, "<Attribute>::Write() Found or established session");

                    // Write attribute
                    MediaClusterBase mediaClusterBase(exchangeMgr, sessionHandle, _attributeContext->mEndpoint->GetId());
                    CHIP_ERROR err = mediaClusterBase.template WriteAttribute<TypeInfo>(
                        _attributeContext->mRequestData, _attributeContext,
                        // Write success handler
                        [](void * __context) {
                            WriteAttributeContext<typename TypeInfo::Type> * __attributeContext =
                                static_cast<WriteAttributeContext<typename TypeInfo::Type> *>(__context);
                            ChipLogProgress(AppServer, "<Attribute>::Write() success");
                            __attributeContext->mSuccessCb(__attributeContext->mClientContext);
                            delete __attributeContext;
                        },
                        // Write failure handler
                        [](void * __context, CHIP_ERROR error) {
                            WriteAttributeContext<typename TypeInfo::Type> * __attributeContext =
                                static_cast<WriteAttributeContext<typename TypeInfo::Type> *>(__context);
                            ChipLogError(AppServer,
                                         "<Attribute>::Write() failure response on EndpointId: %d with error: "
                                         "%" CHIP_ERROR_FORMAT,
                                         __attributeContext->mEndpoint->GetId(), error.Format());
                            __attributeContext->mFailureCb(__attributeContext->mClientContext, error);
                            delete __attributeContext;
                        },
                        _attributeContext->mTimedWriteTimeoutMs);

                    // error in writing to the attribute
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(AppServer,
                                     "<Attribute>::Write() failure in reading attribute on EndpointId: %d with error: "
                                     "%" CHIP_ERROR_FORMAT,
                                     _attributeContext->mEndpoint->GetId(), err.Format());
                        _attributeContext->mFailureCb(_attributeContext->mClientContext, err);
                        delete _attributeContext;
                    }
                },
                // FindOrEstablishSession failure handler
                [](void * _context, const chip::ScopedNodeId & peerId, CHIP_ERROR error) {
                    WriteAttributeContext<typename TypeInfo::Type> * _attributeContext =
                        static_cast<WriteAttributeContext<typename TypeInfo::Type> *>(_context);
                    ChipLogError(AppServer,
                                 "<Attribute>::Write() failure in retrieving session info for peerId.nodeId: "
                                 "0x" ChipLogFormatX64 ", peer.fabricIndex: %d with error: %" CHIP_ERROR_FORMAT,
                                 ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex(), error.Format());
                    _attributeContext->mFailureCb(_attributeContext->mClientContext, error);
                    delete _attributeContext;
                });
        }
        else
        {
            ChipLogError(AppServer, "<Attribute>::Write() failure in retrieving Endpoint");
            failureCb(context, CHIP_ERROR_INCORRECT_STATE);
        }
    }

    /**
     * @brief Subscribes to the value of the Attribute that belongs to the associated Endpoint and corresponding Cluster
     *
     * @param context current context passed back in successCb/FailureCb
     * @param successCb Called when the Attribute is read successfully, with the value of the attribute after reading, as well as
     * before (if the Attribute had been previously read)
     * @param failureCb Called when there is a failure in reading the Attribute
     * @param minIntervalFloorSeconds the requested minimum interval boundary floor in seconds for attribute udpates
     * @param maxIntervalCeilingSeconds the requested maximum interval boundary ceiling in seconds for attribute udpates
     */
    void Subscribe(void * context, ReadResponseSuccessCallbackFn<TypeInfo> successCb, ReadResponseFailureCallbackFn failureCb,
                   uint16_t minIntervalFloorSeconds, uint16_t maxIntervalCeilingSeconds)
    {
        memory::Strong<core::Endpoint> endpoint = this->GetEndpoint().lock();
        if (endpoint)
        {
            SubscribeAttributeContext<TypeInfo> * attributeContext = new SubscribeAttributeContext<TypeInfo>(
                this, endpoint, context, successCb, failureCb, minIntervalFloorSeconds, maxIntervalCeilingSeconds);

            endpoint->GetCastingPlayer()->FindOrEstablishSession(
                attributeContext,
                // FindOrEstablishSession success handler
                [](void * _context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle) {
                    SubscribeAttributeContext<TypeInfo> * _attributeContext =
                        static_cast<SubscribeAttributeContext<TypeInfo> *>(_context);
                    ChipLogProgress(AppServer, "<Attribute>::Subscribe() Found or established session");

                    // Subscribe to attribute
                    MediaClusterBase mediaClusterBase(exchangeMgr, sessionHandle, _attributeContext->mEndpoint->GetId());
                    CHIP_ERROR err = mediaClusterBase.template SubscribeAttribute<TypeInfo>(
                        _attributeContext,
                        // Subscription success handler
                        [](void * __context, typename TypeInfo::DecodableArgType response) {
                            SubscribeAttributeContext<TypeInfo> * __attributeContext =
                                static_cast<SubscribeAttributeContext<TypeInfo> *>(__context);
                            ChipLogProgress(AppServer, "<Attribute>::Subscribe() success");
                            Attribute<TypeInfo> * __attr = static_cast<Attribute<TypeInfo> *>(__attributeContext->mAttribute);
                            if (__attr->hasValue)
                            {
                                typename TypeInfo::DecodableType prevValue = __attr->value;
                                __attr->value                              = response;
                                __attributeContext->mSuccessCb(__attributeContext->mClientContext, chip::MakeOptional(prevValue),
                                                               __attr->value);
                            }
                            else
                            {
                                __attr->hasValue = true;
                                __attr->value    = response;
                                __attributeContext->mSuccessCb(__attributeContext->mClientContext, chip::NullOptional,
                                                               __attr->value);
                            }
                            delete __attributeContext;
                        },
                        // Subscription failure handler
                        [](void * __context, CHIP_ERROR error) {
                            SubscribeAttributeContext<TypeInfo> * __attributeContext =
                                static_cast<SubscribeAttributeContext<TypeInfo> *>(__context);
                            ChipLogError(AppServer,
                                         "<Attribute>::Subscribe() failure response on EndpointId: %d with error: "
                                         "%" CHIP_ERROR_FORMAT,
                                         __attributeContext->mEndpoint->GetId(), error.Format());
                            __attributeContext->mFailureCb(__attributeContext->mClientContext, error);
                            delete __attributeContext;
                        },
                        _attributeContext->mMinIntervalFloorSeconds, _attributeContext->mMaxIntervalCeilingSeconds,
                        nullptr /* SubscriptionEstablishedCallback */, nullptr /* ResubscriptionAttemptCallback */,
                        true /* aIsFabricFiltered */, true /* aKeepPreviousSubscriptions */);

                    // error in subscribing to the attribute
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(AppServer,
                                     "<Attribute>::Subscribe() failure in reading attribute on EndpointId: %d with error: "
                                     "%" CHIP_ERROR_FORMAT,
                                     _attributeContext->mEndpoint->GetId(), err.Format());
                        _attributeContext->mFailureCb(_attributeContext->mClientContext, err);
                        delete _attributeContext;
                    }
                },
                // FindOrEstablishSession failure handler
                [](void * _context, const chip::ScopedNodeId & peerId, CHIP_ERROR error) {
                    SubscribeAttributeContext<TypeInfo> * _attributeContext =
                        static_cast<SubscribeAttributeContext<TypeInfo> *>(_context);
                    ChipLogError(AppServer,
                                 "<Attribute>::Subscribe() failure in retrieving session info for peerId.nodeId: "
                                 "0x" ChipLogFormatX64 ", peer.fabricIndex: %d with error: %" CHIP_ERROR_FORMAT,
                                 ChipLogValueX64(peerId.GetNodeId()), peerId.GetFabricIndex(), error.Format());
                    _attributeContext->mFailureCb(_attributeContext->mClientContext, error);
                    delete _attributeContext;
                });
        }
        else
        {
            ChipLogError(AppServer, "<Attribute>::Subscribe() failure in retrieving Endpoint");
            failureCb(context, CHIP_ERROR_INCORRECT_STATE);
        }
    }
};

/**
 * @brief Context object used by the Attribute class during the Read API's execution
 */
template <typename TypeInfo>
struct ReadAttributeContext
{
    ReadAttributeContext(void * attribute, memory::Strong<core::Endpoint> endpoint, void * clientContext,
                         ReadResponseSuccessCallbackFn<TypeInfo> successCb, ReadResponseFailureCallbackFn failureCb) :
        mEndpoint(endpoint),
        mClientContext(clientContext), mSuccessCb(successCb), mFailureCb(failureCb)
    {
        mAttribute = attribute;
    }

    void * mAttribute;
    memory::Strong<core::Endpoint> mEndpoint;
    void * mClientContext;
    ReadResponseSuccessCallbackFn<TypeInfo> mSuccessCb;
    ReadResponseFailureCallbackFn mFailureCb;
};

/**
 * @brief Context object used by the Attribute class during the Write API's execution
 */
template <typename TypeInfoType>
struct WriteAttributeContext
{
    WriteAttributeContext(memory::Strong<core::Endpoint> endpoint, const TypeInfoType & requestData, void * clientContext,
                          WriteResponseSuccessCallbackFn successCb, WriteResponseFailureCallbackFn failureCb,
                          const chip::Optional<uint16_t> & timedWriteTimeoutMs) :
        mEndpoint(endpoint),
        mClientContext(clientContext), mSuccessCb(successCb), mFailureCb(failureCb)
    {
        mRequestData         = requestData;
        mTimedWriteTimeoutMs = timedWriteTimeoutMs;
    }

    memory::Strong<core::Endpoint> mEndpoint;
    TypeInfoType mRequestData;
    void * mClientContext;
    WriteResponseSuccessCallbackFn mSuccessCb;
    WriteResponseFailureCallbackFn mFailureCb;
    chip::Optional<uint16_t> & mTimedWriteTimeoutMs;
};

/**
 * @brief Context object used by the Attribute class during the Subscribe API's execution
 */
template <typename TypeInfo>
struct SubscribeAttributeContext
{
    SubscribeAttributeContext(void * attribute, memory::Strong<core::Endpoint> endpoint, void * clientContext,
                              ReadResponseSuccessCallbackFn<TypeInfo> successCb, ReadResponseFailureCallbackFn failureCb,
                              uint16_t minIntervalFloorSeconds, uint16_t maxIntervalCeilingSeconds) :
        mEndpoint(endpoint),
        mClientContext(clientContext), mSuccessCb(successCb), mFailureCb(failureCb)
    {
        mAttribute                 = attribute;
        mMinIntervalFloorSeconds   = minIntervalFloorSeconds;
        mMaxIntervalCeilingSeconds = maxIntervalCeilingSeconds;
    }

    void * mAttribute;
    memory::Strong<core::Endpoint> mEndpoint;
    void * mClientContext;
    ReadResponseSuccessCallbackFn<TypeInfo> mSuccessCb;
    ReadResponseFailureCallbackFn mFailureCb;
    uint16_t mMinIntervalFloorSeconds;
    uint16_t mMaxIntervalCeilingSeconds;
};

}; // namespace core
}; // namespace casting
}; // namespace matter

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

#include <cstdarg>
#include <memory>
#include <type_traits>

#include <app/BufferedReadCallback.h>
#include <app/DeviceProxy.h>
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <lib/support/CodeUtils.h>

#include <cstdio>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;

using PyObject = void;

extern "C" {
// Encodes n attribute write requests, follows 3 * n arguments, in the (AttributeWritePath*=void *, uint8_t*, size_t) order.
chip::ChipError::StorageType pychip_WriteClient_WriteAttributes(void * appContext, DeviceProxy * device, size_t n, ...);
chip::ChipError::StorageType pychip_ReadClient_ReadAttributes(void * appContext, DeviceProxy * device, bool isSubscription,
                                                              uint32_t minInterval, uint32_t maxInterval, size_t n, ...);
}

namespace chip {
namespace python {

struct __attribute__((packed)) AttributePath
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::AttributeId attributeId;
};

struct __attribute__((packed)) EventPath
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::EventId eventId;
};

using OnReadAttributeDataCallback       = void (*)(PyObject * appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                             chip::AttributeId attributeId,
                                             std::underlying_type_t<Protocols::InteractionModel::Status> imstatus, uint8_t * data,
                                             uint32_t dataLen);
using OnReadEventDataCallback           = void (*)(PyObject * appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                         chip::EventId eventId, chip::EventNumber eventNumber, uint8_t priority, uint64_t timestamp,
                                         uint8_t timestampType, uint8_t * data, uint32_t dataLen);
using OnSubscriptionEstablishedCallback = void (*)(PyObject * appContext, uint64_t subscriptionId);
using OnReadErrorCallback               = void (*)(PyObject * appContext, uint32_t chiperror);
using OnReadDoneCallback                = void (*)(PyObject * appContext);

OnReadAttributeDataCallback gOnReadAttributeDataCallback             = nullptr;
OnReadEventDataCallback gOnReadEventDataCallback                     = nullptr;
OnSubscriptionEstablishedCallback gOnSubscriptionEstablishedCallback = nullptr;
OnReadErrorCallback gOnReadErrorCallback                             = nullptr;
OnReadDoneCallback gOnReadDoneCallback                               = nullptr;

class ReadClientCallback : public ReadClient::Callback
{
public:
    ReadClientCallback(PyObject * appContext) : mBufferedReadCallback(*this), mAppContext(appContext) {}

    app::BufferedReadCallback * GetBufferedReadCallback() { return &mBufferedReadCallback; }

    void OnAttributeData(const ReadClient * apReadClient, const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const StatusIB & aStatus) override
    {
        //
        // We shouldn't be getting list item operations in the provided path since that should be handled by the buffered read
        // callback. If we do, that's a bug.
        //
        VerifyOrDie(!aPath.IsListItemOperation());
        size_t bufferLen                  = (apData == nullptr ? 0 : apData->GetRemainingLength() + apData->GetLengthRead());
        std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(apData == nullptr ? nullptr : new uint8_t[bufferLen]);
        uint32_t size                     = 0;
        // When the apData is nullptr, means we did not receive a valid attribute data from server, status will be some error
        // status.
        if (apData != nullptr)
        {
            // The TLVReader's read head is not pointing to the first element in the container instead of the container itself, use
            // a TLVWriter to get a TLV with a normalized TLV buffer (Wrapped with a anonymous tag, no extra "end of container" tag
            // at the end.)
            TLV::TLVWriter writer;
            writer.Init(buffer.get(), bufferLen);
            CHIP_ERROR err = writer.CopyElement(TLV::AnonymousTag, *apData);
            if (err != CHIP_NO_ERROR)
            {
                app::StatusIB status;
                status.mStatus = Protocols::InteractionModel::Status::Failure;
                this->OnError(apReadClient, err);
                return;
            }
            size = writer.GetLengthWritten();
        }

        gOnReadAttributeDataCallback(mAppContext, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId,
                                     to_underlying(aStatus.mStatus), buffer.get(), size);
    }

    void OnSubscriptionEstablished(const ReadClient * apReadClient) override
    {
        gOnSubscriptionEstablishedCallback(mAppContext, apReadClient->GetSubscriptionId().ValueOr(0));
    }

    void OnEventData(const ReadClient * apReadClient, const EventHeader & aEventHeader, TLV::TLVReader * apData,
                     const StatusIB * apStatus) override
    {
        uint8_t buffer[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
        uint32_t size  = 0;
        CHIP_ERROR err = CHIP_NO_ERROR;
        // When the apData is nullptr, means we did not receive a valid event data from server, status will be some error
        // status.
        if (apData != nullptr)
        {
            // The TLVReader's read head is not pointing to the first element in the container instead of the container itself, use
            // a TLVWriter to get a TLV with a normalized TLV buffer (Wrapped with a anonymous tag, no extra "end of container" tag
            // at the end.)
            TLV::TLVWriter writer;
            writer.Init(buffer);
            err = writer.CopyElement(TLV::AnonymousTag, *apData);
            if (err != CHIP_NO_ERROR)
            {
                this->OnError(apReadClient, err);
                return;
            }
            size = writer.GetLengthWritten();
        }
        else
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            this->OnError(apReadClient, err);
        }

        gOnReadEventDataCallback(mAppContext, aEventHeader.mPath.mEndpointId, aEventHeader.mPath.mClusterId,
                                 aEventHeader.mPath.mEventId, aEventHeader.mEventNumber, to_underlying(aEventHeader.mPriorityLevel),
                                 aEventHeader.mTimestamp.mValue, to_underlying(aEventHeader.mTimestamp.mType), buffer, size);
    }

    void OnError(const ReadClient * apReadClient, CHIP_ERROR aError) override
    {
        gOnReadErrorCallback(mAppContext, aError.AsInteger());
    }

    void OnDone(ReadClient * apReadClient) override
    {
        gOnReadDoneCallback(mAppContext);
        // delete apReadClient;
        delete this;
    };

private:
    BufferedReadCallback mBufferedReadCallback;
    PyObject * mAppContext;
};

using OnWriteResponseCallback = void (*)(PyObject * appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                         chip::AttributeId attributeId,
                                         std::underlying_type_t<Protocols::InteractionModel::Status> imstatus);
using OnWriteErrorCallback    = void (*)(PyObject * appContext, uint32_t chiperror);
using OnWriteDoneCallback     = void (*)(PyObject * appContext);

OnWriteResponseCallback gOnWriteResponseCallback = nullptr;
OnWriteErrorCallback gOnWriteErrorCallback       = nullptr;
OnWriteDoneCallback gOnWriteDoneCallback         = nullptr;

class WriteClientCallback : public WriteClient::Callback
{
public:
    WriteClientCallback(PyObject * appContext) : mAppContext(appContext) {}

    void OnResponse(const WriteClient * apWriteClient, const ConcreteAttributePath & aPath, app::StatusIB aStatus) override
    {
        gOnWriteResponseCallback(mAppContext, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId,
                                 to_underlying(aStatus.mStatus));
    }

    void OnError(const WriteClient * apWriteClient, const StatusIB &, CHIP_ERROR aProtocolError) override
    {
        gOnWriteErrorCallback(mAppContext, aProtocolError.AsInteger());
    }

    void OnDone(WriteClient * apWriteClient) override
    {
        gOnWriteDoneCallback(mAppContext);
        // delete apWriteClient;
        delete this;
    };

private:
    PyObject * mAppContext = nullptr;
};

} // namespace python
} // namespace chip

using namespace chip::python;

extern "C" {
void pychip_WriteClient_InitCallbacks(OnWriteResponseCallback onWriteResponseCallback, OnWriteErrorCallback onWriteErrorCallback,
                                      OnWriteDoneCallback onWriteDoneCallback)
{
    gOnWriteResponseCallback = onWriteResponseCallback;
    gOnWriteErrorCallback    = onWriteErrorCallback;
    gOnWriteDoneCallback     = onWriteDoneCallback;
}

void pychip_ReadClient_InitCallbacks(OnReadAttributeDataCallback onReadAttributeDataCallback,
                                     OnReadEventDataCallback onReadEventDataCallback,
                                     OnSubscriptionEstablishedCallback onSubscriptionEstablishedCallback,
                                     OnReadErrorCallback onReadErrorCallback, OnReadDoneCallback onReadDoneCallback)
{
    gOnReadAttributeDataCallback       = onReadAttributeDataCallback;
    gOnReadEventDataCallback           = onReadEventDataCallback;
    gOnSubscriptionEstablishedCallback = onSubscriptionEstablishedCallback;
    gOnReadErrorCallback               = onReadErrorCallback;
    gOnReadDoneCallback                = onReadDoneCallback;
}

chip::ChipError::StorageType pychip_WriteClient_WriteAttributes(void * appContext, DeviceProxy * device, size_t n, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::unique_ptr<WriteClientCallback> callback = std::make_unique<WriteClientCallback>(appContext);
    app::WriteClientHandle client;

    va_list args;
    va_start(args, n);

    SuccessOrExit(err = app::InteractionModelEngine::GetInstance()->NewWriteClient(client, callback.get()));

    {
        for (size_t i = 0; i < n; i++)
        {
            void * path = va_arg(args, void *);
            void * tlv  = va_arg(args, void *);
            int length  = va_arg(args, int);

            python::AttributePath pathObj;
            memcpy(&pathObj, path, sizeof(python::AttributePath));
            uint8_t * tlvBuffer = reinterpret_cast<uint8_t *>(tlv);

            TLV::TLVWriter * writer;
            TLV::TLVReader reader;

            SuccessOrExit(err = client->PrepareAttribute(
                              chip::app::AttributePathParams(pathObj.endpointId, pathObj.clusterId, pathObj.attributeId)));
            VerifyOrExit((writer = client->GetAttributeDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

            reader.Init(tlvBuffer, static_cast<uint32_t>(length));
            reader.Next();
            SuccessOrExit(
                err = writer->CopyElement(chip::TLV::ContextTag(to_underlying(chip::app::AttributeDataIB::Tag::kData)), reader));

            SuccessOrExit(err = client->FinishAttribute());
        }
    }

    SuccessOrExit(err = device->SendWriteAttributeRequest(std::move(client), nullptr, nullptr));

    callback.release();

exit:
    va_end(args);
    return err.AsInteger();
}

chip::ChipError::StorageType pychip_ReadClient_ReadAttributes(void * appContext, DeviceProxy * device, bool isSubscription,
                                                              uint32_t minInterval, uint32_t maxInterval, size_t n, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::unique_ptr<ReadClientCallback> callback = std::make_unique<ReadClientCallback>(appContext);

    va_list args;
    va_start(args, n);

    std::unique_ptr<AttributePathParams[]> readPaths(new AttributePathParams[n]);

    {
        for (size_t i = 0; i < n; i++)
        {
            void * path = va_arg(args, void *);

            python::AttributePath pathObj;
            memcpy(&pathObj, path, sizeof(python::AttributePath));

            readPaths[i] = AttributePathParams(pathObj.endpointId, pathObj.clusterId, pathObj.attributeId);
        }
    }

    Optional<SessionHandle> session = device->GetSecureSession();
    ReadClient * readClient;

    VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NOT_CONNECTED);
    {
        app::InteractionModelEngine::GetInstance()->NewReadClient(
            &readClient, isSubscription ? ReadClient::InteractionType::Subscribe : ReadClient::InteractionType::Read,
            callback->GetBufferedReadCallback());
        ReadPrepareParams params(session.Value());
        params.mpAttributePathParamsList    = readPaths.get();
        params.mAttributePathParamsListSize = n;

        if (isSubscription)
        {
            params.mMinIntervalFloorSeconds   = minInterval;
            params.mMaxIntervalCeilingSeconds = maxInterval;

            err = readClient->SendSubscribeRequest(params);
        }
        else
        {
            err = readClient->SendReadRequest(params);
        }

        if (err != CHIP_NO_ERROR)
        {
            readClient->Shutdown();
        }
    }

    callback.release();

exit:
    va_end(args);
    return err.AsInteger();
}

chip::ChipError::StorageType pychip_ReadClient_ReadEvents(void * appContext, DeviceProxy * device, bool isSubscription,
                                                          uint32_t minInterval, uint32_t maxInterval, size_t n, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::unique_ptr<ReadClientCallback> callback = std::make_unique<ReadClientCallback>(appContext);

    va_list args;
    va_start(args, n);

    std::unique_ptr<EventPathParams[]> readPaths(new EventPathParams[n]);

    {
        for (size_t i = 0; i < n; i++)
        {
            void * path = va_arg(args, void *);

            python::EventPath pathObj;
            memcpy(&pathObj, path, sizeof(python::EventPath));

            readPaths[i] = EventPathParams(pathObj.endpointId, pathObj.clusterId, pathObj.eventId);
        }
    }

    Optional<SessionHandle> session = device->GetSecureSession();
    ReadClient * readClient;

    VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NOT_CONNECTED);
    {
        app::InteractionModelEngine::GetInstance()->NewReadClient(
            &readClient, isSubscription ? ReadClient::InteractionType::Subscribe : ReadClient::InteractionType::Read,
            callback.get());
        ReadPrepareParams params(session.Value());
        params.mpEventPathParamsList    = readPaths.get();
        params.mEventPathParamsListSize = n;

        if (isSubscription)
        {
            params.mMinIntervalFloorSeconds   = minInterval;
            params.mMaxIntervalCeilingSeconds = maxInterval;

            err = readClient->SendSubscribeRequest(params);
        }
        else
        {
            err = readClient->SendReadRequest(params);
        }

        if (err != CHIP_NO_ERROR)
        {
            readClient->Shutdown();
        }
    }

    callback.release();

exit:
    va_end(args);
    return err.AsInteger();
}
}

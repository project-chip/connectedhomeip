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

#include "system/SystemClock.h"
#include <cstdarg>
#include <memory>
#include <type_traits>

#include <app/BufferedReadCallback.h>
#include <app/ChunkedWriteCallback.h>
#include <app/DeviceProxy.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <lib/support/CodeUtils.h>

#include <cstdio>
#include <lib/support/logging/CHIPLogging.h>

#include <lib/core/Optional.h>

using namespace chip;
using namespace chip::app;

using PyObject = void;

namespace chip {
namespace python {

struct __attribute__((packed)) AttributePath
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::AttributeId attributeId;
    chip::DataVersion dataVersion;
    uint8_t hasDataVersion;
};

struct __attribute__((packed)) EventPath
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::EventId eventId;
    uint8_t urgentEvent;
};

struct __attribute__((packed)) DataVersionFilter
{
    chip::EndpointId endpointId;
    chip::ClusterId clusterId;
    chip::DataVersion dataVersion;
};

using OnReadAttributeDataCallback       = void (*)(PyObject * appContext, chip::DataVersion version, chip::EndpointId endpointId,
                                             chip::ClusterId clusterId, chip::AttributeId attributeId,
                                             std::underlying_type_t<Protocols::InteractionModel::Status> imstatus, uint8_t * data,
                                             uint32_t dataLen);
using OnReadEventDataCallback           = void (*)(PyObject * appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                         chip::EventId eventId, chip::EventNumber eventNumber, uint8_t priority, uint64_t timestamp,
                                         uint8_t timestampType, uint8_t * data, uint32_t dataLen,
                                         std::underlying_type_t<Protocols::InteractionModel::Status> imstatus);
using OnSubscriptionEstablishedCallback = void (*)(PyObject * appContext, SubscriptionId subscriptionId);
using OnResubscriptionAttemptedCallback = void (*)(PyObject * appContext, uint32_t aTerminationCause,
                                                   uint32_t aNextResubscribeIntervalMsec);
using OnReadErrorCallback               = void (*)(PyObject * appContext, uint32_t chiperror);
using OnReadDoneCallback                = void (*)(PyObject * appContext);
using OnReportBeginCallback             = void (*)(PyObject * appContext);
using OnReportEndCallback               = void (*)(PyObject * appContext);

OnReadAttributeDataCallback gOnReadAttributeDataCallback             = nullptr;
OnReadEventDataCallback gOnReadEventDataCallback                     = nullptr;
OnSubscriptionEstablishedCallback gOnSubscriptionEstablishedCallback = nullptr;
OnResubscriptionAttemptedCallback gOnResubscriptionAttemptedCallback = nullptr;
OnReadErrorCallback gOnReadErrorCallback                             = nullptr;
OnReadDoneCallback gOnReadDoneCallback                               = nullptr;
OnReportBeginCallback gOnReportBeginCallback                         = nullptr;
OnReportBeginCallback gOnReportEndCallback                           = nullptr;

void PythonResubscribePolicy(uint32_t aNumCumulativeRetries, uint32_t & aNextSubscriptionIntervalMsec, bool & aShouldResubscribe)
{
    aShouldResubscribe = true;
}

class ReadClientCallback : public ReadClient::Callback
{
public:
    ReadClientCallback(PyObject * appContext) : mBufferedReadCallback(*this), mAppContext(appContext) {}

    app::BufferedReadCallback * GetBufferedReadCallback() { return &mBufferedReadCallback; }

    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override
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
            CHIP_ERROR err = writer.CopyElement(TLV::AnonymousTag(), *apData);
            if (err != CHIP_NO_ERROR)
            {
                this->OnError(err);
                return;
            }
            size = writer.GetLengthWritten();
        }

        DataVersion version = 0;
        if (aPath.mDataVersion.HasValue())
        {
            version = aPath.mDataVersion.Value();
        }

        gOnReadAttributeDataCallback(mAppContext, version, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId,
                                     to_underlying(aStatus.mStatus), buffer.get(), size);
    }

    void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) override
    {
        gOnSubscriptionEstablishedCallback(mAppContext, aSubscriptionId);
    }

    CHIP_ERROR OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause) override
    {
        ReturnErrorOnFailure(ReadClient::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause));
        gOnResubscriptionAttemptedCallback(mAppContext, aTerminationCause.AsInteger(),
                                           apReadClient->ComputeTimeTillNextSubscription());
        return CHIP_NO_ERROR;
    }

    void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) override
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
            err = writer.CopyElement(TLV::AnonymousTag(), *apData);
            if (err != CHIP_NO_ERROR)
            {
                this->OnError(err);
                return;
            }
            size = writer.GetLengthWritten();
        }
        else if (apStatus != nullptr)
        {
            size = 0;
        }
        else
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            this->OnError(err);
        }

        gOnReadEventDataCallback(
            mAppContext, aEventHeader.mPath.mEndpointId, aEventHeader.mPath.mClusterId, aEventHeader.mPath.mEventId,
            aEventHeader.mEventNumber, to_underlying(aEventHeader.mPriorityLevel), aEventHeader.mTimestamp.mValue,
            to_underlying(aEventHeader.mTimestamp.mType), buffer, size,
            to_underlying(apStatus == nullptr ? Protocols::InteractionModel::Status::Success : apStatus->mStatus));
    }

    void OnError(CHIP_ERROR aError) override { gOnReadErrorCallback(mAppContext, aError.AsInteger()); }

    void OnReportBegin() override { gOnReportBeginCallback(mAppContext); }
    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        if (aReadPrepareParams.mpAttributePathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpAttributePathParamsList;
        }

        if (aReadPrepareParams.mpEventPathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpEventPathParamsList;
        }

        if (aReadPrepareParams.mpDataVersionFilterList != nullptr)
        {
            delete[] aReadPrepareParams.mpDataVersionFilterList;
        }
    }

    void OnReportEnd() override { gOnReportEndCallback(mAppContext); }

    void OnDone(ReadClient *) override
    {
        gOnReadDoneCallback(mAppContext);

        delete this;
    };

    void AdoptReadClient(std::unique_ptr<ReadClient> apReadClient) { mReadClient = std::move(apReadClient); }

private:
    BufferedReadCallback mBufferedReadCallback;

    PyObject * mAppContext;

    std::unique_ptr<ReadClient> mReadClient;
};

extern "C" {

struct __attribute__((packed)) PyReadAttributeParams
{
    uint32_t minInterval; // MinInterval in subscription request
    uint32_t maxInterval; // MaxInterval in subscription request
    bool isSubscription;
    bool isFabricFiltered;
    bool keepSubscriptions;
};

// Encodes n attribute write requests, follows 3 * n arguments, in the (AttributeWritePath*=void *, uint8_t*, size_t) order.
chip::ChipError::StorageType pychip_WriteClient_WriteAttributes(void * appContext, DeviceProxy * device,
                                                                uint16_t timedWriteTimeoutMs, uint16_t interactionTimeoutMs,
                                                                size_t n, ...);
chip::ChipError::StorageType pychip_ReadClient_ReadAttributes(void * appContext, ReadClient ** pReadClient,
                                                              ReadClientCallback ** pCallback, DeviceProxy * device,
                                                              uint8_t * readParamsBuf, size_t n, size_t total, ...);
}

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
    WriteClientCallback(PyObject * appContext) : mCallback(this), mAppContext(appContext) {}

    WriteClient::Callback * GetChunkedCallback() { return &mCallback; }

    void OnResponse(const WriteClient * apWriteClient, const ConcreteDataAttributePath & aPath, app::StatusIB aStatus) override
    {
        gOnWriteResponseCallback(mAppContext, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId,
                                 to_underlying(aStatus.mStatus));
    }

    void OnError(const WriteClient * apWriteClient, CHIP_ERROR aProtocolError) override
    {
        gOnWriteErrorCallback(mAppContext, aProtocolError.AsInteger());
    }

    void OnDone(WriteClient * apWriteClient) override
    {
        gOnWriteDoneCallback(mAppContext);
        delete apWriteClient;
        delete this;
    };

private:
    ChunkedWriteCallback mCallback;
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
                                     OnResubscriptionAttemptedCallback onResubscriptionAttemptedCallback,
                                     OnReadErrorCallback onReadErrorCallback, OnReadDoneCallback onReadDoneCallback,
                                     OnReportBeginCallback onReportBeginCallback, OnReportEndCallback onReportEndCallback)
{
    gOnReadAttributeDataCallback       = onReadAttributeDataCallback;
    gOnReadEventDataCallback           = onReadEventDataCallback;
    gOnSubscriptionEstablishedCallback = onSubscriptionEstablishedCallback;
    gOnResubscriptionAttemptedCallback = onResubscriptionAttemptedCallback;
    gOnReadErrorCallback               = onReadErrorCallback;
    gOnReadDoneCallback                = onReadDoneCallback;
    gOnReportBeginCallback             = onReportBeginCallback;
    gOnReportEndCallback               = onReportEndCallback;
}

chip::ChipError::StorageType pychip_WriteClient_WriteAttributes(void * appContext, DeviceProxy * device,
                                                                uint16_t timedWriteTimeoutMs, uint16_t interactionTimeoutMs,
                                                                size_t n, ...)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::unique_ptr<WriteClientCallback> callback = std::make_unique<WriteClientCallback>(appContext);
    std::unique_ptr<WriteClient> client           = std::make_unique<WriteClient>(
        app::InteractionModelEngine::GetInstance()->GetExchangeManager(), callback->GetChunkedCallback(),
        timedWriteTimeoutMs != 0 ? Optional<uint16_t>(timedWriteTimeoutMs) : Optional<uint16_t>::Missing());

    va_list args;
    va_start(args, n);

    VerifyOrExit(device != nullptr && device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    {
        for (size_t i = 0; i < n; i++)
        {
            void * path = va_arg(args, void *);
            void * tlv  = va_arg(args, void *);
            int length  = va_arg(args, int);

            python::AttributePath pathObj;
            memcpy(&pathObj, path, sizeof(python::AttributePath));
            uint8_t * tlvBuffer = reinterpret_cast<uint8_t *>(tlv);

            TLV::TLVReader reader;
            reader.Init(tlvBuffer, static_cast<uint32_t>(length));
            reader.Next();
            Optional<DataVersion> dataVersion;
            if (pathObj.hasDataVersion == 1)
            {
                dataVersion.SetValue(pathObj.dataVersion);
            }
            SuccessOrExit(
                err = client->PutPreencodedAttribute(
                    chip::app::ConcreteDataAttributePath(pathObj.endpointId, pathObj.clusterId, pathObj.attributeId, dataVersion),
                    reader));
        }
    }

    SuccessOrExit(err = client->SendWriteRequest(device->GetSecureSession().Value(),
                                                 interactionTimeoutMs != 0 ? System::Clock::Milliseconds32(interactionTimeoutMs)
                                                                           : System::Clock::kZero));

    client.release();
    callback.release();

exit:
    va_end(args);
    return err.AsInteger();
}

void pychip_ReadClient_Abort(ReadClient * apReadClient, ReadClientCallback * apCallback)
{
    VerifyOrDie(apReadClient != nullptr);
    VerifyOrDie(apCallback != nullptr);

    delete apCallback;
}

void pychip_ReadClient_OverrideLivenessTimeout(ReadClient * pReadClient, uint32_t livenessTimeoutMs)
{
    VerifyOrDie(pReadClient != nullptr);
    pReadClient->OverrideLivenessTimeout(System::Clock::Milliseconds32(livenessTimeoutMs));
}

chip::ChipError::StorageType pychip_ReadClient_Read(void * appContext, ReadClient ** pReadClient, ReadClientCallback ** pCallback,
                                                    DeviceProxy * device, uint8_t * readParamsBuf, size_t numAttributePaths,
                                                    size_t numDataversionFilters, size_t numEventPaths, ...)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    PyReadAttributeParams pyParams = {};
    // The readParamsBuf might be not aligned, using a memcpy to avoid some unexpected behaviors.
    memcpy(&pyParams, readParamsBuf, sizeof(pyParams));

    std::unique_ptr<ReadClientCallback> callback = std::make_unique<ReadClientCallback>(appContext);

    va_list args;
    va_start(args, numEventPaths);

    std::unique_ptr<AttributePathParams[]> attributePaths(new AttributePathParams[numAttributePaths]);
    std::unique_ptr<chip::app::DataVersionFilter[]> dataVersionFilters(new chip::app::DataVersionFilter[numDataversionFilters]);
    std::unique_ptr<EventPathParams[]> eventPaths(new EventPathParams[numEventPaths]);
    std::unique_ptr<ReadClient> readClient;

    for (size_t i = 0; i < numAttributePaths; i++)
    {
        void * path = va_arg(args, void *);

        python::AttributePath pathObj;
        memcpy(&pathObj, path, sizeof(python::AttributePath));

        attributePaths[i] = AttributePathParams(pathObj.endpointId, pathObj.clusterId, pathObj.attributeId);
    }

    for (size_t i = 0; i < numDataversionFilters; i++)
    {
        void * filter = va_arg(args, void *);

        python::DataVersionFilter filterObj;
        memcpy(&filterObj, filter, sizeof(python::DataVersionFilter));

        dataVersionFilters[i] = chip::app::DataVersionFilter(filterObj.endpointId, filterObj.clusterId, filterObj.dataVersion);
    }

    for (size_t i = 0; i < numEventPaths; i++)
    {
        void * path = va_arg(args, void *);

        python::EventPath pathObj;
        memcpy(&pathObj, path, sizeof(python::EventPath));

        eventPaths[i] = EventPathParams(pathObj.endpointId, pathObj.clusterId, pathObj.eventId, pathObj.urgentEvent == 1);
    }

    Optional<SessionHandle> session = device->GetSecureSession();
    VerifyOrExit(session.HasValue(), err = CHIP_ERROR_NOT_CONNECTED);

    readClient = std::make_unique<ReadClient>(
        InteractionModelEngine::GetInstance(), device->GetExchangeManager(), *callback->GetBufferedReadCallback(),
        pyParams.isSubscription ? ReadClient::InteractionType::Subscribe : ReadClient::InteractionType::Read);
    VerifyOrExit(readClient != nullptr, err = CHIP_ERROR_NO_MEMORY);
    {
        ReadPrepareParams params(session.Value());
        if (numAttributePaths != 0)
        {
            params.mpAttributePathParamsList    = attributePaths.get();
            params.mAttributePathParamsListSize = numAttributePaths;
        }
        if (numDataversionFilters != 0)
        {
            params.mpDataVersionFilterList    = dataVersionFilters.get();
            params.mDataVersionFilterListSize = numDataversionFilters;
        }
        if (numEventPaths != 0)
        {
            params.mpEventPathParamsList    = eventPaths.get();
            params.mEventPathParamsListSize = numEventPaths;
        }

        params.mIsFabricFiltered = pyParams.isFabricFiltered;

        if (pyParams.isSubscription)
        {
            params.mMinIntervalFloorSeconds   = pyParams.minInterval;
            params.mMaxIntervalCeilingSeconds = pyParams.maxInterval;
            params.mKeepSubscriptions         = pyParams.keepSubscriptions;

            dataVersionFilters.release();
            attributePaths.release();
            eventPaths.release();

            err = readClient->SendAutoResubscribeRequest(std::move(params));
            SuccessOrExit(err);
        }
        else
        {
            err = readClient->SendRequest(params);
            SuccessOrExit(err);
        }
    }

    *pReadClient = readClient.get();
    *pCallback   = callback.get();

    callback->AdoptReadClient(std::move(readClient));

    callback.release();

exit:
    va_end(args);
    return err.AsInteger();
}
}

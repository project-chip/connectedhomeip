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
#include <controller/CHIPDeviceController.h>
#include <controller/python/chip/interaction_model/Delegate.h>
#include <controller/python/chip/native/PyChipError.h>
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
using OnResubscriptionAttemptedCallback = void (*)(PyObject * appContext, PyChipError aTerminationCause,
                                                   uint32_t aNextResubscribeIntervalMsec);
using OnReadErrorCallback               = void (*)(PyObject * appContext, PyChipError chiperror);
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
        if (mAutoResubscribe)
        {
            ReturnErrorOnFailure(ReadClient::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause));
        }
        gOnResubscriptionAttemptedCallback(mAppContext, ToPyChipError(aTerminationCause),
                                           apReadClient->ComputeTimeTillNextSubscription());
        if (mAutoResubscribe)
        {
            return CHIP_NO_ERROR;
        }
        return aTerminationCause;
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

    void OnError(CHIP_ERROR aError) override { gOnReadErrorCallback(mAppContext, ToPyChipError(aError)); }

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

    void SetAutoResubscribe(bool autoResubscribe) { mAutoResubscribe = autoResubscribe; }

private:
    BufferedReadCallback mBufferedReadCallback;

    PyObject * mAppContext;

    std::unique_ptr<ReadClient> mReadClient;
    bool mAutoResubscribe = true;
};

extern "C" {

struct __attribute__((packed)) PyReadAttributeParams
{
    uint16_t minInterval; // MinInterval in subscription request
    uint16_t maxInterval; // MaxInterval in subscription request
    bool isSubscription;
    bool isFabricFiltered;
    bool keepSubscriptions;
    bool autoResubscribe;
};

PyChipError pychip_WriteClient_WriteAttributes(void * appContext, DeviceProxy * device, size_t timedWriteTimeoutMsSizeT,
                                               size_t interactionTimeoutMsSizeT, size_t busyWaitMsSizeT,
                                               chip::python::PyWriteAttributeData * writeAttributesData,
                                               size_t attributeDataLength);
PyChipError pychip_WriteClient_WriteGroupAttributes(size_t groupIdSizeT, chip::Controller::DeviceCommissioner * devCtrl,
                                                    size_t busyWaitMsSizeT,
                                                    chip::python::PyWriteAttributeData * writeAttributesData,
                                                    size_t attributeDataLength);
}

using OnWriteResponseCallback = void (*)(PyObject * appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                         chip::AttributeId attributeId,
                                         std::underlying_type_t<Protocols::InteractionModel::Status> imstatus);
using OnWriteErrorCallback    = void (*)(PyObject * appContext, PyChipError chiperror);
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
        gOnWriteErrorCallback(mAppContext, ToPyChipError(aProtocolError));
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

PyChipError pychip_WriteClient_WriteAttributes(void * appContext, DeviceProxy * device, size_t timedWriteTimeoutMsSizeT,
                                               size_t interactionTimeoutMsSizeT, size_t busyWaitMsSizeT,
                                               python::PyWriteAttributeData * writeAttributesData, size_t attributeDataLength)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // The FFI from Python to C when calling a variadic function has issues when the regular, non-variadic, function
    // arguments are unit16_t. As a result we pass these arguments as size_t and cast them to the expected uint16_t.
    uint16_t timedWriteTimeoutMs  = static_cast<uint16_t>(timedWriteTimeoutMsSizeT);
    uint16_t interactionTimeoutMs = static_cast<uint16_t>(interactionTimeoutMsSizeT);
    uint16_t busyWaitMs           = static_cast<uint16_t>(busyWaitMsSizeT);

    std::unique_ptr<WriteClientCallback> callback = std::make_unique<WriteClientCallback>(appContext);
    std::unique_ptr<WriteClient> client           = std::make_unique<WriteClient>(
        app::InteractionModelEngine::GetInstance()->GetExchangeManager(), callback->GetChunkedCallback(),
        timedWriteTimeoutMs != 0 ? Optional<uint16_t>(timedWriteTimeoutMs) : Optional<uint16_t>::Missing());

    VerifyOrExit(device != nullptr && device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    for (size_t i = 0; i < attributeDataLength; i++)
    {
        python::PyAttributePath path = writeAttributesData[i].attributePath;
        void * tlv                   = writeAttributesData[i].tlvData;
        size_t length                = writeAttributesData[i].tlvLength;

        uint8_t * tlvBuffer = reinterpret_cast<uint8_t *>(tlv);

        TLV::TLVReader reader;
        reader.Init(tlvBuffer, static_cast<uint32_t>(length));
        reader.Next();
        Optional<DataVersion> dataVersion;
        if (path.hasDataVersion == 1)
        {
            dataVersion.SetValue(path.dataVersion);
        }
        SuccessOrExit(
            err = client->PutPreencodedAttribute(
                chip::app::ConcreteDataAttributePath(path.endpointId, path.clusterId, path.attributeId, dataVersion), reader));
    }

    SuccessOrExit(err = client->SendWriteRequest(device->GetSecureSession().Value(),
                                                 interactionTimeoutMs != 0 ? System::Clock::Milliseconds32(interactionTimeoutMs)
                                                                           : System::Clock::kZero));

    client.release();
    callback.release();

    if (busyWaitMs)
    {
        usleep(busyWaitMs * 1000);
    }

exit:
    return ToPyChipError(err);
}

PyChipError pychip_WriteClient_WriteGroupAttributes(size_t groupIdSizeT, chip::Controller::DeviceCommissioner * devCtrl,
                                                    size_t busyWaitMsSizeT, python::PyWriteAttributeData * writeAttributesData,
                                                    size_t attributeDataLength)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // The FFI from Python to C when calling a variadic function has issues when the regular, non-variadic, function
    // arguments are unit16_t (which is the type for chip::GroupId). As a result we pass these arguments as size_t
    // and cast them to the expected type here.
    chip::GroupId groupId = static_cast<chip::GroupId>(groupIdSizeT);
    uint16_t busyWaitMs   = static_cast<uint16_t>(busyWaitMsSizeT);

    chip::Messaging::ExchangeManager * exchangeManager = chip::app::InteractionModelEngine::GetInstance()->GetExchangeManager();
    VerifyOrReturnError(exchangeManager != nullptr, ToPyChipError(CHIP_ERROR_INCORRECT_STATE));

    std::unique_ptr<WriteClient> client = std::make_unique<WriteClient>(
        app::InteractionModelEngine::GetInstance()->GetExchangeManager(), nullptr /* callback */, Optional<uint16_t>::Missing());

    for (size_t i = 0; i < attributeDataLength; i++)
    {
        python::PyAttributePath path = writeAttributesData[i].attributePath;
        void * tlv                   = writeAttributesData[i].tlvData;
        size_t length                = writeAttributesData[i].tlvLength;

        uint8_t * tlvBuffer = reinterpret_cast<uint8_t *>(tlv);

        TLV::TLVReader reader;
        reader.Init(tlvBuffer, static_cast<uint32_t>(length));
        reader.Next();
        Optional<DataVersion> dataVersion;
        if (path.hasDataVersion == 1)
        {
            dataVersion.SetValue(path.dataVersion);
        }
        // Using kInvalidEndpointId as that used when sending group write requests.
        SuccessOrExit(
            err = client->PutPreencodedAttribute(
                chip::app::ConcreteDataAttributePath(kInvalidEndpointId, path.clusterId, path.attributeId, dataVersion), reader));
    }

    {
        auto fabricIndex = devCtrl->GetFabricIndex();

        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex);
        SuccessOrExit(err = client->SendWriteRequest(chip::SessionHandle(session), System::Clock::kZero));
    }

    if (busyWaitMs)
    {
        usleep(busyWaitMs * 1000);
    }

exit:
    return ToPyChipError(err);
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

PyChipError pychip_ReadClient_GetReportingIntervals(ReadClient * pReadClient, uint16_t * minIntervalSec, uint16_t * maxIntervalSec)
{
    VerifyOrDie(pReadClient != nullptr);
    CHIP_ERROR err = pReadClient->GetReportingIntervals(*minIntervalSec, *maxIntervalSec);

    return ToPyChipError(err);
}

void pychip_ReadClient_GetSubscriptionTimeoutMs(ReadClient * pReadClient, uint32_t * milliSec)
{
    VerifyOrDie(pReadClient != nullptr);

    Optional<System::Clock::Timeout> duration = pReadClient->GetSubscriptionTimeout();

    // The return value of GetSubscriptionTimeout cannot be 0
    // so milliSec=0 can be considered as the subscription has been abnormal.
    *milliSec = 0;
    if (duration.HasValue())
    {
        System::Clock::Milliseconds32 msec = std::chrono::duration_cast<System::Clock::Milliseconds32>(duration.Value());
        *milliSec                          = msec.count();
    }
}

PyChipError pychip_ReadClient_Read(void * appContext, ReadClient ** pReadClient, ReadClientCallback ** pCallback,
                                   DeviceProxy * device, uint8_t * readParamsBuf, void ** attributePathsFromPython,
                                   size_t numAttributePaths, void ** dataversionFiltersFromPython, size_t numDataversionFilters,
                                   void ** eventPathsFromPython, size_t numEventPaths, uint64_t * eventNumberFilter)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    PyReadAttributeParams pyParams = {};
    // The readParamsBuf might be not aligned, using a memcpy to avoid some unexpected behaviors.
    memcpy(&pyParams, readParamsBuf, sizeof(pyParams));

    std::unique_ptr<ReadClientCallback> callback = std::make_unique<ReadClientCallback>(appContext);

    std::unique_ptr<AttributePathParams[]> attributePaths(new AttributePathParams[numAttributePaths]);
    std::unique_ptr<chip::app::DataVersionFilter[]> dataVersionFilters(new chip::app::DataVersionFilter[numDataversionFilters]);
    std::unique_ptr<EventPathParams[]> eventPaths(new EventPathParams[numEventPaths]);
    std::unique_ptr<ReadClient> readClient;

    for (size_t i = 0; i < numAttributePaths; i++)
    {
        void * path = attributePathsFromPython[i];

        python::AttributePath pathObj;
        memcpy(&pathObj, path, sizeof(python::AttributePath));

        attributePaths[i] = AttributePathParams(pathObj.endpointId, pathObj.clusterId, pathObj.attributeId);
    }

    for (size_t i = 0; i < numDataversionFilters; i++)
    {
        void * filter = dataversionFiltersFromPython[i];

        python::DataVersionFilter filterObj;
        memcpy(&filterObj, filter, sizeof(python::DataVersionFilter));

        dataVersionFilters[i] = chip::app::DataVersionFilter(filterObj.endpointId, filterObj.clusterId, filterObj.dataVersion);
    }

    for (size_t i = 0; i < numEventPaths; i++)
    {
        void * path = eventPathsFromPython[i];

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
        if (eventNumberFilter != nullptr)
        {
            static_assert(sizeof(chip::EventNumber) == sizeof(*eventNumberFilter) &&
                              std::is_unsigned<chip::EventNumber>::value ==
                                  std::is_unsigned<std::remove_pointer<decltype(eventNumberFilter)>::type>::value,
                          "EventNumber type mismatch");
            params.mEventNumber = MakeOptional(EventNumber(*eventNumberFilter));
        }

        params.mIsFabricFiltered = pyParams.isFabricFiltered;

        if (pyParams.isSubscription)
        {
            params.mMinIntervalFloorSeconds   = pyParams.minInterval;
            params.mMaxIntervalCeilingSeconds = pyParams.maxInterval;
            params.mKeepSubscriptions         = pyParams.keepSubscriptions;
            callback->SetAutoResubscribe(pyParams.autoResubscribe);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
            if (!pyParams.autoResubscribe)
            {
                // We want to allow certain kinds of spec-invalid subscriptions so we
                // can test how the server reacts to them.
                err = readClient->SendSubscribeRequestWithoutValidation(params);
            }
            else
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
            {
                dataVersionFilters.release();
                attributePaths.release();
                eventPaths.release();

                err = readClient->SendAutoResubscribeRequest(std::move(params));
            }
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
    return ToPyChipError(err);
}
}

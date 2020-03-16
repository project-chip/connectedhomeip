/*
 *
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *          Contains non-inline method definitions for the
 *          GenericSoftwareUpdateManagerImpl<> template.
 */

#ifndef GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_IPP
#define GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_IPP

#if WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <Weave/Core/WeaveCore.h>
#include <Weave/DeviceLayer/PlatformManager.h>
#include <Weave/DeviceLayer/ConnectivityManager.h>
#include <Weave/DeviceLayer/SoftwareUpdateManager.h>
#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/internal/GenericSoftwareUpdateManagerImpl.h>

#include <Weave/Support/FibonacciUtils.h>
#include <Weave/Support/RandUtils.h>
#include <Weave/Support/TraitEventUtils.h>

#include <nest/trait/firmware/SoftwareUpdateTrait.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

using namespace ::nl::Weave;
using namespace ::nl::Weave::TLV;
using namespace ::nl::Weave::Profiles;
using namespace ::nl::Weave::Profiles::Common;
using namespace ::nl::Weave::Profiles::DataManagement;
using namespace ::nl::Weave::Profiles::SoftwareUpdate;
using namespace ::Schema::Nest::Trait::Firmware;
using namespace ::Schema::Nest::Trait::Firmware::SoftwareUpdateTrait;

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>;

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::DoInit()
{
    mShouldRetry = false;
    mScheduledCheckEnabled = false;
    mIgnorePartialImage = false;

    mEventHandlerCallback = NULL;
    mRetryPolicyCallback = DefaultRetryPolicyCallback;

    mRetryCounter = 0;
    mMinWaitTimeMs = 0;
    mMaxWaitTimeMs = 0;

    mState = SoftwareUpdateManager::kState_Idle;
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::_SetEventCallback(void * const aAppState,
                                                                   const SoftwareUpdateManager::EventCallback aEventCallback)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    mAppState = aAppState;
    mEventHandlerCallback = aEventCallback;

#if DEBUG
    /* Verify that the application's event callback function correctly calls the default handler.
     *
     * NOTE: If your code receives WEAVE_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED it means that the event handler
     * function you supplied for the software update manager does not properly call SoftwareUpdateManager::DefaultEventHandler
     * for unrecognized/unhandled events.
     */
    {
        SoftwareUpdateManager::InEventParam inParam;
        SoftwareUpdateManager::OutEventParam outParam;
        inParam.Clear();
        inParam.Source = &SoftwareUpdateMgrImpl();
        outParam.Clear();
        mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_DefaultCheck, inParam, outParam);
        VerifyOrExit(outParam.DefaultHandlerCalled, err = WEAVE_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED);
    }

exit:
#endif

    return err;

}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::_SetRetryPolicyCallback(const SoftwareUpdateManager::RetryPolicyCallback aRetryPolicyCallback)
{
    mRetryPolicyCallback = (aRetryPolicyCallback != NULL) ? aRetryPolicyCallback : DefaultRetryPolicyCallback;
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::PrepareBinding(intptr_t arg)
{
    WEAVE_ERROR err;
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();

    self->Cleanup();

    if (!ConnectivityMgr().HaveServiceConnectivity())
    {
        WeaveLogProgress(DeviceLayer, "Software Update Check: No service connectivity");
        ExitNow(err = WEAVE_ERROR_NOT_CONNECTED);
    }

    self->mBinding = ExchangeMgr.NewBinding(HandleServiceBindingEvent, NULL);
    VerifyOrExit(self->mBinding != NULL, err = WEAVE_ERROR_NO_MEMORY);

    err = self->mBinding->BeginConfiguration()
            .Target_ServiceEndpoint(WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_ENDPOINT_ID)
            .Transport_UDP_WRM()
            .Exchange_ResponseTimeoutMsec(WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_RESPOSNE_TIMEOUT)
            .Security_SharedCASESession()
            .PrepareBinding();

exit:
    if (err != WEAVE_NO_ERROR)
    {
        self->Impl()->SoftwareUpdateFailed(err, NULL);
    }
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::PrepareQuery(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    ImageQuery imageQuery;
    TLVWriter writer;
    TLVType containerType;

    QueryBeginEvent ev;
    EventOptions evOptions(true);

    char firmwareRev[ConfigurationManager::kMaxFirmwareRevisionLength+1];

    size_t firmwareRevLen;

    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    err = ConfigurationMgr().GetProductId(imageQuery.productSpec.productId);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetVendorId(imageQuery.productSpec.vendorId);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetProductRevision(imageQuery.productSpec.productRev);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetFirmwareRevision(firmwareRev, sizeof(firmwareRev), firmwareRevLen);
    SuccessOrExit(err);

    nl::NullifyAllEventFields(&ev);
    evOptions.relatedEventID = mEventId;
    ev.currentSwVersion = firmwareRev;
    ev.vendorId = imageQuery.productSpec.vendorId;
    ev.vendorProductId = imageQuery.productSpec.productId;
    ev.productRevision = imageQuery.productSpec.productRev;
    ev.SetCurrentSwVersionPresent();
    ev.SetVendorIdPresent();
    ev.SetVendorProductIdPresent();
    ev.SetProductRevisionPresent();

    err = Impl()->GetUpdateSchemeList(&imageQuery.updateSchemes);
    SuccessOrExit(err);

    err = Impl()->GetIntegrityTypeList(&imageQuery.integrityTypes);
    SuccessOrExit(err);

    outParam.PrepareQuery.PackageSpecification = NULL;
    outParam.PrepareQuery.DesiredLocale = NULL;
    outParam.PrepareQuery.Error = WEAVE_NO_ERROR;

    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_PrepareQuery, inParam, outParam);
    VerifyOrExit(mState == SoftwareUpdateManager::kState_PrepareQuery, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

    // Check for a preparation error returned by the application
    err = outParam.PrepareQuery.Error;
    SuccessOrExit(err);

    err = imageQuery.version.init((uint8_t) firmwareRevLen, firmwareRev);
    SuccessOrExit(err);

    // Locale is an optional field in the weave software update protocol. If one is not provided by the application,
    // then skip over and move to the next field.
    if (outParam.PrepareQuery.DesiredLocale != NULL)
    {
        err = imageQuery.localeSpec.init((uint8_t) strlen(outParam.PrepareQuery.DesiredLocale), (char *)outParam.PrepareQuery.DesiredLocale);
        SuccessOrExit(err);

        ev.locale = outParam.PrepareQuery.DesiredLocale;
        ev.SetLocalePresent();
    }

    // Package specification is an option field in the weave software update protocol. If one is not
    // provided by the application, skip and move to the next field.
    if (outParam.PrepareQuery.PackageSpecification != NULL)
    {
        err = imageQuery.packageSpec.init((uint8_t) strlen(outParam.PrepareQuery.PackageSpecification), (char *)outParam.PrepareQuery.PackageSpecification);
        SuccessOrExit(err);
    }

    // Allocate a buffer to hold the image query.
    mImageQueryPacketBuffer = PacketBuffer::New();
    VerifyOrExit(mImageQueryPacketBuffer != NULL, err = WEAVE_ERROR_NO_MEMORY);

    err = imageQuery.pack(mImageQueryPacketBuffer);
    SuccessOrExit(err);

    writer.Init(mImageQueryPacketBuffer);

    err = writer.StartContainer(AnonymousTag, kTLVType_Structure, containerType);
    SuccessOrExit(err);

    inParam.Clear();
    outParam.Clear();

    inParam.PrepareQuery_Metadata.MetaDataWriter = &writer;
    outParam.PrepareQuery_Metadata.Error = WEAVE_NO_ERROR;

    // Call EventHandler Callback to allow application to write meta-data.
    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_PrepareQuery_Metadata, inParam, outParam);
    VerifyOrExit(mState == SoftwareUpdateManager::kState_PrepareQuery, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

    // Check for a preparation error returned by the application
    err = outParam.PrepareQuery_Metadata.Error;
    SuccessOrExit(err);

    err = writer.EndContainer(containerType);
    SuccessOrExit(err);

    writer.Finalize();

    nl::LogEvent(&ev, evOptions);

exit:
    return err;
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::_CheckNow(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(mEventHandlerCallback != NULL, err = WEAVE_ERROR_INCORRECT_STATE);

    if (!Impl()->IsInProgress())
    {
        if (mState == SoftwareUpdateManager::kState_ScheduledHoldoff)
        {
            // Cancel scheduled hold off and trigger software update prepare.
            SystemLayer.CancelTimer(HandleHoldOffTimerExpired, NULL);
        }

        {
            SoftwareUpdateStartEvent ev;
            EventOptions evOptions(true);
            ev.trigger = START_TRIGGER_USER_INITIATED;
            mEventId = nl::LogEvent(&ev, evOptions);
        }

        DriveState(SoftwareUpdateManager::kState_PrepareQuery);
    }

exit:
    return err;
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::GetEventState(int32_t& aEventState)
{
    int32_t event_state = 0;

    switch(mState)
    {
        case SoftwareUpdateManager::kState_Idle:
        case SoftwareUpdateManager::kState_ScheduledHoldoff:
            event_state = STATE_IDLE;
            break;
        case SoftwareUpdateManager::kState_PrepareQuery:
        case SoftwareUpdateManager::kState_Query:
            event_state = STATE_QUERYING;
            break;
        case SoftwareUpdateManager::kState_Download:
            event_state = STATE_DOWNLOADING;
            break;
        case SoftwareUpdateManager::kState_Install:
            event_state = STATE_INSTALLING;
            break;
        default:
            event_state = 0;
            break;
    }

    aEventState = event_state;
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::SoftwareUpdateFailed(WEAVE_ERROR aError,
                                                                       Profiles::StatusReporting::StatusReport * aStatusReport)
{
    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    if (aError == WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED)
    {
        /*
         *  No need to do anything since an abort by the application would have already
         *  called SoftwareUpdateFinished with WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED error
         *  and moved to Idle state.
         */
        ExitNow();
    }

    mShouldRetry = true;
    mRetryCounter++;

    {
        FailureEvent ev;
        EventOptions evOptions(true);
        nl::NullifyAllEventFields(&ev);
        GetEventState(ev.state);
        evOptions.relatedEventID = mEventId;

        ev.platformReturnCode = aError;
        ev.SetPrimaryStatusCodeNull();

        if (aStatusReport)
        {
            ev. SetRemoteStatusCodePresent();
            ev.remoteStatusCode.profileId = aStatusReport->mProfileId;
            ev.remoteStatusCode.statusCode = aStatusReport->mStatusCode;
        }
        else
        {
            ev.SetRemoteStatusCodeNull();
        }

        nl::LogEvent(&ev, evOptions);
    }

    if (mState == SoftwareUpdateManager::kState_PrepareQuery)
    {
        inParam.QueryPrepareFailed.Error = aError;
        inParam.QueryPrepareFailed.StatusReport = aStatusReport;
        mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_QueryPrepareFailed, inParam, outParam);
    }
    else
    {
        inParam.Finished.Error = aError;
        inParam.Finished.StatusReport = aStatusReport;
        mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_Finished, inParam, outParam);
    }

    DriveState(SoftwareUpdateManager::kState_Idle);

exit:
    return ;
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::SoftwareUpdateFinished(WEAVE_ERROR aError)
{
    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    mShouldRetry = false;
    mRetryCounter = 0;

    if (aError == WEAVE_ERROR_NO_SW_UPDATE_AVAILABLE)
    {
        /* Log a Query Finish event with null fields
         * as per the software update trait schema to indicate no update available.
         */
        QueryFinishEvent ev;
        EventOptions evOptions(true);
        nl::NullifyAllEventFields(&ev);
        evOptions.relatedEventID = mEventId;
        nl::LogEvent(&ev, evOptions);
    }
    else if (aError != WEAVE_NO_ERROR)
    {
        /* Log a Failure event to indicate that software update finished
         * because of an error.
         */
        FailureEvent ev;
        EventOptions evOptions(true);
        nl::NullifyAllEventFields(&ev);
        GetEventState(ev.state);
        evOptions.relatedEventID = mEventId;
        ev.platformReturnCode = aError;
        nl::LogEvent(&ev, evOptions);
    }

    inParam.Finished.Error = aError;
    inParam.Finished.StatusReport = NULL;

    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_Finished, inParam, outParam);

    DriveState(SoftwareUpdateManager::kState_Idle);
}

template<class ImplClass>
bool GenericSoftwareUpdateManagerImpl<ImplClass>::_IsInProgress(void)
{
    return ((mState == SoftwareUpdateManager::kState_Idle || mState == SoftwareUpdateManager::kState_ScheduledHoldoff) ?
            false : true );
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::SendQuery(void)
{
    WEAVE_ERROR err;

    SoftwareUpdateManager::InEventParam  inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    // Configure the context
    mExchangeCtx->AppState          = this;
    mExchangeCtx->OnMessageReceived = HandleResponse;
    mExchangeCtx->OnResponseTimeout = OnResponseTimeout;
    mExchangeCtx->OnKeyError        = OnKeyError;

    // Send the query
    err =  mExchangeCtx->SendMessage(kWeaveProfile_SWU,
                                     kMsgType_ImageQuery,
                                     mImageQueryPacketBuffer,
                                     ExchangeContext::kSendFlag_ExpectResponse | ExchangeContext::kSendFlag_RequestAck);
    SuccessOrExit(err);

    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_QuerySent, inParam, outParam);
    VerifyOrExit(mState == SoftwareUpdateManager::kState_Query, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

exit:
    if (err != WEAVE_NO_ERROR)
    {
        Impl()->SoftwareUpdateFailed(err, NULL);
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::OnResponseTimeout(ExchangeContext * aEC)
{
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();
    self->Impl()->SoftwareUpdateFailed(WEAVE_ERROR_TIMEOUT, NULL);
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::OnKeyError(ExchangeContext *aEc, WEAVE_ERROR aKeyError)
{
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();
    self->Impl()->SoftwareUpdateFailed(aKeyError, NULL);
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::HandleResponse(ExchangeContext * ec, const IPPacketInfo * pktInfo, const WeaveMessageInfo * msgInfo,
                                                                 uint32_t profileId, uint8_t msgType, PacketBuffer * payload)
{
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();

    VerifyOrDie(ec == self->mExchangeCtx);

    // Close the exchange.  When using WRM, this will force an standalone-ACK to be
    // sent immediately.
    self->mExchangeCtx->Close();
    self->mExchangeCtx = NULL;

    // We expect to receive one of two possible responses:
    // 1. An ImageQueryResponse message under the weave software update profile indicating
    //    an update might be available or
    // 2. A StatusReport indicating no software update available or a problem with the query.
    //
    // NOTE: the Handle* methods are responsible for releasing the buffer.
    //
    if (profileId == kWeaveProfile_SWU && msgType == kMsgType_ImageQueryResponse)
    {
        self->HandleImageQueryResponse(payload);
    }
    else if (profileId == kWeaveProfile_Common && msgType == kMsgType_StatusReport)
    {
        self->HandleStatusReport(payload);
    }
    else
    {
        // If any other message type is received, fail with an error.
        PacketBuffer::Free(payload);
        self->Impl()->SoftwareUpdateFailed(WEAVE_ERROR_INVALID_MESSAGE_TYPE, NULL);
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::HandleStatusReport(PacketBuffer * payload)
{
    WEAVE_ERROR err;
    StatusReport statusReport;

    // Parse the status report to uncover the underlying errors.
    err = StatusReport::parse(payload, statusReport);

    if (err != WEAVE_NO_ERROR)
    {
        PacketBuffer::Free(payload);
        Impl()->SoftwareUpdateFailed(err, NULL);
    }
    else if ((statusReport.mProfileId == kWeaveProfile_SWU) && (statusReport.mStatusCode == kStatus_NoUpdateAvailable))
    {
        PacketBuffer::Free(payload);
        Impl()->SoftwareUpdateFinished(WEAVE_ERROR_NO_SW_UPDATE_AVAILABLE);
    }
    else
    {
        Impl()->SoftwareUpdateFailed(err, &statusReport);
        PacketBuffer::Free(payload); // Release the buffer *after* the call since the statusReport
                                     // object may contain a pointer to data in the buffer.
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::HandleImageQueryResponse(PacketBuffer * payload)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;
    char versionString[ConfigurationManager::kMaxFirmwareRevisionLength+1];

    {
        ImageQueryResponse imageQueryResponse;

        // Parse out the query response
        err = ImageQueryResponse::parse(payload, imageQueryResponse);
        SuccessOrExit(err);

        // Copy URI and version string since the original payload will be freed after this.
        VerifyOrExit(imageQueryResponse.uri.theLength < WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN, err = WEAVE_ERROR_BUFFER_TOO_SMALL);
        strncpy(mURI, imageQueryResponse.uri.theString, imageQueryResponse.uri.theLength);
        mURI[imageQueryResponse.uri.theLength] = 0;
        VerifyOrExit(imageQueryResponse.versionSpec.theLength < ArraySize(versionString), err = WEAVE_ERROR_BUFFER_TOO_SMALL);
        strncpy(versionString, imageQueryResponse.versionSpec.theString, imageQueryResponse.versionSpec.theLength);
        versionString[imageQueryResponse.versionSpec.theLength] = 0;

        // Save the integrity spec
        mIntegritySpec = imageQueryResponse.integritySpec;

        // Arrange to pass query response information to the application.
        inParam.SoftwareUpdateAvailable.Priority        = imageQueryResponse.updatePriority;
        inParam.SoftwareUpdateAvailable.Condition       = imageQueryResponse.updateCondition;
        inParam.SoftwareUpdateAvailable.IntegrityType   = imageQueryResponse.integritySpec.type;
        inParam.SoftwareUpdateAvailable.Version         = versionString;
        inParam.SoftwareUpdateAvailable.URI             = mURI;
    }

    // Release the packet buffer.
    PacketBuffer::Free(payload);
    payload = NULL;

    // Log a QueryFinish event.
    {
        QueryFinishEvent ev;
        EventOptions evOptions(true);
        nl::NullifyAllEventFields(&ev);
        evOptions.relatedEventID = mEventId;
        ev.imageUrl = mURI;
        ev.imageVersion = versionString;
        ev.SetImageUrlPresent();
        ev.SetImageVersionPresent();
        nl::LogEvent(&ev, evOptions);
    }

    // Set DownloadNow as the default option. Application can override during event callback
    outParam.SoftwareUpdateAvailable.Action = SoftwareUpdateManager::kAction_DownloadNow;

    // Tell the application that a software update is available.
    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_SoftwareUpdateAvailable, inParam, outParam);

    // Check to see which action was selected by the application.
    switch (outParam.SoftwareUpdateAvailable.Action)
    {
    case SoftwareUpdateManager::kAction_Ignore:
        Impl()->SoftwareUpdateFinished(WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_IGNORED);
        break;

    case SoftwareUpdateManager::kAction_DownloadLater:
        // TODO: Support DownloadLater SoftwareUpdateAvailable action
        ExitNow(err = WEAVE_ERROR_NOT_IMPLEMENTED);
        break;

    case SoftwareUpdateManager::kAction_ApplicationManaged:
        DriveState(SoftwareUpdateManager::kState_ApplicationManaged);
        break;

    case SoftwareUpdateManager::kAction_DownloadNow:

        // If not ignoring partial images...
        if (!mIgnorePartialImage)
        {
            inParam.Clear();
            outParam.Clear();

            // Call the application to determine if a partially downloaded copy of the desired firmware image
            // already exists in local storage.  Pass the URI of the desired image, which must match the metadata
            // stored with the image.
            inParam.FetchPartialImageInfo.URI = mURI;
            outParam.FetchPartialImageInfo.PartialImageLen = 0;
            mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_FetchPartialImageInfo, inParam, outParam);
            VerifyOrExit(mState == SoftwareUpdateManager::kState_Query, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

            // If some part of the desired image has already been downloaded...
            if (outParam.FetchPartialImageInfo.PartialImageLen != 0)
            {
                // Use the length of the partial image as the starting offset for the download.
                mStartOffset = outParam.FetchPartialImageInfo.PartialImageLen;

                // Resume downloading the image.
                DriveState(SoftwareUpdateManager::kState_Download);

                break;
            }
        }

        // Start downloading from the image from the beginning.
        mStartOffset = 0;

        // Initiate the process of preparing local storage for new the image.
        DriveState(SoftwareUpdateManager::kState_PrepareImageStorage);

        break;

    default:
        ExitNow(err = WEAVE_ERROR_INVALID_ARGUMENT);
        break;
    }

exit:
    PacketBuffer::Free(payload);
    if (err != WEAVE_NO_ERROR)
    {
        Impl()->SoftwareUpdateFailed(err, NULL);
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::HandleHoldOffTimerExpired(::nl::Weave::System::Layer * aLayer,
                                                                            void * aAppState,
                                                                            ::nl::Weave::System::Error aError)
{
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();

    {
        SoftwareUpdateStartEvent ev;
        EventOptions evOptions(true);
        ev.trigger = START_TRIGGER_SCHEDULED;
        self->mEventId = nl::LogEvent(&ev, evOptions);
    }

    self->DriveState(SoftwareUpdateManager::kState_PrepareQuery);
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::DriveState(SoftwareUpdateManager::State aNextState)
{
    if (mState != SoftwareUpdateManager::kState_Idle &&
        (aNextState == mState || aNextState >= SoftwareUpdateManager::kState_MaxState))
    {
        ExitNow();
    }

    mState = aNextState;

    switch(mState)
    {
    case SoftwareUpdateManager::kState_Idle:
        {
            /* Compute the next wait time interval only if scheduled software update checks are
             * enabled or when the previous attempt failed provided service connectivity is
             * present. Start the timer once we have a valid interval. A Software Update Check
             * will trigger on expiration of the timer unless service connectivity was lost or
             * the application requested a manual software update check.
             */
            if (mScheduledCheckEnabled || mShouldRetry)
            {
                uint32_t timeToNextQueryMS = GetNextWaitTimeInterval();

                // If timeToNextQueryMs is 0, then do nothing.
                if (timeToNextQueryMS)
                {
                    mState = SoftwareUpdateManager::kState_ScheduledHoldoff;
                    SystemLayer.StartTimer(timeToNextQueryMS, HandleHoldOffTimerExpired, NULL);
                }
            }
        }
        break;

    case SoftwareUpdateManager::kState_PrepareQuery:
        {
            PlatformMgr().ScheduleWork(PrepareBinding);
        }
        break;

    case SoftwareUpdateManager::kState_Query:
        {
            SendQuery();
        }
        break;

    case SoftwareUpdateManager::kState_PrepareImageStorage:
        {
            PrepareImageStorage();
        }
        break;

    case SoftwareUpdateManager::kState_Download:
        {
            PlatformMgr().ScheduleWork(StartDownload);
        }
        break;

    case SoftwareUpdateManager::kState_Install:
        {
            StartImageInstall();
        }
        break;

    default:
        break;
    }

exit:
    return;
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::_SetQueryIntervalWindow(uint32_t aMinWaitTimeMs, uint32_t aMaxWaitTimeMs)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(mEventHandlerCallback != NULL, err = WEAVE_ERROR_INCORRECT_STATE);

    if (aMaxWaitTimeMs == 0)
    {
        WeaveLogProgress(DeviceLayer, "Scheduled Software Update Check Disabled");
        mScheduledCheckEnabled = false;
    }
    else
    {
        mMinWaitTimeMs = aMinWaitTimeMs;
        mMaxWaitTimeMs = aMaxWaitTimeMs;

        mScheduledCheckEnabled = true;
    }

    DriveState(SoftwareUpdateManager::kState_Idle);

exit:
    return err;
}

template<class ImplClass>
uint32_t GenericSoftwareUpdateManagerImpl<ImplClass>::GetNextWaitTimeInterval()
{
    uint32_t timeOutMsecs;

    if (mShouldRetry)
    {
        SoftwareUpdateManager::RetryParam param;
        param.NumRetries = mRetryCounter;

        mRetryPolicyCallback(mAppState, param, timeOutMsecs);

        if (timeOutMsecs == 0)
        {
            if (mScheduledCheckEnabled && timeOutMsecs == 0)
            {
                /** If we have exceeded the max. no. retries, and scheduled queries
                 * are enabled, revert to using scheduled query intervals for computing
                 * wait time.
                 */
                timeOutMsecs = ComputeNextScheduledWaitTimeInterval();
            }
        }
        else
        {
            WeaveLogProgress(DeviceLayer, "Retrying Software Update Check in %ums RetryCounter: %u", timeOutMsecs, mRetryCounter);
        }
    }
    else
    {
        timeOutMsecs = ComputeNextScheduledWaitTimeInterval();
    }

    return timeOutMsecs;
}

template<class ImplClass>
uint32_t GenericSoftwareUpdateManagerImpl<ImplClass>::ComputeNextScheduledWaitTimeInterval(void)
{
    uint32_t timeOutMsecs = (mMinWaitTimeMs + (GetRandU32() % (mMaxWaitTimeMs - mMinWaitTimeMs)));

    WeaveLogProgress(DeviceLayer, "Next Scheduled Software Update Check in %ums", timeOutMsecs);

    return timeOutMsecs;
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::_DefaultEventHandler(void *apAppState,
                                                                      SoftwareUpdateManager::EventType aEvent,
                                                                      const SoftwareUpdateManager::InEventParam& aInParam,
                                                                      SoftwareUpdateManager::OutEventParam& aOutParam)
{
    // No actions required for current implementation
    aOutParam.DefaultHandlerCalled = true;
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::HandleServiceBindingEvent(void * appState, ::nl::Weave::Binding::EventType eventType,
    const ::nl::Weave::Binding::InEventParam & aInParam, ::nl::Weave::Binding::OutEventParam & aOutParam)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    StatusReport *statusReport = NULL;
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();

    switch (eventType)
    {
        case nl::Weave::Binding::kEvent_PrepareFailed:
            WeaveLogProgress(DeviceLayer, "Failed to prepare Software Update binding: %s", ErrorStr(aInParam.PrepareFailed.Reason));
            statusReport = aInParam.PrepareFailed.StatusReport;
            err = aInParam.PrepareFailed.Reason;
            break;

        case nl::Weave::Binding::kEvent_BindingFailed:
            WeaveLogProgress(DeviceLayer, "Software Update binding failed: %s", ErrorStr(aInParam.BindingFailed.Reason));
            err = aInParam.PrepareFailed.Reason;
            break;

        case nl::Weave::Binding::kEvent_BindingReady:
            WeaveLogProgress(DeviceLayer, "Software Update binding ready");

            err = self->mBinding->NewExchangeContext(self->mExchangeCtx);
            SuccessOrExit(err);

            self->mBinding->Release();
            self->mBinding = NULL;

            err = self->PrepareQuery();
            SuccessOrExit(err);

            self->DriveState(SoftwareUpdateManager::kState_Query);
            break;

        default:
            nl::Weave::Binding::DefaultEventHandler(appState, eventType, aInParam, aOutParam);
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        if (self->mBinding != NULL)
        {
            self->mBinding->Release();
            self->mBinding = NULL;
        }

        self->Impl()->SoftwareUpdateFailed(err, statusReport);
    }
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::StoreImageBlock(uint32_t aLength, uint8_t *aData)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    inParam.StoreImageBlock.DataBlockLen = aLength;
    inParam.StoreImageBlock.DataBlock = aData;
    outParam.StoreImageBlock.Error = WEAVE_NO_ERROR;

    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_StoreImageBlock, inParam, outParam);
    VerifyOrExit(mState == SoftwareUpdateManager::kState_Download, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

    // Fail if the application didn't handle the StoreImageBlock event.
    VerifyOrExit(!outParam.DefaultHandlerCalled, err = WEAVE_ERROR_NOT_IMPLEMENTED);

    // Check if the application returned an error while storing an image block.
    err = outParam.StoreImageBlock.Error;
    SuccessOrExit(err);

exit:
    return err;
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::PrepareImageStorage(void)
{
    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    // Call the application to begin the process of preparing local storage for the
    // image to be downloaded.
    inParam.Clear();
    outParam.Clear();
    inParam.PrepareImageStorage.URI = mURI;
    inParam.PrepareImageStorage.IntegrityType = mIntegritySpec.type;
    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_PrepareImageStorage, inParam, outParam);

    // If the application didn't handle the PrepareImageStorage event, immediately proceed to the
    // download state.
    if (outParam.DefaultHandlerCalled)
    {
        Impl()->PrepareImageStorageComplete(WEAVE_NO_ERROR);
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::StartDownload(intptr_t arg)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();

    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    self->mEventHandlerCallback(self->mAppState, SoftwareUpdateManager::kEvent_StartImageDownload, inParam, outParam);
    VerifyOrExit(self->mState == SoftwareUpdateManager::kState_Download, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

    err = self->Impl()->StartImageDownload(self->mURI, self->mStartOffset);
    SuccessOrExit(err);

    {
        DownloadStartEvent ev;
        EventOptions evOptions(true);
        nl::NullifyAllEventFields(&ev);
        evOptions.relatedEventID = self->mEventId;
        ev.imageUrl = self->mURI;
        ev.offset = self->mStartOffset;
        ev.SetImageUrlPresent();
        ev.SetOffsetPresent();
        nl::LogEvent(&ev, evOptions);
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        self->Impl()->SoftwareUpdateFailed(err, NULL);
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::DownloadComplete()
{
    DownloadFinishEvent ev;
    EventOptions evOptions(true);
    nl::NullifyAllEventFields(&ev);
    evOptions.relatedEventID = mEventId;
    nl::LogEvent(&ev, evOptions);

    // Download is complete. Check Image Integrity.
    CheckImageIntegrity();
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::CheckImageIntegrity(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    int result = 0;
    uint8_t typeLength = 0;

    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    switch(mIntegritySpec.type)
    {
        case kIntegrityType_SHA160:
            typeLength = kLength_SHA160;
            break;
        case kIntegrityType_SHA256:
            typeLength = kLength_SHA256;
            break;
        case kIntegrityType_SHA512:
            typeLength = kLength_SHA512;
            break;
        default:
            typeLength = 0;
            break;
    }

    uint8_t computedIntegrityValue[typeLength];

    inParam.ComputeImageIntegrity.IntegrityType = mIntegritySpec.type;
    inParam.ComputeImageIntegrity.IntegrityValueBuf = computedIntegrityValue;
    inParam.ComputeImageIntegrity.IntegrityValueBufLen = typeLength;
    outParam.ComputeImageIntegrity.Error = WEAVE_NO_ERROR;

    // Request the application to compute an integrity check value for the stored image.
    // Fail if the application returns an error.
    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_ComputeImageIntegrity, inParam, outParam);
    VerifyOrExit(mState == SoftwareUpdateManager::kState_Download, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);
    err = outParam.ComputeImageIntegrity.Error;
    SuccessOrExit(err);

    // Verify the computed integrity value matches the expected value given
    // in the SoftwareUpdate:ImageQueryResponse.
    result = memcmp(computedIntegrityValue, mIntegritySpec.value, typeLength);
    VerifyOrExit(result == 0, err = WEAVE_ERROR_INTEGRITY_CHECK_FAILED);

    // Given that the integrity check succeeded, allow future software update attempts
    // to restart an interrupted download.  This turns off the defensive mechanism enabled
    // below when an image fails its integrity check.
    mIgnorePartialImage = false;

    // Tell the application that the image is ready to be installed.
    inParam.Clear();
    outParam.Clear();
    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_ReadyToInstall, inParam, outParam);
    VerifyOrExit(mState == SoftwareUpdateManager::kState_Download, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

    // Advance to the install state.
    DriveState(SoftwareUpdateManager::kState_Install);

exit:
    if (err != WEAVE_NO_ERROR && err != WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED)
    {
        /* Since image integrity validation failed, request the application to reset
         * the persisted image state. This will make sure the image is downloaded from
         * scratch on the next attempt.
         */
        inParam.Clear();
        outParam.Clear();
        mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_ResetPartialImageInfo, inParam, outParam);
        err = (mState == SoftwareUpdateManager::kState_Download) ? WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED : err;

        // Arrange to ignore any partial image on the next software update attempt.
        // This is a defensive measure against an infinite software update loop in the
        // case where the application does not properly handle the ResetPartialImageInfo
        // event.
        mIgnorePartialImage = true;

        Impl()->SoftwareUpdateFailed(err, NULL);
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::StartImageInstall(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    SoftwareUpdateManager::InEventParam inParam;
    SoftwareUpdateManager::OutEventParam outParam;

    inParam.Clear();
    outParam.Clear();

    mEventHandlerCallback(mAppState, SoftwareUpdateManager::kEvent_StartInstallImage, inParam, outParam);
    VerifyOrExit(mState == SoftwareUpdateManager::kState_Install, err = WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);

    {
        /* Log an Install Start Event to indicate that software update
         * install phase has started. The subsequent Install Finish Event
         * should be logged by the application once image installation is complete
         * and the device boots to the new image. If image installation fails and a
         * rollback was performed, application must emit Image Rollback Event. It rollback
         * is not a supported feature, application must emit a Failure Event.
         */
        InstallStartEvent ev;
        EventOptions evOptions(true);
        nl::NullifyAllEventFields(&ev);
        evOptions.relatedEventID = mEventId;
        nl::LogEvent(&ev, evOptions);
    }

    err = Impl()->InstallImage();
    if (err == WEAVE_ERROR_NOT_IMPLEMENTED)
    {
        /*
         * Since the platform does not provide a way to install the image, it is uptp
         * the application to do the install and call ImageInstallComplete API
         * to mark completion of image installation.
         */
        err = WEAVE_NO_ERROR;
    }
    SuccessOrExit(err);

exit:
    if (err != WEAVE_NO_ERROR)
    {
        Impl()->SoftwareUpdateFailed(err, NULL);
    }
}

template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::Cleanup(void)
{
    if (mBinding)
    {
        mBinding->Close();
        mBinding = NULL;
    }

    // Shutdown the exchange if its active
    if (mExchangeCtx)
    {
        mExchangeCtx->Abort();
        mExchangeCtx = NULL;
    }
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::_Abort(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    if (mState == SoftwareUpdateManager::kState_Idle || mState == SoftwareUpdateManager::kState_ScheduledHoldoff)
    {
        err = WEAVE_ERROR_INCORRECT_STATE;
    }
    else
    {
        if (mState == SoftwareUpdateManager::kState_Download)
        {
            Impl()->AbortDownload();
        }

        Cleanup();

        Impl()->SoftwareUpdateFinished(WEAVE_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED);
    }

    return err;
}

/**
 *  Default Policy:
 */
template<class ImplClass>
void GenericSoftwareUpdateManagerImpl<ImplClass>::DefaultRetryPolicyCallback(void * const aAppState,
                                                                             SoftwareUpdateManager::RetryParam & aRetryParam,
                                                                             uint32_t & aOutIntervalMsec)
{
    GenericSoftwareUpdateManagerImpl<ImplClass> * self = &SoftwareUpdateMgrImpl();

    IgnoreUnusedVariable(aAppState);

    uint32_t fibonacciNum      = 0;
    uint32_t maxWaitTimeInMsec = 0;
    uint32_t waitTimeInMsec    = 0;
    uint32_t minWaitTimeInMsec = 0;

    if (aRetryParam.NumRetries <= WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_RETRIES)
    {
        fibonacciNum      = GetFibonacciForIndex(aRetryParam.NumRetries);
        maxWaitTimeInMsec = fibonacciNum * WEAVE_DEVICE_CONFIG_SWU_WAIT_TIME_MULTIPLIER_MS;

        if (self->mScheduledCheckEnabled && WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_WAIT_TIME_INTERVAL_MS > self->mMinWaitTimeMs)
        {
            waitTimeInMsec = 0;
            maxWaitTimeInMsec = 0;
        }
        else
        {
            if (maxWaitTimeInMsec > WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_WAIT_TIME_INTERVAL_MS)
            {
                maxWaitTimeInMsec = WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_MAX_WAIT_TIME_INTERVAL_MS;
            }
        }
    }
    else
    {
       maxWaitTimeInMsec = 0;
    }

    if (maxWaitTimeInMsec != 0)
    {
        minWaitTimeInMsec = (WEAVE_DEVICE_CONFIG_SWU_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP * maxWaitTimeInMsec) / 100;
        waitTimeInMsec    = minWaitTimeInMsec + (GetRandU32() % (maxWaitTimeInMsec - minWaitTimeInMsec));

        WeaveLogDetail(DeviceLayer,
               "Computing swu retry policy: attempts %" PRIu32 ", max wait time %" PRIu32 " ms, selected wait time %" PRIu32
               " ms",
               aRetryParam.NumRetries, maxWaitTimeInMsec, waitTimeInMsec);
    }

    aOutIntervalMsec = waitTimeInMsec;

    return;
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::_PrepareImageStorageComplete(WEAVE_ERROR aError)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Fail if called in the wrong state.
    VerifyOrExit(mState == SoftwareUpdateManager::kState_PrepareImageStorage, err = WEAVE_ERROR_INCORRECT_STATE);

    // If the application completed the prepare process successfully, advance to the Download state.
    // Otherwise, fail the software update attempt.
    if (aError == WEAVE_NO_ERROR)
    {
        DriveState(SoftwareUpdateManager::kState_Download);
    }
    else
    {
        Impl()->SoftwareUpdateFailed(aError, NULL);
    }

exit:
    return err;
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::_ImageInstallComplete(WEAVE_ERROR aError)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    if (mState == SoftwareUpdateManager::kState_ApplicationManaged || mState == SoftwareUpdateManager::kState_Install)
    {
        Impl()->SoftwareUpdateFinished(aError);
    }
    else
    {
        err = WEAVE_ERROR_INCORRECT_STATE;
    }

    return err;
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::GetIntegrityTypeList(::nl::Weave::Profiles::SoftwareUpdate::IntegrityTypeList * aIntegrityTypeList)
{
    uint8_t supportedTypes[] = { Profiles::SoftwareUpdate::kIntegrityType_SHA256 };
    aIntegrityTypeList->init(ArraySize(supportedTypes), supportedTypes);

    return WEAVE_NO_ERROR;
}

template<class ImplClass>
WEAVE_ERROR GenericSoftwareUpdateManagerImpl<ImplClass>::InstallImage(void)
{
    return WEAVE_ERROR_NOT_IMPLEMENTED;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#endif // GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_IPP

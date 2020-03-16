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
 *          Provides an generic implementation of SoftwareUpdateManager features
 *          for use on various platforms.
 */

#ifndef GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_H
#define GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_H

// #if WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <Weave/DeviceLayer/SoftwareUpdateManager.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of Software Update Manager features that works on multiple platforms.
 *
 * This template contains implementations of select features from the SoftwareUpdateManager abstract
 * interface that are suitable for use on all platforms.  It is intended to be inherited (directly
 * or indirectly) by the SoftwareUpdateManagerImpl class, which also appears as the template's ImplClass
 * parameter.
 */
template<class ImplClass>
class GenericSoftwareUpdateManagerImpl
{
    using StatusReport = ::nl::Weave::Profiles::StatusReporting::StatusReport;

protected:
    // ===== Methods that implement the SoftwareUpdateManager abstract interface.

    bool _IsInProgress(void);
    SoftwareUpdateManager::State _GetState(void);

    void _SetRetryPolicyCallback(const SoftwareUpdateManager::RetryPolicyCallback aRetryPolicyCallback);

    static void _DefaultEventHandler(void *apAppState, SoftwareUpdateManager::EventType aEvent,
                             const SoftwareUpdateManager::InEventParam& aInParam,
                             SoftwareUpdateManager::OutEventParam& aOutParam);

    WEAVE_ERROR _Abort(void);
    WEAVE_ERROR _CheckNow(void);
    WEAVE_ERROR _PrepareImageStorageComplete(WEAVE_ERROR aError);
    WEAVE_ERROR _ImageInstallComplete(WEAVE_ERROR aError);
    WEAVE_ERROR _SetQueryIntervalWindow(uint32_t aMinWaitTimeMs, uint32_t aMaxWaitTimeMs);
    WEAVE_ERROR _SetEventCallback(void * const aAppState, const SoftwareUpdateManager::EventCallback aEventCallback);

    // ===== Members for use by the implementation subclass.

    void DoInit();
    void DownloadComplete(void);
    void SoftwareUpdateFailed(WEAVE_ERROR aError, StatusReport * aStatusReport);
    void SoftwareUpdateFinished(WEAVE_ERROR aError);

    WEAVE_ERROR InstallImage(void);
    WEAVE_ERROR StoreImageBlock(uint32_t aLength, uint8_t *aData);
    WEAVE_ERROR GetIntegrityTypeList(::nl::Weave::Profiles::SoftwareUpdate::IntegrityTypeList * aIntegrityTypeList);

private:
    // ===== Private members reserved for use by this class only.

    void Cleanup(void);
    void CheckImageState(void);
    void CheckImageIntegrity(void);
    void DriveState(SoftwareUpdateManager::State aNextState);
    void GetEventState(int32_t& aEventState);
    void HandleImageQueryResponse(PacketBuffer * aPayload);
    void HandleStatusReport(PacketBuffer * aPayload);
    void SendQuery(void);
    void StartImageInstall(void);
    void PrepareImageStorage(void);

    WEAVE_ERROR PrepareQuery(void);

    uint32_t GetNextWaitTimeInterval(void);
    uint32_t ComputeNextScheduledWaitTimeInterval(void);

    static void PrepareBinding(intptr_t arg);
    static void StartDownload(intptr_t arg);
    static void HandleHoldOffTimerExpired(::nl::Weave::System::Layer * aLayer,
                                          void * aAppState,
                                          ::nl::Weave::System::Error aError);
    static void HandleServiceBindingEvent(void * appState, ::nl::Weave::Binding::EventType eventType,
                                          const ::nl::Weave::Binding::InEventParam & inParam,
                                          ::nl::Weave::Binding::OutEventParam & outParam);
    static void HandleResponse(ExchangeContext * ec,
                               const IPPacketInfo * pktInfo,
                               const WeaveMessageInfo * msgInfo,
                               uint32_t profileId,
                               uint8_t msgType,
                               PacketBuffer * payload);
    static void OnKeyError(ExchangeContext *aEc, WEAVE_ERROR aKeyError);
    static void OnResponseTimeout(ExchangeContext * aEC);
    static void DefaultRetryPolicyCallback(void * const aAppState,
                                           SoftwareUpdateManager::RetryParam & aRetryParam,
                                           uint32_t & aOutIntervalMsec);

private:

    SoftwareUpdateManager::State mState;

    void * mAppState;

    char mURI[WEAVE_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN];
    ::nl::Weave::Profiles::SoftwareUpdate::IntegritySpec mIntegritySpec;

    SoftwareUpdateManager::EventCallback mEventHandlerCallback;
    SoftwareUpdateManager::RetryPolicyCallback mRetryPolicyCallback;

    PacketBuffer * mImageQueryPacketBuffer;

    bool mScheduledCheckEnabled;
    bool mShouldRetry;
    bool mIgnorePartialImage;

    uint64_t mNumBytesToDownload;
    uint64_t mStartOffset;

    uint32_t mMinWaitTimeMs;
    uint32_t mMaxWaitTimeMs;
    uint32_t mEventId;

    uint16_t mRetryCounter;

    Binding * mBinding;
    ExchangeContext * mExchangeCtx;

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

// #endif // WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#endif // GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_H

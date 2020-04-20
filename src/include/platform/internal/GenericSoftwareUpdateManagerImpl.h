/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

// #if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <platform/SoftwareUpdateManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Inet;

/**
 * Provides a generic implementation of Software Update Manager features that works on multiple platforms.
 *
 * This template contains implementations of select features from the SoftwareUpdateManager abstract
 * interface that are suitable for use on all platforms.  It is intended to be inherited (directly
 * or indirectly) by the SoftwareUpdateManagerImpl class, which also appears as the template's ImplClass
 * parameter.
 */
template <class ImplClass>
class GenericSoftwareUpdateManagerImpl
{

protected:
    // ===== Methods that implement the SoftwareUpdateManager abstract interface.

    bool _IsInProgress(void);
    SoftwareUpdateManager::State _GetState(void);

    void _SetRetryPolicyCallback(const SoftwareUpdateManager::RetryPolicyCallback aRetryPolicyCallback);

    static void _DefaultEventHandler(void * apAppState, SoftwareUpdateManager::EventType aEvent,
                                     const SoftwareUpdateManager::InEventParam & aInParam,
                                     SoftwareUpdateManager::OutEventParam & aOutParam);

    CHIP_ERROR _Abort(void);
    CHIP_ERROR _CheckNow(void);
    CHIP_ERROR _PrepareImageStorageComplete(CHIP_ERROR aError);
    CHIP_ERROR _ImageInstallComplete(CHIP_ERROR aError);
    CHIP_ERROR _SetQueryIntervalWindow(uint32_t aMinWaitTimeMs, uint32_t aMaxWaitTimeMs);
    CHIP_ERROR _SetEventCallback(void * const aAppState, const SoftwareUpdateManager::EventCallback aEventCallback);

    // ===== Members for use by the implementation subclass.

    void DoInit();
    void DownloadComplete(void);
    void SoftwareUpdateFinished(CHIP_ERROR aError);

    CHIP_ERROR InstallImage(void);
    CHIP_ERROR StoreImageBlock(uint32_t aLength, uint8_t * aData);

private:
    // ===== Private members reserved for use by this class only.

    void Cleanup(void);
    void CheckImageState(void);
    void CheckImageIntegrity(void);
    void DriveState(SoftwareUpdateManager::State aNextState);
    void GetEventState(int32_t & aEventState);
    void HandleImageQueryResponse(PacketBuffer * aPayload);
    void SendQuery(void);
    void StartImageInstall(void);
    void PrepareImageStorage(void);

    CHIP_ERROR PrepareQuery(void);

    uint32_t GetNextWaitTimeInterval(void);
    uint32_t ComputeNextScheduledWaitTimeInterval(void);

    static void PrepareBinding(intptr_t arg);
    static void StartDownload(intptr_t arg);
    static void HandleHoldOffTimerExpired(::chip::System::Layer * aLayer, void * aAppState, ::chip::System::Error aError);
    static void DefaultRetryPolicyCallback(void * const aAppState, SoftwareUpdateManager::RetryParam & aRetryParam,
                                           uint32_t & aOutIntervalMsec);

private:
    SoftwareUpdateManager::State mState;

    void * mAppState;

    char mURI[CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN];

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

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

// #endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#endif // GENERIC_SOFTWARE_UPDATE_MANAGER_IMPL_H

/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#pragma once

// #if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <platform/SoftwareUpdateManager.h>

#include <system/SystemPacketBuffer.h>

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

    bool _IsInProgress();
    SoftwareUpdateManager::State _GetState();

    void _SetRetryPolicyCallback(SoftwareUpdateManager::RetryPolicyCallback aRetryPolicyCallback);

    static void _DefaultEventHandler(void * apAppState, SoftwareUpdateManager::EventType aEvent,
                                     const SoftwareUpdateManager::InEventParam & aInParam,
                                     SoftwareUpdateManager::OutEventParam & aOutParam);

    CHIP_ERROR _Abort();
    CHIP_ERROR _CheckNow();
    CHIP_ERROR _PrepareImageStorageComplete(CHIP_ERROR aError);
    CHIP_ERROR _ImageInstallComplete(CHIP_ERROR aError);
    CHIP_ERROR _SetQueryIntervalWindow(uint32_t aMinWaitTimeMs, uint32_t aMaxWaitTimeMs);
    CHIP_ERROR _SetEventCallback(void * aAppState, SoftwareUpdateManager::EventCallback aEventCallback);

    // ===== Members for use by the implementation subclass.

    void DoInit();
    void DownloadComplete();
    void SoftwareUpdateFinished(CHIP_ERROR aError);

    CHIP_ERROR InstallImage();
    CHIP_ERROR StoreImageBlock(uint32_t aLength, uint8_t * aData);

private:
    // ===== Private members reserved for use by this class only.

    void Cleanup();
    void CheckImageState();
    void CheckImageIntegrity();
    void DriveState(SoftwareUpdateManager::State aNextState);
    void GetEventState(int32_t & aEventState);
    void HandleImageQueryResponse(chip::System::PacketBuffer * aPayload);
    void SendQuery();
    void StartImageInstall();
    void PrepareImageStorage();

    CHIP_ERROR PrepareQuery();

    uint32_t GetNextWaitTimeInterval();
    uint32_t ComputeNextScheduledWaitTimeInterval();

    static void PrepareBinding(intptr_t arg);
    static void StartDownload(intptr_t arg);
    static void HandleHoldOffTimerExpired(::chip::System::Layer * aLayer, void * aAppState, ::CHIP_ERROR aError);
    static void DefaultRetryPolicyCallback(void * aAppState, SoftwareUpdateManager::RetryParam & aRetryParam,
                                           uint32_t & aOutIntervalMsec);

    SoftwareUpdateManager::State mState;

    void * mAppState;

    char mURI[CHIP_DEVICE_CONFIG_SOFTWARE_UPDATE_URI_LEN];

    SoftwareUpdateManager::EventCallback mEventHandlerCallback;
    SoftwareUpdateManager::RetryPolicyCallback mRetryPolicyCallback;

    chip::System::PacketBuffer * mImageQueryPacketBuffer;

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

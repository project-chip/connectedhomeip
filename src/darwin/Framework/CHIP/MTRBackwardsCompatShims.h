/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#import <Matter/MTRCommandPayloadsObjc.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRStructsObjc.h>

/**
 * This file defines manual backwards-compat shims of various sorts to handle
 * API changes that happened.
 */

NS_ASSUME_NONNULL_BEGIN

@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesParams ()
/**
 * This command used to incorrectly have a groupKeySetIDs field.
 */
@property (nonatomic, copy) NSArray * groupKeySetIDs MTR_DEPRECATED(
    "This field has been removed", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));

@end

/**
 * FanControl used to have WindSettingMask and WindSupportMask that had
 * identical values.  Those got replaced with a single WindBitmap.  We codegen
 * WindSupportMask as an alias of WindBitmap, but we need a manual shim for
 * WindSettingMask.
 */
typedef NS_OPTIONS(uint8_t, MTRFanControlWindSettingMask) {
    MTRFanControlWindSettingMaskSleepWind MTR_DEPRECATED(
        "Please use MTRFanControlWindBitmapSleepWind", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
    = 0x1,
    MTRFanControlWindSettingMaskNaturalWind MTR_DEPRECATED(
        "Please use MTRFanControlWindBitmapNaturalWind", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0))
    = 0x2,
} MTR_DEPRECATED("Please use MTRFanControlWindBitmap", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));

/**
 * For the OTA clusters, first we changed the names of the clusters, and then we
 * changed the names of the enums.  That means we now have three names for the
 * enums (newest name, name before enum name change, name before cluster name
 * change).  We can only track one old name for things, so just manually add the
 * intermediate names.
 */
typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateProviderOTAApplyUpdateAction) {
    MTROTASoftwareUpdateProviderOTAApplyUpdateActionProceed MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateActionProceed")
    = 0x00,
    MTROTASoftwareUpdateProviderOTAApplyUpdateActionAwaitNextAction MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateActionAwaitNextAction")
    = 0x01,
    MTROTASoftwareUpdateProviderOTAApplyUpdateActionDiscontinue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateActionDiscontinue")
    = 0x02,
} MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderApplyUpdateAction");

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateProviderOTADownloadProtocol) {
    MTROTASoftwareUpdateProviderOTADownloadProtocolBDXSynchronous MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolBDXSynchronous")
    = 0x00,
    MTROTASoftwareUpdateProviderOTADownloadProtocolBDXAsynchronous MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolBDXAsynchronous")
    = 0x01,
    MTROTASoftwareUpdateProviderOTADownloadProtocolHTTPS MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolHTTPS")
    = 0x02,
    MTROTASoftwareUpdateProviderOTADownloadProtocolVendorSpecific MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocolVendorSpecific")
    = 0x03,
} MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderDownloadProtocol");

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateProviderOTAQueryStatus) {
    MTROTASoftwareUpdateProviderOTAQueryStatusUpdateAvailable MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusUpdateAvailable")
    = 0x00,
    MTROTASoftwareUpdateProviderOTAQueryStatusBusy MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusBusy")
    = 0x01,
    MTROTASoftwareUpdateProviderOTAQueryStatusNotAvailable MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusNotAvailable")
    = 0x02,
    MTROTASoftwareUpdateProviderOTAQueryStatusDownloadProtocolNotSupported MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4),
        tvos(16.4)) MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatusDownloadProtocolNotSupported")
    = 0x03,
} MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateProviderStatus");

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateRequestorOTAAnnouncementReason) {
    MTROTASoftwareUpdateRequestorOTAAnnouncementReasonSimpleAnnouncement MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4),
        tvos(16.4)) MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReasonSimpleAnnouncement")
    = 0x00,
    MTROTASoftwareUpdateRequestorOTAAnnouncementReasonUpdateAvailable MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4),
        tvos(16.4)) MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReasonUpdateAvailable")
    = 0x01,
    MTROTASoftwareUpdateRequestorOTAAnnouncementReasonUrgentUpdateAvailable MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4),
        tvos(16.4)) MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReasonUrgentUpdateAvailable")
    = 0x02,
} MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorAnnouncementReason");

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateRequestorOTAChangeReason) {
    MTROTASoftwareUpdateRequestorOTAChangeReasonUnknown MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonUnknown")
    = 0x00,
    MTROTASoftwareUpdateRequestorOTAChangeReasonSuccess MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonSuccess")
    = 0x01,
    MTROTASoftwareUpdateRequestorOTAChangeReasonFailure MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonFailure")
    = 0x02,
    MTROTASoftwareUpdateRequestorOTAChangeReasonTimeOut MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonTimeOut")
    = 0x03,
    MTROTASoftwareUpdateRequestorOTAChangeReasonDelayByProvider MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReasonDelayByProvider")
    = 0x04,
} MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorChangeReason");

typedef NS_ENUM(uint8_t, MTROTASoftwareUpdateRequestorOTAUpdateState) {
    MTROTASoftwareUpdateRequestorOTAUpdateStateUnknown MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateUnknown")
    = 0x00,
    MTROTASoftwareUpdateRequestorOTAUpdateStateIdle MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateIdle")
    = 0x01,
    MTROTASoftwareUpdateRequestorOTAUpdateStateQuerying MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateQuerying")
    = 0x02,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDelayedOnQuery MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDelayedOnQuery")
    = 0x03,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDownloading MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDownloading")
    = 0x04,
    MTROTASoftwareUpdateRequestorOTAUpdateStateApplying MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateApplying")
    = 0x05,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDelayedOnApply MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDelayedOnApply")
    = 0x06,
    MTROTASoftwareUpdateRequestorOTAUpdateStateRollingBack MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateRollingBack")
    = 0x07,
    MTROTASoftwareUpdateRequestorOTAUpdateStateDelayedOnUserConsent MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
        MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateStateDelayedOnUserConsent")
    = 0x08,
} MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
    MTR_NEWLY_DEPRECATED("Please use MTROTASoftwareUpdateRequestorUpdateState");

NS_ASSUME_NONNULL_END

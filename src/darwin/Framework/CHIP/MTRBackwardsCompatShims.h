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

NS_ASSUME_NONNULL_END

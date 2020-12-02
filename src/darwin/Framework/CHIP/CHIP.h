/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// pull together CHIP headers
#import <CHIP/CHIPDevice.h>
#import <CHIP/CHIPDeviceCallback.h>
#import <CHIP/CHIPDeviceController.h>
#import <CHIP/CHIPDevicePairingDelegate.h>
#import <CHIP/CHIPError.h>
#import <CHIP/CHIPManualSetupPayloadParser.h>
#import <CHIP/CHIPOnOff.h>
#import <CHIP/CHIPPersistentStorageDelegate.h>
#import <CHIP/CHIPQRCodeSetupPayloadParser.h>
#import <CHIP/CHIPSetupPayload.h>

#import <Foundation/Foundation.h>
//! Project version number for CHIP.
FOUNDATION_EXPORT double CHIPVersionNumber;

//! Project version string for CHIP.
FOUNDATION_EXPORT const unsigned char CHIPVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <CHIP/PublicHeader.h>

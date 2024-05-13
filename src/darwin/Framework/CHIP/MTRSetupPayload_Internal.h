/**
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#import "MTRSetupPayload.h"

#import "MTRDefines_Internal.h"

#import <setup_payload/SetupPayload.h>

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@interface MTRSetupPayload ()

- (instancetype)initWithSetupPayload:(chip::SetupPayload)setupPayload;
- (nullable instancetype)initWithQRCode:(NSString *)qrCodePayload;
- (nullable instancetype)initWithManualPairingCode:(NSString *)manualCode;

@end

NS_ASSUME_NONNULL_END

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

- (instancetype)initWithSetupPayload:(const chip::SetupPayload &)setupPayload;
- (nullable instancetype)initWithQRCode:(NSString *)qrCodePayload;
/**
 * Initializes the payload object from a "MT:"-prefixed QR Code string.
 *
 * Returns nil and populates `error` (if non-nil) on parse failure. The
 * returned error is in MTRErrorDomain; per-failure codes from the underlying
 * Base38 decode (e.g. MTRErrorCodeInvalidIntegerValue for an out-of-alphabet
 * character, MTRErrorCodeInvalidStringLength for a malformed chunk) are
 * preserved so callers can distinguish typo-class failures from structural
 * rejection.
 *
 * @note MTRErrorCodeInvalidArgument may still be returned for structurally
 *       valid Base38 payloads whose contents fail semantic QR Code validation
 *       (the !isValidQRCodePayload branch). This is intentionally identical
 *       to the corresponding branch in -initWithManualPairingCode:error:.
 */
- (nullable instancetype)initWithQRCode:(NSString *)qrCodePayload error:(NSError * __autoreleasing *)error;
- (nullable instancetype)initWithManualPairingCode:(NSString *)manualCode;

@end

NS_ASSUME_NONNULL_END

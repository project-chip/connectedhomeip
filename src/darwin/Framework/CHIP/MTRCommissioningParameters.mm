/**
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#import "MTRCommissioningParameters.h"

#import <CommonCrypto/CommonDigest.h>

#include <lib/support/BytesToHex.h>

NS_ASSUME_NONNULL_BEGIN

@implementation MTRCommissioningParameters : NSObject

@end

@implementation MTRCommissioningParameters (Deprecated)

- (NSData * _Nullable)CSRNonce
{
    return self.csrNonce;
}

- (void)setCSRNonce:(NSData * _Nullable)CSRNonce
{
    self.csrNonce = CSRNonce;
}

- (NSNumber * _Nullable)failSafeExpiryTimeoutSecs
{
    return self.failSafeTimeout;
}

- (void)setFailSafeExpiryTimeoutSecs:(NSNumber * _Nullable)failSafeExpiryTimeoutSecs
{
    self.failSafeTimeout = failSafeExpiryTimeoutSecs;
}

- (NSString *)description
{
    NSString * ssidString;
    if (self.wifiSSID) {
        // We want to log the SSID, but hash it, so that the actual SSID cannot be
        // recovered from the log.
        uint8_t hashedValue[CC_SHA256_DIGEST_LENGTH];
        CC_SHA256(self.wifiSSID.bytes, static_cast<CC_LONG>(self.wifiSSID.length), hashedValue);

        char hexValue[sizeof(hashedValue) * 2];
        chip::Encoding::BytesToHex(hashedValue, sizeof(hashedValue), hexValue, sizeof(hexValue), chip::Encoding::HexFlags::kUppercase);
        ssidString = [[NSString alloc] initWithBytes:hexValue
                                              length:sizeof(hexValue)
                                            encoding:NSUTF8StringEncoding];
    } else {
        ssidString = nil;
    }
    return [NSString stringWithFormat:@"<MTRCommissioningParameters: %p ssid: %@>", self, ssidString];
}

@end

NS_ASSUME_NONNULL_END

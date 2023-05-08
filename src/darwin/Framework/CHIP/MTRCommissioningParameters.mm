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

@end

NS_ASSUME_NONNULL_END

/**
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

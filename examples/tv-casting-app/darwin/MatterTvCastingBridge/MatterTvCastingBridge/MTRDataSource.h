/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRCommissionableData.h"
#import "MTRDeviceAttestationCredentials.h"

#ifndef MTRDataSource_h
#define MTRDataSource_h

@protocol MTRDataSource <NSObject>

- (dispatch_queue_t _Nonnull)clientQueue;

- (NSData * _Nonnull)castingAppDidReceiveRequestForRotatingDeviceIdUniqueId:(id _Nonnull)sender;
- (MTRCommissionableData * _Nonnull)castingAppDidReceiveRequestForCommissionableData:(id _Nonnull)sender;
- (MTRDeviceAttestationCredentials * _Nonnull)castingAppDidReceiveRequestForDeviceAttestationCredentials:(id _Nonnull)sender;

- (NSData * _Nonnull)castingApp:(id _Nonnull)sender didReceiveRequestToSignCertificateRequest:(NSData * _Nonnull)csrData;

@end

#endif /* MTRDataSource_h */

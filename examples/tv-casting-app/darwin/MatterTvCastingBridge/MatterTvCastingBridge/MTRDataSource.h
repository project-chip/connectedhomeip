/**
 *
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

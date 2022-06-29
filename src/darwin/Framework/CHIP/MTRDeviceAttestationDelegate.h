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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

/**
 * The protocol definition for the MTRDeviceAttestationDelegate
 *
 * All delegate methods will be called on the callers queue.
 */
@protocol MTRDeviceAttestationDelegate <NSObject>
/**
 * Notify the delegate when device attestation fails
 *
 * @param controller Controller corresponding to the commissioning process
 * @param device Handle of device being commissioned
 * @param error NSError representing the error code for the failure
 */
- (void)deviceAttestation:(MTRDeviceController *)controller failedForDevice:(void *)device error:(NSError * _Nonnull)error;

@end

NS_ASSUME_NONNULL_END

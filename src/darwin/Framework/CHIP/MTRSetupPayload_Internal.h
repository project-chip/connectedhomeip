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

#ifdef __cplusplus
#import <lib/core/Optional.h>
#import <setup_payload/SetupPayload.h>
#endif

MTR_DIRECT_MEMBERS
@interface MTRSetupPayload ()

#ifdef __cplusplus
- (instancetype)initWithSetupPayload:(chip::SetupPayload)setupPayload;
- (MTRDiscoveryCapabilities)convertRendezvousFlags:(const chip::Optional<chip::RendezvousInformationFlags> &)value;
- (MTRCommissioningFlow)convertCommissioningFlow:(chip::CommissioningFlow)value;
#endif

@end

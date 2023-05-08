/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "DeviceAttestationCredentialsHolder.h"
#import "OnboardingPayload.h"

#ifndef AppParameters_h
#define AppParameters_h

@interface AppParameters : NSObject

@property NSData * rotatingDeviceIdUniqueId;

@property OnboardingPayload * onboardingPayload;

@property uint32_t spake2pIterationCount;

@property NSData * spake2pSaltBase64;

@property NSData * spake2pVerifierBase64;

@end

#endif /* AppParameters_h */

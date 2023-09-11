/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

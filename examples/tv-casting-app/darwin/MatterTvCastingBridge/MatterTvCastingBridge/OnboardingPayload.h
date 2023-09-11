/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OnboardingPayload_h
#define OnboardingPayload_h

@interface OnboardingPayload : NSObject

@property uint32_t setupPasscode;

@property uint16_t setupDiscriminator;

- (instancetype)initWithSetupPasscode:(uint32_t)setupPasscode setupDiscriminator:(uint16_t)setupDiscriminator;

@end

#endif /* OnboardingPayload_h */

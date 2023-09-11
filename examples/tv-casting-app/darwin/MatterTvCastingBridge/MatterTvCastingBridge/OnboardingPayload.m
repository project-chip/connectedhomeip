/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "OnboardingPayload.h"

@implementation OnboardingPayload

- (instancetype)initWithSetupPasscode:(uint32_t)setupPasscode setupDiscriminator:(uint16_t)setupDiscriminator
{
    if (self = [super init]) {
        _setupPasscode = setupPasscode;
        _setupDiscriminator = setupDiscriminator;
    }
    return self;
}

@end

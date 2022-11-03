/*
 * SPDX-FileCopyrightText: (c) 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "AppParameters.h"

@implementation AppParameters

- (AppParameters *)initWithRotatingDeviceIdUniqueId:(NSData *)rotatingDeviceIdUniqueId
{
    self = [super init];
    if (self) {
        _rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueId;
    }
    return self;
}

@end

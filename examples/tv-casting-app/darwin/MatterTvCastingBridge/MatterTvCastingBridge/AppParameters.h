/*
 * SPDX-FileCopyrightText: (c) 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#ifndef AppParameters_h
#define AppParameters_h

@interface AppParameters : NSObject

@property NSData * rotatingDeviceIdUniqueId;

- (AppParameters *)initWithRotatingDeviceIdUniqueId:(NSData *)rotatingDeviceIdUniqueId;

@end

#endif /* AppParameters_h */

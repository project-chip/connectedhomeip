/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MTR_DEVICE_EXTERNS_H
#define MTR_DEVICE_EXTERNS_H

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/DeviceProxy.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRBaseDevice (InternalIntrospection)

/**
 * Invalidate the CASE session, so an attempt to getConnectedDevice for this
 * device id will have to create a new CASE session.  Ideally this API will go
 * away.
 */
- (void)invalidateCASESession;

@end

NS_ASSUME_NONNULL_END

#endif /* MTR_DEVICE_EXTERNS_H */

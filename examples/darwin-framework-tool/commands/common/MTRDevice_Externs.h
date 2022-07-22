/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

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

#import "MTRBaseDevice.h"
#import <Foundation/Foundation.h>

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteEventPath.h>
#include <app/DeviceProxy.h>

@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

@interface MTRBaseDevice ()

- (instancetype)initWithPASEDevice:(chip::DeviceProxy *)device controller:(MTRDeviceController *)controller;
- (instancetype)initWithNodeID:(chip::NodeId)nodeID controller:(MTRDeviceController *)controller;

/**
 * Only returns non-nil if the device is using a PASE session.  Otherwise, the
 * deviceController + nodeId should be used to get a CASE session.
 */
- (chip::DeviceProxy * _Nullable)paseDevice;

/**
 * Invalidate the CASE session, so an attempt to getConnectedDevice for this
 * device id will have to create a new CASE session.  Ideally this API will go
 * away.
 */
- (void)invalidateCASESession;

/**
 * Controller that that this MTRDevice was gotten from.
 */
@property (nonatomic, strong, readonly) MTRDeviceController * deviceController;

/**
 * Node id for this MTRDevice.  Only set to a usable value if this device
 * represents a CASE session.
 */
@property (nonatomic, assign, readonly) chip::NodeId nodeID;

/**
 * Controllers are created via the MTRControllerFactory object.
 */
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

@interface MTRAttributePath ()
- (instancetype)initWithPath:(const chip::app::ConcreteDataAttributePath &)path;
@end

@interface MTREventPath ()
- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path;
@end

@interface MTRCommandPath ()
- (instancetype)initWithPath:(const chip::app::ConcreteCommandPath &)path;
@end

@interface MTRAttributeReport ()
- (instancetype)initWithPath:(const chip::app::ConcreteDataAttributePath &)path
                       value:(nullable id)value
                       error:(nullable NSError *)error;
@end

@interface MTREventReport ()
- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path
                 eventNumber:(NSNumber *)eventNumber
                    priority:(NSNumber *)priority
                   timestamp:(NSNumber *)timestamp
                       value:(nullable id)value
                       error:(nullable NSError *)error;
@end

// Exported utility function
// Convert TLV data into data-value dictionary as described in MTRDeviceResponseHandler
id _Nullable MTRDecodeDataValueDictionaryFromCHIPTLV(chip::TLV::TLVReader * data);

NS_ASSUME_NONNULL_END

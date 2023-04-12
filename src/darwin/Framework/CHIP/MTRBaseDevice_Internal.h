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

static inline MTRTransportType MTRMakeTransportType(chip::Transport::Type type)
{
    static_assert(MTRTransportTypeUndefined == (uint8_t) chip::Transport::Type::kUndefined, "MTRTransportType != Transport::Type");
    static_assert(MTRTransportTypeUDP == (uint8_t) chip::Transport::Type::kUdp, "MTRTransportType != Transport::Type");
    static_assert(MTRTransportTypeBLE == (uint8_t) chip::Transport::Type::kBle, "MTRTransportType != Transport::Type");
    static_assert(MTRTransportTypeTCP == (uint8_t) chip::Transport::Type::kTcp, "MTRTransportType != Transport::Type");
    return static_cast<MTRTransportType>(type);
}

@interface MTRBaseDevice ()

- (instancetype)initWithPASEDevice:(chip::DeviceProxy *)device controller:(MTRDeviceController *)controller;

/**
 * Invalidate the CASE session, so an attempt to getConnectedDevice for this
 * device id will have to create a new CASE session.  Ideally this API will go
 * away.
 */
- (void)invalidateCASESession;

/**
 * Whether this device represents a PASE session or not.
 */
@property (nonatomic, assign, readonly) BOOL isPASEDevice;

/**
 * Controller that that this MTRDevice was gotten from.
 */
@property (nonatomic, strong, readonly) MTRDeviceController * deviceController;

/**
 * Node id for this MTRDevice.  If this device represents a CASE session, this
 * is set to the node ID of the target node.  If this device represents a PASE
 * session, this is set to the device id of the PASE device.
 */
@property (nonatomic, assign, readonly) chip::NodeId nodeID;

/**
 * Initialize the device object as a CASE device with the given node id and
 * controller.  This will always succeed, even if there is no such node id on
 * the controller's fabric, but attempts to actually use the MTRBaseDevice will
 * fail (asynchronously) in that case.
 */
- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller;

@end

@interface MTRClusterPath ()
- (instancetype)initWithPath:(const chip::app::ConcreteClusterPath &)path;
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
                       value:(id _Nullable)value
                       error:(NSError * _Nullable)error;
@end

@interface MTREventReport ()
- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path
                 eventNumber:(NSNumber *)eventNumber
                    priority:(NSNumber *)priority
                   timestamp:(NSNumber *)timestamp
                       value:(id _Nullable)value
                       error:(NSError * _Nullable)error;
@end

// Exported utility function
// Convert TLV data into data-value dictionary as described in MTRDeviceResponseHandler
id _Nullable MTRDecodeDataValueDictionaryFromCHIPTLV(chip::TLV::TLVReader * data);

NS_ASSUME_NONNULL_END

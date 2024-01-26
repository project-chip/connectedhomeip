/**
 *    Copyright (c) 2024 Project CHIP Authors
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
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRServerEndpoint.h>

typedef NS_ENUM(NSUInteger, MTRServerEndpointState) {
    /**
     * The Initializing state indicates an endpoint or object that hangs off an
     * endpoint (cluster, attribute, etc) that is still being set up by the API
     * consumer and has not been added to a controller yet.
     */
    MTRServerEndpointStateInitializing,
    /**
     * The Active state indicates an endpoint or object that hangs off an
     * endpoint (cluster, attribute, etc) that is now attached to a controller.
     * This state is entered from the Initializing state on the consumer's
     * thread/queue.  In this state, any mutations to Matter-observable state
     * must happen on the Matter queue.  While in this state, the object holds a
     * non-nil weak reference to the controller.
     */
    MTRServerEndpointStateActive,
    /**
     * The Defunct state indicates an endpoint or object that hangs off an
     * endpoint (cluster, attribute, etc) that used to be attached to a
     * controller but isn't any longer. This could be because the controller
     * shut down, or because the endpoint was removed from the controller.
     */
    MTRServerEndpointStateDefunct,
};

@interface MTRServerEndpoint ()

/**
 * Mark this endpoint as associated with a particular controller.
 */
- (BOOL)associateWithController:(MTRDeviceController *)controller;

/**
 * Mark this endpoint as being in a Defunct state.
 */
- (void)invalidate;

/**
 * The access grants the Matter stack can observe.  Only modified while in
 * Initializing state or on the Matter queue.
 */
@property (nonatomic, strong, readonly) NSSet<MTRAccessGrant *> * matterAccessGrants;

@end

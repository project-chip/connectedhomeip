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

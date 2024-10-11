/**
 *    Copyright (c) 2023 Project CHIP Authors
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
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, MTRStorageSecurityLevel) {
    // Data must be stored in secure (encrypted) storage.
    MTRStorageSecurityLevelSecure,
    // Data may be stored in the clear.
    MTRStorageSecurityLevelNotSecure,
} MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

typedef NS_ENUM(NSUInteger, MTRStorageSharingType) {
    // Data must not be shared at all (just store locally).
    MTRStorageSharingTypeNotShared,

    // Data must be shared, but only between controllers that have the same node
    // identity (same fabric, same node ID, same CATs).
    MTRStorageSharingTypeSameIdentity,

    // Data must be shared, but only between controllers that have the same
    // access to devices (e.g. controllers that all have the same CATs if ACLs
    // are being done via CATs).
    MTRStorageSharingTypeSameACLs,

    // Data must be shared across all controllers on a given fabric.
    MTRStorageSharingTypeSameFabric,
} MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6));

/**
 * Protocol for storing and retrieving controller-specific data.
 *
 * Implementations of this protocol MUST keep these things in mind:
 *
 * 1) The controller provided to the delegate methods may not be fully
 *    initialized when the callbacks are called.  The only safe thing to do with
 *    it is to get its controllerID.
 *
 * 2) The delegate method calls will happen on the queue that was provided along
 *    with the delegate.  All Matter work will be blocked until the method
 *    completes, and these calls may themselves block other Matter API calls
 *    from completing.  Attempting to call any Matter API on the queue used for
 *    this delegate, apart from de-serializing and serializing the items being
 *    stored and calling MTRDeviceControllerStorageClasses(), is likely to lead
 *    to deadlocks.
 *
 * 3) Security level and sharing type will always be the same for any given key value
 *    and are provided to describe the data should the storage delegate choose to
 *    implement separating storage location by security level and sharing type.
 */
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@protocol MTRDeviceControllerStorageDelegate <NSObject>
@required
/**
 * Return the stored value for the given key, if any, for the provided
 * controller.  Returns nil if there is no stored value.
 *
 * The set of classes that might be decoded by this function is available by
 * calling MTRDeviceControllerStorageClasses().
 */
- (nullable id<NSSecureCoding>)controller:(MTRDeviceController *)controller
                              valueForKey:(NSString *)key
                            securityLevel:(MTRStorageSecurityLevel)securityLevel
                              sharingType:(MTRStorageSharingType)sharingType;

/**
 * Store a value for the given key.  Returns whether the store succeeded.
 */
- (BOOL)controller:(MTRDeviceController *)controller
        storeValue:(id<NSSecureCoding>)value
            forKey:(NSString *)key
     securityLevel:(MTRStorageSecurityLevel)securityLevel
       sharingType:(MTRStorageSharingType)sharingType;

/**
 * Remove the stored value for the given key.  Returns whether the remove succeeded.
 */
- (BOOL)controller:(MTRDeviceController *)controller
    removeValueForKey:(NSString *)key
        securityLevel:(MTRStorageSecurityLevel)securityLevel
          sharingType:(MTRStorageSharingType)sharingType;

@optional
/**
 * Return all keys and values stored, if any, for the provided controller, in a
 * dictionary. Returns nil if there are no stored values.
 *
 * securityLevel and sharingType are provided as a hint for the storage delegate
 * to load from the right security level and sharing type, if the implementation
 * stores them separately. If the implementation includes key/value pairs from other
 * security levels or sharing types, they will be ignored by the caller.
 *
 * The set of classes that might be decoded by this function is available by
 * calling MTRDeviceControllerStorageClasses().
 */
- (nullable NSDictionary<NSString *, id<NSSecureCoding>> *)valuesForController:(MTRDeviceController *)controller
                                                                 securityLevel:(MTRStorageSecurityLevel)securityLevel
                                                                   sharingType:(MTRStorageSharingType)sharingType;

/**
 * Store a list of key/value pairs in the form of a dictionary. Returns whether
 * the store succeeded. Specifically, if any keys in this dictionary fail to store,
 * the storage delegate should return NO.
 */
- (BOOL)controller:(MTRDeviceController *)controller
       storeValues:(NSDictionary<NSString *, id<NSSecureCoding>> *)values
     securityLevel:(MTRStorageSecurityLevel)securityLevel
       sharingType:(MTRStorageSharingType)sharingType;
@end

MTR_EXTERN MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
    NSSet<Class> * MTRDeviceControllerStorageClasses(void);

NS_ASSUME_NONNULL_END

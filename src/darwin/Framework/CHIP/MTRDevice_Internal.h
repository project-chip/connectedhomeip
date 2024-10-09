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

#import <Foundation/Foundation.h>
#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRDevice.h>
#import <os/lock.h>

#import "MTRAsyncWorkQueue.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceStorageBehaviorConfiguration_Internal.h"

NS_ASSUME_NONNULL_BEGIN

@class MTRAsyncWorkQueue;

typedef NSDictionary<NSString *, id> * MTRDeviceDataValueDictionary;

typedef void (^MTRDevicePerformAsyncBlock)(MTRBaseDevice * baseDevice);

typedef NS_ENUM(NSUInteger, MTRInternalDeviceState) {
    // Unsubscribed means we do not have a subscription and are not trying to set one up.
    MTRInternalDeviceStateUnsubscribed = 0,
    // Subscribing means we are actively trying to establish our initial subscription (e.g. doing
    // DNS-SD discovery, trying to establish CASE to the peer, getting priming reports, etc).
    MTRInternalDeviceStateSubscribing = 1,
    // InitialSubscriptionEstablished means we have at some point finished setting up a
    // subscription.  That subscription may have dropped since then, but if so it's the ReadClient's
    // responsibility to re-establish it.
    MTRInternalDeviceStateInitialSubscriptionEstablished = 2,
    // Resubscribing means we had established a subscription, but then
    // detected a subscription drop due to not receiving a report on time. This
    // covers all the actions that happen when re-subscribing (discovery, CASE,
    // getting priming reports, etc).
    MTRInternalDeviceStateResubscribing = 3,
    // LaterSubscriptionEstablished meant that we had a subscription drop and
    // then re-created a subscription.
    MTRInternalDeviceStateLaterSubscriptionEstablished = 4,
};

/**
 * Information about a cluster: data version and known attribute values.
 */
MTR_TESTABLE
@interface MTRDeviceClusterData : NSObject <NSSecureCoding, NSCopying>
@property (nonatomic, nullable) NSNumber * dataVersion;
@property (nonatomic, readonly) NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * attributes; // attributeID => data-value dictionary

- (void)storeValue:(MTRDeviceDataValueDictionary _Nullable)value forAttribute:(NSNumber *)attribute;
- (void)removeValueForAttribute:(NSNumber *)attribute;

- (nullable instancetype)initWithDataVersion:(NSNumber * _Nullable)dataVersion attributes:(NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _Nullable)attributes;
@end

// Consider moving utility classes to their own file
#pragma mark - Utility Classes

/**
 * container of MTRDevice delegate weak reference, its queue, and its interested
 * paths for attribute reports.
 */
MTR_DIRECT_MEMBERS
@interface MTRDeviceDelegateInfo : NSObject {
@private
    void * _delegatePointerValue;
    __weak id _delegate;
    dispatch_queue_t _queue;
}

// Array of interested cluster paths, attribute paths, or endpointID, for attribute report filtering.
@property (readonly, nullable) NSArray * interestedPathsForAttributes;

// Array of interested cluster paths, attribute paths, or endpointID, for event report filtering.
@property (readonly, nullable) NSArray * interestedPathsForEvents;

// Expose delegate
@property (readonly) id delegate;

// Pointer value for logging purpose only
@property (readonly) void * delegatePointerValue;

- (instancetype)initWithDelegate:(id<MTRDeviceDelegate>)delegate queue:(dispatch_queue_t)queue interestedPathsForAttributes:(NSArray * _Nullable)interestedPathsForAttributes interestedPathsForEvents:(NSArray * _Nullable)interestedPathsForEvents;

// Returns YES if delegate and queue are both non-null, and the block is scheduled to run.
- (BOOL)callDelegateWithBlock:(void (^)(id<MTRDeviceDelegate>))block;

#ifdef DEBUG
// Only used for unit test purposes - normal delegate should not expect or handle being called back synchronously.
- (BOOL)callDelegateSynchronouslyWithBlock:(void (^)(id<MTRDeviceDelegate>))block;
#endif
@end

#pragma mark - MTRDevice internal extensions

@interface MTRDevice () {
    // Ivars needed to implement shared MTRDevice functionality.
@protected
    // Lock that protects overall device state, including delegate storage.
    os_unfair_lock _lock;
    NSMutableSet<MTRDeviceDelegateInfo *> * _delegates;

    // Our node ID, with the ivar declared explicitly so it's accessible to
    // subclasses.
    NSNumber * _nodeID;

    // Our controller.  Declared nullable because our property is, though in
    // practice it does not look like we ever set it to nil.  If this changes,
    // fix _concreteController on MTRDevice_Concrete accordingly.
    MTRDeviceController * _Nullable _deviceController;
}

- (instancetype)initForSubclassesWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller;

// called by controller to clean up and shutdown
- (void)invalidate;

- (BOOL)_callDelegatesWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block;

// Called by MTRDevice_XPC to forward delegate callbacks
- (BOOL)_lockAndCallDelegatesWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block;

/**
 * Like the public invokeCommandWithEndpointID but:
 *
 * 1) Allows passing through a serverSideProcessingTimeout.
 * 2) Expects one of the command payload structs as commandPayload
 * 3) On success, returns an instance of responseClass via the completion (or
 *    nil if there is no responseClass, which indicates a status-only command).
 */
- (void)_invokeKnownCommandWithEndpointID:(NSNumber *)endpointID
                                clusterID:(NSNumber *)clusterID
                                commandID:(NSNumber *)commandID
                           commandPayload:(id)commandPayload
                           expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
                    expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                       timedInvokeTimeout:(NSNumber * _Nullable)timeout
              serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                            responseClass:(Class _Nullable)responseClass
                                    queue:(dispatch_queue_t)queue
                               completion:(void (^)(id _Nullable response, NSError * _Nullable error))completion;

// Queue used for various internal bookkeeping work.
@property (nonatomic) dispatch_queue_t queue;
@property (nonatomic, readonly) MTRAsyncWorkQueue<MTRDevice *> * asyncWorkQueue;

// Method to insert persisted cluster data
//   Contains data version information and attribute values.
- (void)setPersistedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData;

// Method to insert persisted data that pertains to the whole device.
- (void)setPersistedDeviceData:(NSDictionary<NSString *, id> *)data;

#ifdef DEBUG
- (NSUInteger)unitTestAttributeCount;
#endif

- (void)setStorageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration *)storageBehaviorConfiguration;

// Returns whether this MTRDevice uses Thread for communication
- (BOOL)deviceUsesThread;

#pragma mark - MTRDevice functionality to deal with delegates.

// Returns YES if any non-null delegates were found
- (BOOL)_iterateDelegatesWithBlock:(void(NS_NOESCAPE ^ _Nullable)(MTRDeviceDelegateInfo * delegateInfo))block;

- (BOOL)_delegateExists;

// Must be called by subclasses or MTRDevice implementation only.
- (void)_delegateAdded;

#ifdef DEBUG
// Only used for unit test purposes - normal delegate should not expect or handle being called back synchronously
// Returns YES if a delegate is called
- (void)_callFirstDelegateSynchronouslyWithBlock:(void (^)(id<MTRDeviceDelegate> delegate))block;
#endif

// Used to generate attribute report that contains all known attributes, taking into consideration expected values
- (NSArray<NSDictionary<NSString *, id> *> *)getAllAttributesReport;

// Hooks for controller suspend/resume.
- (void)controllerSuspended;
- (void)controllerResumed;

@end

#pragma mark - MTRDevice internal state monitoring
@protocol MTRDeviceInternalStateDelegate
- (void)devicePrivateInternalStateChanged:(MTRDevice *)device internalState:(NSDictionary *)state;
@end

#pragma mark - Constants

static NSString * const kDefaultSubscriptionPoolSizeOverrideKey = @"subscriptionPoolSizeOverride";
static NSString * const kTestStorageUserDefaultEnabledKey = @"enableTestStorage";

// ex-MTRDeviceClusterData constants
static NSString * const sDataVersionKey = @"dataVersion";
static NSString * const sAttributesKey = @"attributes";
static NSString * const sLastInitialSubscribeLatencyKey = @"lastInitialSubscribeLatency";

// Declared inside platform, but noting here for reference
// static NSString * const kSRPTimeoutInMsecsUserDefaultKey = @"SRPTimeoutInMSecsOverride";

// Concrete to XPC internal state property dictionary keys
static NSString * const kMTRDeviceInternalPropertyKeyVendorID = @"MTRDeviceInternalStateKeyVendorID";
static NSString * const kMTRDeviceInternalPropertyKeyProductID = @"MTRDeviceInternalStateKeyProductID";
static NSString * const kMTRDeviceInternalPropertyNetworkFeatures = @"MTRDeviceInternalPropertyNetworkFeatures";
static NSString * const kMTRDeviceInternalPropertyDeviceState = @"MTRDeviceInternalPropertyDeviceState";
static NSString * const kMTRDeviceInternalPropertyLastSubscriptionAttemptWait = @"kMTRDeviceInternalPropertyLastSubscriptionAttemptWait";
static NSString * const kMTRDeviceInternalPropertyMostRecentReportTime = @"MTRDeviceInternalPropertyMostRecentReportTime";
static NSString * const kMTRDeviceInternalPropertyLastSubscriptionFailureTime = @"MTRDeviceInternalPropertyLastSubscriptionFailureTime";

NS_ASSUME_NONNULL_END

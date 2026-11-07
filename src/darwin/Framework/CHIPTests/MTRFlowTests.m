/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#import <Matter/Matter.h>
#import <XCTest/XCTest.h>
#import <objc/runtime.h>
#import <os/lock.h>

#import "MTRDelegateManager.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDevice_Internal.h"
#import "MTRMetricsScope_Internal.h"
#import "MTRMetrics_Internal.h"
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"

static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kFlowTestNodeID = 0x7A3C1001;

static NSString * const kRecordName = @"name";
static NSString * const kRecordFlow = @"flow";
static NSString * const kRecordPayload = @"payload";

static NSString * const kStateChanged = @"stateChanged";
static NSString * const kAttributeReport = @"receivedAttributeReport";
static NSString * const kEventReport = @"receivedEventReport";
static NSString * const kBecameActive = @"deviceBecameActive";
static NSString * const kCachePrimed = @"deviceCachePrimed";
static NSString * const kConfigurationChanged = @"deviceConfigurationChanged";
static NSString * const kInternalStateUpdated = @"internalStateUpdated";

@interface MTRDevice (FlowTest)
- (void)_handleUnsolicitedMessageFromPublisher;
- (void)_handleSessionFailureWithError:(NSError *)error retryDelay:(NSNumber * _Nullable)retryDelay;
@end

@interface MTRDeviceController (FlowTest)
- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler;
- (void)unitTestGetSessionForNode:(NSNumber *)nodeID completion:(void (^)(NSError * _Nullable error))completion;
@end

static id CurrentFlowIDOrNull(void)
{
    return MTRMetrics.currentFlowID ?: NSNull.null;
}

static NSArray * TestAttributeReport(NSNumber * clusterID, NSNumber * attributeID, unsigned value)
{
    return @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:clusterID attributeID:attributeID],
        MTRDataKey : @ {
            MTRDataVersionKey : @(value),
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(value),
        },
    } ];
}

static NSArray * TestLevelReport(unsigned value)
{
    return TestAttributeReport(@(MTRClusterIDTypeLevelControlID), @(MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID), value);
}

static NSArray * TestConfigurationReport(unsigned value)
{
    return TestAttributeReport(@(MTRClusterIDTypeLevelControlID), @(MTRAttributeIDTypeGlobalAttributeFeatureMapID), value);
}

static NSArray * TestEventReport(void)
{
    static uint64_t eventNumber = 0x200000000llu;
    return @[ @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) eventID:@(0)],
        MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
        MTREventTimestampDateKey : [NSDate date],
        MTREventIsHistoricalKey : @(NO),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventNumberKey : @(eventNumber++),
        MTRDataKey : @ { MTRTypeKey : MTRStructureValueType, MTRValueKey : @[] },
    } ];
}

static NSArray * TestInvokeResponse(NSNumber * commandID)
{
    return @[ @{MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) commandID:commandID]} ];
}

// What a server does: adds the flow ID to each response-value dictionary.
static NSArray * StampedValues(NSArray * values, id flowID)
{
    NSMutableArray * stamped = [NSMutableArray array];
    for (NSDictionary * value in values) {
        NSMutableDictionary * copy = [value mutableCopy];
        copy[MTRDeviceResponseFlowIDKey] = flowID;
        [stamped addObject:copy];
    }
    return stamped;
}

#pragma mark - Delegates

@interface MTRFlowRecordingDelegate : NSObject <MTRDeviceDelegate>
- (void)_record:(NSString *)name payload:(id _Nullable)payload;
- (NSArray<NSDictionary *> *)records;
- (NSArray<NSDictionary *> *)recordsNamed:(NSString *)name;
@end

@implementation MTRFlowRecordingDelegate {
    os_unfair_lock _lock;
    NSMutableArray<NSDictionary *> * _records;
}

- (instancetype)init
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _records = [NSMutableArray array];
    }
    return self;
}

- (void)_record:(NSString *)name payload:(id _Nullable)payload
{
    NSMutableDictionary * record = [@{ kRecordName : name, kRecordFlow : CurrentFlowIDOrNull() } mutableCopy];
    if (payload) {
        record[kRecordPayload] = payload;
    }
    os_unfair_lock_lock(&_lock);
    [_records addObject:record];
    os_unfair_lock_unlock(&_lock);
}

- (NSArray<NSDictionary *> *)records
{
    os_unfair_lock_lock(&_lock);
    NSArray * records = [_records copy];
    os_unfair_lock_unlock(&_lock);
    return records;
}

- (NSArray<NSDictionary *> *)recordsNamed:(NSString *)name
{
    return [self.records filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K == %@", kRecordName, name]];
}

- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
    [self _record:kStateChanged payload:@(state)];
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    [self _record:kAttributeReport payload:attributeReport];
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    [self _record:kEventReport payload:eventReport];
}

- (void)deviceBecameActive:(MTRDevice *)device
{
    [self _record:kBecameActive payload:nil];
}

- (void)deviceCachePrimed:(MTRDevice *)device
{
    [self _record:kCachePrimed payload:nil];
}

- (void)deviceConfigurationChanged:(MTRDevice *)device
{
    [self _record:kConfigurationChanged payload:nil];
}

- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device
{
    return NO;
}

- (BOOL)unitTestSuppressTimeBasedReachabilityChanges:(MTRDevice *)device
{
    return YES;
}

@end

@interface MTRFlowInternalStateRecordingDelegate : MTRFlowRecordingDelegate
@end

@implementation MTRFlowInternalStateRecordingDelegate

- (void)device:(MTRDevice *)device internalStateUpdated:(NSDictionary *)dictionary
{
    [self _record:kInternalStateUpdated payload:dictionary];
}

@end

@interface MTRFlowOriginalMethodsDelegate : NSObject <MTRDeviceDelegate>
@property (atomic, copy) NSArray<NSString *> * calls;
@property (atomic, copy, nullable) NSArray * lastAttributeReport;
@property (atomic, copy, nullable) NSArray * lastEventReport;
@end

@implementation MTRFlowOriginalMethodsDelegate

- (instancetype)init
{
    if (self = [super init]) {
        _calls = @[];
    }
    return self;
}

- (void)_note:(NSString *)name
{
    @synchronized(self) {
        self.calls = [self.calls arrayByAddingObject:name];
    }
}

- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
    [self _note:kStateChanged];
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    self.lastAttributeReport = attributeReport;
    [self _note:kAttributeReport];
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    self.lastEventReport = eventReport;
    [self _note:kEventReport];
}

- (void)deviceBecameActive:(MTRDevice *)device
{
    [self _note:kBecameActive];
}

- (void)deviceCachePrimed:(MTRDevice *)device
{
    [self _note:kCachePrimed];
}

- (void)deviceConfigurationChanged:(MTRDevice *)device
{
    [self _note:kConfigurationChanged];
}

- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device
{
    return NO;
}

- (BOOL)unitTestSuppressTimeBasedReachabilityChanges:(MTRDevice *)device
{
    return YES;
}

@end

#pragma mark - Shared test helpers

@interface MTRFlowTestCase : MTRTestCase
- (void)waitForRecordsOf:(MTRFlowRecordingDelegate *)delegate named:(NSString *)name count:(NSUInteger)count;
@end

@implementation MTRFlowTestCase

- (void)waitForRecordsOf:(MTRFlowRecordingDelegate *)delegate named:(NSString *)name count:(NSUInteger)count
{
    NSPredicate * predicate = [NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return [delegate recordsNamed:name].count >= count;
    }];
    XCTestExpectation * expectation = [self expectationForPredicate:predicate evaluatedWithObject:delegate handler:nil];
    expectation.expectationDescription = [NSString stringWithFormat:@"%lu %@ callback(s)", (unsigned long) count, name];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

@end

#pragma mark - Current flow and scopes

@interface MTRFlowScopeTests : XCTestCase
@end

@implementation MTRFlowScopeTests

- (void)test001_NestedScopesRestore
{
    XCTAssertNil(MTRMetrics.currentFlowID);
    NSUUID * outer = [NSUUID UUID];
    NSUUID * inner = [NSUUID UUID];

    __block id seenOuterBefore;
    __block id seenInner;
    __block id seenOuterAfter;
    [MTRMetrics _runWithFlowID:outer
                         block:^{
                             seenOuterBefore = MTRMetrics.currentFlowID;
                             [MTRMetrics _runWithFlowID:inner
                                                  block:^{
                                                      seenInner = MTRMetrics.currentFlowID;
                                                  }];
                             seenOuterAfter = MTRMetrics.currentFlowID;
                         }];

    XCTAssertEqualObjects(seenOuterBefore, outer);
    XCTAssertEqualObjects(seenInner, inner);
    XCTAssertEqualObjects(seenOuterAfter, outer, @"the inner scope did not restore the outer flow");
    XCTAssertNil(MTRMetrics.currentFlowID);
}

- (void)test002_DispatchCarriesCurrentToAnotherQueue
{
    NSUUID * flowID = [NSUUID UUID];
    dispatch_queue_t queue = dispatch_queue_create("flow-test-callback", DISPATCH_QUEUE_SERIAL);

    XCTestExpectation * carried = [self expectationWithDescription:@"callback ran in the caller's flow"];
    __block id carriedFlow;
    [MTRMetrics _runWithFlowID:flowID
                         block:^{
                             MTRDispatchCallbackWithFlowID(queue, ^{
                                 carriedFlow = CurrentFlowIDOrNull();
                                 [carried fulfill];
                             });
                         }];

    XCTestExpectation * fresh = [self expectationWithDescription:@"callback ran in a new flow"];
    __block id freshFlow;
    MTRDispatchCallbackWithFlowID(queue, ^{
        freshFlow = CurrentFlowIDOrNull();
        [fresh fulfill];
    });

    XCTestExpectation * plain = [self expectationWithDescription:@"plain dispatch ran with no flow"];
    __block id plainFlow;
    dispatch_async(queue, ^{
        plainFlow = CurrentFlowIDOrNull();
        [plain fulfill];
    });

    [self waitForExpectations:@[ carried, fresh, plain ] timeout:kTimeoutInSeconds];
    XCTAssertEqualObjects(carriedFlow, flowID);
    XCTAssertTrue([freshFlow isKindOfClass:NSUUID.class], @"%@", freshFlow);
    XCTAssertNotEqualObjects(freshFlow, flowID);
    XCTAssertEqualObjects(plainFlow, NSNull.null);
}

- (void)test003_FlowIDFromAnotherProcessIsKept
{
    NSUUID * sent = [NSUUID UUID];
    XCTAssertEqualObjects([MTRMetrics _flowIDWithString:sent.UUIDString], sent);

    for (id invalid in @[ @(5), @"not a UUID" ]) {
        NSUUID * fresh = [MTRMetrics _flowIDWithString:invalid];
        XCTAssertTrue([fresh isKindOfClass:NSUUID.class], @"%@", invalid);
        XCTAssertNotEqualObjects(fresh, sent);
    }
    XCTAssertNotEqualObjects([MTRMetrics _flowIDWithString:@"not a UUID"], [MTRMetrics _flowIDWithString:@"not a UUID"]);
}

- (void)test004_DelegateCallbackRunsInTheCallersFlow
{
    NSObject * delegate = [[NSObject alloc] init];
    __auto_type * info = [[MTRDelegateInfo alloc] initWithDelegate:delegate queue:dispatch_queue_create("flow-test-delegate", DISPATCH_QUEUE_SERIAL)];
    NSUUID * flowID = [NSUUID UUID];

    XCTestExpectation * called = [self expectationWithDescription:@"delegate called"];
    __block id flow;
    [MTRMetrics _runWithFlowID:flowID
                         block:^{
                             [info callDelegateWithBlock:^(id calledDelegate) {
                                 flow = CurrentFlowIDOrNull();
                                 [called fulfill];
                             }];
                         }];
    [self waitForExpectations:@[ called ] timeout:kTimeoutInSeconds];
    XCTAssertEqualObjects(flow, flowID);
}

@end

#pragma mark - In-process MTRDevice

@interface MTRFlowLocalTests : MTRFlowTestCase
@end

@implementation MTRFlowLocalTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _controller = [self createControllerOnTestFabric];
    _device = [MTRDevice deviceWithNodeID:@(kFlowTestNodeID) controller:_controller];
}

- (void)tearDown
{
    [_controller removeDevice:_device];
    _device = nil;
    _controller = nil;
    [super tearDown];
}

- (void)test001_DelegatesOfOneCallbackShareOneFlow
{
    __auto_type * first = [[MTRFlowRecordingDelegate alloc] init];
    __auto_type * second = [[MTRFlowRecordingDelegate alloc] init];
    [_device addDelegate:first queue:dispatch_queue_create("flow-test-first", DISPATCH_QUEUE_SERIAL)];
    [_device addDelegate:second queue:dispatch_queue_create("flow-test-second", DISPATCH_QUEUE_SERIAL)];

    [_device unitTestInjectAttributeReport:TestConfigurationReport(1) fromSubscription:YES];
    [self waitForRecordsOf:first named:kCachePrimed count:1];
    [self waitForRecordsOf:second named:kCachePrimed count:1];

    for (NSString * name in @[ kStateChanged, kAttributeReport, kConfigurationChanged, kCachePrimed ]) {
        id firstFlow = [first recordsNamed:name].firstObject[kRecordFlow];
        id secondFlow = [second recordsNamed:name].firstObject[kRecordFlow];
        XCTAssertTrue([firstFlow isKindOfClass:NSUUID.class], @"%@: %@", name, firstFlow);
        XCTAssertEqualObjects(firstFlow, secondFlow, @"%@", name);
    }
    [_device removeDelegate:first];
    [_device removeDelegate:second];
}

// The log callback gets the message of the one line -[MTRDevice addDelegate:queue:] logs for this delegate.
- (NSArray<NSString *> *)messagesLoggedByAddingDelegate:(id<MTRDeviceDelegate>)delegate inFlow:(NSUUID * _Nullable)flowID
{
    os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock_t lockPtr = &lock;
    NSMutableArray<NSString *> * messages = [NSMutableArray array];
    NSString * delegateDescription = [NSString stringWithFormat:@"%@", delegate];
    MTRSetLogCallback(MTRLogTypeDetail, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        if ([message containsString:@"addDelegate"] && [message containsString:delegateDescription]) {
            os_unfair_lock_lock(lockPtr);
            [messages addObject:message];
            os_unfair_lock_unlock(lockPtr);
        }
    });
    dispatch_queue_t queue = dispatch_queue_create("flow-test-log", DISPATCH_QUEUE_SERIAL);
    if (flowID) {
        [MTRMetrics _runWithFlowID:flowID
                             block:^{
                                 [self->_device addDelegate:delegate queue:queue];
                             }];
    } else {
        [_device addDelegate:delegate queue:queue];
    }
    MTRSetLogCallback(0, nil);
    [_device removeDelegate:delegate];
    os_unfair_lock_lock(lockPtr);
    NSArray<NSString *> * result = [messages copy];
    os_unfair_lock_unlock(lockPtr);
    return result;
}

// Part C, item 1: the log callback gets "[Flow: <UUID>] " in front of a line logged in a flow.
- (void)test004_LogCallbackMessageStartsWithTheCurrentFlow
{
    NSUUID * flowID = [NSUUID UUID];
    NSArray<NSString *> * messages = [self messagesLoggedByAddingDelegate:[[MTRFlowRecordingDelegate alloc] init] inFlow:flowID];
    XCTAssertEqual(messages.count, 1, @"%@", messages);
    NSString * prefix = [NSString stringWithFormat:@"[Flow: %@] ", flowID.UUIDString];
    XCTAssertTrue([messages.firstObject hasPrefix:prefix], @"expected prefix %@ in: %@", prefix, messages.firstObject);
}

// Part C, item 1: a line logged outside any flow is delivered unchanged.
- (void)test005_LogCallbackMessageHasNoPrefixWithoutAFlow
{
    XCTAssertNil(MTRMetrics.currentFlowID);
    NSArray<NSString *> * messages = [self messagesLoggedByAddingDelegate:[[MTRFlowRecordingDelegate alloc] init] inFlow:nil];
    XCTAssertEqual(messages.count, 1, @"%@", messages);
    XCTAssertFalse([messages.firstObject hasPrefix:@"[Flow: "], @"%@", messages.firstObject);
    XCTAssertTrue([messages.firstObject containsString:@" addDelegate "], @"%@", messages.firstObject);
}

- (void)test002_InternalStateCarriesTheFlowOfItsCallback
{
    __auto_type * delegate = [[MTRFlowInternalStateRecordingDelegate alloc] init];
    [_device addDelegate:delegate queue:dispatch_queue_create("flow-test-internal-state", DISPATCH_QUEUE_SERIAL)];
    [_device unitTestInjectAttributeReport:TestConfigurationReport(2) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kCachePrimed count:1];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
    [self waitForRecordsOf:delegate named:kInternalStateUpdated count:1];

    NSArray * updates = [delegate recordsNamed:kInternalStateUpdated];
    XCTAssertGreaterThan(updates.count, 0);
    for (NSDictionary * update in updates) {
        NSUUID * flow = update[kRecordFlow];
        XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"%@", flow);
        XCTAssertEqualObjects(update[kRecordPayload][kMTRDeviceInternalPropertyFlowID], flow.UUIDString, @"%@", update[kRecordPayload]);
    }
    [_device removeDelegate:delegate];
}

// guard, passes both: in-process report dictionaries do not change.
- (void)test003_DelegateWithOnlyOriginalMethodsGetsEveryCallback
{
    __auto_type * delegate = [[MTRFlowOriginalMethodsDelegate alloc] init];
    [_device addDelegate:delegate queue:dispatch_get_main_queue()];

    NSArray * attributeReport = TestConfigurationReport(9);
    NSArray * eventReport = TestEventReport();
    [_device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];
    [_device unitTestInjectEventReport:eventReport];
    MTRDevice * device = _device;
    XCTestExpectation * ran = [self expectationWithDescription:@"ran on Matter queue"];
    [_controller asyncDispatchToMatterQueue:^{
        [device _handleUnsolicitedMessageFromPublisher];
        [ran fulfill];
    } errorHandler:nil];
    [self waitForExpectations:@[ ran ] timeout:kTimeoutInSeconds];

    NSArray * expected = @[ kStateChanged, kAttributeReport, kConfigurationChanged, kCachePrimed, kEventReport, kBecameActive ];
    XCTestExpectation * allCalled = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return [[NSSet setWithArray:expected] isSubsetOfSet:[NSSet setWithArray:delegate.calls]];
    }]
                                              evaluatedWithObject:delegate
                                                          handler:nil];
    [self waitForExpectations:@[ allCalled ] timeout:kTimeoutInSeconds];

    XCTAssertEqual(delegate.calls.count, expected.count);
    XCTAssertEqualObjects(delegate.lastAttributeReport.firstObject[MTRAttributePathKey], attributeReport.firstObject[MTRAttributePathKey]);
    XCTAssertEqualObjects(delegate.lastAttributeReport.firstObject[MTRDataKey][MTRValueKey], @(9));
    XCTAssertEqualObjects(delegate.lastEventReport.firstObject[MTREventNumberKey], eventReport.firstObject[MTREventNumberKey]);
    XCTAssertEqualObjects([NSSet setWithArray:[delegate.lastAttributeReport.firstObject allKeys]], [NSSet setWithArray:[attributeReport.firstObject allKeys]]);
    [_device removeDelegate:delegate];
}

@end

#pragma mark - XPC

// A server that stamps its replies the way a real one would.
@interface MTRFlowTestXPCServer : NSObject <NSXPCListenerDelegate>
@property (nonatomic, readonly) NSXPCListener * listener;
@property (atomic, strong, nullable) NSXPCConnection * clientConnection;
@property (atomic, copy, nullable) NSDictionary * lastControllerConfiguration;
@property (atomic, strong, nullable) id replyFlowID;
@property (atomic, strong, nullable) NSError * replyError;
- (id<MTRXPCClientProtocol>)client;
@end

@implementation MTRFlowTestXPCServer

- (instancetype)init
{
    if (self = [super init]) {
        _listener = [NSXPCListener anonymousListener];
        _listener.delegate = self;
        [_listener resume];
    }
    return self;
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    NSXPCInterface * exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];
    NSSet * commandClasses = [NSSet setWithObjects:NSArray.class, NSDictionary.class, NSString.class, NSNumber.class, NSData.class, MTRCommandPath.class, MTRCommandWithRequiredResponse.class, nil];
    SEL invoke = @selector(deviceController:nodeID:invokeCommandWithEndpointID:clusterID:commandID:commandFields:expectedValues:expectedValueInterval:timedInvokeTimeout:serverSideProcessingTimeout:completion:);
    [exportedInterface setClasses:commandClasses forSelector:invoke argumentIndex:5 ofReply:NO];
    [exportedInterface setClasses:commandClasses forSelector:invoke argumentIndex:6 ofReply:NO];
    [exportedInterface setClasses:commandClasses forSelector:@selector(deviceController:nodeID:invokeCommands:completion:) argumentIndex:2 ofReply:NO];

    newConnection.exportedInterface = exportedInterface;
    newConnection.exportedObject = self;
    newConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];
    self.clientConnection = newConnection;
    [newConnection activate];
    return YES;
}

- (id<MTRXPCClientProtocol>)client
{
    return [self.clientConnection remoteObjectProxy];
}

- (oneway void)deviceController:(NSUUID *)controller updateControllerConfiguration:(NSDictionary *)controllerState
{
    self.lastControllerConfiguration = controllerState;
}

- (oneway void)deviceController:(NSUUID *)controller getNodesWithStoredDataWithReply:(void (^)(NSArray<NSNumber *> *))reply
{
    reply(@[]);
}

- (NSError *)_stampedReplyError
{
    NSMutableDictionary * userInfo = [self.replyError.userInfo mutableCopy] ?: [NSMutableDictionary dictionary];
    userInfo[MTRErrorFlowIDKey] = self.replyFlowID;
    return [NSError errorWithDomain:self.replyError.domain code:self.replyError.code userInfo:userInfo];
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommandWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID commandID:(NSNumber *)commandID commandFields:(id)commandFields expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedInvokeTimeout:(NSNumber * _Nullable)timeout serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                     completion:(MTRDeviceResponseHandler)completion
{
    if (self.replyError) {
        completion(nil, [self _stampedReplyError]);
        return;
    }
    completion(StampedValues(TestInvokeResponse(commandID), self.replyFlowID), nil);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands completion:(MTRDeviceResponseHandler)completion
{
    completion(StampedValues(TestInvokeResponse(@(0)), self.replyFlowID), nil);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID downloadLogOfType:(MTRDiagnosticLogType)type timeout:(NSTimeInterval)timeout completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    completion(nil, [self _stampedReplyError]);
}

@end

@interface MTRFlowXPCTests : MTRFlowTestCase
@end

@implementation MTRFlowXPCTests {
    BOOL _localTestStorageEnabledBeforeTest;
    MTRFlowTestXPCServer * _server;
    MTRDeviceController * _controller;
    MTRDevice * _device;
    MTRFlowRecordingDelegate * _delegate;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _localTestStorageEnabledBeforeTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = NO;

    _server = [[MTRFlowTestXPCServer alloc] init];
    NSXPCListenerEndpoint * endpoint = _server.listener.endpoint;
    __auto_type * parameters = [[MTRXPCDeviceControllerParameters alloc] initWithXPCConnectionBlock:^NSXPCConnection * {
        return [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
    } uniqueIdentifier:[NSUUID UUID]];
    NSError * error;
    _controller = [[MTRDeviceController alloc] initWithParameters:parameters error:&error];
    XCTAssertNotNil(_controller, @"%@", error);

    _device = [MTRDevice deviceWithNodeID:@(kFlowTestNodeID) controller:_controller];
    _delegate = [[MTRFlowRecordingDelegate alloc] init];
    [_device addDelegate:_delegate queue:dispatch_queue_create("flow-test-xpc-delegate", DISPATCH_QUEUE_SERIAL)];

    MTRFlowTestXPCServer * server = _server;
    XCTestExpectation * registered = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return server.clientConnection != nil && server.lastControllerConfiguration != nil;
    }]
                                               evaluatedWithObject:server
                                                           handler:nil];
    [self waitForExpectations:@[ registered ] timeout:kTimeoutInSeconds];
}

- (void)tearDown
{
    [_device removeDelegate:_delegate];
    [_controller shutdown];
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeTest;
    [super tearDown];
}

- (NSDictionary *)internalStateWithState:(MTRDeviceState)state primed:(BOOL)primed flowID:(NSUUID * _Nullable)flowID
{
    NSMutableDictionary * internalState = [@{
        kMTRDeviceInternalPropertyDeviceState : @(state),
        kMTRDeviceInternalPropertyLastSubscriptionAttemptWait : @(0),
        kMTRDeviceInternalPropertyDeviceCachePrimed : @(primed),
    } mutableCopy];
    if (flowID) {
        internalState[kMTRDeviceInternalPropertyFlowID] = flowID.UUIDString;
    }
    return internalState;
}

- (void)test001_AttributeReportFlowKeyBecomesCurrentAndIsStripped
{
    NSUUID * flow = [NSUUID UUID];
    NSArray * report = [TestLevelReport(11) arrayByAddingObjectsFromArray:TestConfigurationReport(12)];
    [[_server client] device:@(kFlowTestNodeID) receivedAttributeReport:StampedValues(report, flow.UUIDString)];
    [self waitForRecordsOf:_delegate named:kAttributeReport count:1];

    NSDictionary * record = [_delegate recordsNamed:kAttributeReport].firstObject;
    XCTAssertEqualObjects(record[kRecordFlow], flow);
    XCTAssertEqualObjects(record[kRecordPayload], report);
}

- (void)test002_EventReportFlowKeyBecomesCurrentAndIsStripped
{
    NSUUID * flow = [NSUUID UUID];
    NSArray * report = TestEventReport();
    [[_server client] device:@(kFlowTestNodeID) receivedEventReport:StampedValues(report, flow.UUIDString)];
    [self waitForRecordsOf:_delegate named:kEventReport count:1];

    NSDictionary * record = [_delegate recordsNamed:kEventReport].firstObject;
    XCTAssertEqualObjects(record[kRecordFlow], flow);
    XCTAssertEqualObjects(record[kRecordPayload], report);
}

- (void)test003_InternalStateFlowReachesStateAndPrimedCallbacks
{
    NSUUID * flow = [NSUUID UUID];
    [[_server client] device:@(kFlowTestNodeID) internalStateUpdated:[self internalStateWithState:MTRDeviceStateReachable primed:YES flowID:flow]];
    [self waitForRecordsOf:_delegate named:kCachePrimed count:1];

    NSArray * records = [_delegate.records filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K IN %@", kRecordName, @[ kStateChanged, kCachePrimed ]]];
    XCTAssertEqual(records.count, 2);
    for (NSDictionary * record in records) {
        XCTAssertEqualObjects(record[kRecordFlow], flow, @"%@", record[kRecordName]);
    }
    NSDictionary * storedState = [_device valueForKey:@"_internalState"];
    XCTAssertNotNil(storedState);
    XCTAssertNil(storedState[kMTRDeviceInternalPropertyFlowID], @"the flow is not part of the stored state: %@", storedState);
}

// guard, passes both: the flow key in internal state does not change which callbacks run.
- (void)test004_InternalStateFlowKeyDoesNotChangeCallbacks
{
    __auto_type * plainDevice = [MTRDevice deviceWithNodeID:@(kFlowTestNodeID + 1) controller:_controller];
    __auto_type * plainDelegate = [[MTRFlowRecordingDelegate alloc] init];
    [plainDevice addDelegate:plainDelegate queue:dispatch_queue_create("flow-test-xpc-plain-delegate", DISPATCH_QUEUE_SERIAL)];
    MTRFlowTestXPCServer * server = _server;
    XCTestExpectation * registered = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        NSArray * nodes = server.lastControllerConfiguration[MTRDeviceControllerRegistrationNodeIDsKey];
        return [[nodes valueForKey:MTRDeviceControllerRegistrationNodeIDKey] containsObject:@(kFlowTestNodeID + 1)];
    }]
                                               evaluatedWithObject:server
                                                           handler:nil];
    [self waitForExpectations:@[ registered ] timeout:kTimeoutInSeconds];

    NSArray<NSArray *> * sequence = @[ @[ @(MTRDeviceStateReachable), @NO ], @[ @(MTRDeviceStateUnreachable), @YES ], @[ @(MTRDeviceStateReachable), @YES ] ];
    for (NSArray * step in sequence) {
        MTRDeviceState state = (MTRDeviceState)[step[0] unsignedIntegerValue];
        BOOL primed = [step[1] boolValue];
        [[_server client] device:@(kFlowTestNodeID) internalStateUpdated:[self internalStateWithState:state primed:primed flowID:[NSUUID UUID]]];
        [[_server client] device:@(kFlowTestNodeID + 1) internalStateUpdated:[self internalStateWithState:state primed:primed flowID:nil]];
    }
    [[_server client] deviceConfigurationChanged:@(kFlowTestNodeID)];
    [[_server client] deviceConfigurationChanged:@(kFlowTestNodeID + 1)];
    [self waitForRecordsOf:_delegate named:kConfigurationChanged count:1];
    [self waitForRecordsOf:plainDelegate named:kConfigurationChanged count:1];

    XCTAssertEqualObjects([_delegate.records valueForKey:kRecordName], [plainDelegate.records valueForKey:kRecordName]);
    XCTAssertEqualObjects([[_delegate recordsNamed:kStateChanged] valueForKey:kRecordPayload], [[plainDelegate recordsNamed:kStateChanged] valueForKey:kRecordPayload]);
    XCTAssertEqual(_device.state, plainDevice.state);
    XCTAssertEqual(_device.deviceCachePrimed, plainDevice.deviceCachePrimed);
    XCTAssertGreaterThan([_delegate recordsNamed:kStateChanged].count, 1);
    [plainDevice removeDelegate:plainDelegate];
}

- (void)test005_MalformedFlowKeyGetsFreshFlow
{
    NSArray * report = TestLevelReport(13);
    [[_server client] device:@(kFlowTestNodeID) receivedAttributeReport:StampedValues(report, @(13))];
    [[_server client] device:@(kFlowTestNodeID) receivedAttributeReport:StampedValues(report, @"not a UUID")];
    [self waitForRecordsOf:_delegate named:kAttributeReport count:2];

    NSArray * records = [_delegate recordsNamed:kAttributeReport];
    for (NSDictionary * record in records) {
        XCTAssertEqualObjects(record[kRecordPayload], report);
        XCTAssertTrue([record[kRecordFlow] isKindOfClass:NSUUID.class], @"%@", record[kRecordFlow]);
    }
    XCTAssertNotEqualObjects(records[0][kRecordFlow], records[1][kRecordFlow]);
}

// deviceBecameActive: and deviceConfigurationChanged: carry no dictionary, so each gets a new flow.
- (void)test006_CallbacksWithoutDictionaryGetFreshFlow
{
    [[_server client] deviceBecameActive:@(kFlowTestNodeID)];
    [[_server client] deviceConfigurationChanged:@(kFlowTestNodeID)];
    [self waitForRecordsOf:_delegate named:kConfigurationChanged count:1];

    id activeFlow = [_delegate recordsNamed:kBecameActive].firstObject[kRecordFlow];
    id configurationFlow = [_delegate recordsNamed:kConfigurationChanged].firstObject[kRecordFlow];
    XCTAssertTrue([activeFlow isKindOfClass:NSUUID.class], @"%@", activeFlow);
    XCTAssertTrue([configurationFlow isKindOfClass:NSUUID.class], @"%@", configurationFlow);
    XCTAssertNotEqualObjects(activeFlow, configurationFlow);
}

- (id)invokeToggleWithValues:(NSArray * __autoreleasing *)valuesOut error:(NSError * __autoreleasing *)errorOut
{
    XCTestExpectation * invoked = [self expectationWithDescription:@"invoke completed"];
    __block id flow;
    __block NSArray * values;
    __block NSError * error;
    [_device invokeCommandWithEndpointID:@(1)
                               clusterID:@(MTRClusterIDTypeOnOffID)
                               commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)
                           commandFields:@{ MTRTypeKey : MTRStructureValueType, MTRValueKey : @[] }
                          expectedValues:nil
                   expectedValueInterval:nil
                                   queue:dispatch_queue_create("flow-test-invoke", DISPATCH_QUEUE_SERIAL)
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable responseValues, NSError * _Nullable responseError) {
                                  values = responseValues;
                                  error = responseError;
                                  flow = CurrentFlowIDOrNull();
                                  [invoked fulfill];
                              }];
    [self waitForExpectations:@[ invoked ] timeout:kTimeoutInSeconds];
    *valuesOut = values;
    *errorOut = error;
    return flow;
}

- (void)test007_InvokeCompletionCarriesServerFlow
{
    NSUUID * sent = [NSUUID UUID];
    _server.replyFlowID = sent.UUIDString;
    NSArray * values;
    NSError * error;
    id flow = [self invokeToggleWithValues:&values error:&error];

    XCTAssertNil(error);
    XCTAssertEqualObjects(values, TestInvokeResponse(@(MTRCommandIDTypeClusterOnOffCommandToggleID)));
    XCTAssertEqualObjects(flow, sent);
}

- (void)test008_InvokeCommandsCompletionCarriesServerFlow
{
    NSUUID * sent = [NSUUID UUID];
    _server.replyFlowID = sent.UUIDString;
    __auto_type * command = [[MTRCommandWithRequiredResponse alloc] initWithPath:[MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)]
                                                                   commandFields:nil
                                                                requiredResponse:nil];
    XCTestExpectation * invoked = [self expectationWithDescription:@"invokeCommands completed"];
    __block id flow;
    __block NSArray * values;
    [_device invokeCommands:@[ @[ command ] ]
                      queue:dispatch_queue_create("flow-test-invoke-commands", DISPATCH_QUEUE_SERIAL)
                 completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable responseValues, NSError * _Nullable error) {
                     XCTAssertNil(error);
                     values = responseValues;
                     flow = CurrentFlowIDOrNull();
                     [invoked fulfill];
                 }];
    [self waitForExpectations:@[ invoked ] timeout:kTimeoutInSeconds];

    XCTAssertEqualObjects(values, TestInvokeResponse(@(0)));
    XCTAssertEqualObjects(flow, sent);
}

- (void)test009_InvokeErrorUserInfoFlowKey
{
    NSUUID * sent = [NSUUID UUID];
    _server.replyFlowID = sent.UUIDString;
    _server.replyError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil];
    NSArray * values;
    NSError * error;
    id flow = [self invokeToggleWithValues:&values error:&error];

    XCTAssertNil(values);
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeTimeout);
    XCTAssertNil(error.userInfo[MTRErrorFlowIDKey]);
    XCTAssertEqualObjects(flow, sent);
}

- (void)test010_DownloadLogErrorUserInfoFlowKey
{
    NSUUID * sent = [NSUUID UUID];
    _server.replyFlowID = sent.UUIDString;
    _server.replyError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeNotFound userInfo:nil];
    XCTestExpectation * downloaded = [self expectationWithDescription:@"download completed"];
    __block id flow;
    __block NSError * error;
    [_device downloadLogOfType:MTRDiagnosticLogTypeEndUserSupport
                       timeout:10
                         queue:dispatch_queue_create("flow-test-download", DISPATCH_QUEUE_SERIAL)
                    completion:^(NSURL * _Nullable url, NSError * _Nullable downloadError) {
                        XCTAssertNil(url);
                        error = downloadError;
                        flow = CurrentFlowIDOrNull();
                        [downloaded fulfill];
                    }];
    [self waitForExpectations:@[ downloaded ] timeout:kTimeoutInSeconds];

    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeNotFound);
    XCTAssertNil(error.userInfo[MTRErrorFlowIDKey]);
    XCTAssertEqualObjects(flow, sent);
}

@end

#pragma mark - Hops to the Matter queue

@interface MTRFlowMatterQueueTests : MTRFlowTestCase
@end

@implementation MTRFlowMatterQueueTests

// A flow that is current when work is scheduled on the Matter queue is current when it runs.
- (void)test001_AsyncDispatchToMatterQueueCarriesTheCurrentFlow
{
    MTRDeviceController * controller = [self createControllerOnTestFabric];
    XCTAssertNotNil(controller);

    NSUUID * flowID = [NSUUID UUID];
    XCTestExpectation * carried = [self expectationWithDescription:@"block ran in the caller's flow"];
    __block id carriedFlow;
    [MTRMetrics _runWithFlowID:flowID
                         block:^{
                             [controller asyncDispatchToMatterQueue:^{
                                 carriedFlow = CurrentFlowIDOrNull();
                                 [carried fulfill];
                             }
                                                       errorHandler:nil];
                         }];

    XCTestExpectation * plain = [self expectationWithDescription:@"block ran with no flow"];
    __block id plainFlow;
    [controller asyncDispatchToMatterQueue:^{
        plainFlow = CurrentFlowIDOrNull();
        [plain fulfill];
    }
                              errorHandler:nil];

    [self waitForExpectations:@[ carried, plain ] timeout:kTimeoutInSeconds];
    XCTAssertEqualObjects(carriedFlow, flowID);
    XCTAssertEqualObjects(plainFlow, NSNull.null, @"a block scheduled with no current flow must not get one");
}

@end

#pragma mark - Flows that survive copies and hops

@interface MTRFlowCopyTests : MTRFlowTestCase
@end

@implementation MTRFlowCopyTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _controller = [self createControllerOnTestFabric];
    _device = [MTRDevice deviceWithNodeID:@(kFlowTestNodeID + 0x100) controller:_controller];
}

- (void)tearDown
{
    [_controller removeDevice:_device];
    _device = nil;
    _controller = nil;
    [super tearDown];
}

// A report whose array is copied into a smaller array for a delegate that has interested paths.
- (void)test001_FlowSurvivesAFilteredArrayCopy
{
    __auto_type * everything = [[MTRFlowRecordingDelegate alloc] init];
    __auto_type * filtered = [[MTRFlowRecordingDelegate alloc] init];
    [_device addDelegate:everything queue:dispatch_queue_create("flow-test-copy-everything", DISPATCH_QUEUE_SERIAL)];
    // Only the Level Control cluster; the report below also has an On/Off attribute.
    [_device addDelegate:filtered
                               queue:dispatch_queue_create("flow-test-copy-filtered", DISPATCH_QUEUE_SERIAL)
        interestedPathsForAttributes:@[ [MTRClusterPath clusterPathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID)] ]
            interestedPathsForEvents:nil];

    NSUUID * flowID = [NSUUID UUID];
    NSArray * report = [TestAttributeReport(@(MTRClusterIDTypeOnOffID), @(MTRAttributeIDTypeClusterOnOffAttributeOnOffID), 1)
        arrayByAddingObjectsFromArray:TestConfigurationReport(22)];
    [MTRMetrics _runWithFlowID:flowID
                         block:^{
                             [_device unitTestInjectAttributeReport:report fromSubscription:YES];
                         }];
    [self waitForRecordsOf:everything named:kAttributeReport count:1];
    [self waitForRecordsOf:filtered named:kAttributeReport count:1];

    NSDictionary * everythingRecord = [everything recordsNamed:kAttributeReport].firstObject;
    NSDictionary * filteredRecord = [filtered recordsNamed:kAttributeReport].firstObject;
    XCTAssertEqual([everythingRecord[kRecordPayload] count], 2);
    XCTAssertEqual([filteredRecord[kRecordPayload] count], 1, @"the report was not copied into a smaller array");
    XCTAssertEqualObjects(everythingRecord[kRecordFlow], flowID);
    XCTAssertEqualObjects(filteredRecord[kRecordFlow], flowID);
    [_device removeDelegate:everything];
    [_device removeDelegate:filtered];
}

// Reports whose entries are copied into mutable dictionaries (an event gets its historical flag, a changed
// attribute gets its previous value).
- (void)test002_FlowSurvivesAMutableCopy
{
    __auto_type * delegate = [[MTRFlowRecordingDelegate alloc] init];
    [_device addDelegate:delegate queue:dispatch_queue_create("flow-test-mutable-copy", DISPATCH_QUEUE_SERIAL)];

    NSUUID * eventFlowID = [NSUUID UUID];
    NSArray * eventReport = TestEventReport();
    [MTRMetrics _runWithFlowID:eventFlowID
                         block:^{
                             [_device unitTestInjectEventReport:eventReport];
                         }];
    [self waitForRecordsOf:delegate named:kEventReport count:1];
    NSDictionary * eventRecord = [delegate recordsNamed:kEventReport].firstObject;
    XCTAssertTrue([eventRecord[kRecordPayload][0] isKindOfClass:NSMutableDictionary.class], @"the event entry was not copied");
    XCTAssertEqualObjects(eventRecord[kRecordFlow], eventFlowID);

    [_device unitTestInjectAttributeReport:TestLevelReport(31) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kAttributeReport count:1];
    NSUUID * changeFlowID = [NSUUID UUID];
    [MTRMetrics _runWithFlowID:changeFlowID
                         block:^{
                             [_device unitTestInjectAttributeReport:TestLevelReport(32) fromSubscription:YES];
                         }];
    [self waitForRecordsOf:delegate named:kAttributeReport count:2];
    NSDictionary * changeRecord = [delegate recordsNamed:kAttributeReport].lastObject;
    XCTAssertTrue([changeRecord[kRecordPayload][0] isKindOfClass:NSMutableDictionary.class], @"the attribute entry was not copied");
    XCTAssertNotNil(changeRecord[kRecordPayload][0][MTRPreviousDataKey]);
    XCTAssertEqualObjects(changeRecord[kRecordFlow], changeFlowID);
    [_device removeDelegate:delegate];
}

// A report with an error entry, whose NSError is a copy.
- (void)test003_FlowSurvivesAnErrorCopy
{
    __auto_type * delegate = [[MTRFlowRecordingDelegate alloc] init];
    [_device addDelegate:delegate queue:dispatch_queue_create("flow-test-error-copy", DISPATCH_QUEUE_SERIAL)];

    // An error made by the framework, so it carries the framework's own underlying-error holder.
    NSError * original;
    XCTAssertNil([MTRSetupPayload setupPayloadWithOnboardingPayload:@"not a setup payload" error:&original]);
    XCTAssertNotNil(original);
    NSError * copied = [original copy];
    NSError * rebuilt = [NSError errorWithDomain:original.domain code:original.code userInfo:original.userInfo];
    NSData * archive = [NSKeyedArchiver archivedDataWithRootObject:original requiringSecureCoding:YES error:nil];
    NSError * unarchived = [NSKeyedUnarchiver unarchivedObjectOfClass:NSError.class fromData:archive error:nil];
    XCTAssertNotNil(unarchived);
    NSArray * report = @[
        @{
            MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID)],
            MTRErrorKey : copied,
        },
        @{
            MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeGlobalAttributeFeatureMapID)],
            MTRErrorKey : rebuilt,
        },
        @{
            MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID)],
            MTRErrorKey : unarchived,
        },
    ];
    NSUUID * flowID = [NSUUID UUID];
    [MTRMetrics _runWithFlowID:flowID
                         block:^{
                             [_device unitTestInjectAttributeReport:report fromSubscription:YES];
                         }];
    [self waitForRecordsOf:delegate named:kAttributeReport count:1];

    NSDictionary * record = [delegate recordsNamed:kAttributeReport].firstObject;
    XCTAssertEqual([record[kRecordPayload] count], 3);
    for (NSDictionary * entry in record[kRecordPayload]) {
        NSError * delivered = entry[MTRErrorKey];
        XCTAssertEqual(delivered.code, original.code);
    }
    XCTAssertEqualObjects(record[kRecordFlow], flowID);
    [_device removeDelegate:delegate];
}

@end

#pragma mark - Subscription reset and failure

@interface MTRFlowSubscriptionResetTests : MTRFlowTestCase
@end

@implementation MTRFlowSubscriptionResetTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
    MTRFlowInternalStateRecordingDelegate * _delegate;
    dispatch_queue_t _delegateQueue;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _controller = [self createControllerOnTestFabric];
    _device = [MTRDevice deviceWithNodeID:@(kFlowTestNodeID + 0x200) controller:_controller];
    _delegate = [[MTRFlowInternalStateRecordingDelegate alloc] init];
    _delegateQueue = dispatch_queue_create("flow-test-reset-delegate", DISPATCH_QUEUE_SERIAL);
    [_device addDelegate:_delegate queue:_delegateQueue];
}

- (void)tearDown
{
    [_device removeDelegate:_delegate];
    [_controller removeDevice:_device];
    _device = nil;
    _controller = nil;
    [super tearDown];
}

// Makes the device reachable, so that a reset reports a state change.  Returns once every callback caused
// by that has been delivered.
- (void)makeDeviceReachable
{
    MTRDevice * device = _device;
    XCTestExpectation * ran = [self expectationWithDescription:@"ran on Matter queue"];
    [_controller asyncDispatchToMatterQueue:^{
        [device _handleUnsolicitedMessageFromPublisher];
        [ran fulfill];
    }
                               errorHandler:nil];
    [self waitForExpectations:@[ ran ] timeout:kTimeoutInSeconds];
    dispatch_sync(_delegateQueue, ^ {});
    XCTAssertEqual([_delegate recordsNamed:kStateChanged].count, 1);
}

// The next `count` callbacks recorded after the first `from`.
- (NSArray<NSDictionary *> *)recordsSince:(NSUInteger)from count:(NSUInteger)count
{
    __auto_type * delegate = _delegate;
    XCTestExpectation * arrived = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return delegate.records.count >= from + count;
    }]
                                            evaluatedWithObject:_delegate
                                                        handler:nil];
    [self waitForExpectations:@[ arrived ] timeout:kTimeoutInSeconds];
    return [delegate.records subarrayWithRange:NSMakeRange(from, count)];
}

- (void)checkAllShareOneFlow:(NSArray<NSDictionary *> *)records
{
    NSUUID * flow = records.firstObject[kRecordFlow];
    XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"%@", flow);
    for (NSDictionary * record in records) {
        XCTAssertEqualObjects(record[kRecordFlow], flow, @"%@", records);
    }
}

// _resetSubscriptionWithReasonString: and the hop to the Matter queue.  The reset reports the state change
// to unreachable, the internal state update that goes with it, and the internal state update at the end of
// the reset (the new retry wait): all three are one flow.
- (void)test001_ResetWithReasonSharesOneFlowAcrossTheHop
{
    [self makeDeviceReachable];
    NSUInteger from = _delegate.records.count;
    [_device unitTestResetSubscription];
    NSArray * records = [self recordsSince:from count:3];
    XCTAssertEqualObjects(records.firstObject[kRecordName], kStateChanged, @"%@", records);
    XCTAssertEqualObjects(records.firstObject[kRecordPayload], @(MTRDeviceStateUnreachable));
    [self checkAllShareOneFlow:records];
}

// _resetSubscription, called on the Matter queue by _deviceMayBeReachable: the state change to unreachable and
// its internal state update are one flow.
- (void)test002_DirectResetSharesOneFlow
{
    [self makeDeviceReachable];
    NSUInteger from = _delegate.records.count;
    [_device _deviceMayBeReachable];
    NSArray * records = [self recordsSince:from count:2];
    XCTAssertEqualObjects(records.firstObject[kRecordName], kStateChanged, @"%@", records);
    XCTAssertEqualObjects(records.firstObject[kRecordPayload], @(MTRDeviceStateUnreachable));
    [self checkAllShareOneFlow:records];
}

@end

@interface MTRFlowSubscriptionFailureTests : MTRFlowTestCase
@end

@implementation MTRFlowSubscriptionFailureTests

// The session-failure block: the subscription's CASE attempt failed and reported it with no flow around it
// (as it does when the controller is suspended or stopping).  The state change and the internal state updates
// caused by the failure are one flow, though they run after a hop to the Matter queue.
- (void)test001_SessionFailureSharesOneFlowAcrossTheHop
{
    [self setContinueAfterFailure:NO];
    MTRDeviceController * controller = [self createControllerOnTestFabric];
    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kFlowTestNodeID + 0x300) controller:controller];
    __auto_type * delegate = [[MTRFlowInternalStateRecordingDelegate alloc] init];
    dispatch_queue_t delegateQueue = dispatch_queue_create("flow-test-failure-delegate", DISPATCH_QUEUE_SERIAL);
    [device addDelegate:delegate queue:delegateQueue];

    XCTestExpectation * ran = [self expectationWithDescription:@"ran on Matter queue"];
    [controller asyncDispatchToMatterQueue:^{
        [device _handleUnsolicitedMessageFromPublisher];
        [ran fulfill];
    }
                              errorHandler:nil];
    [self waitForExpectations:@[ ran ] timeout:kTimeoutInSeconds];
    dispatch_sync(delegateQueue, ^ {});
    NSUInteger from = delegate.records.count;

    XCTAssertNil(MTRMetrics.currentFlowID);
    [device _handleSessionFailureWithError:[NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil] retryDelay:nil];

    XCTestExpectation * arrived = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return delegate.records.count >= from + 3;
    }]
                                            evaluatedWithObject:delegate
                                                        handler:nil];
    [self waitForExpectations:@[ arrived ] timeout:kTimeoutInSeconds];
    NSArray * records = [delegate.records subarrayWithRange:NSMakeRange(from, 3)];
    XCTAssertEqualObjects(records.firstObject[kRecordName], kStateChanged, @"%@", records);
    XCTAssertEqualObjects(records.firstObject[kRecordPayload], @(MTRDeviceStateUnreachable));
    NSUUID * flow = records.firstObject[kRecordFlow];
    XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"%@", flow);
    for (NSDictionary * record in records) {
        XCTAssertEqualObjects(record[kRecordFlow], flow, @"%@", records);
    }

    [device removeDelegate:delegate];
    [controller removeDevice:device];
}

@end

#pragma mark - MTRBaseDevice and the CASE result, on a running accessory

static const uint64_t kFlowWireNodeID = 0x7A3C4001;
static const NSTimeInterval kFlowWireTimeoutInSeconds = 30;
static MTRDeviceController * sFlowWireController;

// What an MTRBaseDevice report handler or established handler saw.
@interface MTRFlowReportRecorder : NSObject
- (void)recordValues:(NSArray * _Nullable)values error:(NSError * _Nullable)error;
- (void)recordEstablished;
- (NSArray<NSDictionary *> *)calls;
@end

@implementation MTRFlowReportRecorder {
    os_unfair_lock _lock;
    NSMutableArray<NSDictionary *> * _calls;
}

- (instancetype)init
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _calls = [NSMutableArray array];
    }
    return self;
}

- (void)_add:(NSDictionary *)call
{
    os_unfair_lock_lock(&_lock);
    [_calls addObject:call];
    os_unfair_lock_unlock(&_lock);
}

- (void)recordValues:(NSArray * _Nullable)values error:(NSError * _Nullable)error
{
    [self _add:@{ kRecordName : @"report", kRecordFlow : CurrentFlowIDOrNull(), kRecordPayload : values ?: @[] }];
}

- (void)recordEstablished
{
    [self _add:@{ kRecordName : @"established", kRecordFlow : CurrentFlowIDOrNull() }];
}

- (NSArray<NSDictionary *> *)calls
{
    os_unfair_lock_lock(&_lock);
    NSArray * calls = [_calls copy];
    os_unfair_lock_unlock(&_lock);
    return calls;
}

@end

#pragma mark - Report decoding

static BOOL OnMatterQueue(void)
{
    return strcmp(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL), "org.csa-iot.matter.workqueue") == 0;
}

// Records the flow that is current on the Matter queue each time the replaced method runs there.
@interface MTRFlowMatterQueueProbe : NSObject
- (instancetype)initWithMethod:(Method)method block:(id)block;
- (void)note;
- (NSArray *)flows;
- (void)remove;
@end

@implementation MTRFlowMatterQueueProbe {
    os_unfair_lock _lock;
    NSMutableArray * _flows;
    Method _method;
    IMP _original;
}

- (instancetype)initWithMethod:(Method)method block:(id)block
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _flows = [NSMutableArray array];
        _method = method;
        _original = method_getImplementation(method);
        method_setImplementation(method, imp_implementationWithBlock(block));
    }
    return self;
}

- (void)note
{
    if (!OnMatterQueue()) {
        return;
    }
    id flow = CurrentFlowIDOrNull();
    os_unfair_lock_lock(&_lock);
    [_flows addObject:flow];
    os_unfair_lock_unlock(&_lock);
}

- (NSArray *)flows
{
    os_unfair_lock_lock(&_lock);
    NSArray * flows = [_flows copy];
    os_unfair_lock_unlock(&_lock);
    return flows;
}

- (void)remove
{
    // The replacement block is not freed: a Matter-queue call may still be running it.
    if (_method != NULL) {
        method_setImplementation(_method, _original);
        _method = NULL;
    }
}

@end

@interface MTRFlowSubscribingDelegate : MTRFlowRecordingDelegate
@end

@implementation MTRFlowSubscribingDelegate
- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device
{
    return YES;
}
@end

@interface MTRFlowWireTests : MTRFlowTestCase
@end

@implementation MTRFlowWireTests

+ (void)setUp
{
    [super setUp];
    sFlowWireController = [self startCommissionedAppWithName:@"all-clusters" arguments:@[] nodeID:@(kFlowWireNodeID)];
    XCTAssertNotNil(sFlowWireController);
}

+ (void)tearDown
{
    sFlowWireController = nil;
    [super tearDown];
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (MTRFlowReportRecorder *)subscribeWithAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths eventPaths:(NSArray<MTREventRequestPath *> *)eventPaths
{
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:@(kFlowWireNodeID) controller:sFlowWireController];
    __auto_type * recorder = [[MTRFlowReportRecorder alloc] init];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;
    XCTestExpectation * established = [self expectationWithDescription:@"subscription established"];
    [baseDevice subscribeToAttributePaths:attributePaths
        eventPaths:eventPaths
        params:params
        queue:dispatch_queue_create("flow-test-wire-report", DISPATCH_QUEUE_SERIAL)
        reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
            [recorder recordValues:values error:error];
        }
        subscriptionEstablished:^{
            [recorder recordEstablished];
            [established fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ established ] timeout:kFlowWireTimeoutInSeconds];
    return recorder;
}

- (NSArray<NSDictionary *> *)primingCallsOf:(MTRFlowReportRecorder *)recorder
{
    NSMutableArray * priming = [NSMutableArray array];
    for (NSDictionary * call in recorder.calls) {
        if ([call[kRecordName] isEqualToString:@"established"]) {
            break;
        }
        [priming addObject:call];
    }
    return priming;
}

- (void)checkAllShareOneFlow:(NSArray<NSDictionary *> *)calls
{
    XCTAssertGreaterThan(calls.count, 0);
    id flow = calls.firstObject[kRecordFlow];
    XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"%@", flow);
    for (NSDictionary * call in calls) {
        XCTAssertEqualObjects(call[kRecordFlow], flow, @"%@", call[kRecordPayload]);
    }
}

// Item 1: every attribute of one report shares a flow, and the next report has another.
- (void)test001_MultipleAttributesOfOneReportShareOneFlow
{
    __auto_type * recorder = [self subscribeWithAttributePaths:@[ [MTRAttributeRequestPath requestPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) attributeID:nil] ]
                                                    eventPaths:nil];
    NSArray * priming = [self primingCallsOf:recorder];
    XCTAssertGreaterThan(priming.count, 1, @"wanted a report with several attributes: %@", priming);
    NSDictionary * established = recorder.calls[priming.count];
    XCTAssertTrue([established[kRecordFlow] isKindOfClass:NSUUID.class], @"subscriptionEstablished ran in no flow: %@", established);
    [self checkAllShareOneFlow:priming];
    id primingFlow = [priming.firstObject objectForKey:kRecordFlow];

    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:@(kFlowWireNodeID) controller:sFlowWireController];
    __auto_type * onOff = [[MTRBaseClusterOnOff alloc] initWithDevice:baseDevice endpointID:@(1) queue:dispatch_get_main_queue()];
    XCTestExpectation * toggled = [self expectationWithDescription:@"toggled"];
    [onOff toggleWithCompletion:^(NSError * _Nullable error) {
        XCTAssertNil(error);
        [toggled fulfill];
    }];
    [self waitForExpectations:@[ toggled ] timeout:kFlowWireTimeoutInSeconds];

    XCTestExpectation * reported = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return recorder.calls.count > priming.count + 1;
    }]
                                             evaluatedWithObject:recorder
                                                         handler:nil];
    [self waitForExpectations:@[ reported ] timeout:kFlowWireTimeoutInSeconds];
    NSArray * allCalls = recorder.calls; // one snapshot: a report can arrive between two reads of recorder.calls
    NSArray * after = [allCalls subarrayWithRange:NSMakeRange(priming.count + 1, allCalls.count - priming.count - 1)];
    [self checkAllShareOneFlow:after];
    XCTAssertNotEqualObjects(after.firstObject[kRecordFlow], primingFlow);
}

// Item 1: an attribute error is reported in the flow of the report it came in.
- (void)test002_AttributeErrorSharesTheFlowOfItsReport
{
    __auto_type * recorder = [self subscribeWithAttributePaths:@[
        [MTRAttributeRequestPath requestPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnOffID)],
        [MTRAttributeRequestPath requestPathWithEndpointID:@(1) clusterID:@(5678) attributeID:@(1)],
    ]
                                                    eventPaths:nil];
    NSArray * priming = [self primingCallsOf:recorder];
    BOOL sawData = NO;
    BOOL sawError = NO;
    for (NSDictionary * call in priming) {
        for (NSDictionary * value in call[kRecordPayload]) {
            sawData = sawData || value[MTRDataKey] != nil;
            sawError = sawError || value[MTRErrorKey] != nil;
        }
    }
    XCTAssertTrue(sawData && sawError, @"%@", priming);
    [self checkAllShareOneFlow:priming];
}

// Item 1: an event error and an attribute of one report share a flow.
- (void)test003_EventErrorAndAttributeOfOneReportShareOneFlow
{
    __auto_type * recorder = [self subscribeWithAttributePaths:@[ [MTRAttributeRequestPath requestPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnOffID)] ]
                                                    eventPaths:@[ [MTREventRequestPath requestPathWithEndpointID:@(1000) clusterID:@(MTRClusterIDTypeBasicInformationID) eventID:@(MTREventIDTypeClusterBasicInformationEventStartUpID)] ]];
    NSArray * priming = [self primingCallsOf:recorder];
    BOOL sawAttribute = NO;
    BOOL sawEvent = NO;
    for (NSDictionary * call in priming) {
        for (NSDictionary * value in call[kRecordPayload]) {
            sawAttribute = sawAttribute || value[MTRAttributePathKey] != nil;
            sawEvent = sawEvent || value[MTREventPathKey] != nil;
        }
    }
    XCTAssertTrue(sawAttribute && sawEvent, @"%@", priming);
    [self checkAllShareOneFlow:priming];
}

// Item 3: the CASE result runs in a flow, though nothing around the request is in one.
- (void)test004_CASEResultRunsInAFlow
{
    XCTAssertNil(MTRMetrics.currentFlowID);
    XCTestExpectation * connected = [self expectationWithDescription:@"session result"];
    __block id flow;
    __block NSError * sessionError;
    [sFlowWireController unitTestGetSessionForNode:@(kFlowWireNodeID)
                                        completion:^(NSError * _Nullable error) {
                                            flow = CurrentFlowIDOrNull();
                                            sessionError = error;
                                            [connected fulfill];
                                        }];
    [self waitForExpectations:@[ connected ] timeout:kFlowWireTimeoutInSeconds];
    XCTAssertNil(sessionError);
    XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"%@", flow);
}

// Item 3: so does a failed CASE attempt (no one answers for this node).
- (void)test005_CASEFailureRunsInAFlow
{
    XCTestExpectation * failed = [self expectationWithDescription:@"session failure"];
    __block id flow;
    __block NSError * sessionError;
    [sFlowWireController unitTestGetSessionForNode:@(kFlowWireNodeID + 1)
                                        completion:^(NSError * _Nullable error) {
                                            flow = CurrentFlowIDOrNull();
                                            sessionError = error;
                                            [failed fulfill];
                                        }];
    [self waitForExpectations:@[ failed ] timeout:120];
    XCTAssertNotNil(sessionError);
    XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"%@", flow);
}

// Item 4: the events of an MTRDevice's priming report are decoded in a flow.
- (void)test006_MTRDeviceEventDataIsDecodedInAFlow
{
    Method method = class_getClassMethod(MTRBaseDevice.class, NSSelectorFromString(@"eventReportForHeader:andData:"));
    XCTAssertTrue(method != NULL);
    IMP original = method_getImplementation(method);
    __block __weak MTRFlowMatterQueueProbe * weakProbe;
    __auto_type * probe = [[MTRFlowMatterQueueProbe alloc] initWithMethod:method block:^NSDictionary *(id self, const void * header, id data) {
        [weakProbe note];
        return ((NSDictionary * (*) (id, SEL, const void *, id)) original)(self, NSSelectorFromString(@"eventReportForHeader:andData:"), header, data);
    }];
    weakProbe = probe;
    [self addTeardownBlock:^{
        [probe remove];
    }];

    __auto_type * device = [MTRDevice deviceWithNodeID:@(kFlowWireNodeID) controller:sFlowWireController];
    __auto_type * delegate = [[MTRFlowSubscribingDelegate alloc] init];
    [device addDelegate:delegate queue:dispatch_queue_create("flow-test-decode-delegate", DISPATCH_QUEUE_SERIAL)];
    XCTestExpectation * primed = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return [delegate recordsNamed:kCachePrimed].count >= 1;
    }]
                                           evaluatedWithObject:delegate
                                                       handler:nil];
    [self waitForExpectations:@[ primed ] timeout:kFlowWireTimeoutInSeconds];
    [probe remove];
    [sFlowWireController removeDevice:device];

    NSArray * flows = probe.flows;
    XCTAssertGreaterThan(flows.count, 0, @"no event data was decoded on the Matter queue");
    for (id flow in flows) {
        XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"event data decoded with no flow");
    }
}

// Item 4: so are the attributes of a subscription made with an MTRBaseDevice.
- (void)test007_MTRBaseDeviceSubscriptionAttributeDataIsDecodedInAFlow
{
    Method method = class_getInstanceMethod(MTRAttributeReport.class, NSSelectorFromString(@"initWithPath:value:error:"));
    XCTAssertTrue(method != NULL);
    IMP original = method_getImplementation(method);
    __block __weak MTRFlowMatterQueueProbe * weakProbe;
    __auto_type * probe = [[MTRFlowMatterQueueProbe alloc] initWithMethod:method block:^id(id self, const void * path, id value, id error) {
        [weakProbe note];
        return ((id(*)(id, SEL, const void *, id, id)) original)(self, NSSelectorFromString(@"initWithPath:value:error:"), path, value, error);
    }];
    weakProbe = probe;
    [self addTeardownBlock:^{
        [probe remove];
    }];

    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:@(kFlowWireNodeID) controller:sFlowWireController];
    XCTestExpectation * reported = [self expectationWithDescription:@"first attribute report"];
    XCTestExpectation * established = [self expectationWithDescription:@"subscription established"];
    __block BOOL sawReport = NO;
    [baseDevice subscribeWithQueue:dispatch_queue_create("flow-test-decode-subscription", DISPATCH_QUEUE_SERIAL)
        params:[[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(10)]
        clusterStateCacheContainer:nil
        attributeReportHandler:^(NSArray * value) {
            if (!sawReport) {
                sawReport = YES;
                [reported fulfill];
            }
        }
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
        }
        subscriptionEstablished:^{
            [established fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ reported, established ] timeout:kFlowWireTimeoutInSeconds];
    [probe remove];

    NSArray * flows = probe.flows;
    XCTAssertGreaterThan(flows.count, 0, @"no attribute data was decoded on the Matter queue");
    for (id flow in flows) {
        XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"attribute data decoded with no flow");
    }
}

// Item 1, MTRDevice: every delegate callback caused by the priming report of an MTRDevice subscription (its
// attributes, events, configuration change and cache-primed) runs in the one flow of that report, across the hops
// from the Matter queue to the device queue.
- (void)test008_MTRDeviceSubscriptionReportCallbacksShareOneFlow
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kFlowWireNodeID) controller:sFlowWireController];
    __auto_type * delegate = [[MTRFlowSubscribingDelegate alloc] init];
    [device addDelegate:delegate queue:dispatch_queue_create("flow-test-device-report-delegate", DISPATCH_QUEUE_SERIAL)];
    XCTestExpectation * primed = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return [delegate recordsNamed:kCachePrimed].count >= 1;
    }]
                                           evaluatedWithObject:delegate
                                                       handler:nil];
    [self waitForExpectations:@[ primed ] timeout:kFlowWireTimeoutInSeconds];
    [sFlowWireController removeDevice:device];

    // The callbacks of the priming report: the attribute, event and configuration reports up to and including cache-primed.
    NSMutableArray<NSDictionary *> * report = [NSMutableArray array];
    NSUInteger attributeCount = 0;
    NSUInteger eventCount = 0;
    for (NSDictionary * record in delegate.records) {
        NSString * name = record[kRecordName];
        if ([name isEqualToString:kAttributeReport]) {
            attributeCount += [record[kRecordPayload] count];
        } else if ([name isEqualToString:kEventReport]) {
            eventCount += [record[kRecordPayload] count];
        } else if (![name isEqualToString:kConfigurationChanged] && ![name isEqualToString:kCachePrimed]) {
            continue;
        }
        [report addObject:record];
        if ([name isEqualToString:kCachePrimed]) {
            break;
        }
    }
    XCTAssertGreaterThan(attributeCount, 1, @"wanted a priming report with several attributes: %@", report);
    XCTAssertGreaterThan(eventCount, 0, @"wanted a priming report with an event: %@", report);
    XCTAssertGreaterThan(report.count, 2, @"%@", report);
    id flow = report.firstObject[kRecordFlow];
    XCTAssertTrue([flow isKindOfClass:NSUUID.class], @"%@", flow);
    for (NSDictionary * record in report) {
        XCTAssertEqualObjects(record[kRecordFlow], flow, @"%@", record[kRecordName]);
    }
}

@end

#pragma mark - OTA provider completions

static const uint16_t kFlowOTATimeoutInSeconds = 60;
static os_unfair_lock sFlowProbeLock = OS_UNFAIR_LOCK_INIT;
static NSMutableDictionary<NSString *, NSMutableArray *> * sFlowProbes;

// The framework reads each response object on the Matter queue, inside the completion the application called
// later from another queue, so the flow current at that read is the flow the completion runs in.
static void NoteResponseFlow(NSString * name)
{
    if (strcmp(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL), "org.csa-iot.matter.workqueue") != 0) {
        return;
    }
    id flow = CurrentFlowIDOrNull();
    os_unfair_lock_lock(&sFlowProbeLock);
    NSMutableArray * flows = sFlowProbes[name];
    if (flows == nil) {
        flows = [NSMutableArray array];
        sFlowProbes[name] = flows;
    }
    [flows addObject:flow];
    os_unfair_lock_unlock(&sFlowProbeLock);
}

static NSArray * ResponseFlows(NSString * name)
{
    os_unfair_lock_lock(&sFlowProbeLock);
    NSArray * flows = [sFlowProbes[name] copy];
    os_unfair_lock_unlock(&sFlowProbeLock);
    return flows ?: @[];
}

@interface MTRFlowProbeQueryImageResponse : MTROTASoftwareUpdateProviderClusterQueryImageResponseParams
@end
@implementation MTRFlowProbeQueryImageResponse
- (NSNumber *)status
{
    NoteResponseFlow(@"QueryImage");
    return [super status];
}
@end

@interface MTRFlowProbeApplyUpdateResponse : MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams
@end
@implementation MTRFlowProbeApplyUpdateResponse
- (NSNumber *)action
{
    NoteResponseFlow(@"ApplyUpdateRequest");
    return [super action];
}
@end

@interface MTRFlowProbeNotifyError : NSError
@end
@implementation MTRFlowProbeNotifyError
- (NSString *)description
{
    NoteResponseFlow(@"NotifyUpdateApplied");
    return [super description];
}
@end

// Every response is sent from another queue, later, where no flow is current.
@interface MTRFlowOTAProviderDelegate : NSObject <MTROTAProviderDelegate>
@property (atomic, nullable) id queryImageFlow;
@property (atomic, nullable) id applyUpdateFlow;
@property (atomic, nullable) id notifyUpdateAppliedFlow;
@end

@implementation MTRFlowOTAProviderDelegate

- (void)_later:(dispatch_block_t)block
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC), dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), block);
}

- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params
                       completion:(void (^)(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error))completion
{
    self.queryImageFlow = CurrentFlowIDOrNull();
    [self _later:^{
        __auto_type * response = [[MTRFlowProbeQueryImageResponse alloc] init];
        response.status = @(MTROTASoftwareUpdateProviderStatusNotAvailable);
        completion(response, nil);
    }];
}

- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                               completion:(void (^)(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error))completion
{
    self.applyUpdateFlow = CurrentFlowIDOrNull();
    [self _later:^{
        __auto_type * response = [[MTRFlowProbeApplyUpdateResponse alloc] init];
        response.action = @(MTROTASoftwareUpdateProviderApplyUpdateActionDiscontinue);
        response.delayedActionTime = @(0);
        completion(response, nil);
    }];
}

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                                completion:(MTRStatusCompletion)completion
{
    self.notifyUpdateAppliedFlow = CurrentFlowIDOrNull();
    [self _later:^{
        completion([[MTRFlowProbeNotifyError alloc] initWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }];
}

- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber *)nodeID
                                    controller:(MTRDeviceController *)controller
                                fileDesignator:(NSString *)fileDesignator
                                        offset:(NSNumber *)offset
                                    completion:(MTRStatusCompletion)completion
{
    completion([NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
}

- (void)handleBDXQueryForNodeID:(NSNumber *)nodeID
                     controller:(MTRDeviceController *)controller
                      blockSize:(NSNumber *)blockSize
                     blockIndex:(NSNumber *)blockIndex
                    bytesToSkip:(NSNumber *)bytesToSkip
                     completion:(void (^)(NSData * _Nullable data, BOOL isEOF))completion
{
    completion(nil, YES);
}

@end

@interface MTRFlowOTAProviderTests : MTRTestCase
@end

@implementation MTRFlowOTAProviderTests {
    dispatch_queue_t _storageQueue;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    [MTRDeviceControllerFactory.sharedInstance stopControllerFactory];
    _storageQueue = dispatch_queue_create("flow-test-ota-storage", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
}

- (void)tearDown
{
    MTRSetLogCallback(0, nil);
    [MTRDeviceControllerFactory.sharedInstance stopControllerFactory];
    [super tearDown];
}

- (MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys nodeID:(NSNumber *)nodeID otaProviderDelegate:(id<MTROTAProviderDelegate> _Nullable)otaProviderDelegate
{
    NSError * error;
    MTRCertificateDERBytes root = [MTRCertificates createRootCertificate:rootKeys issuerID:@(1) fabricID:nil error:&error];
    XCTAssertNotNil(root, @"%@", error);
    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    SecKeyRef publicKey = [operationalKeys copyPublicKey];
    MTRCertificateDERBytes operational = [MTRCertificates createOperationalCertificate:rootKeys
                                                                    signingCertificate:root
                                                                  operationalPublicKey:publicKey
                                                                              fabricID:@(0x7A3C)
                                                                                nodeID:nodeID
                                                                 caseAuthenticatedTags:nil
                                                                                 error:&error];
    CFRelease(publicKey);
    XCTAssertNotNil(operational, @"%@", error);

    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    __auto_type * params = [[MTRDeviceControllerExternalCertificateParameters alloc] initWithStorageDelegate:storage
                                                                                        storageDelegateQueue:_storageQueue
                                                                                            uniqueIdentifier:storage.controllerID
                                                                                                         ipk:rootKeys.ipk
                                                                                                    vendorID:@(0xFFF1)
                                                                                          operationalKeypair:operationalKeys
                                                                                      operationalCertificate:operational
                                                                                     intermediateCertificate:nil
                                                                                             rootCertificate:root];
    params.shouldAdvertiseOperational = YES;
    if (otaProviderDelegate) {
        [params setOTAProviderDelegate:otaProviderDelegate queue:dispatch_queue_create("flow-test-ota-provider", DISPATCH_QUEUE_SERIAL)];
    }
    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithParameters:params error:&error];
    XCTAssertNotNil(controller, @"%@", error);
    return controller;
}

// Item 1: each request's delegate callback and its completion run in one flow of their own, though the
// application calls the completion later, on another queue.
- (void)test001_ProviderResponsesRunInTheirRequestFlow
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    __auto_type * providerDelegate = [[MTRFlowOTAProviderDelegate alloc] init];
    // Both controllers share one fabric; commands for it reach the first one started, the provider.
    MTRDeviceController * provider = [self startControllerWithRootKeys:rootKeys nodeID:@(0x7A3C2001) otaProviderDelegate:providerDelegate];
    MTRDeviceController * requestor = [self startControllerWithRootKeys:rootKeys nodeID:@(0x7A3C2002) otaProviderDelegate:nil];

    __auto_type * cluster = [[MTRBaseClusterOTASoftwareUpdateProvider alloc] initWithDevice:[MTRBaseDevice deviceWithNodeID:provider.controllerNodeID controller:requestor]
                                                                                 endpointID:@(0)
                                                                                      queue:dispatch_get_main_queue()];
    NSData * updateToken = [@"flow-test-token" dataUsingEncoding:NSUTF8StringEncoding];
    os_unfair_lock_lock(&sFlowProbeLock);
    sFlowProbes = [NSMutableDictionary dictionary];
    os_unfair_lock_unlock(&sFlowProbeLock);

    XCTestExpectation * queried = [self expectationWithDescription:@"QueryImage response"];
    __auto_type * queryParams = [[MTROTASoftwareUpdateProviderClusterQueryImageParams alloc] init];
    queryParams.vendorID = @(0xFFF1);
    queryParams.productID = @(0x8001);
    queryParams.softwareVersion = @(1);
    queryParams.protocolsSupported = @[ @(MTROTASoftwareUpdateProviderDownloadProtocolBDXSynchronous) ];
    [cluster queryImageWithParams:queryParams
                       completion:^(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error) {
                           XCTAssertNil(error);
                           XCTAssertEqualObjects(data.status, @(MTROTASoftwareUpdateProviderStatusNotAvailable));
                           [queried fulfill];
                       }];
    [self waitForExpectations:@[ queried ] timeout:kFlowOTATimeoutInSeconds];

    XCTestExpectation * applied = [self expectationWithDescription:@"ApplyUpdateRequest response"];
    __auto_type * applyParams = [[MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];
    applyParams.updateToken = updateToken;
    applyParams.newVersion = @(2);
    [cluster applyUpdateRequestWithParams:applyParams
                               completion:^(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error) {
                                   XCTAssertNil(error);
                                   [applied fulfill];
                               }];
    [self waitForExpectations:@[ applied ] timeout:kFlowOTATimeoutInSeconds];

    XCTestExpectation * notified = [self expectationWithDescription:@"NotifyUpdateApplied response"];
    __auto_type * notifyParams = [[MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];
    notifyParams.updateToken = updateToken;
    notifyParams.softwareVersion = @(2);
    [cluster notifyUpdateAppliedWithParams:notifyParams
                                completion:^(NSError * _Nullable error) {
                                    XCTAssertNotNil(error);
                                    [notified fulfill];
                                }];
    [self waitForExpectations:@[ notified ] timeout:kFlowOTATimeoutInSeconds];

    NSDictionary * checks = @{
        @"QueryImage" : providerDelegate.queryImageFlow ?: NSNull.null,
        @"ApplyUpdateRequest" : providerDelegate.applyUpdateFlow ?: NSNull.null,
        @"NotifyUpdateApplied" : providerDelegate.notifyUpdateAppliedFlow ?: NSNull.null,
    };
    for (NSString * request in checks) {
        id requestFlow = checks[request];
        XCTAssertTrue([requestFlow isKindOfClass:NSUUID.class], @"delegate callback for %@ ran in no flow", request);
        NSArray * flows = ResponseFlows(request);
        XCTAssertGreaterThan(flows.count, 0, @"the %@ completion did not run on the Matter queue", request);
        for (id flow in flows) {
            XCTAssertEqualObjects(flow, requestFlow, @"the %@ completion ran in flow %@, request flow %@", request, flow, requestFlow);
        }
    }
    NSSet * requestFlows = [NSSet setWithArray:[checks allValues]];
    XCTAssertEqual(requestFlows.count, 3, @"each request has a flow of its own: %@", checks);

    [requestor shutdown];
    [provider shutdown];
}

@end

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
#import <os/lock.h>

#import "MTRDelegateManager.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDevice_Internal.h"
#import "MTRMetricsScope_Internal.h"
#import "MTRMetrics_Internal.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"

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
@end

@interface MTRDeviceController (FlowTest)
- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler;
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

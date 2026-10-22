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

#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDevice_Internal.h"
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"

static const uint16_t kTimeoutInSeconds = 3;
static const uint16_t kSubscriptionTimeoutInSeconds = 60;
static const uint64_t kLocalNodeID = 0x7A3C1001;
static const uint64_t kWireNodeID = 0x7A3C1002;
static const uint64_t kTracingServerControllerNodeID = 0x7A3C1003;

static NSString * const kRecordName = @"name";
static NSString * const kRecordTrace = @"trace";
static NSString * const kRecordPayload = @"payload";
static NSString * const kRecordMetricData = @"metricData";

static NSString * const kStateChanged = @"stateChanged";
static NSString * const kAttributeReport = @"receivedAttributeReport";
static NSString * const kEventReport = @"receivedEventReport";
static NSString * const kBecameActive = @"deviceBecameActive";
static NSString * const kCachePrimed = @"deviceCachePrimed";
static NSString * const kConfigurationChanged = @"deviceConfigurationChanged";
static NSString * const kCollectedMetrics = @"collectedMetrics";
static NSString * const kInternalStateUpdated = @"internalStateUpdated";

@interface MTRDevice (TraceContextTest)
- (void)_handleUnsolicitedMessageFromPublisher;
@end

@interface MTRDeviceController (TraceContextTest)
- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler;
@property (atomic, readonly) BOOL serverSupportsTraceIDs;
@end

static id CurrentTraceIDOrNull(void)
{
    NSUUID * traceID = MTRTraceContext.currentContext.traceID;
    return traceID ?: NSNull.null;
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

@interface MTRTraceOldFormatMetricData : NSObject <NSSecureCoding>
@end

@implementation MTRTraceOldFormatMetricData
+ (BOOL)supportsSecureCoding
{
    return YES;
}
- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    return [self init];
}
- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:@(5) forKey:@"v"];
    [coder encodeObject:nil forKey:@"e"];
    [coder encodeObject:@(0.5) forKey:@"d"];
}
@end

#pragma mark - Delegates

@interface MTRTraceRecordingDelegate : NSObject <MTRDeviceDelegate>
@property (atomic) BOOL subscribe;
- (void)_record:(NSString *)name payload:(id _Nullable)payload;
- (NSArray<NSDictionary *> *)records;
- (NSArray<NSDictionary *> *)recordsNamed:(NSString *)name;
@end

@implementation MTRTraceRecordingDelegate {
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
    NSMutableDictionary * record = [@{ kRecordName : name, kRecordTrace : CurrentTraceIDOrNull(), kRecordMetricData : [MTRMetricData new] } mutableCopy];
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

- (void)device:(MTRDevice *)device collectedMetrics:(MTRMetrics *)metrics
{
    [self _record:kCollectedMetrics payload:metrics];
}

- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device
{
    return self.subscribe;
}

- (BOOL)unitTestSuppressTimeBasedReachabilityChanges:(MTRDevice *)device
{
    return YES;
}

@end

@interface MTRTraceInternalStateRecordingDelegate : MTRTraceRecordingDelegate
@end

@implementation MTRTraceInternalStateRecordingDelegate

- (void)device:(MTRDevice *)device internalStateUpdated:(NSDictionary *)dictionary
{
    [self _record:kInternalStateUpdated payload:dictionary];
}

@end

@interface MTRTraceOriginalMethodsDelegate : NSObject <MTRDeviceDelegate>
@property (atomic, copy) NSArray<NSString *> * calls;
@property (atomic, copy, nullable) NSArray * lastAttributeReport;
@property (atomic, copy, nullable) NSArray * lastEventReport;
@end

@implementation MTRTraceOriginalMethodsDelegate

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

@interface MTRTraceContextTestCase : MTRTestCase
- (void)waitForRecordsOf:(MTRTraceRecordingDelegate *)delegate named:(NSString *)name count:(NSUInteger)count timeout:(NSTimeInterval)timeout;
- (void)assertRecordsHaveOneSharedTrace:(NSArray<NSDictionary *> *)records;
- (void)startCapturingLogs;
- (NSArray<NSString *> *)capturedLogs;
- (void)stopCapturingLogs;
@end

static os_unfair_lock sLogLock = OS_UNFAIR_LOCK_INIT;
static NSMutableArray<NSString *> * sLogs;

@implementation MTRTraceContextTestCase

- (void)waitForRecordsOf:(MTRTraceRecordingDelegate *)delegate named:(NSString *)name count:(NSUInteger)count timeout:(NSTimeInterval)timeout
{
    NSPredicate * predicate = [NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return [delegate recordsNamed:name].count >= count;
    }];
    XCTestExpectation * expectation = [self expectationForPredicate:predicate evaluatedWithObject:delegate handler:nil];
    expectation.expectationDescription = [NSString stringWithFormat:@"%lu %@ callback(s)", (unsigned long) count, name];
    [self waitForExpectations:@[ expectation ] timeout:timeout];
}

- (void)assertRecordsHaveOneSharedTrace:(NSArray<NSDictionary *> *)records
{
    XCTAssertGreaterThan(records.count, 0);
    id firstTrace = records.firstObject[kRecordTrace];
    XCTAssertTrue([firstTrace isKindOfClass:NSUUID.class], @"callback %@ ran with no trace", records.firstObject[kRecordName]);
    for (NSDictionary * record in records) {
        XCTAssertEqualObjects(record[kRecordTrace], firstTrace, @"callback %@ ran with a different trace", record[kRecordName]);
    }
}

- (void)startCapturingLogs
{
    os_unfair_lock_lock(&sLogLock);
    sLogs = [NSMutableArray array];
    os_unfair_lock_unlock(&sLogLock);
    MTRSetLogCallback(MTRLogTypeProgress, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        os_unfair_lock_lock(&sLogLock);
        [sLogs addObject:message];
        os_unfair_lock_unlock(&sLogLock);
    });
}

- (NSArray<NSString *> *)capturedLogs
{
    os_unfair_lock_lock(&sLogLock);
    NSArray * logs = [sLogs copy];
    os_unfair_lock_unlock(&sLogLock);
    return logs;
}

- (void)stopCapturingLogs
{
    MTRSetLogCallback(0, nil);
}

@end

static NSArray<NSString *> * LogsContaining(NSArray<NSString *> * logs, NSString * text)
{
    return [logs filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"SELF CONTAINS %@", text]];
}

static BOOL OnMatterQueue(void)
{
    return strcmp(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL), "org.csa-iot.matter.workqueue") == 0;
}

// Records the trace current on the Matter queue each time the replaced method runs there.
@interface MTRTraceMatterQueueProbe : NSObject
@property (nonatomic, readonly) NSArray * traces;
- (instancetype)initWithMethod:(Method)method block:(id)block;
- (void)note;
- (void)remove;
@end

@implementation MTRTraceMatterQueueProbe {
    os_unfair_lock _lock;
    NSMutableArray * _traces;
    Method _method;
    IMP _original;
    IMP _replacement;
}

- (instancetype)initWithMethod:(Method)method block:(id)block
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _traces = [NSMutableArray array];
        _method = method;
        _original = method_getImplementation(method);
        _replacement = imp_implementationWithBlock(block);
        method_setImplementation(method, _replacement);
    }
    return self;
}

- (void)note
{
    if (!OnMatterQueue()) {
        return;
    }
    id trace = CurrentTraceIDOrNull();
    os_unfair_lock_lock(&_lock);
    [_traces addObject:trace];
    os_unfair_lock_unlock(&_lock);
}

- (NSArray *)traces
{
    os_unfair_lock_lock(&_lock);
    NSArray * traces = [_traces copy];
    os_unfair_lock_unlock(&_lock);
    return traces;
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

#pragma mark - Local delegate path

@interface MTRTraceContextLocalTests : MTRTraceContextTestCase
@end

@implementation MTRTraceContextLocalTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _controller = [self createControllerOnTestFabric];
    _device = [MTRDevice deviceWithNodeID:@(kLocalNodeID) controller:_controller];
}

- (void)tearDown
{
    [self stopCapturingLogs];
    [_controller removeDevice:_device];
    _device = nil;
    _controller = nil;
    [super tearDown];
}

- (MTRTraceRecordingDelegate *)addRecordingDelegate
{
    __auto_type * delegate = [[MTRTraceRecordingDelegate alloc] init];
    [_device addDelegate:delegate queue:dispatch_queue_create("trace-test-delegate", DISPATCH_QUEUE_SERIAL)];
    return delegate;
}

- (void)runOnMatterQueue:(dispatch_block_t)block
{
    XCTestExpectation * ran = [self expectationWithDescription:@"ran on Matter queue"];
    [_controller asyncDispatchToMatterQueue:^{
        block();
        [ran fulfill];
    } errorHandler:nil];
    [self waitForExpectations:@[ ran ] timeout:kTimeoutInSeconds];
}

- (void)test001_AllCallbacksOfOneReportShareOneTraceID
{
    __auto_type * delegate = [self addRecordingDelegate];
    [_device unitTestInjectAttributeReport:TestConfigurationReport(1) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kCachePrimed count:1 timeout:kTimeoutInSeconds];

    NSArray * records = delegate.records;
    NSArray * names = [records valueForKey:kRecordName];
    XCTAssertTrue([names containsObject:kStateChanged]);
    XCTAssertTrue([names containsObject:kAttributeReport]);
    XCTAssertTrue([names containsObject:kConfigurationChanged]);
    XCTAssertTrue([names containsObject:kCachePrimed]);
    [self assertRecordsHaveOneSharedTrace:records];
}

- (void)test002_DistinctReportsGetDistinctTraceIDs
{
    __auto_type * delegate = [self addRecordingDelegate];
    [_device unitTestInjectAttributeReport:TestLevelReport(1) fromSubscription:YES];
    [_device unitTestInjectAttributeReport:TestLevelReport(2) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kAttributeReport count:2 timeout:kTimeoutInSeconds];

    NSArray * reports = [delegate recordsNamed:kAttributeReport];
    id first = reports[0][kRecordTrace];
    id second = reports[1][kRecordTrace];
    XCTAssertTrue([first isKindOfClass:NSUUID.class]);
    XCTAssertTrue([second isKindOfClass:NSUUID.class]);
    XCTAssertNotEqualObjects(first, second);
}

- (void)test003_InternalStateUpdateCarriesTraceIDOfItsChange
{
    __auto_type * delegate = [[MTRTraceInternalStateRecordingDelegate alloc] init];
    [_device addDelegate:delegate queue:dispatch_queue_create("trace-test-internal-state-delegate", DISPATCH_QUEUE_SERIAL)];
    [_device unitTestInjectAttributeReport:TestConfigurationReport(2) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kCachePrimed count:1 timeout:kTimeoutInSeconds];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
    [self waitForRecordsOf:delegate named:kInternalStateUpdated count:1 timeout:kTimeoutInSeconds];

    NSUUID * reportTrace = [delegate recordsNamed:kAttributeReport].firstObject[kRecordTrace];
    XCTAssertTrue([reportTrace isKindOfClass:NSUUID.class]);
    NSArray * updates = [delegate recordsNamed:kInternalStateUpdated];
    XCTAssertGreaterThan(updates.count, 0);
    for (NSDictionary * update in updates) {
        XCTAssertEqualObjects(update[kRecordPayload][kMTRDeviceInternalPropertyTraceID], reportTrace.UUIDString, @"%@", update[kRecordPayload]);
    }
}

- (void)test004_TraceIDAppearsInFrameworkLogs
{
    __auto_type * delegate = [self addRecordingDelegate];
    [self startCapturingLogs];
    [_device unitTestInjectAttributeReport:TestLevelReport(5) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kAttributeReport count:1 timeout:kTimeoutInSeconds];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
    [self stopCapturingLogs];

    NSUUID * trace = [delegate recordsNamed:kAttributeReport].firstObject[kRecordTrace];
    XCTAssertTrue([trace isKindOfClass:NSUUID.class]);
    NSArray * tagged = LogsContaining(self.capturedLogs, [NSString stringWithFormat:@"trace=%@", trace.UUIDString]);
    XCTAssertGreaterThanOrEqual(tagged.count, 2, @"%@", tagged);
}

- (void)test005_MetricDataTraceIDSurvivesSecureCoding
{
    __auto_type * delegate = [self addRecordingDelegate];
    [_device unitTestInjectAttributeReport:TestLevelReport(6) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kAttributeReport count:1 timeout:kTimeoutInSeconds];
    NSDictionary * record = [delegate recordsNamed:kAttributeReport].firstObject;
    MTRMetricData * data = record[kRecordMetricData];
    XCTAssertTrue([record[kRecordTrace] isKindOfClass:NSUUID.class]);
    XCTAssertEqualObjects(data.traceID, record[kRecordTrace]);

    NSError * error;
    NSData * archive = [NSKeyedArchiver archivedDataWithRootObject:data requiringSecureCoding:YES error:&error];
    XCTAssertNotNil(archive, @"%@", error);
    MTRMetricData * decoded = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRMetricData.class fromData:archive error:&error];
    XCTAssertNotNil(decoded, @"%@", error);
    XCTAssertEqualObjects(decoded.traceID, record[kRecordTrace]);

    NSKeyedArchiver * archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
    [archiver setClassName:@"MTRMetricData" forClass:MTRTraceOldFormatMetricData.class];
    [archiver encodeObject:[MTRTraceOldFormatMetricData new] forKey:NSKeyedArchiveRootObjectKey];
    [archiver finishEncoding];
    decoded = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRMetricData.class fromData:archiver.encodedData error:&error];
    XCTAssertNotNil(decoded, @"%@", error);
    XCTAssertEqualObjects(decoded.value, @(5));
    XCTAssertNil(decoded.traceID);
}

- (void)test006_NoTraceOutsideCallbacks
{
    XCTAssertNil(MTRTraceContext.currentContext);
    __auto_type * delegate = [self addRecordingDelegate];
    [_device unitTestInjectAttributeReport:TestLevelReport(4) fromSubscription:YES];
    [self waitForRecordsOf:delegate named:kAttributeReport count:1 timeout:kTimeoutInSeconds];
    XCTAssertNil(MTRTraceContext.currentContext);

    __block id traceOnPlainQueue = @"not run";
    XCTestExpectation * ran = [self expectationWithDescription:@"ran on a worker thread"];
    dispatch_async(dispatch_queue_create("trace-test-plain", DISPATCH_QUEUE_SERIAL), ^{
        traceOnPlainQueue = CurrentTraceIDOrNull();
        [ran fulfill];
    });
    [self waitForExpectations:@[ ran ] timeout:kTimeoutInSeconds];
    XCTAssertEqualObjects(traceOnPlainQueue, NSNull.null);
}

- (void)test007_DelegateWithOnlyOriginalMethodsGetsEveryCallback
{
    __auto_type * delegate = [[MTRTraceOriginalMethodsDelegate alloc] init];
    [_device addDelegate:delegate queue:dispatch_get_main_queue()];

    NSArray * attributeReport = TestConfigurationReport(9);
    NSArray * eventReport = TestEventReport();
    [_device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];
    [_device unitTestInjectEventReport:eventReport];
    MTRDevice * device = _device;
    [self runOnMatterQueue:^{
        [device _handleUnsolicitedMessageFromPublisher];
    }];

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

- (void)test008_UnsolicitedMessageCallbacksShareOneTraceID
{
    __auto_type * delegate = [self addRecordingDelegate];
    MTRDevice * device = _device;
    [self runOnMatterQueue:^{
        [device _handleUnsolicitedMessageFromPublisher];
    }];
    [self waitForRecordsOf:delegate named:kBecameActive count:1 timeout:kTimeoutInSeconds];
    [self waitForRecordsOf:delegate named:kStateChanged count:1 timeout:kTimeoutInSeconds];

    [self assertRecordsHaveOneSharedTrace:delegate.records];
}

@end

#pragma mark - XPC

@interface MTRTraceContextTestXPCServer : NSObject <NSXPCListenerDelegate>
@property (nonatomic, readonly) NSXPCListener * listener;
@property (atomic, strong, nullable) NSXPCConnection * clientConnection;
@property (atomic, copy, nullable) NSDictionary * lastControllerConfiguration;
@property (atomic) NSUInteger untracedRepliesSent;
@property (atomic, copy, nullable) NSArray * readAttributePathsReply;
- (id<MTRXPCClientProtocol>)client;
@end

@implementation MTRTraceContextTestXPCServer

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
    SEL readAttributePaths = @selector(deviceController:nodeID:readAttributePaths:withReply:);
    NSSet * readClasses = [[exportedInterface classesForSelector:readAttributePaths argumentIndex:2 ofReply:NO] setByAddingObject:[MTRAttributeRequestPath class]];
    [exportedInterface setClasses:readClasses forSelector:readAttributePaths argumentIndex:2 ofReply:NO];
    [exportedInterface setClasses:readClasses forSelector:@selector(deviceController:nodeID:readAttributePaths:withTracedReply:) argumentIndex:2 ofReply:NO];
    NSSet * commandClasses = [NSSet setWithObjects:NSArray.class, NSDictionary.class, NSString.class, NSNumber.class, NSData.class, MTRCommandPath.class, MTRCommandWithRequiredResponse.class, nil];
    for (NSValue * selectorValue in @[ [NSValue valueWithPointer:@selector(deviceController:nodeID:invokeCommandWithEndpointID:clusterID:commandID:commandFields:expectedValues:expectedValueInterval:timedInvokeTimeout:serverSideProcessingTimeout:completion:)],
             [NSValue valueWithPointer:@selector(deviceController:nodeID:invokeCommandWithEndpointID:clusterID:commandID:commandFields:expectedValues:expectedValueInterval:timedInvokeTimeout:serverSideProcessingTimeout:tracedCompletion:)] ]) {
        SEL invoke = (SEL) selectorValue.pointerValue;
        [exportedInterface setClasses:commandClasses forSelector:invoke argumentIndex:5 ofReply:NO];
        [exportedInterface setClasses:commandClasses forSelector:invoke argumentIndex:6 ofReply:NO];
    }
    [exportedInterface setClasses:commandClasses forSelector:@selector(deviceController:nodeID:invokeCommands:completion:) argumentIndex:2 ofReply:NO];
    [exportedInterface setClasses:commandClasses forSelector:@selector(deviceController:nodeID:invokeCommands:tracedCompletion:) argumentIndex:2 ofReply:NO];

    newConnection.exportedInterface = exportedInterface;
    newConnection.exportedObject = self;
    NSXPCInterface * clientInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];
    [clientInterface setClasses:[NSSet setWithObjects:MTRMetrics.class, MTRMetricData.class, NSDictionary.class, NSString.class, NSNumber.class, NSUUID.class, nil]
                    forSelector:@selector(device:collectedMetrics:)
                  argumentIndex:1
                        ofReply:NO];
    newConnection.remoteObjectInterface = clientInterface;
    self.clientConnection = newConnection;
    [newConnection activate];
    return YES;
}

- (id<MTRXPCClientProtocol>)client
{
    return [self.clientConnection remoteObjectProxy];
}

- (BOOL)clientSupportsTraceIDs
{
    NSDictionary * context = self.lastControllerConfiguration[MTRDeviceControllerRegistrationControllerContextKey];
    return [context[MTRDeviceControllerRegistrationTraceIDsSupportedKey] isEqual:@YES];
}

- (oneway void)deviceController:(NSUUID *)controller updateControllerConfiguration:(NSDictionary *)controllerState
{
    self.lastControllerConfiguration = controllerState;
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths withReply:(void (^)(NSArray<NSDictionary<NSString *, id> *> *))reply
{
    NSArray * replyValues = self.readAttributePathsReply;
    if (replyValues) {
        self.untracedRepliesSent += 1;
    }
    reply(replyValues ?: @[]);
}

- (oneway void)deviceController:(NSUUID *)controller getNodesWithStoredDataWithReply:(void (^)(NSArray<NSNumber *> *))reply
{
    reply(@[]);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommandWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID commandID:(NSNumber *)commandID commandFields:(id)commandFields expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedInvokeTimeout:(NSNumber * _Nullable)timeout serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                     completion:(MTRDeviceResponseHandler)completion
{
    self.untracedRepliesSent += 1;
    completion(TestInvokeResponse(commandID), nil);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands completion:(MTRDeviceResponseHandler)completion
{
    self.untracedRepliesSent += 1;
    completion(TestInvokeResponse(@(0)), nil);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID params:(MTRReadParams * _Nullable)params withReply:(void (^)(NSDictionary<NSString *, id> * _Nullable))reply
{
    self.untracedRepliesSent += 1;
    reply(TestLevelReport(41).firstObject[MTRDataKey]);
}

@end

@interface MTRTraceContextTracingTestXPCServer : MTRTraceContextTestXPCServer
@property (atomic, strong) NSUUID * replyTraceID;
@property (atomic) BOOL pushesConfiguration;
@property (atomic) NSUInteger tracedRepliesSent;
@end

@implementation MTRTraceContextTracingTestXPCServer

- (instancetype)init
{
    if (self = [super init]) {
        _replyTraceID = [NSUUID UUID];
        _pushesConfiguration = YES;
    }
    return self;
}

- (oneway void)deviceController:(NSUUID *)controller updateControllerConfiguration:(NSDictionary *)controllerState
{
    [super deviceController:controller updateControllerConfiguration:controllerState];
    if (!self.pushesConfiguration) {
        return;
    }
    [[self client] controller:controller controllerConfigurationUpdated:@{
        MTRDeviceControllerRegistrationControllerContextKey : @ {
            MTRDeviceControllerRegistrationTraceIDsSupportedKey : @YES,
            MTRDeviceControllerRegistrationControllerNodeIDKey : @(kTracingServerControllerNodeID),
        },
        MTRDeviceControllerRegistrationNodeIDsKey : @[],
    }];
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommandWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID commandID:(NSNumber *)commandID commandFields:(id)commandFields expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedInvokeTimeout:(NSNumber * _Nullable)timeout serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
               tracedCompletion:(void (^)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error, NSUUID * _Nullable traceID))completion
{
    self.tracedRepliesSent += 1;
    completion(TestInvokeResponse(commandID), nil, self.replyTraceID);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands tracedCompletion:(void (^)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error, NSUUID * _Nullable traceID))completion
{
    self.tracedRepliesSent += 1;
    completion(TestInvokeResponse(@(0)), nil, self.replyTraceID);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID params:(MTRReadParams * _Nullable)params withTracedReply:(void (^)(NSDictionary<NSString *, id> * _Nullable value, NSUUID * _Nullable traceID))reply
{
    self.tracedRepliesSent += 1;
    reply(TestLevelReport(42).firstObject[MTRDataKey], self.replyTraceID);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths withTracedReply:(void (^)(NSArray<NSDictionary<NSString *, id> *> * values, NSUUID * _Nullable traceID))reply
{
    self.tracedRepliesSent += 1;
    reply(TestLevelReport(43), self.replyTraceID);
}

@end

@interface MTRTraceContextXPCTests : MTRTraceContextTestCase
@end

@implementation MTRTraceContextXPCTests {
    BOOL _localTestStorageEnabledBeforeTest;
    MTRTraceContextTestXPCServer * _server;
    MTRDeviceController * _controller;
    MTRDevice * _device;
    MTRTraceRecordingDelegate * _delegate;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _localTestStorageEnabledBeforeTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = NO;

    _server = [[MTRTraceContextTestXPCServer alloc] init];
    NSXPCListenerEndpoint * endpoint = _server.listener.endpoint;
    __auto_type * parameters = [[MTRXPCDeviceControllerParameters alloc] initWithXPCConnectionBlock:^NSXPCConnection * {
        return [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
    } uniqueIdentifier:[NSUUID UUID]];
    NSError * error;
    _controller = [[MTRDeviceController alloc] initWithParameters:parameters error:&error];
    XCTAssertNotNil(_controller, @"%@", error);

    _device = [MTRDevice deviceWithNodeID:@(kLocalNodeID) controller:_controller];
    _delegate = [[MTRTraceRecordingDelegate alloc] init];
    [_device addDelegate:_delegate queue:dispatch_queue_create("trace-test-xpc-delegate", DISPATCH_QUEUE_SERIAL)];

    MTRTraceContextTestXPCServer * server = _server;
    XCTestExpectation * registered = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return server.clientConnection != nil && server.lastControllerConfiguration != nil;
    }]
                                               evaluatedWithObject:server
                                                           handler:nil];
    [self waitForExpectations:@[ registered ] timeout:kTimeoutInSeconds];
}

- (void)tearDown
{
    [self stopCapturingLogs];
    [_device removeDelegate:_delegate];
    [_controller shutdown];
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeTest;
    [super tearDown];
}

- (NSDictionary *)internalStateWithState:(MTRDeviceState)state primed:(BOOL)primed traceID:(NSUUID * _Nullable)traceID
{
    NSMutableDictionary * internalState = [@{
        kMTRDeviceInternalPropertyDeviceState : @(state),
        kMTRDeviceInternalPropertyLastSubscriptionAttemptWait : @(0),
        kMTRDeviceInternalPropertyDeviceCachePrimed : @(primed),
    } mutableCopy];
    if (traceID) {
        internalState[kMTRDeviceInternalPropertyTraceID] = traceID.UUIDString;
    }
    return internalState;
}

- (void)test001_ClientAdvertisesTraceIDSupport
{
    XCTAssertTrue([_server clientSupportsTraceIDs], @"%@", _server.lastControllerConfiguration);
}

- (void)test002_ReportSelectorsCarryServerTraceID
{
    XCTAssertTrue([_server clientSupportsTraceIDs]);
    NSUUID * attributeTrace = [NSUUID UUID];
    NSUUID * eventTrace = [NSUUID UUID];
    NSUUID * activeTrace = [NSUUID UUID];
    NSUUID * configurationTrace = [NSUUID UUID];
    NSArray * attributeReport = TestLevelReport(11);
    NSArray * eventReport = TestEventReport();
    [[_server client] device:@(kLocalNodeID) receivedAttributeReport:attributeReport traceID:attributeTrace];
    [[_server client] device:@(kLocalNodeID) receivedEventReport:eventReport traceID:eventTrace];
    [[_server client] deviceBecameActive:@(kLocalNodeID) traceID:activeTrace];
    [[_server client] deviceConfigurationChanged:@(kLocalNodeID) traceID:configurationTrace];
    [self waitForRecordsOf:_delegate named:kConfigurationChanged count:1 timeout:kTimeoutInSeconds];

    NSDictionary * attribute = [_delegate recordsNamed:kAttributeReport].firstObject;
    NSDictionary * event = [_delegate recordsNamed:kEventReport].firstObject;
    XCTAssertEqualObjects(attribute[kRecordPayload], attributeReport);
    XCTAssertEqualObjects(attribute[kRecordTrace], attributeTrace);
    XCTAssertEqualObjects(event[kRecordPayload], eventReport);
    XCTAssertEqualObjects(event[kRecordTrace], eventTrace);
    XCTAssertEqualObjects([_delegate recordsNamed:kBecameActive].firstObject[kRecordTrace], activeTrace);
    XCTAssertEqualObjects([_delegate recordsNamed:kConfigurationChanged].firstObject[kRecordTrace], configurationTrace);
}

- (void)test003_InternalStateTraceIDReachesStateAndPrimedCallbacks
{
    NSUUID * traceID = [NSUUID UUID];
    [[_server client] device:@(kLocalNodeID) internalStateUpdated:[self internalStateWithState:MTRDeviceStateReachable primed:YES traceID:traceID]];
    [self waitForRecordsOf:_delegate named:kCachePrimed count:1 timeout:kTimeoutInSeconds];

    NSArray * records = [_delegate.records filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K IN %@", kRecordName, @[ kStateChanged, kCachePrimed ]]];
    XCTAssertEqual(records.count, 2);
    for (NSDictionary * record in records) {
        XCTAssertEqualObjects(record[kRecordTrace], traceID, @"%@", record[kRecordName]);
    }
}

- (void)test004_InternalStateTraceKeyDoesNotChangeCallbacks
{
    __auto_type * plainDevice = [MTRDevice deviceWithNodeID:@(kLocalNodeID + 1) controller:_controller];
    __auto_type * plainDelegate = [[MTRTraceRecordingDelegate alloc] init];
    [plainDevice addDelegate:plainDelegate queue:dispatch_queue_create("trace-test-xpc-plain-delegate", DISPATCH_QUEUE_SERIAL)];
    MTRTraceContextTestXPCServer * server = _server;
    XCTestExpectation * registered = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        NSArray * nodes = server.lastControllerConfiguration[MTRDeviceControllerRegistrationNodeIDsKey];
        return [[nodes valueForKey:MTRDeviceControllerRegistrationNodeIDKey] containsObject:@(kLocalNodeID + 1)];
    }]
                                               evaluatedWithObject:server
                                                           handler:nil];
    [self waitForExpectations:@[ registered ] timeout:kTimeoutInSeconds];

    NSArray<NSArray *> * sequence = @[ @[ @(MTRDeviceStateReachable), @NO ], @[ @(MTRDeviceStateUnreachable), @YES ], @[ @(MTRDeviceStateReachable), @YES ] ];
    for (NSArray * step in sequence) {
        MTRDeviceState state = (MTRDeviceState)[step[0] unsignedIntegerValue];
        BOOL primed = [step[1] boolValue];
        [[_server client] device:@(kLocalNodeID) internalStateUpdated:[self internalStateWithState:state primed:primed traceID:[NSUUID UUID]]];
        [[_server client] device:@(kLocalNodeID + 1) internalStateUpdated:[self internalStateWithState:state primed:primed traceID:nil]];
    }
    [[_server client] deviceConfigurationChanged:@(kLocalNodeID)];
    [[_server client] deviceConfigurationChanged:@(kLocalNodeID + 1)];
    [self waitForRecordsOf:_delegate named:kConfigurationChanged count:1 timeout:kTimeoutInSeconds];
    [self waitForRecordsOf:plainDelegate named:kConfigurationChanged count:1 timeout:kTimeoutInSeconds];

    NSArray * withKey = [_delegate.records valueForKey:kRecordName];
    XCTAssertEqualObjects(withKey, [plainDelegate.records valueForKey:kRecordName]);
    XCTAssertEqualObjects([[_delegate recordsNamed:kStateChanged] valueForKey:kRecordPayload], [[plainDelegate recordsNamed:kStateChanged] valueForKey:kRecordPayload]);
    XCTAssertEqual(_device.state, plainDevice.state);
    XCTAssertEqual(_device.deviceCachePrimed, plainDevice.deviceCachePrimed);
    XCTAssertGreaterThan([_delegate recordsNamed:kStateChanged].count, 1);
    [plainDevice removeDelegate:plainDelegate];
}

@end

#pragma mark - XPC replies

@interface MTRTraceContextXPCReplyTests : MTRTraceContextTestCase
@end

@implementation MTRTraceContextXPCReplyTests {
    BOOL _localTestStorageEnabledBeforeTest;
    MTRDeviceController * _controller;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _localTestStorageEnabledBeforeTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = NO;
}

- (void)tearDown
{
    [self stopCapturingLogs];
    [_controller shutdown];
    _controller = nil;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeTest;
    [super tearDown];
}

- (MTRDevice *)deviceForServer:(MTRTraceContextTestXPCServer *)server
{
    NSXPCListenerEndpoint * endpoint = server.listener.endpoint;
    __auto_type * parameters = [[MTRXPCDeviceControllerParameters alloc] initWithXPCConnectionBlock:^NSXPCConnection * {
        return [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
    } uniqueIdentifier:[NSUUID UUID]];
    NSError * error;
    _controller = [[MTRDeviceController alloc] initWithParameters:parameters error:&error];
    XCTAssertNotNil(_controller, @"%@", error);
    MTRDevice * device = [MTRDevice deviceWithNodeID:@(kLocalNodeID) controller:_controller];
    [device addDelegate:[[MTRTraceRecordingDelegate alloc] init] queue:dispatch_queue_create("trace-test-xpc-reply-delegate", DISPATCH_QUEUE_SERIAL)];
    XCTestExpectation * registered = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return server.clientConnection != nil && server.lastControllerConfiguration != nil;
    }]
                                               evaluatedWithObject:server
                                                           handler:nil];
    [self waitForExpectations:@[ registered ] timeout:kTimeoutInSeconds];
    if ([server isKindOfClass:MTRTraceContextTracingTestXPCServer.class]) {
        MTRDeviceController * controller = _controller;
        XCTestExpectation * pushed = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
            return [controller.controllerNodeID isEqual:@(kTracingServerControllerNodeID)];
        }]
                                               evaluatedWithObject:controller
                                                           handler:nil];
        [self waitForExpectations:@[ pushed ] timeout:kTimeoutInSeconds];
    }
    return device;
}

- (id)invokeToggleOn:(MTRDevice *)device values:(NSArray * __autoreleasing *)valuesOut
{
    XCTestExpectation * invoked = [self expectationWithDescription:@"invoke completed"];
    __block id trace;
    __block NSArray * values;
    [device invokeCommandWithEndpointID:@(1)
                              clusterID:@(MTRClusterIDTypeOnOffID)
                              commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)
                          commandFields:@{ MTRTypeKey : MTRStructureValueType, MTRValueKey : @[] }
                         expectedValues:nil
                  expectedValueInterval:nil
                                  queue:dispatch_queue_create("trace-test-invoke", DISPATCH_QUEUE_SERIAL)
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable responseValues, NSError * _Nullable error) {
                                 XCTAssertNil(error);
                                 values = responseValues;
                                 trace = CurrentTraceIDOrNull();
                                 [invoked fulfill];
                             }];
    [self waitForExpectations:@[ invoked ] timeout:kTimeoutInSeconds];
    *valuesOut = values;
    return trace;
}

- (void)test001_InvokeCompletionCarriesServerTraceID
{
    __auto_type * server = [[MTRTraceContextTracingTestXPCServer alloc] init];
    MTRDevice * device = [self deviceForServer:server];
    NSArray * values;
    id trace = [self invokeToggleOn:device values:&values];

    XCTAssertEqualObjects(values, TestInvokeResponse(@(MTRCommandIDTypeClusterOnOffCommandToggleID)));
    XCTAssertEqual(server.tracedRepliesSent, 1);
    XCTAssertEqualObjects(trace, server.replyTraceID);
}

- (void)test002_OldServerReadRepliesUnchanged
{
    __auto_type * server = [[MTRTraceContextTestXPCServer alloc] init];
    MTRDevice * device = [self deviceForServer:server];
    NSDictionary * value = [device readAttributeWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID) params:nil];

    XCTAssertEqualObjects(value, TestLevelReport(41).firstObject[MTRDataKey]);
    XCTAssertEqual(server.untracedRepliesSent, 1);
}

- (void)test003_LostConnectionForgetsServerSupport
{
    __auto_type * server = [[MTRTraceContextTracingTestXPCServer alloc] init];
    MTRDevice * device = [self deviceForServer:server];
    NSArray * values;
    [self invokeToggleOn:device values:&values];
    XCTAssertEqual(server.tracedRepliesSent, 1);

    NSXPCConnection * clientConnection = server.clientConnection;
    server.pushesConfiguration = NO;
    [clientConnection invalidate];
    MTRDeviceController * controller = _controller;
    XCTestExpectation * forgotten = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary * bindings) {
        return !controller.serverSupportsTraceIDs;
    }]
                                              evaluatedWithObject:controller
                                                          handler:nil];
    [self waitForExpectations:@[ forgotten ] timeout:kTimeoutInSeconds];
}

- (void)test004_InvokeCommandsCompletionCarriesServerTraceID
{
    __auto_type * server = [[MTRTraceContextTracingTestXPCServer alloc] init];
    MTRDevice * device = [self deviceForServer:server];
    __auto_type * command = [[MTRCommandWithRequiredResponse alloc] initWithPath:[MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)]
                                                                   commandFields:nil
                                                                requiredResponse:nil];
    XCTestExpectation * invoked = [self expectationWithDescription:@"invokeCommands completed"];
    __block id trace;
    __block NSArray * values;
    [device invokeCommands:@[ @[ command ] ]
                     queue:dispatch_queue_create("trace-test-invoke-commands", DISPATCH_QUEUE_SERIAL)
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable responseValues, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    values = responseValues;
                    trace = CurrentTraceIDOrNull();
                    [invoked fulfill];
                }];
    [self waitForExpectations:@[ invoked ] timeout:kTimeoutInSeconds];

    XCTAssertEqualObjects(values, TestInvokeResponse(@(0)));
    XCTAssertEqual(server.tracedRepliesSent, 1);
    XCTAssertEqualObjects(trace, server.replyTraceID);
}

- (void)test005_ReadRepliesCarryServerTraceIDIntoClientLogs
{
    __auto_type * server = [[MTRTraceContextTracingTestXPCServer alloc] init];
    MTRDevice * device = [self deviceForServer:server];
    [self startCapturingLogs];
    NSDictionary * value = [device readAttributeWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID) params:nil];
    NSArray * values = [device readAttributePaths:@[ [MTRAttributeRequestPath requestPathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:nil] ]];
    [self stopCapturingLogs];
    XCTAssertNil(MTRTraceContext.currentContext);

    XCTAssertEqualObjects(value, TestLevelReport(42).firstObject[MTRDataKey]);
    XCTAssertEqualObjects(values, TestLevelReport(43));
    XCTAssertEqual(server.tracedRepliesSent, 2);
    NSArray * tagged = LogsContaining(self.capturedLogs, [NSString stringWithFormat:@"trace=%@", server.replyTraceID.UUIDString]);
    XCTAssertGreaterThanOrEqual(tagged.count, 2, @"%@", tagged);
}

- (void)test006_OldServerReadPathsAndInvokeCommandsUseOriginalSelectors
{
    __auto_type * server = [[MTRTraceContextTestXPCServer alloc] init];
    server.readAttributePathsReply = TestLevelReport(44);
    MTRDevice * device = [self deviceForServer:server];
    NSArray * values = [device readAttributePaths:@[ [MTRAttributeRequestPath requestPathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:nil] ]];
    XCTAssertEqualObjects(values, TestLevelReport(44));

    __auto_type * command = [[MTRCommandWithRequiredResponse alloc] initWithPath:[MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID) commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)]
                                                                   commandFields:nil
                                                                requiredResponse:nil];
    XCTestExpectation * invoked = [self expectationWithDescription:@"invokeCommands completed"];
    __block id trace;
    [device invokeCommands:@[ @[ command ] ]
                     queue:dispatch_queue_create("trace-test-old-invoke-commands", DISPATCH_QUEUE_SERIAL)
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable responseValues, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertEqualObjects(responseValues, TestInvokeResponse(@(0)));
                    trace = CurrentTraceIDOrNull();
                    [invoked fulfill];
                }];
    [self waitForExpectations:@[ invoked ] timeout:kTimeoutInSeconds];

    XCTAssertEqual(server.untracedRepliesSent, 2);
    XCTAssertTrue([trace isKindOfClass:NSUUID.class]);
}

@end

#pragma mark - Real subscription to a running accessory

static MTRDeviceController * sWireController = nil;

@interface MTRTraceContextWireTests : MTRTraceContextTestCase
@end

@implementation MTRTraceContextWireTests

+ (void)setUp
{
    [super setUp];
    sWireController = [self startCommissionedAppWithName:@"all-clusters" arguments:@[] nodeID:@(kWireNodeID)];
    XCTAssertNotNil(sWireController);
}

+ (void)tearDown
{
    sWireController = nil;
    [super tearDown];
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    [sWireController.controllerDataStore clearAllStoredClusterData];
    [sWireController.controllerDataStore clearDeviceDataForNodeID:@(kWireNodeID)];
}

- (void)tearDown
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kWireNodeID) controller:sWireController];
    [sWireController removeDevice:device];
    [super tearDown];
}

- (MTRTraceRecordingDelegate *)primedDeviceDelegate:(MTRDevice * __autoreleasing *)deviceOut
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kWireNodeID) controller:sWireController];
    __auto_type * delegate = [[MTRTraceRecordingDelegate alloc] init];
    delegate.subscribe = YES;
    [device addDelegate:delegate queue:dispatch_queue_create("trace-test-wire-delegate", DISPATCH_QUEUE_SERIAL)];
    [self waitForRecordsOf:delegate named:kCachePrimed count:1 timeout:kSubscriptionTimeoutInSeconds];
    *deviceOut = device;
    return delegate;
}

- (void)test001_PrimingReportCallbacksShareOneTraceID
{
    MTRDevice * device;
    __auto_type * delegate = [self primedDeviceDelegate:&device];

    NSArray * records = delegate.records;
    NSUInteger primedIndex = [[records valueForKey:kRecordName] indexOfObject:kCachePrimed];
    NSArray * priming = [records subarrayWithRange:NSMakeRange(0, primedIndex + 1)];
    priming = [priming filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K IN %@", kRecordName, @[ kAttributeReport, kEventReport, kCachePrimed ]]];
    NSArray * primingReports = [priming filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K == %@", kRecordName, kAttributeReport]];
    XCTAssertGreaterThan(primingReports.count, 1);
    [self assertRecordsHaveOneSharedTrace:priming];
}

- (void)test002_UnsolicitedReportSharesTraceWithItsMessage
{
    MTRDevice * device;
    __auto_type * delegate = [self primedDeviceDelegate:&device];
    MTRBaseDevice * baseDevice = [MTRBaseDevice deviceWithNodeID:@(kWireNodeID) controller:sWireController];
    NSUInteger recordsBefore = delegate.records.count;
    NSNumber * current = [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID) params:nil][MTRValueKey];
    unsigned onLevel = [current isKindOfClass:NSNumber.class] && current.unsignedIntValue == 100 ? 101 : 100;
    [self startCapturingLogs];

    XCTestExpectation * written = [self expectationWithDescription:@"write completed"];
    [baseDevice writeAttributeWithEndpointID:@(1)
                                   clusterID:@(MTRClusterIDTypeLevelControlID)
                                 attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID)
                                       value:@{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(onLevel) }
                           timedWriteTimeout:nil
                                       queue:dispatch_get_main_queue()
                                  completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                      XCTAssertNil(error);
                                      [written fulfill];
                                  }];
    [self waitForExpectations:@[ written ] timeout:kSubscriptionTimeoutInSeconds];
    MTRAttributePath * onLevelPath = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID)];
    __block NSDictionary * report;
    XCTestExpectation * reported = [self expectationForPredicate:[NSPredicate predicateWithBlock:^BOOL(MTRTraceRecordingDelegate * evaluated, NSDictionary * bindings) {
        NSArray * all = evaluated.records;
        NSArray * after = [all subarrayWithRange:NSMakeRange(recordsBefore, all.count - recordsBefore)];
        for (NSDictionary * record in after) {
            if ([record[kRecordName] isEqual:kAttributeReport] && [[record[kRecordPayload] valueForKey:MTRAttributePathKey] containsObject:onLevelPath]) {
                report = record;
                return YES;
            }
        }
        return NO;
    }]
                                             evaluatedWithObject:delegate
                                                         handler:nil];
    [self waitForExpectations:@[ reported ] timeout:kSubscriptionTimeoutInSeconds];
    [self stopCapturingLogs];

    XCTAssertTrue([report[kRecordTrace] isKindOfClass:NSUUID.class]);
    NSMutableSet<NSString *> * messageTraces = [NSMutableSet set];
    for (NSString * line in LogsContaining(self.capturedLogs, @"got unsolicited message from publisher trace=")) {
        [messageTraces addObject:[line componentsSeparatedByString:@"trace="].lastObject];
    }
    XCTAssertTrue([messageTraces containsObject:[report[kRecordTrace] UUIDString]], @"report trace %@ is not an unsolicited message's: %@", report[kRecordTrace], messageTraces);
    NSString * reportBegin = [NSString stringWithFormat:@"got report begin trace=%@", [report[kRecordTrace] UUIDString]];
    NSString * lastMessageTrace;
    for (NSString * line in self.capturedLogs) {
        if ([line containsString:@"got unsolicited message from publisher trace="]) {
            lastMessageTrace = [line componentsSeparatedByString:@"trace="].lastObject;
        } else if ([line containsString:reportBegin]) {
            break;
        }
    }
    XCTAssertEqualObjects(lastMessageTrace, [report[kRecordTrace] UUIDString]);
    NSArray * allRecords = delegate.records;
    NSArray * after = [allRecords subarrayWithRange:NSMakeRange(recordsBefore, allRecords.count - recordsBefore)];
    for (NSDictionary *active in [after filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K == %@", kRecordName, kBecameActive]]) {
        XCTAssertTrue([messageTraces containsObject:[active[kRecordTrace] UUIDString]], @"became-active trace %@ is not an unsolicited message's", active[kRecordTrace]);
    }
}

- (void)test003_SubscriptionMetricsCallbackHasTraceIDOfItsMessage
{
    [self startCapturingLogs];
    MTRDevice * device;
    __auto_type * delegate = [self primedDeviceDelegate:&device];
    [self waitForRecordsOf:delegate named:kCollectedMetrics count:1 timeout:kSubscriptionTimeoutInSeconds];
    [self stopCapturingLogs];

    NSUUID * primingTrace = [delegate recordsNamed:kCachePrimed].firstObject[kRecordTrace];
    NSDictionary * record = [delegate recordsNamed:kCollectedMetrics].firstObject;
    XCTAssertTrue([record[kRecordTrace] isKindOfClass:NSUUID.class]);
    XCTAssertNotEqualObjects(record[kRecordTrace], primingTrace);
    NSString * established = [NSString stringWithFormat:@"got subscription established trace=%@", [record[kRecordTrace] UUIDString]];
    XCTAssertEqual(LogsContaining(self.capturedLogs, established).count, 1, @"%@", LogsContaining(self.capturedLogs, @"got subscription established"));
}

- (void)test004_BaseDeviceReadCompletionIsTraced
{
    MTRBaseDevice * baseDevice = [MTRBaseDevice deviceWithNodeID:@(kWireNodeID) controller:sWireController];
    __block id first;
    __block id second;
    for (int i = 0; i < 2; i++) {
        XCTestExpectation * read = [self expectationWithDescription:@"read completed"];
        [baseDevice readAttributesWithEndpointID:@(0)
                                       clusterID:@(MTRClusterIDTypeBasicInformationID)
                                     attributeID:@(MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID)
                                          params:nil
                                           queue:dispatch_get_main_queue()
                                      completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                          XCTAssertNil(error);
                                          if (i == 0) {
                                              first = CurrentTraceIDOrNull();
                                          } else {
                                              second = CurrentTraceIDOrNull();
                                          }
                                          [read fulfill];
                                      }];
        [self waitForExpectations:@[ read ] timeout:kSubscriptionTimeoutInSeconds];
    }
    XCTAssertTrue([first isKindOfClass:NSUUID.class]);
    XCTAssertTrue([second isKindOfClass:NSUUID.class]);
    XCTAssertNotEqualObjects(first, second);
}

- (void)test005_BaseDeviceEmptyReadCompletionIsTraced
{
    MTRBaseDevice * baseDevice = [MTRBaseDevice deviceWithNodeID:@(kWireNodeID) controller:sWireController];
    NSMutableArray * traces = [NSMutableArray array];
    for (int i = 0; i < 2; i++) {
        XCTestExpectation * read = [self expectationWithDescription:@"empty read completed"];
        [baseDevice readAttributePaths:@[]
                            eventPaths:nil
                                params:nil
                                 queue:dispatch_get_main_queue()
                            completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                XCTAssertEqualObjects(values, @[]);
                                [traces addObject:CurrentTraceIDOrNull()];
                                [read fulfill];
                            }];
        [self waitForExpectations:@[ read ] timeout:kTimeoutInSeconds];
    }
    XCTAssertTrue([traces[0] isKindOfClass:NSUUID.class]);
    XCTAssertTrue([traces[1] isKindOfClass:NSUUID.class]);
    XCTAssertNotEqualObjects(traces[0], traces[1]);
}

- (void)test006_PrimingEventDataCallbacksRunInReportTrace
{
    Method method = class_getClassMethod(MTRBaseDevice.class, NSSelectorFromString(@"eventReportForHeader:andData:"));
    XCTAssertTrue(method != NULL);
    IMP original = method_getImplementation(method);
    __block __weak MTRTraceMatterQueueProbe * weakProbe;
    __auto_type * probe = [[MTRTraceMatterQueueProbe alloc] initWithMethod:method block:^NSDictionary *(id self, const void * header, id data) {
        [weakProbe note];
        return ((NSDictionary * (*) (id, SEL, const void *, id)) original)(self, NSSelectorFromString(@"eventReportForHeader:andData:"), header, data);
    }];
    weakProbe = probe;
    [self addTeardownBlock:^{
        [probe remove];
    }];

    MTRDevice * device;
    __auto_type * delegate = [self primedDeviceDelegate:&device];
    [probe remove];

    NSUUID * primingTrace = [delegate recordsNamed:kCachePrimed].firstObject[kRecordTrace];
    XCTAssertTrue([primingTrace isKindOfClass:NSUUID.class]);
    NSArray * traces = probe.traces;
    XCTAssertGreaterThan(traces.count, 0);
    XCTAssertTrue([traces containsObject:primingTrace], @"event data decoded in traces %@, report trace %@", traces, primingTrace);
    for (id trace in traces) {
        XCTAssertTrue([trace isKindOfClass:NSUUID.class], @"event data decoded with no trace");
    }
}

- (void)test007_BaseDeviceSubscriptionDataCallbacksRunInReportTrace
{
    Method method = class_getInstanceMethod(MTRAttributeReport.class, NSSelectorFromString(@"initWithPath:value:error:"));
    XCTAssertTrue(method != NULL);
    IMP original = method_getImplementation(method);
    __block __weak MTRTraceMatterQueueProbe * weakProbe;
    __auto_type * probe = [[MTRTraceMatterQueueProbe alloc] initWithMethod:method block:^id(id self, const void * path, id value, id error) {
        [weakProbe note];
        return ((id(*)(id, SEL, const void *, id, id)) original)(self, NSSelectorFromString(@"initWithPath:value:error:"), path, value, error);
    }];
    weakProbe = probe;
    [self addTeardownBlock:^{
        [probe remove];
    }];

    MTRBaseDevice * baseDevice = [MTRBaseDevice deviceWithNodeID:@(kWireNodeID) controller:sWireController];
    __block id reportTrace;
    XCTestExpectation * reported = [self expectationWithDescription:@"first attribute report"];
    XCTestExpectation * established = [self expectationWithDescription:@"subscription established"];
    dispatch_queue_t queue = dispatch_queue_create("trace-test-base-subscription", DISPATCH_QUEUE_SERIAL);
    [baseDevice subscribeWithQueue:queue
        params:[[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(10)]
        clusterStateCacheContainer:nil
        attributeReportHandler:^(NSArray * value) {
            if (reportTrace == nil) {
                reportTrace = CurrentTraceIDOrNull();
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
    [self waitForExpectations:@[ reported, established ] timeout:kSubscriptionTimeoutInSeconds];
    [probe remove];

    XCTAssertTrue([reportTrace isKindOfClass:NSUUID.class]);
    NSArray * traces = probe.traces;
    XCTAssertGreaterThan(traces.count, 0);
    XCTAssertTrue([traces containsObject:reportTrace], @"attribute data decoded in traces %@, report trace %@", traces, reportTrace);
    for (id trace in traces) {
        XCTAssertTrue([trace isKindOfClass:NSUUID.class], @"attribute data decoded with no trace");
    }
}

@end

#pragma mark - OTA provider completions

static os_unfair_lock sTracedLogLock = OS_UNFAIR_LOCK_INIT;
static NSMutableArray<NSArray *> * sTracedLogs;

static void StartCapturingTracedLogs(void)
{
    os_unfair_lock_lock(&sTracedLogLock);
    sTracedLogs = [NSMutableArray array];
    os_unfair_lock_unlock(&sTracedLogLock);
    MTRSetLogCallback(MTRLogTypeDetail, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        id trace = CurrentTraceIDOrNull();
        os_unfair_lock_lock(&sTracedLogLock);
        [sTracedLogs addObject:@[ message, trace ]];
        os_unfair_lock_unlock(&sTracedLogLock);
    });
}

// The trace current when each log line containing text was emitted.
static NSArray * TracesOfLogsContaining(NSString * text)
{
    os_unfair_lock_lock(&sTracedLogLock);
    NSArray * logs = [sTracedLogs copy];
    os_unfair_lock_unlock(&sTracedLogLock);
    NSMutableArray * traces = [NSMutableArray array];
    for (NSArray * log in logs) {
        if ([log[0] containsString:text]) {
            [traces addObject:log[1]];
        }
    }
    return traces;
}

@interface MTRTraceOTAProviderDelegate : NSObject <MTROTAProviderDelegate>
@property (atomic, nullable) id queryImageTrace;
@property (atomic, nullable) id applyUpdateTrace;
@property (atomic, nullable) id notifyUpdateAppliedTrace;
@end

@implementation MTRTraceOTAProviderDelegate

- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params
                       completion:(void (^)(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error))completion
{
    self.queryImageTrace = CurrentTraceIDOrNull();
    __auto_type * response = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];
    response.status = @(MTROTASoftwareUpdateProviderStatusNotAvailable);
    completion(response, nil);
}

- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                               completion:(void (^)(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error))completion
{
    self.applyUpdateTrace = CurrentTraceIDOrNull();
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC), dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
        __auto_type * response = [[MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams alloc] init];
        response.action = @(MTROTASoftwareUpdateProviderApplyUpdateActionDiscontinue);
        response.delayedActionTime = @(0);
        completion(response, nil);
    });
}

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                                completion:(MTRStatusCompletion)completion
{
    self.notifyUpdateAppliedTrace = CurrentTraceIDOrNull();
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC), dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
        completion([NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    });
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

@interface MTRTraceContextOTAProviderTests : MTRTestCase
@end

@implementation MTRTraceContextOTAProviderTests {
    dispatch_queue_t _storageQueue;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    [MTRDeviceControllerFactory.sharedInstance stopControllerFactory];
    _storageQueue = dispatch_queue_create("trace-test-storage", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
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
        [params setOTAProviderDelegate:otaProviderDelegate queue:dispatch_queue_create("trace-test-ota-provider", DISPATCH_QUEUE_SERIAL)];
    }
    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithParameters:params error:&error];
    XCTAssertNotNil(controller, @"%@", error);
    return controller;
}

- (void)test001_ProviderResponsesRunInTheirRequestTrace
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    __auto_type * providerDelegate = [[MTRTraceOTAProviderDelegate alloc] init];
    // Both controllers share one fabric; commands for it reach the first one started, the provider.
    MTRDeviceController * provider = [self startControllerWithRootKeys:rootKeys nodeID:@(0x7A3C2001) otaProviderDelegate:providerDelegate];
    MTRDeviceController * requestor = [self startControllerWithRootKeys:rootKeys nodeID:@(0x7A3C2002) otaProviderDelegate:nil];

    __auto_type * cluster = [[MTRBaseClusterOTASoftwareUpdateProvider alloc] initWithDevice:[MTRBaseDevice deviceWithNodeID:provider.controllerNodeID controller:requestor]
                                                                                 endpointID:@(0)
                                                                                      queue:dispatch_get_main_queue()];
    NSData * updateToken = [@"trace-test-token" dataUsingEncoding:NSUTF8StringEncoding];
    StartCapturingTracedLogs();

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
    [self waitForExpectations:@[ queried ] timeout:kSubscriptionTimeoutInSeconds];

    XCTestExpectation * applied = [self expectationWithDescription:@"ApplyUpdateRequest response"];
    __auto_type * applyParams = [[MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];
    applyParams.updateToken = updateToken;
    applyParams.newVersion = @(2);
    [cluster applyUpdateRequestWithParams:applyParams
                               completion:^(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error) {
                                   XCTAssertNil(error);
                                   [applied fulfill];
                               }];
    [self waitForExpectations:@[ applied ] timeout:kSubscriptionTimeoutInSeconds];

    XCTestExpectation * notified = [self expectationWithDescription:@"NotifyUpdateApplied response"];
    __auto_type * notifyParams = [[MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];
    notifyParams.updateToken = updateToken;
    notifyParams.softwareVersion = @(2);
    [cluster notifyUpdateAppliedWithParams:notifyParams
                                completion:^(NSError * _Nullable error) {
                                    XCTAssertNotNil(error);
                                    [notified fulfill];
                                }];
    [self waitForExpectations:@[ notified ] timeout:kSubscriptionTimeoutInSeconds];
    MTRSetLogCallback(0, nil);

    NSDictionary * checks = @{
        @"QueryImage: application responded with" : providerDelegate.queryImageTrace ?: NSNull.null,
        @"ApplyUpdateRequest: application responded with" : providerDelegate.applyUpdateTrace ?: NSNull.null,
        @"NotifyUpdateApplied: application returned error" : providerDelegate.notifyUpdateAppliedTrace ?: NSNull.null,
    };
    for (NSString * line in checks) {
        id requestTrace = checks[line];
        XCTAssertTrue([requestTrace isKindOfClass:NSUUID.class], @"delegate callback for \"%@\" ran with no trace", line);
        NSArray * traces = TracesOfLogsContaining(line);
        XCTAssertGreaterThan(traces.count, 0, @"%@", line);
        XCTAssertEqualObjects(traces.firstObject, requestTrace, @"\"%@\" logged in trace %@, request trace %@", line, traces.firstObject, requestTrace);
        for (id trace in traces) {
            XCTAssertEqualObjects(trace, requestTrace, @"%@", line);
        }
    }

    [requestor shutdown];
    [provider shutdown];
}

@end

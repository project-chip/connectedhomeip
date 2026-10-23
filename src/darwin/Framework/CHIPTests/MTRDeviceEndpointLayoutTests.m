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

#import "MTRDeviceClusterData.h"
#import "MTRDeviceTestDelegate.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"

static const uint16_t kTestVendorId = 0xFFF1u;
static const uint16_t kTimeoutInSeconds = 3;
static const uint32_t kBridgedNodeDeviceType = 0x0013;
static const uint32_t kAggregatorDeviceType = 0x000E;
static const uint32_t kRootNodeDeviceType = 0x0016;
static const uint32_t kOnOffLightDeviceType = 0x0100;

@interface MTRDevice (EndpointLayoutTests)
- (void)setPersistedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)clusterData;
- (void)unitTestRemoveClusters:(NSSet<MTRClusterPath *> *)clusterPaths;
- (void)unitTestRemoveCachedAttribute:(NSNumber *)attributeID fromCluster:(MTRClusterPath *)clusterPath;
- (void)unitTestReconcilePersistedClustersWithStorage;
@end

@interface MTRDeviceController (EndpointLayoutTests)
- (void)unitTestReplaceXPCConnection:(NSXPCConnection *)connection;
@property (nullable, atomic, retain, readwrite) NSXPCConnection * xpcConnection;
@end

@protocol MTRDeviceEndpointLayoutTestDataStore
- (void)clearStoredClusterDataForNodeID:(NSNumber *)nodeID endpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID;
- (void)synchronouslyPerformBlock:(void (^_Nullable)(void))block;
@end

@protocol MTRDeviceEndpointLayout
@property (nonatomic, readonly, copy, nullable) NSDictionary<NSNumber *, MTREndpointInfo *> * endpointsById;
- (nullable NSArray<MTRAttributePath *> *)attributePathsForEndpointID:(NSNumber *)endpointID;
- (nullable NSArray<MTRCommandPath *> *)acceptedCommandPathsForEndpointID:(NSNumber *)endpointID;
@end

// The wire value of MTRDeviceControllerRegistrationControllerServesEndpointLayoutKey.
static NSString * const kServesEndpointLayoutKey = @"MTRDeviceControllerRegistrationControllerServesEndpointLayout";

static NSDictionary * UnsignedValue(NSUInteger value)
{
    return @{MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(value)};
}

static NSDictionary * NumberList(NSArray<NSNumber *> * numbers)
{
    NSMutableArray * items = [NSMutableArray array];
    for (NSNumber * number in numbers) {
        [items addObject:@ { MTRDataKey : UnsignedValue(number.unsignedIntegerValue) }];
    }
    return @ { MTRTypeKey : MTRArrayValueType, MTRValueKey : items };
}

static NSDictionary * DeviceTypeList(NSArray<NSNumber *> * deviceTypes)
{
    NSMutableArray * items = [NSMutableArray array];
    for (NSNumber * deviceType in deviceTypes) {
        [items addObject:@{MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{MTRContextTagKey : @(0),
                    MTRDataKey : UnsignedValue(deviceType.unsignedIntegerValue)},
                @{MTRContextTagKey : @(1),
                    MTRDataKey : UnsignedValue(1)},
            ],
        }}];
    }
    return @ { MTRTypeKey : MTRArrayValueType, MTRValueKey : items };
}

static NSDictionary * Report(uint16_t endpoint, uint32_t cluster, uint32_t attribute, NSDictionary * value)
{
    NSMutableDictionary * data = [value mutableCopy];
    data[MTRDataVersionKey] = @(1);
    return @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(endpoint) clusterID:@(cluster) attributeID:@(attribute)],
        MTRDataKey : data,
    };
}

static NSArray * DescriptorReport(uint16_t endpoint, NSArray<NSNumber *> * deviceTypes, NSArray<NSNumber *> * servers, NSArray<NSNumber *> * parts)
{
    return @[
        Report(endpoint, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID, DeviceTypeList(deviceTypes)),
        Report(endpoint, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributeServerListID, NumberList(servers)),
        Report(endpoint, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributePartsListID, NumberList(parts)),
    ];
}

static NSArray * ClusterLayoutReport(uint16_t endpoint, uint32_t cluster, NSArray<NSNumber *> * attributes, NSArray<NSNumber *> * commands)
{
    return @[
        Report(endpoint, cluster, MTRAttributeIDTypeGlobalAttributeAttributeListID, NumberList(attributes)),
        Report(endpoint, cluster, MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID, NumberList(commands)),
    ];
}

static NSArray<NSNumber *> * DescriptorAttributes(void)
{
    return @[ @(0), @(1), @(2), @(3), @(0xFFF9), @(0xFFFB), @(0xFFFC), @(0xFFFD) ];
}

static NSArray<NSNumber *> * OnOffAttributes(void)
{
    return @[ @(0), @(0xFFF9), @(0xFFFB), @(0xFFFC), @(0xFFFD) ];
}

static NSArray<NSNumber *> * OnOffCommands(void)
{
    return @[ @(0), @(1), @(2) ];
}

// Root (0) -> aggregator (1) -> bridged lights (2, 3), as a bridge reports them:
// the root and the aggregator both list every descendant.
static NSArray * BridgeReport(void)
{
    NSMutableArray * report = [NSMutableArray array];
    [report addObjectsFromArray:DescriptorReport(0, @[ @(kRootNodeDeviceType) ], @[ @(MTRClusterIDTypeDescriptorID) ], @[ @(1), @(2), @(3) ])];
    [report addObjectsFromArray:DescriptorReport(1, @[ @(kAggregatorDeviceType) ], @[ @(MTRClusterIDTypeDescriptorID) ], @[ @(2), @(3) ])];
    for (uint16_t endpoint = 2; endpoint <= 3; endpoint++) {
        [report addObjectsFromArray:DescriptorReport(endpoint, @[ @(kOnOffLightDeviceType), @(kBridgedNodeDeviceType) ],
                                        @[ @(MTRClusterIDTypeDescriptorID), @(MTRClusterIDTypeOnOffID) ], @[])];
        [report addObjectsFromArray:ClusterLayoutReport(endpoint, MTRClusterIDTypeDescriptorID, DescriptorAttributes(), @[])];
        [report addObjectsFromArray:ClusterLayoutReport(endpoint, MTRClusterIDTypeOnOffID, OnOffAttributes(), OnOffCommands())];
    }
    [report addObjectsFromArray:ClusterLayoutReport(0, MTRClusterIDTypeDescriptorID, DescriptorAttributes(), @[])];
    [report addObjectsFromArray:ClusterLayoutReport(1, MTRClusterIDTypeDescriptorID, DescriptorAttributes(), @[])];
    return report;
}

static NSSet<MTRAttributePath *> * AttributePaths(uint16_t endpoint, uint32_t cluster, NSArray<NSNumber *> * attributes)
{
    NSMutableSet * paths = [NSMutableSet set];
    for (NSNumber * attribute in attributes) {
        [paths addObject:[MTRAttributePath attributePathWithEndpointID:@(endpoint) clusterID:@(cluster) attributeID:attribute]];
    }
    return paths;
}

static NSSet<MTRCommandPath *> * CommandPaths(uint16_t endpoint, uint32_t cluster, NSArray<NSNumber *> * commands)
{
    NSMutableSet * paths = [NSMutableSet set];
    for (NSNumber * command in commands) {
        [paths addObject:[MTRCommandPath commandPathWithEndpointID:@(endpoint) clusterID:@(cluster) commandID:command]];
    }
    return paths;
}

@protocol MTRTestPreLayoutXPCServerProtocol <NSObject>
- (oneway void)deviceController:(NSUUID *)controller updateControllerConfiguration:(NSDictionary *)controllerState;
- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths withReply:(void (^)(NSArray<NSDictionary<NSString *, id> *> *))reply;
@end

@interface MTREndpointLayoutTestXPCServer : NSObject <NSXPCListenerDelegate>
- (instancetype)initServingLayoutOfDevice:(MTRDevice * _Nullable)device;
- (void)sendControllerContext:(NSDictionary *)context;
- (void)stopServingAndDropConnection;
@property (nonatomic, readonly) NSXPCListener * listener;
@property (atomic, strong, nullable) id malformedReply;
@property (atomic, strong, nullable) id<MTRDeviceEndpointLayout> device;
@property (atomic, readonly) NSUInteger connectionCount;
@property (atomic, readonly) NSUInteger readCount;
@property (atomic, readonly, nullable) NSArray * lastLayoutRequest;
@end

@interface MTREndpointLayoutTestXPCServer ()
@property (atomic, strong, nullable) NSXPCConnection * connection;
@property (atomic, strong, nullable) NSUUID * controllerID;
@property (atomic, readwrite) NSUInteger connectionCount;
@property (atomic, readwrite) NSUInteger readCount;
@property (atomic, readwrite, nullable) NSArray * lastLayoutRequest;
@end

@implementation MTREndpointLayoutTestXPCServer

- (instancetype)initServingLayoutOfDevice:(MTRDevice * _Nullable)device
{
    if (self = [super init]) {
        self.device = (id<MTRDeviceEndpointLayout>) device;
        _listener = [NSXPCListener anonymousListener];
        _listener.delegate = self;
        [_listener resume];
    }
    return self;
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    Protocol * protocol = self.device ? @protocol(MTRXPCServerProtocol) : @protocol(MTRTestPreLayoutXPCServerProtocol);
    NSXPCInterface * interface = [NSXPCInterface interfaceWithProtocol:protocol];
    SEL readAttributePaths = @selector(deviceController:nodeID:readAttributePaths:withReply:);
    NSSet * readClasses = [[interface classesForSelector:readAttributePaths argumentIndex:2 ofReply:NO] setByAddingObject:[MTRAttributeRequestPath class]];
    [interface setClasses:readClasses forSelector:readAttributePaths argumentIndex:2 ofReply:NO];
    newConnection.exportedInterface = interface;
    newConnection.exportedObject = self;
    newConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];
    self.connection = newConnection;
    self.connectionCount += 1;
    [newConnection activate];
    return YES;
}

- (void)sendControllerContext:(NSDictionary *)context
{
    [[self.connection remoteObjectProxy] controller:self.controllerID controllerConfigurationUpdated:@{ MTRDeviceControllerRegistrationControllerContextKey : context }];
}

- (void)stopServingAndDropConnection
{
    self.device = nil;
    [self.connection invalidate];
}

- (oneway void)deviceController:(NSUUID *)controller updateControllerConfiguration:(NSDictionary *)controllerState
{
    self.controllerID = controller;
    if (self.device) {
        [[self.connection remoteObjectProxy] controller:controller
                         controllerConfigurationUpdated:@{
                             MTRDeviceControllerRegistrationControllerContextKey : @ { kServesEndpointLayoutKey : @YES },
                         }];
    }
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths withReply:(void (^)(NSArray<NSDictionary<NSString *, id> *> *))reply
{
    self.readCount += 1;
    reply(@[]);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID getEndpointsByIdWithReply:(void (^)(NSDictionary<NSNumber *, MTREndpointInfo *> * _Nullable))reply
{
    self.lastLayoutRequest = @[ controller, nodeID ];
    reply(self.malformedReply ?: self.device.endpointsById);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID attributePathsForEndpointID:(NSNumber *)endpointID withReply:(void (^)(NSArray<MTRAttributePath *> * _Nullable))reply
{
    self.lastLayoutRequest = @[ controller, nodeID ];
    reply(self.malformedReply ?: [self.device attributePathsForEndpointID:endpointID]);
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID acceptedCommandPathsForEndpointID:(NSNumber *)endpointID withReply:(void (^)(NSArray<MTRCommandPath *> * _Nullable))reply
{
    self.lastLayoutRequest = @[ controller, nodeID ];
    reply(self.malformedReply ?: [self.device acceptedCommandPathsForEndpointID:endpointID]);
}

@end

@interface MTRDeviceEndpointLayoutTests : XCTestCase
@end

@implementation MTRDeviceEndpointLayoutTests {
    dispatch_queue_t _storageQueue;
    MTRDeviceController * _controller;
    MTRDeviceTestDelegateWithSubscriptionSetupOverride * _delegate;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];

    _storageQueue = dispatch_queue_create("test.storage.queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSError * error;
    __auto_type * root = [MTRCertificates createRootCertificate:rootKeys issuerID:@(1) fabricID:nil error:&error];
    XCTAssertNil(error);
    __auto_type * publicKey = operationalKeys.copyPublicKey;
    CFAutorelease(publicKey);
    __auto_type * operational = [MTRCertificates createOperationalCertificate:rootKeys
                                                           signingCertificate:root
                                                         operationalPublicKey:publicKey
                                                                     fabricID:@(0x5A5A)
                                                                       nodeID:@(0x1001)
                                                        caseAuthenticatedTags:nil
                                                                        error:&error];
    XCTAssertNil(error);

    __auto_type * params = [[MTRDeviceControllerExternalCertificateParameters alloc] initWithStorageDelegate:storage
                                                                                        storageDelegateQueue:_storageQueue
                                                                                            uniqueIdentifier:storage.controllerID
                                                                                                         ipk:rootKeys.ipk
                                                                                                    vendorID:@(kTestVendorId)
                                                                                          operationalKeypair:operationalKeys
                                                                                      operationalCertificate:operational
                                                                                     intermediateCertificate:nil
                                                                                             rootCertificate:root];
    params.shouldAdvertiseOperational = NO;
    _controller = [[MTRDeviceController alloc] initWithParameters:params error:&error];
    XCTAssertNil(error);
    XCTAssertTrue(_controller.running);

    _delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    _delegate.skipSetupSubscription = YES;
}

- (void)tearDown
{
    [_controller shutdown];
    _controller = nil;
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
    _storageQueue = nil;
    [super tearDown];
}

- (MTRDevice *)deviceWithNodeID:(uint64_t)nodeID
{
    MTRDevice * device = [MTRDevice deviceWithNodeID:@(nodeID) controller:_controller];
    [device setDelegate:_delegate queue:dispatch_get_main_queue()];
    return device;
}

- (void)inject:(NSArray *)report intoDevice:(MTRDevice *)device
{
    XCTestExpectation * reportEnd = [self expectationWithDescription:@"Injected report ended"];
    _delegate.onReportEnd = ^{
        [reportEnd fulfill];
    };
    [device unitTestInjectAttributeReport:report fromSubscription:YES];
    [self waitForExpectations:@[ reportEnd ] timeout:kTimeoutInSeconds];
    _delegate.onReportEnd = nil;
}

- (id<MTRDeviceEndpointLayout>)layoutOfDevice:(MTRDevice *)device
{
    BOOL serves = [device respondsToSelector:@selector(endpointsById)]
        && [device respondsToSelector:@selector(attributePathsForEndpointID:)]
        && [device respondsToSelector:@selector(acceptedCommandPathsForEndpointID:)];
    XCTAssertTrue(serves, @"MTRDevice does not serve the endpoint layout from its cache");
    return (id<MTRDeviceEndpointLayout>) device;
}

- (id<MTRDeviceEndpointLayout>)bridgeLayout
{
    MTRDevice * device = [self deviceWithNodeID:0x2001];
    [self inject:BridgeReport() intoDevice:device];
    return [self layoutOfDevice:device];
}

- (MTRDeviceController *)xpcControllerForServer:(MTREndpointLayoutTestXPCServer *)server
{
    NSXPCListenerEndpoint * endpoint = server.listener.endpoint;
    __auto_type * parameters = [[MTRXPCDeviceControllerParameters alloc] initWithXPCConnectionBlock:^NSXPCConnection * {
        return [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
    } uniqueIdentifier:[NSUUID UUID]];
    NSError * error;
    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithParameters:parameters error:&error];
    XCTAssertNotNil(controller, @"%@", error);
    return controller;
}

- (void)sendContext:(NSDictionary *)context from:(MTREndpointLayoutTestXPCServer *)server to:(MTRDeviceController *)controller
{
    NSNumber * nodeID = context[MTRDeviceControllerRegistrationControllerNodeIDKey];
    [server sendControllerContext:context];
    XCTNSPredicateExpectation * delivered = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return [controller.controllerNodeID isEqual:nodeID];
    }]
                                                                                          object:nil];
    [self waitForExpectations:@[ delivered ] timeout:kTimeoutInSeconds];
}

- (void)test001_EndpointsByIdComesFromTheDescriptorCache
{
    __auto_type * layout = [self bridgeLayout];
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = layout.endpointsById;

    XCTAssertEqualObjects([NSSet setWithArray:endpoints.allKeys], ([NSSet setWithArray:@[ @(0), @(1), @(2), @(3) ]]));
    XCTAssertEqualObjects(endpoints[@(1)].partsList, (@[ @(2), @(3) ]));
    XCTAssertEqualObjects(endpoints[@(2)].deviceTypes, (@[
        [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(kOnOffLightDeviceType) revision:@(1)],
        [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(kBridgedNodeDeviceType) revision:@(1)],
    ]));
}

- (void)test002_EndpointsByIdResolvesTheBridgeComposition
{
    __auto_type * layout = [self bridgeLayout];
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = layout.endpointsById;

    XCTAssertEqualObjects([endpoints[@(0)].children valueForKey:@"endpointID"], (@[ @(1) ]));
    XCTAssertEqualObjects([endpoints[@(1)].children valueForKey:@"endpointID"], (@[ @(2), @(3) ]));
    XCTAssertEqual(endpoints[@(2)].children.count, 0u);
}

- (void)test003_EndpointsByIdIsOneObjectForEveryCaller
{
    __auto_type * layout = [self bridgeLayout];
    NSDictionary * first = layout.endpointsById;
    XCTAssertNotNil(first);
    for (int caller = 0; caller < 30; caller++) {
        XCTAssertTrue(layout.endpointsById == first, @"caller %d got its own copy", caller);
    }
}

- (void)test004_AttributePathsAreScopedToTheEndpoint
{
    __auto_type * layout = [self bridgeLayout];
    NSMutableSet * expected = [NSMutableSet set];
    [expected unionSet:AttributePaths(2, MTRClusterIDTypeDescriptorID, DescriptorAttributes())];
    [expected unionSet:AttributePaths(2, MTRClusterIDTypeOnOffID, OnOffAttributes())];

    NSArray<MTRAttributePath *> * paths = [layout attributePathsForEndpointID:@(2)];
    XCTAssertEqual(paths.count, expected.count);
    XCTAssertEqualObjects([NSSet setWithArray:paths], expected);
}

- (void)test005_AcceptedCommandPathsAreScopedToTheEndpoint
{
    __auto_type * layout = [self bridgeLayout];
    NSArray<MTRCommandPath *> * paths = [layout acceptedCommandPathsForEndpointID:@(3)];
    XCTAssertEqual(paths.count, 3u);
    XCTAssertEqualObjects([NSSet setWithArray:paths], CommandPaths(3, MTRClusterIDTypeOnOffID, OnOffCommands()));
    XCTAssertEqualObjects([layout acceptedCommandPathsForEndpointID:@(1)], @[]);
}

- (void)test006_EndpointPathsAreOneObjectForEveryCaller
{
    __auto_type * layout = [self bridgeLayout];
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];
    NSArray * commands = [layout acceptedCommandPathsForEndpointID:@(2)];
    XCTAssertNotNil(attributes);
    XCTAssertNotNil(commands);
    for (int caller = 0; caller < 30; caller++) {
        XCTAssertTrue([layout attributePathsForEndpointID:@(2)] == attributes, @"caller %d got its own attribute paths", caller);
        XCTAssertTrue([layout acceptedCommandPathsForEndpointID:@(2)] == commands, @"caller %d got its own command paths", caller);
    }
}

- (void)test007_UnknownEndpointHasNoPaths
{
    __auto_type * layout = [self bridgeLayout];
    XCTAssertNil([layout attributePathsForEndpointID:@(9)]);
    XCTAssertNil([layout acceptedCommandPathsForEndpointID:@(9)]);
}

- (void)test008_EmptyCacheHasNoLayout
{
    MTRDevice * device = [self deviceWithNodeID:0x2002];
    __auto_type * layout = [self layoutOfDevice:device];
    XCTAssertNil(layout.endpointsById);
    XCTAssertNil([layout attributePathsForEndpointID:@(0)]);
}

- (void)test009_PartsListChangeReplacesTheEndpoints
{
    MTRDevice * device = [self deviceWithNodeID:0x2003];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSDictionary * before = layout.endpointsById;
    XCTAssertEqual(before.count, 4u);

    [self inject:@[
        Report(0, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributePartsListID, NumberList(@[ @(1), @(2), @(3), @(4) ])),
        Report(1, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributePartsListID, NumberList(@[ @(2), @(3), @(4) ])),
    ]
        intoDevice:device];

    NSDictionary<NSNumber *, MTREndpointInfo *> * after = layout.endpointsById;
    XCTAssertFalse(after == before);
    XCTAssertEqual(before.count, 4u);
    XCTAssertEqualObjects([NSSet setWithArray:after.allKeys], ([NSSet setWithArray:@[ @(0), @(1), @(2), @(3), @(4) ]]));
    XCTAssertEqualObjects([after[@(1)].children valueForKey:@"endpointID"], (@[ @(2), @(3), @(4) ]));
    XCTAssertEqualObjects(after[@(4)].deviceTypes, @[]);
}

- (void)test010_AttributeListChangeReplacesOnlyThatEndpointsPaths
{
    MTRDevice * device = [self deviceWithNodeID:0x2004];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSArray * endpoint2Before = [layout attributePathsForEndpointID:@(2)];
    NSArray * endpoint3Before = [layout attributePathsForEndpointID:@(3)];
    NSArray * endpoint2Commands = [layout acceptedCommandPathsForEndpointID:@(2)];
    XCTAssertNotNil(endpoint2Before);
    XCTAssertNotNil(endpoint3Before);

    NSMutableArray * onOffAttributes = [OnOffAttributes() mutableCopy];
    [onOffAttributes addObject:@(0x4000)];
    [self inject:@[ Report(2, MTRClusterIDTypeOnOffID, MTRAttributeIDTypeGlobalAttributeAttributeListID, NumberList(onOffAttributes)) ] intoDevice:device];

    NSArray<MTRAttributePath *> * endpoint2After = [layout attributePathsForEndpointID:@(2)];
    XCTAssertFalse(endpoint2After == endpoint2Before);
    XCTAssertTrue([endpoint2After containsObject:[MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(MTRClusterIDTypeOnOffID) attributeID:@(0x4000)]]);
    XCTAssertEqual(endpoint2After.count, endpoint2Before.count + 1);
    XCTAssertTrue([layout attributePathsForEndpointID:@(3)] == endpoint3Before);
    XCTAssertTrue([layout acceptedCommandPathsForEndpointID:@(2)] == endpoint2Commands);
}

- (void)test011_ServerListChangeDropsTheRemovedClustersPaths
{
    MTRDevice * device = [self deviceWithNodeID:0x2005];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    XCTAssertEqual([layout acceptedCommandPathsForEndpointID:@(2)].count, 3u);
    XCTAssertEqual([layout attributePathsForEndpointID:@(2)].count, DescriptorAttributes().count + OnOffAttributes().count);
    NSDictionary * endpoints = layout.endpointsById;
    NSArray * endpoint3Attributes = [layout attributePathsForEndpointID:@(3)];
    NSArray * endpoint3Commands = [layout acceptedCommandPathsForEndpointID:@(3)];

    [self inject:@[ Report(2, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributeServerListID, NumberList(@[ @(MTRClusterIDTypeDescriptorID) ])) ] intoDevice:device];

    XCTAssertEqualObjects([layout acceptedCommandPathsForEndpointID:@(2)], @[]);
    XCTAssertEqualObjects([NSSet setWithArray:[layout attributePathsForEndpointID:@(2)]], AttributePaths(2, MTRClusterIDTypeDescriptorID, DescriptorAttributes()));
    XCTAssertTrue(layout.endpointsById == endpoints);
    XCTAssertTrue([layout attributePathsForEndpointID:@(3)] == endpoint3Attributes);
    XCTAssertTrue([layout acceptedCommandPathsForEndpointID:@(3)] == endpoint3Commands);
}

- (void)test012_ReturnedLayoutIsNotMutatedByLaterReports
{
    MTRDevice * device = [self deviceWithNodeID:0x2006];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSArray * held = [layout attributePathsForEndpointID:@(2)];
    NSSet * heldContents = [NSSet setWithArray:held];

    [self inject:@[ Report(2, MTRClusterIDTypeOnOffID, MTRAttributeIDTypeGlobalAttributeAttributeListID, NumberList(@[ @(0) ])) ] intoDevice:device];

    XCTAssertEqualObjects([NSSet setWithArray:held], heldContents);
    XCTAssertFalse([held isKindOfClass:[NSMutableArray class]]);
}

- (void)test013_XPCClientGetsTheSameLayout
{
    MTRDevice * device = [self deviceWithNodeID:0x2007];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * local = [self layoutOfDevice:device];

    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x2007) controller:xpcController]];

    [self waitForLayoutOf:remote];

    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = remote.endpointsById;
    XCTAssertEqualObjects(endpoints, local.endpointsById);
    XCTAssertEqualObjects(server.lastLayoutRequest, (@[ xpcController.uniqueIdentifier, @(0x2007) ]));
    XCTAssertEqualObjects([endpoints[@(1)].children valueForKey:@"endpointID"], (@[ @(2), @(3) ]));
    XCTAssertEqualObjects(endpoints[@(2)].deviceTypes, local.endpointsById[@(2)].deviceTypes);

    server.lastLayoutRequest = nil;
    NSArray<MTRAttributePath *> * attributes = [remote attributePathsForEndpointID:@(2)];
    XCTAssertEqualObjects(server.lastLayoutRequest, (@[ xpcController.uniqueIdentifier, @(0x2007) ]));
    XCTAssertEqual(attributes.count, DescriptorAttributes().count + OnOffAttributes().count);
    XCTAssertEqualObjects(attributes, [local attributePathsForEndpointID:@(2)]);
    server.lastLayoutRequest = nil;
    NSArray<MTRCommandPath *> * commands = [remote acceptedCommandPathsForEndpointID:@(3)];
    XCTAssertEqualObjects(server.lastLayoutRequest, (@[ xpcController.uniqueIdentifier, @(0x2007) ]));
    XCTAssertEqual(commands.count, OnOffCommands().count);
    XCTAssertEqualObjects(commands, [local acceptedCommandPathsForEndpointID:@(3)]);
    XCTAssertNil([remote attributePathsForEndpointID:@(9)]);

    [xpcController shutdown];
}

// A server built before these calls existed exports no such selectors and never sets
// kServesEndpointLayoutKey: the client answers nil and sends nothing that drops its connection.
- (void)test014_ServerWithoutLayoutSupportGivesNil
{
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:nil];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    MTRDevice * remoteDevice = [MTRDevice deviceWithNodeID:@(0x2008) controller:xpcController];
    __auto_type * remote = [self layoutOfDevice:remoteDevice];

    XCTAssertNil(remote.endpointsById);
    XCTAssertNil([remote attributePathsForEndpointID:@(0)]);
    XCTAssertNil([remote acceptedCommandPathsForEndpointID:@(0)]);

    __auto_type * path = [MTRAttributeRequestPath requestPathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:nil];
    [remoteDevice readAttributePaths:@[ path ]];
    XCTAssertEqual(server.readCount, 1u);

    XCTestExpectation * noReconnect = [self expectationWithDescription:@"No reconnect"];
    noReconnect.inverted = YES;
    [self waitForExpectations:@[ noReconnect ] timeout:1];
    XCTAssertEqual(server.connectionCount, 1u);

    [xpcController shutdown];
}

- (void)test015_AcceptedCommandListChangeReplacesOnlyThatEndpointsCommandPaths
{
    MTRDevice * device = [self deviceWithNodeID:0x2009];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSArray * commandsBefore = [layout acceptedCommandPathsForEndpointID:@(2)];
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];
    NSArray * endpoint3Commands = [layout acceptedCommandPathsForEndpointID:@(3)];
    NSDictionary * endpoints = layout.endpointsById;

    [self inject:@[ Report(2, MTRClusterIDTypeOnOffID, MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID, NumberList(@[ @(0), @(1), @(2), @(0x40) ])) ] intoDevice:device];

    NSArray<MTRCommandPath *> * commandsAfter = [layout acceptedCommandPathsForEndpointID:@(2)];
    XCTAssertFalse(commandsAfter == commandsBefore);
    XCTAssertEqualObjects([NSSet setWithArray:commandsAfter], CommandPaths(2, MTRClusterIDTypeOnOffID, @[ @(0), @(1), @(2), @(0x40) ]));
    XCTAssertTrue([layout attributePathsForEndpointID:@(2)] == attributes);
    XCTAssertTrue([layout acceptedCommandPathsForEndpointID:@(3)] == endpoint3Commands);
    XCTAssertTrue(layout.endpointsById == endpoints);
}

- (void)test016_DeviceTypeListChangeReplacesTheEndpoints
{
    MTRDevice * device = [self deviceWithNodeID:0x200A];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSDictionary * before = layout.endpointsById;
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];

    [self inject:@[ Report(2, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID, DeviceTypeList(@[ @(kBridgedNodeDeviceType) ])) ] intoDevice:device];

    NSDictionary<NSNumber *, MTREndpointInfo *> * after = layout.endpointsById;
    XCTAssertFalse(after == before);
    XCTAssertEqualObjects(after[@(2)].deviceTypes, (@[ [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(kBridgedNodeDeviceType) revision:@(1)] ]));
    XCTAssertEqualObjects(after[@(2)].partsList, @[]);
    XCTAssertTrue([layout attributePathsForEndpointID:@(2)] == attributes);
}

// Attributes 0, 1 and 3 of a cluster other than Descriptor share IDs with DeviceTypeList,
// ServerList and PartsList, and must leave the layout alone.
- (void)test017_SameIDsOnOtherClustersKeepTheLayout
{
    MTRDevice * device = [self deviceWithNodeID:0x200B];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSDictionary * endpoints = layout.endpointsById;
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];
    NSArray * commands = [layout acceptedCommandPathsForEndpointID:@(2)];

    [self inject:@[
        Report(2, MTRClusterIDTypeOnOffID, 0, @{ MTRTypeKey : MTRBooleanValueType, MTRValueKey : @YES }),
        Report(2, MTRClusterIDTypeOnOffID, 1, NumberList(@[ @(5) ])),
        Report(2, MTRClusterIDTypeOnOffID, 3, NumberList(@[ @(7) ])),
    ]
        intoDevice:device];

    XCTAssertTrue(layout.endpointsById == endpoints);
    XCTAssertTrue([layout attributePathsForEndpointID:@(2)] == attributes);
    XCTAssertTrue([layout acceptedCommandPathsForEndpointID:@(2)] == commands);
}

- (void)test018_PersistedClusterDataReplacesTheLayout
{
    MTRDevice * device = [self deviceWithNodeID:0x200C];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSDictionary * endpoints = layout.endpointsById;
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];

    __auto_type * onOff = [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(MTRClusterIDTypeOnOffID)];
    NSMutableArray * persistedAttributes = [OnOffAttributes() mutableCopy];
    [persistedAttributes addObject:@(0x4000)];
    __auto_type * data = [[MTRDeviceClusterData alloc] initWithDataVersion:@(2) attributes:@{ @(MTRAttributeIDTypeGlobalAttributeAttributeListID) : NumberList(persistedAttributes) }];
    [device setPersistedClusterData:@{ onOff : data }];

    NSDictionary<NSNumber *, MTREndpointInfo *> * endpointsAfter = layout.endpointsById;
    XCTAssertFalse(endpointsAfter == endpoints);
    XCTAssertEqualObjects(endpointsAfter, endpoints);
    XCTAssertEqualObjects([endpointsAfter[@(1)].children valueForKey:@"endpointID"], (@[ @(2), @(3) ]));
    NSArray<MTRAttributePath *> * attributesAfter = [layout attributePathsForEndpointID:@(2)];
    XCTAssertFalse(attributesAfter == attributes);
    NSMutableSet * expected = [NSMutableSet setWithArray:attributes];
    [expected addObject:[MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(MTRClusterIDTypeOnOffID) attributeID:@(0x4000)]];
    XCTAssertEqualObjects([NSSet setWithArray:attributesAfter], expected);
}

- (void)test019_XPCClientRejectsMalformedReplies
{
    MTRDevice * device = [self deviceWithNodeID:0x200D];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x200D) controller:xpcController]];
    [self waitForLayoutOf:remote];

    server.malformedReply = @[ @(1) ];
    XCTAssertNil([remote attributePathsForEndpointID:@(2)]);
    XCTAssertNil([remote acceptedCommandPathsForEndpointID:@(2)]);
    server.malformedReply = @{ @"key" : @(1) };
    XCTAssertNil(remote.endpointsById);
    XCTAssertNil([remote attributePathsForEndpointID:@(2)]);
    server.malformedReply = @{ @(2) : @"endpoint" };
    XCTAssertNil(remote.endpointsById);
    server.malformedReply = @{ @"0" : [(id<MTRDeviceEndpointLayout>) device endpointsById][@(0)] };
    XCTAssertNil(remote.endpointsById);
    server.malformedReply = @[ @"path" ];
    XCTAssertNil(remote.endpointsById);
    server.malformedReply = nil;
    XCTAssertNotNil([remote attributePathsForEndpointID:@(2)]);

    [xpcController shutdown];
}

- (void)test020_XPCSupportFollowsTheServer
{
    MTRDevice * device = [self deviceWithNodeID:0x200E];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x200E) controller:xpcController]];
    [self waitForLayoutOf:remote];

    [server sendControllerContext:@{ MTRDeviceControllerRegistrationControllerNodeIDKey : @(0x5A) }];
    XCTNSPredicateExpectation * delivered = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return [xpcController.controllerNodeID isEqual:@(0x5A)];
    }]
                                                                                          object:nil];
    [self waitForExpectations:@[ delivered ] timeout:kTimeoutInSeconds];
    XCTAssertNotNil(remote.endpointsById);

    [server sendControllerContext:@{ kServesEndpointLayoutKey : @NO }];
    XCTNSPredicateExpectation * withdrawn = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return remote.endpointsById == nil;
    }]
                                                                                          object:nil];
    [self waitForExpectations:@[ withdrawn ] timeout:kTimeoutInSeconds];

    [xpcController shutdown];
}

- (void)test021_ReconnectForgetsXPCSupport
{
    MTRDevice * device = [self deviceWithNodeID:0x200F];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x200F) controller:xpcController]];
    [self waitForLayoutOf:remote];

    [server stopServingAndDropConnection];
    XCTNSPredicateExpectation * reconnected = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return server.connectionCount == 2;
    }]
                                                                                            object:nil];
    [self waitForExpectations:@[ reconnected ] timeout:kTimeoutInSeconds];

    XCTAssertNil(remote.endpointsById);
    XCTAssertNil([remote attributePathsForEndpointID:@(2)]);
    XCTestExpectation * noReconnect = [self expectationWithDescription:@"No further reconnect"];
    noReconnect.inverted = YES;
    [self waitForExpectations:@[ noReconnect ] timeout:1];
    XCTAssertEqual(server.connectionCount, 2u);

    [xpcController shutdown];
}

- (void)waitForLayoutOf:(id<MTRDeviceEndpointLayout>)remote
{
    XCTNSPredicateExpectation * served = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return remote.endpointsById != nil;
    }]
                                                                                       object:nil];
    [self waitForExpectations:@[ served ] timeout:kTimeoutInSeconds];
}

- (void)test022_RepeatedIDsAreListedOnce
{
    MTRDevice * device = [self deviceWithNodeID:0x2010];
    NSMutableArray * report = [BridgeReport() mutableCopy];
    [report addObject:Report(0, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributePartsListID, NumberList(@[ @(1), @(2), @(2), @(3), @(0xFFFF), @(0x10000) ]))];
    [report addObject:Report(2, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributeServerListID, NumberList(@[ @(MTRClusterIDTypeDescriptorID), @(MTRClusterIDTypeOnOffID), @(MTRClusterIDTypeOnOffID) ]))];
    [report addObject:Report(2, MTRClusterIDTypeOnOffID, MTRAttributeIDTypeGlobalAttributeAttributeListID, NumberList(@[ @(0), @(0), @(0xFFFD) ]))];
    [report addObject:Report(2, MTRClusterIDTypeOnOffID, MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID, NumberList(@[ @(1), @(1) ]))];
    [self inject:report intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];

    XCTAssertEqualObjects([NSSet setWithArray:layout.endpointsById.allKeys], ([NSSet setWithArray:@[ @(0), @(1), @(2), @(3) ]]));
    XCTAssertNil([layout attributePathsForEndpointID:@(0x10000)]);
    XCTAssertNil([layout attributePathsForEndpointID:@(0xFFFF)]);
    NSArray<MTRAttributePath *> * attributes = [layout attributePathsForEndpointID:@(2)];
    XCTAssertEqual(attributes.count, [NSSet setWithArray:attributes].count);
    XCTAssertEqual(attributes.count, DescriptorAttributes().count + 2);
    XCTAssertEqualObjects([layout acceptedCommandPathsForEndpointID:@(2)], (@[ [MTRCommandPath commandPathWithEndpointID:@(2) clusterID:@(MTRClusterIDTypeOnOffID) commandID:@(1)] ]));
}

- (void)test023_RemovedClusterReplacesThatEndpointsPaths
{
    MTRDevice * device = [self deviceWithNodeID:0x2011];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSDictionary * endpoints = layout.endpointsById;
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];
    NSArray * commands = [layout acceptedCommandPathsForEndpointID:@(2)];
    NSArray * endpoint3Attributes = [layout attributePathsForEndpointID:@(3)];

    [device unitTestRemoveClusters:[NSSet setWithObject:[MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(MTRClusterIDTypeOnOffID)]]];

    XCTAssertTrue(layout.endpointsById == endpoints);
    XCTAssertFalse([layout attributePathsForEndpointID:@(2)] == attributes);
    XCTAssertFalse([layout acceptedCommandPathsForEndpointID:@(2)] == commands);
    XCTAssertTrue([layout attributePathsForEndpointID:@(3)] == endpoint3Attributes);

    [device unitTestRemoveClusters:[NSSet setWithObject:[MTRClusterPath clusterPathWithEndpointID:@(3) clusterID:@(MTRClusterIDTypeDescriptorID)]]];
    XCTAssertFalse(layout.endpointsById == endpoints);
}

- (void)test024_ClustersMissingFromStorageReplaceTheLayout
{
    MTRDevice * device = [self deviceWithNodeID:0x2012];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSDictionary * endpoints = layout.endpointsById;
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];
    NSArray * endpoint3Attributes = [layout attributePathsForEndpointID:@(3)];

    id<MTRDeviceEndpointLayoutTestDataStore> dataStore = (id<MTRDeviceEndpointLayoutTestDataStore>) _controller.controllerDataStore;
    [dataStore clearStoredClusterDataForNodeID:@(0x2012) endpointID:@(2) clusterID:@(MTRClusterIDTypeDescriptorID)];
    [dataStore synchronouslyPerformBlock:nil];
    [device unitTestReconcilePersistedClustersWithStorage];

    XCTAssertFalse(layout.endpointsById == endpoints);
    XCTAssertFalse([layout attributePathsForEndpointID:@(2)] == attributes);
    XCTAssertTrue([layout attributePathsForEndpointID:@(3)] == endpoint3Attributes);
}

- (void)test025_RemovedAttributeListReplacesThatEndpointsPaths
{
    MTRDevice * device = [self deviceWithNodeID:0x2013];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSDictionary * endpoints = layout.endpointsById;
    NSArray * attributes = [layout attributePathsForEndpointID:@(2)];
    NSArray * commands = [layout acceptedCommandPathsForEndpointID:@(2)];

    [self inject:@[ Report(2, MTRClusterIDTypeOnOffID, 0, @{ MTRTypeKey : MTRBooleanValueType, MTRValueKey : @YES }) ] intoDevice:device];
    XCTAssertTrue([layout attributePathsForEndpointID:@(2)] == attributes);

    __auto_type * onOff = [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(MTRClusterIDTypeOnOffID)];
    [device unitTestRemoveCachedAttribute:@(MTRAttributeIDTypeGlobalAttributeAttributeListID) fromCluster:onOff];

    XCTAssertTrue(layout.endpointsById == endpoints);
    NSArray<MTRAttributePath *> * attributesAfter = [layout attributePathsForEndpointID:@(2)];
    XCTAssertFalse(attributesAfter == attributes);
    XCTAssertEqualObjects([NSSet setWithArray:attributesAfter], AttributePaths(2, MTRClusterIDTypeDescriptorID, DescriptorAttributes()));
    XCTAssertTrue([layout acceptedCommandPathsForEndpointID:@(2)] == commands);
}

- (void)test026_EndpointThatLeavesTheLayoutGetsFreshPathsWhenItReturns
{
    MTRDevice * device = [self deviceWithNodeID:0x2014];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];
    NSArray * attributes = [layout attributePathsForEndpointID:@(3)];
    XCTAssertNotNil(attributes);

    [self inject:@[ Report(0, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributePartsListID, NumberList(@[ @(1), @(2) ])) ] intoDevice:device];
    XCTAssertNil([layout attributePathsForEndpointID:@(3)]);

    [self inject:@[ Report(0, MTRClusterIDTypeDescriptorID, MTRAttributeIDTypeClusterDescriptorAttributePartsListID, NumberList(@[ @(1), @(2), @(3) ])) ] intoDevice:device];
    NSArray * attributesAfter = [layout attributePathsForEndpointID:@(3)];
    XCTAssertNotNil(attributesAfter);
    XCTAssertFalse(attributesAfter == attributes);
}

// A build that pages in a cluster storage no longer has sees the cache change under it, and is not kept.
- (void)test027_LayoutBuiltAcrossAStorageMissIsRebuilt
{
    MTRDevice * device = [self deviceWithNodeID:0x2015];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];

    id<MTRDeviceEndpointLayoutTestDataStore> dataStore = (id<MTRDeviceEndpointLayoutTestDataStore>) _controller.controllerDataStore;
    [dataStore clearStoredClusterDataForNodeID:@(0x2015) endpointID:@(2) clusterID:@(MTRClusterIDTypeOnOffID)];
    [dataStore synchronouslyPerformBlock:nil];
    [device unitTestClearClusterData];

    NSArray * acrossTheMiss = [layout attributePathsForEndpointID:@(2)];
    NSArray * rebuilt = [layout attributePathsForEndpointID:@(2)];
    XCTAssertFalse(rebuilt == acrossTheMiss);
    XCTAssertTrue([layout attributePathsForEndpointID:@(2)] == rebuilt);
    XCTAssertEqualObjects([NSSet setWithArray:rebuilt], AttributePaths(2, MTRClusterIDTypeDescriptorID, DescriptorAttributes()));
}

- (void)test028_EndpointsBuiltAcrossAStorageMissAreRebuilt
{
    MTRDevice * device = [self deviceWithNodeID:0x2016];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * layout = [self layoutOfDevice:device];

    id<MTRDeviceEndpointLayoutTestDataStore> dataStore = (id<MTRDeviceEndpointLayoutTestDataStore>) _controller.controllerDataStore;
    [dataStore clearStoredClusterDataForNodeID:@(0x2016) endpointID:@(3) clusterID:@(MTRClusterIDTypeDescriptorID)];
    [dataStore synchronouslyPerformBlock:nil];
    [device unitTestClearClusterData];

    NSDictionary * acrossTheMiss = layout.endpointsById;
    NSDictionary * rebuilt = layout.endpointsById;
    XCTAssertNotNil(rebuilt);
    XCTAssertFalse(rebuilt == acrossTheMiss);
    XCTAssertTrue(layout.endpointsById == rebuilt);
}

- (void)test029_SupportIsTiedToTheConnectionItArrivedOn
{
    MTRDevice * device = [self deviceWithNodeID:0x2017];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x2017) controller:xpcController]];
    [self waitForLayoutOf:remote];

    __attribute__((objc_precise_lifetime)) NSXPCConnection * served = xpcController.xpcConnection;
    XCTAssertNotNil(served);
    NSXPCConnection * replacement = [[NSXPCConnection alloc] initWithListenerEndpoint:server.listener.endpoint];
    [xpcController unitTestReplaceXPCConnection:replacement];

    XCTAssertNil(remote.endpointsById);
    XCTAssertNil([remote attributePathsForEndpointID:@(2)]);

    [xpcController unitTestReplaceXPCConnection:served];
    XCTAssertNotNil(remote.endpointsById);

    [replacement activate];
    [replacement invalidate];
    [xpcController shutdown];
}

// A context from a connection that is no longer the current one cannot change support.
- (void)test030_ContextFromAnotherConnectionIsIgnored
{
    MTRDevice * device = [self deviceWithNodeID:0x2018];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x2018) controller:xpcController]];
    [self waitForLayoutOf:remote];

    __attribute__((objc_precise_lifetime)) NSXPCConnection * served = xpcController.xpcConnection;
    NSXPCConnection * replacement = [[NSXPCConnection alloc] initWithListenerEndpoint:server.listener.endpoint];
    [xpcController unitTestReplaceXPCConnection:replacement];

    [server sendControllerContext:@{ kServesEndpointLayoutKey : @NO, MTRDeviceControllerRegistrationControllerNodeIDKey : @(0x5B) }];
    XCTNSPredicateExpectation * delivered = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return [xpcController.controllerNodeID isEqual:@(0x5B)];
    }]
                                                                                          object:nil];
    [self waitForExpectations:@[ delivered ] timeout:kTimeoutInSeconds];

    [xpcController unitTestReplaceXPCConnection:served];
    XCTAssertNotNil(remote.endpointsById);

    [replacement activate];
    [replacement invalidate];
    [xpcController shutdown];
}

// After a reconnect, a server that sends the key on the new connection is served again.
- (void)test031_NewConnectionWithSupportIsServed
{
    MTRDevice * device = [self deviceWithNodeID:0x2019];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x2019) controller:xpcController]];
    [self waitForLayoutOf:remote];

    [server.connection invalidate];
    XCTNSPredicateExpectation * reconnected = [[XCTNSPredicateExpectation alloc] initWithPredicate:[NSPredicate predicateWithBlock:^BOOL(id object, NSDictionary * bindings) {
        return server.connectionCount == 2;
    }]
                                                                                            object:nil];
    [self waitForExpectations:@[ reconnected ] timeout:kTimeoutInSeconds];
    [self waitForLayoutOf:remote];
    XCTAssertEqualObjects(remote.endpointsById, [(id<MTRDeviceEndpointLayout>) device endpointsById]);

    [xpcController shutdown];
}

- (void)test032_XPCClientRejectsEndpointUnderAnotherID
{
    MTRDevice * device = [self deviceWithNodeID:0x201A];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x201A) controller:xpcController]];
    [self waitForLayoutOf:remote];

    NSDictionary<NSNumber *, MTREndpointInfo *> * local = [(id<MTRDeviceEndpointLayout>) device endpointsById];
    NSMutableDictionary<NSNumber *, MTREndpointInfo *> * misfiled = [local mutableCopy];
    misfiled[@(3)] = local[@(2)];
    server.malformedReply = misfiled;
    XCTAssertNil(remote.endpointsById);
    server.malformedReply = @{};
    XCTAssertEqualObjects(remote.endpointsById, @{});
    server.malformedReply = nil;
    XCTAssertEqualObjects(remote.endpointsById, local);

    [xpcController shutdown];
}

- (void)test033_XPCClientRejectsPathsForAnotherEndpoint
{
    MTRDevice * device = [self deviceWithNodeID:0x201B];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x201B) controller:xpcController]];
    [self waitForLayoutOf:remote];

    id<MTRDeviceEndpointLayout> local = (id<MTRDeviceEndpointLayout>) device;
    NSArray<MTRAttributePath *> * attributes = [local attributePathsForEndpointID:@(2)];
    NSArray<MTRCommandPath *> * commands = [local acceptedCommandPathsForEndpointID:@(2)];
    NSMutableArray * mixedAttributes = [attributes mutableCopy];
    [mixedAttributes insertObject:[local attributePathsForEndpointID:@(3)].firstObject atIndex:1];
    NSMutableArray * mixedCommands = [commands mutableCopy];
    [mixedCommands insertObject:[local acceptedCommandPathsForEndpointID:@(3)].firstObject atIndex:1];

    server.malformedReply = mixedAttributes;
    XCTAssertNil([remote attributePathsForEndpointID:@(2)]);
    server.malformedReply = mixedCommands;
    XCTAssertNil([remote acceptedCommandPathsForEndpointID:@(2)]);
    server.malformedReply = @[];
    XCTAssertEqualObjects([remote attributePathsForEndpointID:@(2)], @[]);
    XCTAssertEqualObjects([remote acceptedCommandPathsForEndpointID:@(2)], @[]);
    server.malformedReply = nil;
    XCTAssertEqualObjects([remote attributePathsForEndpointID:@(2)], attributes);
    XCTAssertEqualObjects([remote acceptedCommandPathsForEndpointID:@(2)], commands);

    [xpcController shutdown];
}

// A support claim from a connection that is no longer the current one cannot turn support on.
- (void)test034_SupportFromAnotherConnectionIsIgnored
{
    MTRDevice * device = [self deviceWithNodeID:0x201C];
    [self inject:BridgeReport() intoDevice:device];
    __auto_type * server = [[MTREndpointLayoutTestXPCServer alloc] initServingLayoutOfDevice:device];
    MTRDeviceController * xpcController = [self xpcControllerForServer:server];
    __auto_type * remote = [self layoutOfDevice:[MTRDevice deviceWithNodeID:@(0x201C) controller:xpcController]];
    [self waitForLayoutOf:remote];

    [self sendContext:@{ kServesEndpointLayoutKey : @NO, MTRDeviceControllerRegistrationControllerNodeIDKey : @(0x5C) } from:server to:xpcController];
    XCTAssertNil(remote.endpointsById);

    __attribute__((objc_precise_lifetime)) NSXPCConnection * served = xpcController.xpcConnection;
    NSXPCConnection * replacement = [[NSXPCConnection alloc] initWithListenerEndpoint:server.listener.endpoint];
    [xpcController unitTestReplaceXPCConnection:replacement];
    [self sendContext:@{ kServesEndpointLayoutKey : @YES, MTRDeviceControllerRegistrationControllerNodeIDKey : @(0x5D) } from:server to:xpcController];

    [xpcController unitTestReplaceXPCConnection:served];
    XCTAssertNil(remote.endpointsById);

    [replacement activate];
    [replacement invalidate];
    [xpcController shutdown];
}

@end

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

#import "MTREndpointInfo_Test.h"
#import <Matter/Matter.h>

#import <XCTest/XCTest.h>
#import <malloc/malloc.h>
#if __has_feature(address_sanitizer)
#import <sanitizer/allocator_interface.h>
#endif

#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceTestDelegate.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static const NSUInteger kBridgeAdapters = 17;
static const NSUInteger kBridgeEndpoints = 18;
static const NSUInteger kBridgeNodePathCount = 144;
static const NSUInteger kLightAttributes = 12;
static const NSUInteger kLightCommands = 4;
static NSNumber * const kBridgeNodeID = @(0x5EED);

static MTRDeviceDataValueDictionary UnsignedList(NSArray<NSNumber *> * numbers)
{
    NSMutableArray * items = [NSMutableArray array];
    for (NSNumber * number in numbers) {
        [items addObject:@{MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : number }}];
    }
    return @ { MTRTypeKey : MTRArrayValueType, MTRValueKey : items };
}

static size_t HeapBytesInUse(void)
{
#if __has_feature(address_sanitizer)
    return __sanitizer_get_current_allocated_bytes();
#else
    malloc_statistics_t stats;
    malloc_zone_statistics(NULL, &stats);
    return stats.size_in_use;
#endif
}

static NSArray<NSNumber *> * Range(NSUInteger start, NSUInteger end)
{
    NSMutableArray * numbers = [NSMutableArray array];
    for (NSUInteger number = start; number < end; number++) {
        [numbers addObject:@(number)];
    }
    return numbers;
}

static NSArray<NSNumber *> * LightClusters(void)
{
    return @[ @(MTRClusterIDTypeDescriptorID), @(MTRClusterIDTypeIdentifyID), @(MTRClusterIDTypeGroupsID), @(MTRClusterIDTypeOnOffID),
        @(MTRClusterIDTypeLevelControlID), @(MTRClusterIDTypeColorControlID), @(MTRClusterIDTypeBridgedDeviceBasicInformationID) ];
}

static NSDictionary<MTRClusterPath *, NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> *> * BridgeClusters(NSArray<NSNumber *> * serverList, NSUInteger attributes, NSUInteger commands)
{
    NSArray * attributeList = [Range(0, attributes - 2) arrayByAddingObjectsFromArray:@[ @(MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID), @(MTRAttributeIDTypeGlobalAttributeAttributeListID) ]];
    NSDictionary * globals = @{
        @(MTRAttributeIDTypeGlobalAttributeAttributeListID) : UnsignedList(attributeList),
        @(MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID) : UnsignedList(Range(0, commands)),
    };
    NSMutableDictionary * clusters = [NSMutableDictionary dictionary];
    for (NSUInteger endpoint = 0; endpoint < kBridgeEndpoints; endpoint++) {
        for (NSNumber * cluster in serverList) {
            clusters[[MTRClusterPath clusterPathWithEndpointID:@(endpoint) clusterID:cluster]] = globals;
        }
        NSMutableDictionary * descriptor = [globals mutableCopy];
        descriptor[@(MTRAttributeIDTypeClusterDescriptorAttributePartsListID)] = UnsignedList(endpoint == 0 ? Range(1, kBridgeEndpoints) : @[]);
        descriptor[@(MTRAttributeIDTypeClusterDescriptorAttributeServerListID)] = UnsignedList(serverList);
        clusters[[MTRClusterPath clusterPathWithEndpointID:@(endpoint) clusterID:@(MTRClusterIDTypeDescriptorID)]] = descriptor;
    }
    return clusters;
}

static NSArray<NSNumber *> * ListValue(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, MTRAttributeIDType attribute)
{
    NSMutableArray<NSNumber *> * numbers = [NSMutableArray array];
    for (NSDictionary * item in [device readAttributeWithEndpointID:endpoint clusterID:cluster attributeID:@(attribute) params:nil][MTRValueKey]) {
        [numbers addObject:item[MTRDataKey][MTRValueKey]];
    }
    return numbers;
}

static NSArray<MTRClusterPath *> * FetchNodePaths(MTRDevice * device)
{
    NSNumber * descriptor = @(MTRClusterIDTypeDescriptorID);
    NSArray<NSNumber *> * endpoints = [@[ @0 ] arrayByAddingObjectsFromArray:ListValue(device, @0, descriptor, MTRAttributeIDTypeClusterDescriptorAttributePartsListID)];
    NSMutableArray<MTRClusterPath *> * paths = [NSMutableArray array];
    for (NSNumber * endpoint in endpoints) {
        for (NSNumber * cluster in ListValue(device, endpoint, descriptor, MTRAttributeIDTypeClusterDescriptorAttributeServerListID)) {
            for (NSNumber * attribute in ListValue(device, endpoint, cluster, MTRAttributeIDTypeGlobalAttributeAttributeListID)) {
                [paths addObject:[MTRAttributePath attributePathWithEndpointID:endpoint clusterID:cluster attributeID:attribute]];
            }
            for (NSNumber * command in ListValue(device, endpoint, cluster, MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID)) {
                [paths addObject:[MTRCommandPath commandPathWithEndpointID:endpoint clusterID:cluster commandID:command]];
            }
        }
    }
    return paths;
}

@protocol MTRPathEchoProtocol
- (void)echoPath:(MTRAttributePath *)path reply:(void (^)(MTRAttributePath * path))reply;
@end

@interface MTRPathEcho : NSObject <MTRPathEchoProtocol, NSXPCListenerDelegate>
@property (atomic, strong) MTRAttributePath * received;
@end

@implementation MTRPathEcho
- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)connection
{
    connection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRPathEchoProtocol)];
    connection.exportedObject = self;
    [connection resume];
    return YES;
}

- (void)echoPath:(MTRAttributePath *)path reply:(void (^)(MTRAttributePath * path))reply
{
    self.received = path;
    reply(path);
}
@end

@interface MTRTestAttributePathSubclass : MTRAttributePath
@end

@implementation MTRTestAttributePathSubclass
@end

@interface MTREndpointInfoTests : XCTestCase
@end

@implementation MTREndpointInfoTests

static MTREndpointInfo * MakeEndpoint(NSNumber * endpointID, NSArray<NSNumber *> * parts)
{
    return [[MTREndpointInfo alloc] initWithEndpointID:endpointID deviceTypes:@[] partsList:parts];
}

static NSArray<NSNumber *> * ChildEndpointIDs(MTREndpointInfo * endpoint)
{
    return [[endpoint.children valueForKey:@"endpointID"] sortedArrayUsingSelector:@selector(compare:)];
}

static NSArray<NSNumber *> * Exclude(NSArray<NSNumber *> * numbers, NSNumber * numberToExclude)
{
    NSMutableArray * result = [numbers mutableCopy];
    [result removeObject:numberToExclude];
    return result;
}

- (NSDictionary<NSNumber *, MTREndpointInfo *> *)indexEndpoints:(NSArray<MTREndpointInfo *> *)endpoints
{
    NSMutableDictionary * indexed = [[NSMutableDictionary alloc] init];
    for (MTREndpointInfo * endpoint in endpoints) {
        indexed[endpoint.endpointID] = endpoint;
    }
    XCTAssertEqual(indexed.count, endpoints.count, @"Duplicate endpoint IDs");
    return indexed;
}

- (void)testPopulateChildren
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[]),
    ]];
    XCTAssertTrue([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@1]), (@[ @2, @3 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@4]), (@[ @5 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@5]), (@[ @6 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@6]), (@[]));
}

- (void)testPopulateChildren2
{
    // Same as testPopulateChildren, but with reversed PartsLists
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @6, @5, @4, @3, @2, @1 ]), // full-family pattern
        MakeEndpoint(@1, @[ @3, @2 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @6, @5 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[]),
    ]];
    XCTAssertTrue([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@1]), (@[ @2, @3 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@4]), (@[ @5 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@5]), (@[ @6 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@6]), (@[]));
}

- (void)testPopulateChildrenRootOnly
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[]),
    ]];
    XCTAssertTrue([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[]));
}

- (void)testPopulateChildrenInvalidCompositionCycle
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[ @4 ]), // not valid per spec: cycle 4 -> 5 -> 6 -> 4
    ]];
    XCTAssertFalse([MTREndpointInfo populateChildrenForEndpoints:endpoints]);
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@1]), (@[ @2, @3 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    // We make no promises about child lists for endpoints involved in a cycle
}

- (void)testPopulateChildrenInvalidNonTree
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3, @6 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[]), // not valid per spec: 6 is a child of both 1 and 5
    ]];
    // Note: Not asserting a false return value here, this scenario is currently not detected.
    [MTREndpointInfo populateChildrenForEndpoints:endpoints];
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@0]), (@[ @1, @4 ]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@2]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@3]), (@[]));
    XCTAssertEqualObjects(ChildEndpointIDs(endpoints[@4]), (@[ @5 ]));
    // Endpoint 6 has multiple parents, so we make no guarantees where (or if) it shows up
    XCTAssertEqualObjects(Exclude(ChildEndpointIDs(endpoints[@1]), @6), (@[ @2, @3 ]));
    XCTAssertEqualObjects(Exclude(ChildEndpointIDs(endpoints[@5]), @6), (@[]));
}

- (void)testEqualityAndCopying
{
    MTRDeviceTypeRevision * doorLock = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@0x0A revision:@1];
    MTRDeviceTypeRevision * rootNode = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@0x16 revision:@1];
    MTREndpointInfo * a1 = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ rootNode ] partsList:@[]];
    XCTAssertTrue([a1 isEqual:a1]);
    XCTAssertTrue([a1 isEqual:[a1 copy]]);
    XCTAssertFalse([a1 isEqual:nil]);
    XCTAssertFalse([a1 isEqual:@"hello"]);
    MTREndpointInfo * a2 = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ rootNode ] partsList:@[]];
    XCTAssertTrue([a1 isEqual:a2]);
    XCTAssertTrue([a2 isEqual:a1]);
    XCTAssertEqual(a1.hash, a2.hash);
    MTREndpointInfo * b = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ rootNode ] partsList:@[ @2 ]];
    XCTAssertFalse([a1 isEqual:b]);
    XCTAssertFalse([b isEqual:a1]);
    MTREndpointInfo * c = [[MTREndpointInfo alloc] initWithEndpointID:@1 deviceTypes:@[ doorLock ] partsList:@[]];
    XCTAssertFalse([a1 isEqual:c]);
    XCTAssertFalse([c isEqual:a1]);
    MTREndpointInfo * d = [[MTREndpointInfo alloc] initWithEndpointID:@2 deviceTypes:@[ rootNode ] partsList:@[]];
    XCTAssertFalse([a1 isEqual:d]);
    XCTAssertFalse([d isEqual:a1]);
}

- (void)testSecureCoding
{
    NSDictionary<NSNumber *, MTREndpointInfo *> * endpoints = [self indexEndpoints:@[
        MakeEndpoint(@0, @[ @1, @2, @3, @4, @5, @6 ]), // full-family pattern
        MakeEndpoint(@1, @[ @2, @3 ]),
        MakeEndpoint(@2, @[]),
        MakeEndpoint(@3, @[]),
        MakeEndpoint(@4, @[ @5, @6 ]), // full-family pattern
        MakeEndpoint(@5, @[ @6 ]),
        MakeEndpoint(@6, @[]),
    ]];
    XCTAssertTrue([MTREndpointInfo populateChildrenForEndpoints:endpoints]);

    NSData * data = [NSKeyedArchiver archivedDataWithRootObject:endpoints.allValues requiringSecureCoding:YES error:NULL];
    NSArray<MTREndpointInfo *> * decodedEndpoints = [NSKeyedUnarchiver unarchivedArrayOfObjectsOfClass:MTREndpointInfo.class fromData:data error:NULL];

    XCTAssertNotNil(decodedEndpoints);
    XCTAssertEqualObjects(decodedEndpoints, endpoints.allValues);

    // Deeply compare by hand as well, `children` is not checked by isEqual:
    [decodedEndpoints enumerateObjectsUsingBlock:^(MTREndpointInfo * decoded, NSUInteger idx, BOOL * stop) {
        MTREndpointInfo * original = endpoints.allValues[idx];
        XCTAssertTrue([decoded isEqual:original]);
        XCTAssertEqualObjects(decoded.endpointID, original.endpointID);
        XCTAssertEqualObjects(decoded.deviceTypes, original.deviceTypes);
        XCTAssertEqualObjects(decoded.partsList, original.partsList);
        XCTAssertEqualObjects(decoded.children, original.children);
    }];
}

- (void)withBridgeDevice:(void (^)(MTRDevice * device))block
{
    [self withBridgeClusters:BridgeClusters(@[ @(MTRClusterIDTypeDescriptorID), @(MTRClusterIDTypeOnOffID) ], 2, 2) device:block];
}

- (void)withBridgeClusters:(NSDictionary<MTRClusterPath *, NSDictionary *> *)clusters device:(void (^)(MTRDevice * device))block
{
    BOOL localTestStorageEnabled = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = YES;
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertTrue([factory startControllerFactory:[[MTRDeviceControllerFactoryParams alloc] initWithStorage:[[MTRTestStorage alloc] init]] error:NULL]);
    __auto_type * keys = [[MTRTestKeys alloc] init];
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:keys.ipk fabricID:@(1) nocSigner:keys];
    params.vendorID = @(0xFFF1);
    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:NULL];
    XCTAssertNotNil(controller);
    [controller suspend];
    NSMutableDictionary<MTRClusterPath *, MTRDeviceClusterData *> * clusterData = [NSMutableDictionary dictionary];
    [clusters enumerateKeysAndObjectsUsingBlock:^(MTRClusterPath * cluster, NSDictionary * attributes, BOOL * stop) {
        clusterData[cluster] = [[MTRDeviceClusterData alloc] initWithDataVersion:@1 attributes:attributes];
    }];
    [controller.controllerDataStore storeClusterData:clusterData forNodeID:kBridgeNodeID];

    MTRDevice * device = [MTRDevice deviceWithNodeID:kBridgeNodeID controller:controller];
    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate.skipSetupSubscription = YES;
    [device addDelegate:delegate queue:dispatch_get_main_queue()];

    block(device);

    [device removeDelegate:delegate];
    [controller.controllerDataStore clearAllStoredClusterData];
    [controller shutdown];
    [factory stopControllerFactory];
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = localTestStorageEnabled;
}

- (void)checkAdaptersShareNodeTableOf:(NSUInteger)pathCount onDevice:(MTRDevice *)device
{
    NSMutableArray<NSArray<MTRClusterPath *> *> * adapterTables = [NSMutableArray array];
    NSHashTable<MTRClusterPath *> * distinctPaths = [NSHashTable hashTableWithOptions:NSPointerFunctionsStrongMemory | NSPointerFunctionsObjectPointerPersonality];
    for (NSUInteger adapter = 0; adapter < kBridgeAdapters; adapter++) {
        NSArray<MTRClusterPath *> * paths = FetchNodePaths(device);
        XCTAssertEqual(paths.count, pathCount);
        [adapterTables addObject:paths];
        for (MTRClusterPath * path in paths) {
            [distinctPaths addObject:path];
        }
    }

    size_t tableBytes = 0;
    for (MTRClusterPath * path in adapterTables.firstObject) {
        tableBytes += malloc_size((__bridge const void *) path);
    }
    size_t distinctBytes = 0;
    for (MTRClusterPath * path in distinctPaths) {
        distinctBytes += malloc_size((__bridge const void *) path);
    }
    XCTAssertEqual(distinctPaths.count, pathCount);
    XCTAssertEqual(distinctBytes, tableBytes);
}

- (void)testBridgeAdaptersSharePathObjects
{
    [self withBridgeDevice:^(MTRDevice * device) {
        [self checkAdaptersShareNodeTableOf:kBridgeNodePathCount onDevice:device];
    }];
}

- (void)testLargeBridgeAdaptersSharePathObjects
{
    [self withBridgeClusters:BridgeClusters(LightClusters(), kLightAttributes, kLightCommands) device:^(MTRDevice * device) {
        [self checkAdaptersShareNodeTableOf:kBridgeEndpoints * LightClusters().count * (kLightAttributes + kLightCommands) onDevice:device];
    }];
}

- (void)testReleasedSharedPathsAreDropped
{
    MTRAttributePath * held = [MTRAttributePath attributePathWithEndpointID:@3 clusterID:@(0xFFF1FC01) attributeID:@0];
    __weak MTRAttributePath * weakPath;
    size_t before = 0;
    size_t after = 0;
    for (NSUInteger round = 0; round < 2; round++) {
        before = HeapBytesInUse();
        @autoreleasepool {
            NSMutableArray<MTRAttributePath *> * burst = [NSMutableArray array];
            for (NSUInteger i = 0; i < 30000; i++) {
                [burst addObject:[MTRAttributePath attributePathWithEndpointID:@2 clusterID:@(0xFFF1FC01) attributeID:@(i)]];
            }
            weakPath = burst.lastObject;
        }
        for (NSUInteger i = 0; i < 300000; i++) {
            @autoreleasepool {
                (void) [MTRAttributePath attributePathWithEndpointID:@1 clusterID:@(0xFFF1FC01) attributeID:@(i)];
            }
        }
        after = HeapBytesInUse();
    }
    XCTAssertNil(weakPath);
    XCTAssertLessThan((NSInteger) after - (NSInteger) before, 32 * 1024);
    XCTAssertIdentical([MTRAttributePath attributePathWithEndpointID:@3 clusterID:@(0xFFF1FC01) attributeID:@0], held);
}

- (void)testSharedPathsFollowObjectLifetime
{
    __weak MTRAttributePath * weakPath;
    @autoreleasepool {
        __autoreleasing MTRAttributePath * pooled = [MTRAttributePath attributePathWithEndpointID:@4 clusterID:@(0xFFF1FC02) attributeID:@0];
        weakPath = pooled;
        NSMutableArray<MTRAttributePath *> * live = [NSMutableArray array];
        for (NSUInteger i = 0; i < 5000; i++) {
            [live addObject:[MTRAttributePath attributePathWithEndpointID:@5 clusterID:@(0xFFF1FC02) attributeID:@(i)]];
        }
        XCTAssertIdentical([MTRAttributePath attributePathWithEndpointID:@4 clusterID:@(0xFFF1FC02) attributeID:@0], weakPath);
    }
    XCTAssertNil(weakPath);
}

- (void)testRequestedSharedPathSurvivesChurn
{
    __weak MTRAttributePath * requested;
    @autoreleasepool {
        requested = [MTRAttributePath attributePathWithEndpointID:@3 clusterID:@(0xFFF1FC01) attributeID:@0];
    }
    NSUInteger misses = 0;
    for (NSUInteger i = 0; i < 50000; i++) {
        @autoreleasepool {
            (void) [MTRCommandPath commandPathWithEndpointID:@3 clusterID:@(0xFFF1FC01) commandID:@(i)];
            misses += [MTRAttributePath attributePathWithEndpointID:@3 clusterID:@(0xFFF1FC01) attributeID:@0] != requested;
        }
    }
    XCTAssertNotNil(requested);
    XCTAssertEqual(misses, 0);
}

- (void)testReportAndDecodedPathsAreShared
{
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointID:@6 clusterID:@(0xFFF1FC03) attributeID:@1];
    MTREventPath * eventPath = [MTREventPath eventPathWithEndpointID:@6 clusterID:@(0xFFF1FC03) eventID:@1];
    NSError * failure = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
    MTRAttributeReport * attributeReport = [[MTRAttributeReport alloc] initWithResponseValue:@{ MTRAttributePathKey : attributePath, MTRErrorKey : failure } error:nil];
    MTRAttributeReport * copiedReport = [attributeReport copy];
    XCTAssertIdentical(copiedReport.path, attributePath);
    MTREventReport * eventReport = [[MTREventReport alloc] initWithResponseValue:@{ MTREventPathKey : eventPath, MTRErrorKey : failure } error:nil];
    XCTAssertIdentical(eventReport.path, eventPath);

    MTRCommandPath * commandPath = [MTRCommandPath commandPathWithEndpointID:@6 clusterID:@(0xFFF1FC03) commandID:@1];
    MTRClusterPath * clusterPath = [MTRClusterPath clusterPathWithEndpointID:@6 clusterID:@(0xFFF1FC03)];
    for (MTRClusterPath * path in @[ attributePath, eventPath, commandPath, clusterPath ]) {
        NSData * archive = [NSKeyedArchiver archivedDataWithRootObject:path requiringSecureCoding:YES error:nil];
        XCTAssertIdentical([NSKeyedUnarchiver unarchivedObjectOfClass:path.class fromData:archive error:nil], path);
    }

    MTRPathEcho * echo = [[MTRPathEcho alloc] init];
    NSXPCListener * listener = [NSXPCListener anonymousListener];
    listener.delegate = echo;
    [listener resume];
    NSXPCConnection * connection = [[NSXPCConnection alloc] initWithListenerEndpoint:listener.endpoint];
    connection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRPathEchoProtocol)];
    [connection resume];
    __block MTRAttributePath * echoed;
    [[connection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * error) {
        XCTFail(@"%@", error);
    }] echoPath:attributePath reply:^(MTRAttributePath * path) {
        echoed = path;
    }];
    [connection invalidate];
    [listener invalidate];
    XCTAssertEqualObjects(echoed, attributePath);
    XCTAssertIdentical(echoed, attributePath);
    XCTAssertIdentical(echo.received, attributePath);
}

- (void)testDecodedPathSubclassIsNotShared
{
    MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@6 clusterID:@(0xFFF1FC04) attributeID:@1];
    NSKeyedArchiver * archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
    [archiver setClassName:NSStringFromClass(MTRTestAttributePathSubclass.class) forClass:MTRAttributePath.class];
    [archiver encodeObject:path forKey:NSKeyedArchiveRootObjectKey];
    MTRAttributePath * decoded = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRTestAttributePathSubclass.class fromData:archiver.encodedData error:nil];
    XCTAssertEqual(decoded.class, MTRTestAttributePathSubclass.class);
    XCTAssertEqualObjects(decoded.attribute, path.attribute);
}

- (void)testSharedPathsMatchTheirRequest
{
    NSUInteger mismatches = 0;
    for (NSUInteger i = 0; i < 4096; i++) {
        @autoreleasepool {
            NSNumber * random = @(arc4random_uniform(UINT16_MAX));
            for (NSArray<NSNumber *> * ids in @[ @[ random, @6, @0 ], @[ @1, random, @0 ], @[ @1, @6, random ] ]) {
                MTRClusterPath * cluster = [MTRClusterPath clusterPathWithEndpointID:ids[0] clusterID:ids[1]];
                MTRAttributePath * attribute = [MTRAttributePath attributePathWithEndpointID:ids[0] clusterID:ids[1] attributeID:ids[2]];
                MTREventPath * event = [MTREventPath eventPathWithEndpointID:ids[0] clusterID:ids[1] eventID:ids[2]];
                MTRCommandPath * command = [MTRCommandPath commandPathWithEndpointID:ids[0] clusterID:ids[1] commandID:ids[2]];
                for (MTRClusterPath * path in @[ cluster, attribute, event, command ]) {
                    mismatches += ![path.endpoint isEqual:ids[0]] || ![path.cluster isEqual:ids[1]];
                }
                mismatches += cluster.class != MTRClusterPath.class || attribute.class != MTRAttributePath.class || event.class != MTREventPath.class || command.class != MTRCommandPath.class;
                mismatches += ![attribute.attribute isEqual:ids[2]] || ![event.event isEqual:ids[2]] || ![command.command isEqual:ids[2]];
            }
        }
    }
    XCTAssertEqual(mismatches, 0);
}

- (void)testSharedPathLookupRacesDeallocation
{
    __block NSUInteger mismatches = 0;
    dispatch_apply(8, DISPATCH_APPLY_AUTO, ^(size_t worker) {
        NSUInteger workerMismatches = 0;
        for (NSUInteger i = 0; i < 200000; i++) {
            @autoreleasepool {
                NSUInteger key = (i + worker) % 2048;
                MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@(key % 16) clusterID:@(0xFFF1FC06) attributeID:@(key / 16)];
                workerMismatches += path.class != MTRAttributePath.class || path.endpoint.unsignedIntegerValue != key % 16
                    || path.cluster.unsignedIntegerValue != 0xFFF1FC06 || path.attribute.unsignedIntegerValue != key / 16;
            }
        }
        @synchronized(self) {
            mismatches += workerMismatches;
        }
    });
    XCTAssertEqual(mismatches, 0);
}

- (void)testSharedPathsSurviveConcurrentEviction
{
    __block NSUInteger mismatches = 0;
    dispatch_apply(8, DISPATCH_APPLY_AUTO, ^(size_t worker) {
        NSUInteger workerMismatches = 0;
        MTRAttributePath * previous = [MTRAttributePath attributePathWithEndpointID:@0 clusterID:@(0xFFF1FC05) attributeID:@0];
        for (NSUInteger i = 0; i < 200000; i++) {
            @autoreleasepool {
                NSUInteger key = (i * 7919 + worker * 131) % 1024;
                MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@(key % 16) clusterID:@(0xFFF1FC05) attributeID:@(key / 16)];
                workerMismatches += path.endpoint.unsignedIntegerValue != key % 16 || path.attribute.unsignedIntegerValue != key / 16
                    || previous.cluster.unsignedIntegerValue != 0xFFF1FC05;
                previous = path;
            }
        }
        @synchronized(self) {
            mismatches += workerMismatches;
        }
    });
    XCTAssertEqual(mismatches, 0);
}

@end

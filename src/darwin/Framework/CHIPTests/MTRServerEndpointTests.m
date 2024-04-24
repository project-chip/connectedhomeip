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

#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

@interface MTRServerEndpointTests : XCTestCase

@end

@implementation MTRServerEndpointTests

- (void)testAccessGrant
{
    // Try to create an access grant with an invalid node ID
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForNodeID:@(0) privilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNil(grant);
    }

    // Try to create an access grant with a group-range node ID
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForNodeID:@(0xFFFFFFFFFFFF0001) privilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNil(grant);
    }

    // Try to create an access grant with a local node ID
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForNodeID:@(0xFFFFFFFE00020002) privilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNil(grant);
    }

    // Try to create an access grant with a CAT-range node ID
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForNodeID:@(0xFFFFFFFD00020002) privilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNil(grant);
    }

    // Try to create an access grant with an operational node ID
    {
        NSNumber * nodeID = @(2);
        __auto_type * grant = [MTRAccessGrant accessGrantForNodeID:nodeID privilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, nodeID);
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeView);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
        XCTAssertEqualObjects([grant description], @"<MTRAccessGrant node 0x0000000000000002 can View>");
    }

    // Try different privileges
    {
        NSNumber * nodeID = @(2);
        __auto_type * grant = [MTRAccessGrant accessGrantForNodeID:nodeID privilege:MTRAccessControlEntryPrivilegeAdminister];
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, nodeID);
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeAdminister);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
        XCTAssertEqualObjects([grant description], @"<MTRAccessGrant node 0x0000000000000002 can Administer>");
    }

    // Try a CAT
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForCASEAuthenticatedTag:@(0x00020003) privilege:MTRAccessControlEntryPrivilegeManage];
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, @(0xFFFFFFFD00020003));
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeManage);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
        XCTAssertEqualObjects([grant description], @"<MTRAccessGrant nodes with CASE Authenticated Tag 0x00020003 can Manage>");
    }

    // Try some invalid CATs
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForCASEAuthenticatedTag:@(0x100000000) privilege:MTRAccessControlEntryPrivilegeManage];
        XCTAssertNil(grant);
    }

    {
        __auto_type * grant = [MTRAccessGrant accessGrantForCASEAuthenticatedTag:@(0x00020000) privilege:MTRAccessControlEntryPrivilegeManage];
        XCTAssertNil(grant);
    }

    // Try a group ID
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForGroupID:@(0x0005) privilege:MTRAccessControlEntryPrivilegeOperate];
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, @(0xFFFFFFFFFFFF0005));
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeOperate);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeGroup);
        XCTAssertEqualObjects([grant description], @"<MTRAccessGrant group 0x5 can Operate>");
    }

    // Try an invalid group ID
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForGroupID:@(0) privilege:MTRAccessControlEntryPrivilegeOperate];
        XCTAssertNil(grant);
    }

    // Try a wildcard subject.
    {
        __auto_type * grant = [MTRAccessGrant accessGrantForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNotNil(grant);
        XCTAssertNil(grant.subjectID);
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeView);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
        XCTAssertEqualObjects([grant description], @"<MTRAccessGrant all nodes can View>");
    }
}

- (void)testServerAttribute
{
    __auto_type * unsignedIntValue = @{
        MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(5),
    };

    __auto_type * listOfStringsValue = @{
        MTRTypeKey : MTRArrayValueType,
        MTRValueKey : @[
            @{
                MTRDataKey : @ {
                    MTRTypeKey : MTRUTF8StringValueType,
                    MTRValueKey : @"str1",
                },
            },
            @{
                MTRDataKey : @ {
                    MTRTypeKey : MTRUTF8StringValueType,
                    MTRValueKey : @"str2",
                },
            },
        ],
    };

    // Basic readonly attribute.
    {
        __auto_type * attr = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0) initialValue:unsignedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNotNil(attr);
        XCTAssertEqualObjects(attr.attributeID, @(0));
        XCTAssertEqualObjects(attr.value, unsignedIntValue);
        XCTAssertEqual(attr.writable, NO);
    }

    // list-typed readonly attribute.
    {
        __auto_type * attr = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(5) initialValue:listOfStringsValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNotNil(attr);
        XCTAssertEqualObjects(attr.attributeID, @(5));
        XCTAssertEqualObjects(attr.value, listOfStringsValue);
        XCTAssertEqual(attr.writable, NO);
    }

    // Vendor-specific attribute.
    {
        NSNumber * vendorSpecificID = @(0xFFF10004);
        __auto_type * attr = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:vendorSpecificID initialValue:unsignedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNotNil(attr);
        XCTAssertEqualObjects(attr.attributeID, vendorSpecificID);
        XCTAssertEqualObjects(attr.value, unsignedIntValue);
        XCTAssertEqual(attr.writable, NO);
    }

    // Invalid attribute ID
    {
        NSNumber * invalidID = @(0x00005000);
        __auto_type * attr = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:invalidID initialValue:unsignedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNil(attr);
    }

    // Invalid "global" attribute ID
    {
        NSNumber * invalidID = @(0x0000FFFF);
        __auto_type * attr = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:invalidID initialValue:unsignedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNil(attr);
    }

    // Invalid vendor prefix on attribute ID
    {
        NSNumber * invalidID = @(0xFFFF0000);
        __auto_type * attr = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:invalidID initialValue:unsignedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNil(attr);
    }
}

- (void)testDeviceType
{
    // Invalid device type ID
    {
        __auto_type * deviceType = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(0xC000) revision:@(1)];
        XCTAssertNil(deviceType);
    }

    // Another invalid device type ID
    {
        __auto_type * deviceType = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(0xFFFF1234) revision:@(1)];
        XCTAssertNil(deviceType);
    }

    // Another invalid device type ID
    {
        __auto_type * deviceType = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(0x100000000) revision:@(1)];
        XCTAssertNil(deviceType);
    }

    // Invalid device type revision
    {
        __auto_type * deviceType = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(0x1234) revision:@(0)];
        XCTAssertNil(deviceType);
    }

    // Another invalid device type revision
    {
        __auto_type * deviceType = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(0x1234) revision:@(0x10000)];
        XCTAssertNil(deviceType);
    }

    // Valid device type
    {
        NSNumber * deviceTypeID = @(0x1234);
        NSNumber * deviceTypeRevision = @(1);
        __auto_type * deviceType = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:deviceTypeID revision:deviceTypeRevision];
        XCTAssertNotNil(deviceType);
        XCTAssertEqualObjects(deviceType.deviceTypeID, deviceTypeID);
        XCTAssertEqualObjects(deviceType.deviceTypeRevision, deviceTypeRevision);
    }
}

- (void)testClusterDescription
{
    // Standard cluster.
    {
        NSNumber * clusterID = @(6);
        NSNumber * clusterRevision = @(1);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNotNil(cluster);
        XCTAssertEqualObjects(cluster.clusterID, clusterID);
        XCTAssertEqualObjects(cluster.clusterRevision, clusterRevision);
        XCTAssertEqualObjects(cluster.accessGrants, @[]);
        XCTAssertEqualObjects(cluster.attributes, @[]);
    }

    // Vendor-specific cluster.
    {
        NSNumber * clusterID = @(0xFFF1FC01);
        NSNumber * clusterRevision = @(1);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNotNil(cluster);
        XCTAssertEqualObjects(cluster.clusterID, clusterID);
        XCTAssertEqualObjects(cluster.clusterRevision, clusterRevision);
        XCTAssertEqualObjects(cluster.accessGrants, @[]);
        XCTAssertEqualObjects(cluster.attributes, @[]);
    }

    // Invalid "standard" cluster.
    {
        NSNumber * clusterID = @(0x8000);
        NSNumber * clusterRevision = @(1);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNil(cluster);
    }

    // Invalid vendor-specific cluster.
    {
        NSNumber * clusterID = @(0xFFF10002);
        NSNumber * clusterRevision = @(1);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNil(cluster);
    }

    // Cluster ID out of range.
    {
        NSNumber * clusterID = @(0x100000000);
        NSNumber * clusterRevision = @(1);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNil(cluster);
    }

    // Revision too small.
    {
        NSNumber * clusterID = @(6);
        NSNumber * clusterRevision = @(0);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNil(cluster);
    }

    // Revision too big.
    {
        NSNumber * clusterID = @(6);
        NSNumber * clusterRevision = @(0x10000);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNil(cluster);
    }

    // Descriptor cluster wrong method.
    {
        NSNumber * clusterID = @(0x001D);
        NSNumber * clusterRevision = @(0x10000);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNil(cluster);
    }

    __auto_type * emptyListValue = @{
        MTRTypeKey : MTRArrayValueType,
        MTRValueKey : @[],
    };

    __auto_type * unsignedIntValue = @{
        MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(2),
    };

    // Descriptor cluster right method
    {
        NSNumber * clusterID = @(0x001D);
        __auto_type * cluster = [MTRServerCluster newDescriptorCluster];
        XCTAssertNotNil(cluster);
        XCTAssertEqualObjects(cluster.clusterID, clusterID);
        // Don't hardcode the cluster revision here; we want it to be able to
        // change without updating this test.
        XCTAssertEqualObjects(cluster.accessGrants, @[]);
        XCTAssertEqualObjects(cluster.attributes, @[]);

        // Adding descriptor's list attributes should fail.
        __auto_type * deviceTypeListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:deviceTypeListAttribute]);

        __auto_type * serverListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(1) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:serverListAttribute]);

        __auto_type * clientListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(2) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:clientListAttribute]);

        __auto_type * partsListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(3) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:partsListAttribute]);

        // Adding global attributes should fail.
        __auto_type * attributeListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFFB) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:attributeListAttribute]);

        __auto_type * acceptedCommandListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFF9) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:acceptedCommandListAttribute]);

        __auto_type * generatedCommandListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFF8) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:generatedCommandListAttribute]);

        __auto_type * clusterRevisionAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFFD) initialValue:unsignedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:clusterRevisionAttribute]);

        // Adding random attributes should succeed.
        __auto_type * randomListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFF10000) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertTrue([cluster addAttribute:randomListAttribute]);
    }

    // Adding some attributes
    {
        NSNumber * clusterID = @(0xFFF1FC01);
        NSNumber * clusterRevision = @(1);
        __auto_type * cluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];
        XCTAssertNotNil(cluster);
        XCTAssertEqualObjects(cluster.accessGrants, @[]);
        XCTAssertEqualObjects(cluster.attributes, @[]);

        __auto_type * grants = @[
            [MTRAccessGrant accessGrantForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeManage],
            [MTRAccessGrant accessGrantForNodeID:@(1) privilege:MTRAccessControlEntryPrivilegeView],
        ];
        for (MTRAccessGrant * grant in grants) {
            [cluster addAccessGrant:grant];
        }
        XCTAssertEqualObjects(cluster.accessGrants, grants);

        __auto_type * signedIntValue = @{
            MTRTypeKey : MTRSignedIntegerValueType,
            MTRValueKey : @(5),
        };

        __auto_type * attributes = @[ [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0) initialValue:signedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView] ];
        XCTAssertNotNil(attributes);
        for (MTRServerAttribute * attribute in attributes) {
            XCTAssertTrue([cluster addAttribute:attribute]);
        }
        XCTAssertEqualObjects(cluster.attributes, attributes);

        __auto_type * otherCluster = [[MTRServerCluster alloc] initWithClusterID:clusterID revision:clusterRevision];

        // Adding an already-added attribute should fail.
        XCTAssertFalse([otherCluster addAttribute:attributes[0]]);

        MTRServerAttribute * otherAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0) initialValue:signedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];

        // Adding same-id attribute should fail.
        XCTAssertFalse([cluster addAttribute:otherAttribute]);

        // Adding the same-id attribute to a different cluster should work.
        XCTAssertTrue([otherCluster addAttribute:otherAttribute]);

        // Adding global attributes should fail.
        __auto_type * attributeListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFFB) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:attributeListAttribute]);

        __auto_type * acceptedCommandListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFF9) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:acceptedCommandListAttribute]);

        __auto_type * generatedCommandListAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFF8) initialValue:emptyListValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:generatedCommandListAttribute]);

        __auto_type * clusterRevisionAttribute = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(0xFFFD) initialValue:unsignedIntValue requiredPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertFalse([cluster addAttribute:clusterRevisionAttribute]);
    }
}

- (void)testEndpointDescription
{
    NSArray<MTRDeviceTypeRevision *> * deviceTypes;
    {
        deviceTypes = @[ [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(0xFFF11234) revision:@(2)] ];
        XCTAssertNotNil(deviceTypes);
    }

    // Invalid endpoint ID.
    {
        NSNumber * endpointID = @(0);
        __auto_type * endpoint = [[MTRServerEndpoint alloc] initWithEndpointID:endpointID deviceTypes:deviceTypes];
        XCTAssertNil(endpoint);
    }

    // Too-large endpoint ID.
    {
        NSNumber * endpointID = @(0x10000);
        __auto_type * endpoint = [[MTRServerEndpoint alloc] initWithEndpointID:endpointID deviceTypes:deviceTypes];
        XCTAssertNil(endpoint);
    }

    // Invalid device type list.
    {
        NSNumber * endpointID = @(1);
        __auto_type * endpoint = [[MTRServerEndpoint alloc] initWithEndpointID:endpointID deviceTypes:@[]];
        XCTAssertNil(endpoint);
    }

    // Valid endpoint definition.
    {
        NSNumber * endpointID = @(1);
        __auto_type * endpoint = [[MTRServerEndpoint alloc] initWithEndpointID:endpointID deviceTypes:deviceTypes];
        XCTAssertNotNil(endpoint);

        XCTAssertEqualObjects(endpoint.endpointID, endpointID);
        XCTAssertEqualObjects(endpoint.deviceTypes, deviceTypes);
        XCTAssertEqualObjects(endpoint.accessGrants, @[]);
        XCTAssertEqualObjects(endpoint.serverClusters, @[]);

        __auto_type * grants = @[
            [MTRAccessGrant accessGrantForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeManage],
            [MTRAccessGrant accessGrantForGroupID:@(1) privilege:MTRAccessControlEntryPrivilegeAdminister],
        ];
        for (MTRAccessGrant * grant in grants) {
            [endpoint addAccessGrant:grant];
        }
        XCTAssertEqualObjects(endpoint.accessGrants, grants);

        __auto_type * clusters = @[
            [[MTRServerCluster alloc] initWithClusterID:@(6) revision:@(1)],
        ];
        for (MTRServerCluster * cluster in clusters) {
            XCTAssertTrue([endpoint addServerCluster:cluster]);
        }
        XCTAssertEqualObjects(endpoint.serverClusters, clusters);

        __auto_type * otherEndpoint = [[MTRServerEndpoint alloc] initWithEndpointID:endpointID deviceTypes:deviceTypes];

        // Adding an already-added cluster should fail.
        XCTAssertFalse([otherEndpoint addServerCluster:clusters[0]]);

        MTRServerCluster * otherCluster = [[MTRServerCluster alloc] initWithClusterID:@(6) revision:@(1)];

        // Adding same-id cluster should fail.
        XCTAssertFalse([endpoint addServerCluster:otherCluster]);

        // Adding the same-id cluster to a different endpoint should work.
        XCTAssertTrue([otherEndpoint addServerCluster:otherCluster]);
    }
}

@end

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

@interface MTREndpointDescriptionTests : XCTestCase

@end

@implementation MTREndpointDescriptionTests

- (void)testAccessGrant
{
    // Try to create an access grant with an invalid node ID
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithNodeID:@(0) privilege:MTRAccessControlEntryPrivilegeView error:&error];
        XCTAssertNil(grant);
        XCTAssertNotNil(error);
    }

    // Try to create an access grant with a group-range node ID
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithNodeID:@(0xFFFFFFFFFFFF0001) privilege:MTRAccessControlEntryPrivilegeView error:&error];
        XCTAssertNil(grant);
        XCTAssertNotNil(error);
    }

    // Try to create an access grant with a local node ID
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithNodeID:@(0xFFFFFFFE00020002) privilege:MTRAccessControlEntryPrivilegeView error:&error];
        XCTAssertNil(grant);
        XCTAssertNotNil(error);
    }

    // Try to create an access grant with a CAT-range node ID
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithNodeID:@(0xFFFFFFFD00020002) privilege:MTRAccessControlEntryPrivilegeView error:&error];
        XCTAssertNil(grant);
        XCTAssertNotNil(error);
    }

    // Try to create an access grant with an operational node ID
    {
        NSNumber * nodeID = @(2);
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithNodeID:nodeID privilege:MTRAccessControlEntryPrivilegeView error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, nodeID);
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeView);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
    }

    // Try different privileges
    {
        NSNumber * nodeID = @(2);
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithNodeID:nodeID privilege:MTRAccessControlEntryPrivilegeAdminister error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, nodeID);
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeAdminister);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
    }

    // Try a CAT
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithCASEAuthenticatedTag:@(0x00020003) privilege:MTRAccessControlEntryPrivilegeManage error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, @(0xFFFFFFFD00020003));
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeManage);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
    }

    // Try some invalid CATs
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithCASEAuthenticatedTag:@(0x100000000) privilege:MTRAccessControlEntryPrivilegeManage error:&error];
        XCTAssertNil(grant);
        XCTAssertNotNil(error);
    }

    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithCASEAuthenticatedTag:@(0x00020000) privilege:MTRAccessControlEntryPrivilegeManage error:&error];
        XCTAssertNil(grant);
        XCTAssertNotNil(error);
    }

    // Try a group ID
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithGroupID:@(0x0005) privilege:MTRAccessControlEntryPrivilegeOperate error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(grant);
        XCTAssertEqualObjects(grant.subjectID, @(0xFFFFFFFFFFFF0005));
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeOperate);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeGroup);
    }

    // Try an invalid group ID
    {
        NSError * error;
        __auto_type * grant = [[MTRAccessGrant alloc] initWithGroupID:@(0) privilege:MTRAccessControlEntryPrivilegeOperate error:&error];
        XCTAssertNil(grant);
        XCTAssertNotNil(error);
    }

    // Try a wildcard subject.
    {
        __auto_type * grant = [[MTRAccessGrant alloc] initForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeView];
        XCTAssertNotNil(grant);
        XCTAssertNil(grant.subjectID);
        XCTAssertEqual(grant.grantedPrivilege, MTRAccessControlEntryPrivilegeView);
        XCTAssertEqual(grant.authenticationMode, MTRAccessControlEntryAuthModeCASE);
    }
}

- (void)testAttributeDescription
{
    __auto_type * unsignedIntValue = @{
        MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(5),
    };

    __auto_type * listOfStringsValue = @{
        MTRTypeKey : MTRArrayValueType,
        MTRValueKey : @[
            @{
                MTRTypeKey : MTRUTF8StringValueType,
                MTRValueKey : @"str1",
            },
            @{
                MTRTypeKey : MTRUTF8StringValueType,
                MTRValueKey : @"str2",
            },
        ],
    };

    // Basic readonly attribute.
    {
        NSError * error;
        __auto_type * attr = [[MTRAttributeDescription alloc] initReadonlyAttributeWithID:@(0) initialValue:unsignedIntValue error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(attr);
        XCTAssertEqualObjects(attr.attributeID, @(0));
        XCTAssertEqualObjects(attr.value, unsignedIntValue);
        XCTAssertEqual(attr.writable, NO);
    }

    // Basic writable attribute.
    {
        NSError * error;
        __auto_type * attr = [[MTRAttributeDescription alloc] initWritableAttributeWithID:@(0) initialValue:unsignedIntValue error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(attr);
        XCTAssertEqualObjects(attr.attributeID, @(0));
        XCTAssertEqualObjects(attr.value, unsignedIntValue);
        XCTAssertEqual(attr.writable, YES);
    }

    // list-typed readonly attribute.
    {
        NSError * error;
        __auto_type * attr = [[MTRAttributeDescription alloc] initReadonlyAttributeWithID:@(5) initialValue:listOfStringsValue error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(attr);
        XCTAssertEqualObjects(attr.attributeID, @(5));
        XCTAssertEqualObjects(attr.value, listOfStringsValue);
        XCTAssertEqual(attr.writable, NO);
    }

    // Vendor-specific attribute.
    {
        NSNumber * vendorSpecificID = @(0xFFF10004);
        NSError * error;
        __auto_type * attr = [[MTRAttributeDescription alloc] initReadonlyAttributeWithID:vendorSpecificID initialValue:unsignedIntValue error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(attr);
        XCTAssertEqualObjects(attr.attributeID, vendorSpecificID);
        XCTAssertEqualObjects(attr.value, unsignedIntValue);
        XCTAssertEqual(attr.writable, NO);
    }

    // Invalid attribute ID
    {
        NSNumber * invalidID = @(0x00005000);
        NSError * error;
        __auto_type * attr = [[MTRAttributeDescription alloc] initReadonlyAttributeWithID:invalidID initialValue:unsignedIntValue error:&error];
        XCTAssertNil(attr);
        XCTAssertNotNil(error);
    }

    // Invalid "global" attribute ID
    {
        NSNumber * invalidID = @(0x0000FFFF);
        NSError * error;
        __auto_type * attr = [[MTRAttributeDescription alloc] initReadonlyAttributeWithID:invalidID initialValue:unsignedIntValue error:&error];
        XCTAssertNil(attr);
        XCTAssertNotNil(error);
    }

    // Invalid vendor prefix on attribute ID
    {
        NSNumber * invalidID = @(0xFFFF0000);
        NSError * error;
        __auto_type * attr = [[MTRAttributeDescription alloc] initReadonlyAttributeWithID:invalidID initialValue:unsignedIntValue error:&error];
        XCTAssertNil(attr);
        XCTAssertNotNil(error);
    }
}

- (void)testDeviceType
{
    // Invalid device type ID
    {
        NSError * error;
        __auto_type * deviceType = [[MTRDeviceType alloc] initWithDeviceTypeID:@(0xC000) revision:@(1) error:&error];
        XCTAssertNil(deviceType);
        XCTAssertNotNil(error);
    }

    // Another invalid device type ID
    {
        NSError * error;
        __auto_type * deviceType = [[MTRDeviceType alloc] initWithDeviceTypeID:@(0xFFFF1234) revision:@(1) error:&error];
        XCTAssertNil(deviceType);
        XCTAssertNotNil(error);
    }

    // Another invalid device type ID
    {
        NSError * error;
        __auto_type * deviceType = [[MTRDeviceType alloc] initWithDeviceTypeID:@(0x100000000) revision:@(1) error:&error];
        XCTAssertNil(deviceType);
        XCTAssertNotNil(error);
    }

    // Invalid device type revision
    {
        NSError * error;
        __auto_type * deviceType = [[MTRDeviceType alloc] initWithDeviceTypeID:@(0x1234) revision:@(0) error:&error];
        XCTAssertNil(deviceType);
        XCTAssertNotNil(error);
    }

    // Another invalid device type revision
    {
        NSError * error;
        __auto_type * deviceType = [[MTRDeviceType alloc] initWithDeviceTypeID:@(0x1234) revision:@(0x10000) error:&error];
        XCTAssertNil(deviceType);
        XCTAssertNotNil(error);
    }

    // Valid device type
    {
        NSNumber * deviceTypeID = @(0x1234);
        NSNumber * deviceTypeRevision = @(1);
        NSError * error;
        __auto_type * deviceType = [[MTRDeviceType alloc] initWithDeviceTypeID:deviceTypeID revision:deviceTypeRevision error:&error];
        XCTAssertNil(error);
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
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(error);
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
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(error);
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
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(cluster);
        XCTAssertNotNil(error);
    }

    // Invalid vendor-specific cluster.
    {
        NSNumber * clusterID = @(0xFFF10002);
        NSNumber * clusterRevision = @(1);
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(cluster);
        XCTAssertNotNil(error);
    }

    // Cluster ID out of range.
    {
        NSNumber * clusterID = @(0x100000000);
        NSNumber * clusterRevision = @(1);
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(cluster);
        XCTAssertNotNil(error);
    }

    // Revision too small.
    {
        NSNumber * clusterID = @(6);
        NSNumber * clusterRevision = @(0);
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(cluster);
        XCTAssertNotNil(error);
    }

    // Revision too big.
    {
        NSNumber * clusterID = @(6);
        NSNumber * clusterRevision = @(0x10000);
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(cluster);
        XCTAssertNotNil(error);
    }

    // Descriptor cluster wrong method.
    {
        NSNumber * clusterID = @(0x001D);
        NSNumber * clusterRevision = @(0x10000);
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(cluster);
        XCTAssertNotNil(error);
    }

    // Descriptor cluster right method
    {
        NSNumber * clusterID = @(0x001D);
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initDescriptorCluster];
        XCTAssertNil(error);
        XCTAssertNotNil(cluster);
        XCTAssertEqualObjects(cluster.clusterID, clusterID);
        // Don't hardcode the cluster revision here; we want it to be able to
        // change without updating this test.
        XCTAssertEqualObjects(cluster.accessGrants, @[]);
        XCTAssertEqualObjects(cluster.attributes, @[]);
    }

    // Adding some attributes
    {
        NSNumber * clusterID = @(0xFFF1FC01);
        NSNumber * clusterRevision = @(1);
        NSError * error;
        __auto_type * cluster = [[MTRServerClusterDescription alloc] initWithClusterID:clusterID clusterRevision:clusterRevision error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(cluster);
        XCTAssertEqualObjects(cluster.accessGrants, @[]);
        XCTAssertEqualObjects(cluster.attributes, @[]);

        __auto_type * grants = @[
            [[MTRAccessGrant alloc] initForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeManage],
            [[MTRAccessGrant alloc] initWithNodeID:@(1) privilege:MTRAccessControlEntryPrivilegeView error:&error],
        ];
        XCTAssertNil(error);
        cluster.accessGrants = grants;
        XCTAssertEqualObjects(cluster.accessGrants, grants);

        __auto_type * signedIntValue = @{
            MTRTypeKey : MTRSignedIntegerValueType,
            MTRValueKey : @(5),
        };

        __auto_type * attributes = @[ [[MTRAttributeDescription alloc] initReadonlyAttributeWithID:@(0) initialValue:signedIntValue error:&error] ];
        XCTAssertNil(error);
        XCTAssertNotNil(attributes);
        cluster.attributes = attributes;
        XCTAssertEqualObjects(cluster.attributes, attributes);
    }
}

- (void)testEndpointDescription
{
    NSArray<MTRDeviceType *> * deviceTypes;
    {
        NSError * error;
        deviceTypes = @[ [[MTRDeviceType alloc] initWithDeviceTypeID:@(0xFFF11234) revision:@(2) error:&error] ];
        XCTAssertNil(error);
        XCTAssertNotNil(deviceTypes);
    }

    // Invalid endpoint ID.
    {
        NSNumber * endpointID = @(0);
        NSError * error;
        __auto_type * endpoint = [[MTREndpointDescription alloc] initWithEndpointID:endpointID deviceTypes:deviceTypes error:&error];
        XCTAssertNil(endpoint);
        XCTAssertNotNil(error);
    }

    // Too-large endpoint ID.
    {
        NSNumber * endpointID = @(0x10000);
        NSError * error;
        __auto_type * endpoint = [[MTREndpointDescription alloc] initWithEndpointID:endpointID deviceTypes:deviceTypes error:&error];
        XCTAssertNil(endpoint);
        XCTAssertNotNil(error);
    }

    // Invalid device type list.
    {
        NSNumber * endpointID = @(1);
        NSError * error;
        __auto_type * endpoint = [[MTREndpointDescription alloc] initWithEndpointID:endpointID deviceTypes:@[] error:&error];
        XCTAssertNil(endpoint);
        XCTAssertNotNil(error);
    }

    // Valid endpoint definition.
    {
        NSNumber * endpointID = @(1);
        NSError * error;
        __auto_type * endpoint = [[MTREndpointDescription alloc] initWithEndpointID:endpointID deviceTypes:deviceTypes error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(endpoint);

        XCTAssertEqualObjects(endpoint.endpointID, endpointID);
        XCTAssertEqualObjects(endpoint.deviceTypes, deviceTypes);
        XCTAssertEqualObjects(endpoint.accessGrants, @[]);
        XCTAssertEqualObjects(endpoint.serverClusters, @[]);
        XCTAssertEqual(endpoint.autoEnable, YES);

        __auto_type * grants = @[
            [[MTRAccessGrant alloc] initForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeManage],
            [[MTRAccessGrant alloc] initWithGroupID:@(1) privilege:MTRAccessControlEntryPrivilegeAdminister error:&error],
        ];
        XCTAssertNil(error);
        endpoint.accessGrants = grants;
        XCTAssertEqualObjects(endpoint.accessGrants, grants);

        __auto_type * clusters = @[
            [[MTRServerClusterDescription alloc] initWithClusterID:@(6) clusterRevision:@(1) error:&error],
        ];
        XCTAssertNil(error);
        endpoint.serverClusters = clusters;
        XCTAssertEqualObjects(endpoint.serverClusters, clusters);

        endpoint.autoEnable = NO;
        XCTAssertEqual(endpoint.autoEnable, NO);
    }
}

@end

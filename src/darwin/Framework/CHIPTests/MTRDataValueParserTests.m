/*
 *
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

#import <Matter/Matter.h>

#import <XCTest/XCTest.h>

@interface MTRDataValueParserTests : XCTestCase
@end

@implementation MTRDataValueParserTests

- (void)setUp
{
    // Per-test setup, runs before each test.
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)test001_UnsignedIntAttribute
{
    // Pressure Measurement, Tolerance
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0403) attributeID:@(3)],
        MTRDataKey : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(5),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, @(5));
    XCTAssertNil(report.error);
}

- (void)test002_SignedIntAttribute
{
    // Pressure Measurement, MeasuredValue
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0403) attributeID:@(2)],
        MTRDataKey : @ {
            MTRTypeKey : MTRSignedIntegerValueType,
            MTRValueKey : @(7),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, @(7));
    XCTAssertNil(report.error);
}

- (void)test003_BooleanAttribute
{
    // On/Off, OnOff
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0006) attributeID:@(0)],
        MTRDataKey : @ {
            MTRTypeKey : MTRBooleanValueType,
            MTRValueKey : @(YES),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, @(YES));
    XCTAssertNil(report.error);
}

- (void)test004_StringAttribute
{
    // Basic Information, SerialNumber
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0028) attributeID:@(0xf)],
        MTRDataKey : @ {
            MTRTypeKey : MTRUTF8StringValueType,
            MTRValueKey : @"hello",
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, @"hello");
    XCTAssertNil(report.error);
}

- (void)test005_OctetStringAttribute
{
    // Thread Network Diagnostics, ChannelPage0Mask
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(5) clusterID:@(0x0035) attributeID:@(0x3c)],
        MTRDataKey : @ {
            MTRTypeKey : MTROctetStringValueType,
            MTRValueKey : [@"binary" dataUsingEncoding:NSUTF8StringEncoding],
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, [@"binary" dataUsingEncoding:NSUTF8StringEncoding]);
    XCTAssertNil(report.error);
}

- (void)test006_NullableOctetStringAttribute
{
    // Thread Network Diagnostics, ChannelPage0Mask
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(5) clusterID:@(0x0035) attributeID:@(0x3c)],
        MTRDataKey : @ {
            MTRTypeKey : MTRNullValueType,
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNil(report.value);
    XCTAssertNil(report.error);
}

- (void)test007_FloatAttribute
{
    // Media Playback, PlaybackSpeed
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(17) clusterID:@(0x0506) attributeID:@(4)],
        MTRDataKey : @ {
            MTRTypeKey : MTRFloatValueType,
            MTRValueKey : @(1.5),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, @(1.5));
    XCTAssertNil(report.error);
}

- (void)test008_DoubleAttribute
{
    // Unit Testing, float_double
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(17) clusterID:@(0xFFF1FC05) attributeID:@(0x18)],
        MTRDataKey : @ {
            MTRTypeKey : MTRDoubleValueType,
            MTRValueKey : @(1.5),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, @(1.5));
    XCTAssertNil(report.error);
}

- (void)test009_NullableDoubleAttribute
{
    // Unit Testing, nullable_float_double
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(17) clusterID:@(0xFFF1FC05) attributeID:@(0x4018)],
        MTRDataKey : @ {
            MTRTypeKey : MTRNullValueType,
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNil(report.value);
    XCTAssertNil(report.error);
}

- (void)test010_StructAttribute
{
    // Basic Information, CapabilityMinima
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0028) attributeID:@(0x0013)],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(0), // CaseSessionsPerFabric
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(4),
                    },
                },
                @{
                    MTRContextTagKey : @(1), // SubscriptionsPerFabric
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(3),
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNotNil(report.value);
    XCTAssertTrue([report.value isKindOfClass:[MTRBasicInformationClusterCapabilityMinimaStruct class]]);

    MTRBasicInformationClusterCapabilityMinimaStruct * data = report.value;
    XCTAssertEqualObjects(data.caseSessionsPerFabric, @(4));
    XCTAssertEqualObjects(data.subscriptionsPerFabric, @(3));

    XCTAssertNil(report.error);
}

- (void)test011_StructAttributeOtherOrder
{
    // Basic Information, CapabilityMinima
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0028) attributeID:@(0x0013)],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(1), // SubscriptionsPerFabric
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(3),
                    },
                },
                @{
                    MTRContextTagKey : @(0), // CaseSessionsPerFabric
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(4),
                    },
                },
            ]
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNotNil(report.value);
    XCTAssertTrue([report.value isKindOfClass:[MTRBasicInformationClusterCapabilityMinimaStruct class]]);

    MTRBasicInformationClusterCapabilityMinimaStruct * data = report.value;
    XCTAssertEqualObjects(data.caseSessionsPerFabric, @(4));
    XCTAssertEqualObjects(data.subscriptionsPerFabric, @(3));

    XCTAssertNil(report.error);
}

- (void)test012_ListAttribute
{
    // Descriptor, DeviceTypeList
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x001d) attributeID:@(0)],
        MTRDataKey : @ {
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : @[
                @{
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[
                            @{
                                MTRContextTagKey : @(0), // DeviceType
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRUnsignedIntegerValueType,
                                    MTRValueKey : @(100),
                                },
                            },
                            @{
                                MTRContextTagKey : @(1), // Revision
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRUnsignedIntegerValueType,
                                    MTRValueKey : @(17),
                                },
                            },
                        ],
                    },
                },
                @{
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[
                            @{
                                MTRContextTagKey : @(1), // Revision
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRUnsignedIntegerValueType,
                                    MTRValueKey : @(19),
                                },
                            },
                            @{
                                MTRContextTagKey : @(0), // DeviceType
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRUnsignedIntegerValueType,
                                    MTRValueKey : @(200),
                                },
                            },
                        ],
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNotNil(report.value);
    XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);

    NSArray * array = report.value;
    XCTAssertTrue([array[0] isKindOfClass:[MTRDescriptorClusterDeviceTypeStruct class]]);
    MTRDescriptorClusterDeviceTypeStruct * deviceType = array[0];
    XCTAssertEqualObjects(deviceType.deviceType, @(100));
    XCTAssertEqualObjects(deviceType.revision, @(17));

    XCTAssertTrue([array[1] isKindOfClass:[MTRDescriptorClusterDeviceTypeStruct class]]);
    deviceType = array[1];
    XCTAssertEqualObjects(deviceType.deviceType, @(200));
    XCTAssertEqualObjects(deviceType.revision, @(19));

    XCTAssertNil(report.error);
}

- (void)test013_UnsignedIntAttributeSignMismatch
{
    // Pressure Measurement, Tolerance
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0403) attributeID:@(3)],
        MTRDataKey : @ {
            MTRTypeKey : MTRSignedIntegerValueType,
            MTRValueKey : @(5),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test014_SignedIntAttributeSignMismatch
{
    // Pressure Measurement, MeasuredValue
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0403) attributeID:@(2)],
        MTRDataKey : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(7),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test015_UnknownAttribute
{
    // On/Off, nonexistent attribute
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(7) attributeID:@(0x1000)],
        MTRDataKey : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(7),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeUnknownSchema);
}

- (void)test016_UnknownCluster
{
    // Unknown cluster.
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0xFFF1FFF1) attributeID:@(0)],
        MTRDataKey : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(7),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeUnknownSchema);
}

- (void)test017_StringVsOctetStringMismatch
{
    // Basic Information, SerialNumber
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0028) attributeID:@(0xf)],
        MTRDataKey : @ {
            MTRTypeKey : MTROctetStringValueType,
            MTRValueKey : [@"binary" dataUsingEncoding:NSUTF8StringEncoding],
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test018_OctetStringVsStringMismatch
{
    // Thread Network Diagnostics, ChannelPage0Mask
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(5) clusterID:@(0x0035) attributeID:@(0x3c)],
        MTRDataKey : @ {
            MTRTypeKey : MTRUTF8StringValueType,
            MTRValueKey : @"hello",
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test019_DoubleVsFloatMismatch
{
    // Unit Testing, float_double
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(17) clusterID:@(0xFFF1FC05) attributeID:@(0x18)],
        MTRDataKey : @ {
            MTRTypeKey : MTRFloatValueType,
            MTRValueKey : @(1.5),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    // If float was encoded, decoding as double is allowed.
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertEqualObjects(report.value, @(1.5));

    XCTAssertNil(report.error);
}

- (void)test020_FloatVsDoubleMismatch
{
    // Media Playback, PlaybackSpeed
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(17) clusterID:@(0x0506) attributeID:@(4)],
        MTRDataKey : @ {
            MTRTypeKey : MTRDoubleValueType,
            MTRValueKey : @(1.5),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test021_StringVsNullMismatch
{
    // Basic Information, SerialNumber
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0028) attributeID:@(0xf)],
        MTRDataKey : @ {
            MTRTypeKey : MTRNullValueType,
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test022_OctetStringVsNullMismatch
{
    // Unit Testing, octet_string
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(17) clusterID:@(0xFFF1FC05) attributeID:@(0x19)],
        MTRDataKey : @ {
            MTRTypeKey : MTRNullValueType,
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test023_DoubleVsNullMismatch
{
    // Unit Testing, float_double
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(17) clusterID:@(0xFFF1FC05) attributeID:@(0x18)],
        MTRDataKey : @ {
            MTRTypeKey : MTRNullValueType,
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test024_StructFieldIntegerTypeMismatch
{
    // Descriptor, DeviceTypeList
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x001d) attributeID:@(0)],
        MTRDataKey : @ {
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : @[
                @{
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[
                            @{
                                MTRContextTagKey : @(0), // DeviceType
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRUnsignedIntegerValueType,
                                    MTRValueKey : @(100),
                                },
                            },
                            @{
                                MTRContextTagKey : @(1), // Revision
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRUnsignedIntegerValueType,
                                    MTRValueKey : @(17),
                                },
                            },
                        ],
                    },
                },
                @{
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[
                            @{
                                MTRContextTagKey : @(1), // Revision
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRSignedIntegerValueType, // Wrong type here.
                                    MTRValueKey : @(19),
                                },
                            },
                            @{
                                MTRContextTagKey : @(0), // DeviceType
                                MTRDataKey : @ {
                                    MTRTypeKey : MTRUnsignedIntegerValueType,
                                    MTRValueKey : @(200),
                                },
                            },
                        ],
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test025_EventPayloadWithSystemUptime
{
    // Access Control, AccessControlExtensionChanged
    NSDictionary * input = @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(0x001F) eventID:@(1)],
        MTREventNumberKey : @(180),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventTimeTypeKey : @(MTREventTimeTypeSystemUpTime),
        MTREventSystemUpTimeKey : @(27.5),
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(1), // AdminNodeID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(177),
                    },
                },
                @{
                    MTRContextTagKey : @(2), // AdminPasscodeID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRNullValueType,
                    },
                },
                @{
                    MTRContextTagKey : @(3), // ChangeType
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(MTRAccessControlChangeTypeAdded),
                    },
                },
                @{
                    MTRContextTagKey : @(4), // LatestValue
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[
                            @{
                                MTRContextTagKey : @(1), // Data
                                MTRDataKey : @ {
                                    MTRTypeKey : MTROctetStringValueType,
                                    MTRValueKey : [@"extension" dataUsingEncoding:NSUTF8StringEncoding],
                                },
                            },
                        ],
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * report = [[MTREventReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertNotNil(report.path);
    XCTAssertEqualObjects(report.path.endpoint, @(0));
    XCTAssertEqualObjects(report.path.cluster, @(0x001F));
    XCTAssertEqualObjects(report.path.event, @(1));

    XCTAssertEqualObjects(report.eventNumber, @(180));
    XCTAssertEqualObjects(report.priority, @(MTREventPriorityInfo));
    XCTAssertEqual(report.eventTimeType, MTREventTimeTypeSystemUpTime);
    XCTAssertEqual(report.systemUpTime, 27.5);

    XCTAssertNotNil(report.value);

    XCTAssertTrue([report.value isKindOfClass:[MTRAccessControlClusterAccessControlExtensionChangedEvent class]]);

    MTRAccessControlClusterAccessControlExtensionChangedEvent * payload = report.value;
    XCTAssertEqualObjects(payload.adminNodeID, @(177));
    XCTAssertNil(payload.adminPasscodeID);
    XCTAssertEqualObjects(payload.changeType, @(MTRAccessControlChangeTypeAdded));
    XCTAssertNotNil(payload.latestValue);

    XCTAssertTrue([payload.latestValue isKindOfClass:[MTRAccessControlClusterAccessControlExtensionStruct class]]);
    XCTAssertEqualObjects(payload.latestValue.data, [@"extension" dataUsingEncoding:NSUTF8StringEncoding]);

    XCTAssertNil(report.error);
}

- (void)test026_EventReportWithTimestampDate
{
    // Basic Information, Shutdown
    NSDictionary * input = @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(0x0028) eventID:@(1)],
        MTREventNumberKey : @(190),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
        MTREventTimestampDateKey : [NSDate date],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[],
        },
    };

    NSError * error;
    __auto_type * report = [[MTREventReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertNotNil(report.path);
    XCTAssertEqualObjects(report.path.endpoint, @(0));
    XCTAssertEqualObjects(report.path.cluster, @(0x0028));
    XCTAssertEqualObjects(report.path.event, @(1));

    XCTAssertEqualObjects(report.eventNumber, @(190));
    XCTAssertEqualObjects(report.priority, @(MTREventPriorityInfo));
    XCTAssertEqual(report.eventTimeType, MTREventTimeTypeTimestampDate);
    XCTAssertEqualObjects(report.timestampDate, input[MTREventTimestampDateKey]);

    XCTAssertNotNil(report.value);
    XCTAssertTrue([report.value isKindOfClass:[MTRBasicInformationClusterShutDownEvent class]]);
}

- (void)test027_AttributeWithDataAndError
{
    // Pressure Measurement, Tolerance
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x0403) attributeID:@(3)],
        MTRErrorKey : [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil],
        // Include data too, which should be ignored.
        MTRDataKey : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(5),
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNil(report.value);
    XCTAssertEqualObjects(report.error, input[MTRErrorKey]);
}

- (void)test028_EventReportWithDataAndError
{
    // Basic Information, Shutdown
    NSDictionary * input = @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(0x0028) eventID:@(1)],
        MTRErrorKey : [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil],

        // All the other keys should be ignored
        MTREventNumberKey : @(190),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
        MTREventTimestampDateKey : [NSDate date],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[],
        },
    };

    NSError * error;
    __auto_type * report = [[MTREventReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertNotNil(report.path);
    XCTAssertEqualObjects(report.path.endpoint, @(0));
    XCTAssertEqualObjects(report.path.cluster, @(0x0028));
    XCTAssertEqualObjects(report.path.event, @(1));

    XCTAssertEqualObjects(report.eventNumber, @(0));
    XCTAssertEqualObjects(report.priority, @(0));
    XCTAssertNil(report.value);
    XCTAssertEqualObjects(report.error, input[MTRErrorKey]);
}

- (void)test029_EventPayloadFailingSchemaCheck
{
    // Access Control, AccessControlExtensionChanged
    NSDictionary * input = @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(0x001F) eventID:@(1)],
        MTREventNumberKey : @(180),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventTimeTypeKey : @(MTREventTimeTypeSystemUpTime),
        MTREventSystemUpTimeKey : @(27.5),
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(1), // AdminNodeID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(177),
                    },
                },
                @{
                    MTRContextTagKey : @(2), // AdminPasscodeID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRNullValueType,
                    },
                },
                @{
                    MTRContextTagKey : @(3), // ChangeType
                    MTRDataKey : @ {
                        MTRTypeKey : MTRSignedIntegerValueType, // Should be unsigned
                        MTRValueKey : @(MTRAccessControlChangeTypeAdded),
                    },
                },
                @{
                    MTRContextTagKey : @(4), // LatestValue
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[
                            @{
                                MTRContextTagKey : @(1), // Data
                                MTRDataKey : @ {
                                    MTRTypeKey : MTROctetStringValueType,
                                    MTRValueKey : [@"extension" dataUsingEncoding:NSUTF8StringEncoding],
                                },
                            },
                        ],
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * report = [[MTREventReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test030_EventReportWithUnknownCluster
{
    NSDictionary * input = @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(0xFF1FF1) eventID:@(0)],
        MTREventNumberKey : @(190),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
        MTREventTimestampDateKey : [NSDate date],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[],
        },
    };

    NSError * error;
    __auto_type * report = [[MTREventReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeUnknownSchema);
}

- (void)test031_EventReportWithUnknownEvent
{
    NSDictionary * input = @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(0x0028) eventID:@(1000)],
        MTREventNumberKey : @(190),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
        MTREventTimestampDateKey : [NSDate date],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[],
        },
    };

    NSError * error;
    __auto_type * report = [[MTREventReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(report);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeUnknownSchema);
}

- (void)test032_VeryLongListAttribute
{
    NSDictionary * singleListItem = @{
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(0), // DeviceType
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(100),
                    },
                },
                @{
                    MTRContextTagKey : @(1), // Revision
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(17),
                    },
                },
            ],
        },
    };

    NSUInteger arrayLength = 1000;
    NSMutableArray * inputArray = [[NSMutableArray alloc] initWithCapacity:arrayLength];
    for (NSUInteger i = 0; i < arrayLength; ++i) {
        [inputArray addObject:singleListItem];
    }

    // Descriptor, DeviceTypeList
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(0x001d) attributeID:@(0)],
        MTRDataKey : @ {
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : inputArray,
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(report);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNotNil(report.value);
    XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);

    NSArray * array = report.value;
    XCTAssertTrue(array.count == inputArray.count);
    for (id item in array) {
        XCTAssertTrue([item isKindOfClass:[MTRDescriptorClusterDeviceTypeStruct class]]);
        MTRDescriptorClusterDeviceTypeStruct * deviceType = item;
        XCTAssertEqualObjects(deviceType.deviceType, @(100));
        XCTAssertEqualObjects(deviceType.revision, @(17));
    }

    XCTAssertNil(report.error);
}

- (void)test033_CommandResponse
{
    NSDictionary * input = @{
        MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0)
                                                            clusterID:@(MTRClusterIDTypeGroupsID)
                                                            commandID:@(MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID)],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(0), // Status
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(1),
                    },
                },
                @{
                    MTRContextTagKey : @(1), // GroupID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(0x176),
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * payload = [[MTRGroupsClusterAddGroupResponseParams alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(payload);
    XCTAssertEqualObjects(payload.status, @(1));
    XCTAssertEqualObjects(payload.groupID, @(0x176));
}

- (void)test034_CommandResponseWrongCommandID
{
    NSDictionary * input = @{
        MTRCommandPathKey :
            [MTRCommandPath commandPathWithEndpointID:@(0)
                                            clusterID:@(MTRClusterIDTypeGroupsID)
                                            commandID:@(MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID)],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(0), // Status
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(1),
                    },
                },
                @{
                    MTRContextTagKey : @(1), // GroupID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(0x176),
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * payload = [[MTRGroupsClusterAddGroupResponseParams alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(payload);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test035_CommandResponseWrongClusterID
{
    NSDictionary * input = @{
        MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0)
                                                            clusterID:@(MTRClusterIDTypeOnOffID)
                                                            commandID:@(MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID)],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(0), // Status
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(1),
                    },
                },
                @{
                    MTRContextTagKey : @(1), // GroupID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(0x176),
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * payload = [[MTRGroupsClusterAddGroupResponseParams alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(payload);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)test036_CommandResponseWrongData
{
    NSDictionary * input = @{
        MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0)
                                                            clusterID:@(MTRClusterIDTypeGroupsID)
                                                            commandID:@(MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID)],
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[
                @{
                    MTRContextTagKey : @(0), // Status
                    MTRDataKey : @ {
                        MTRTypeKey : MTRSignedIntegerValueType, // Wrong data type
                        MTRValueKey : @(1),
                    },
                },
                @{
                    MTRContextTagKey : @(1), // GroupID
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(0x176),
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * payload = [[MTRGroupsClusterAddGroupResponseParams alloc] initWithResponseValue:input error:&error];
    XCTAssertNil(payload);
    XCTAssertNotNil(error);
    XCTAssertEqual(error.code, MTRErrorCodeSchemaMismatch);
}

- (void)globalListAttributeHelper:(NSNumber *)clusterID
{
    // clusterID, AcceptedCommandList
    NSDictionary * input = @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:clusterID attributeID:@(0xFFF9)],
        MTRDataKey : @ {
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : @[
                @{
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(5),
                    },
                },
                @{
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(8),
                    },
                },
            ],
        },
    };

    NSError * error;
    __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:input error:&error];
    XCTAssertNotNil(report);
    XCTAssertNil(error);

    XCTAssertEqualObjects(input[MTRAttributePathKey], report.path);
    XCTAssertNotNil(report.value);
    XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);

    NSArray * array = report.value;
    XCTAssertTrue([array[0] isKindOfClass:[NSNumber class]]);
    XCTAssertEqualObjects(array[0], @(5));

    XCTAssertTrue([array[1] isKindOfClass:[NSNumber class]]);
    XCTAssertEqualObjects(array[1], @(8));

    XCTAssertNil(report.error);
}

- (void)test037_GlobalListAttributeStandardCluster
{
    // OnOff cluster.
    [self globalListAttributeHelper:@(0x0006)];
}

- (void)test037_GlobalListAttributeManufacturerSpecific
{
    // Manufacturer specific cluster IDs start at 0xFC00.
    [self globalListAttributeHelper:@(0xFFF4FD01)];
}

@end

/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#pragma once

#import <Matter/Matter.h>

#include <commands/common/Command.h>

class PrintDeviceCommand : public CHIPCommandBridge {
public:
    PrintDeviceCommand()
        : CHIPCommandBridge("print", "Prints the endpoints, clusters, commands, attributes, and events that are "
                                     "available on a device instance.")
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId, "The Node ID of the device whose details will be printed.");
        AddArgument("endpoint-id", 0, UINT16_MAX, &mEndPointId, "If provided, restrict the output to this single endpoint.");
    }

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override
    {
        auto * device = DeviceWithNodeId(mNodeId);
        VerifyOrReturnError(device != nil, CHIP_ERROR_INCORRECT_STATE);

        AddDeviceToBuffer(device);

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

    // Our command is synchronous, so no need to wait.
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::kZero; }

private:
    void AddDeviceToBuffer(MTRDevice * device)
    {
        mLogBuffer = [NSMutableString string];

        __auto_type * rootEndPointID = @(0);
        __auto_type * endpoints = ReadAsNumbers(
            device,
            rootEndPointID,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeClusterDescriptorAttributePartsListID));

        AddEndPointToBuffer(device, rootEndPointID);
        for (NSNumber * endpoint in endpoints) {
            AddEndPointToBuffer(device, endpoint);
        }

        // Everything should be buffered now, prints it.
        NSLog(@"%@", mLogBuffer);
    }

    void AddEndPointToBuffer(MTRDevice * device, NSNumber * endpoint)
    {
        if (mEndPointId.HasValue() && ![endpoint isEqual:@(mEndPointId.Value())]) {
            return;
        }

        __auto_type * deviceType = GetDeviceType(device, endpoint);
        __auto_type * clusters = ReadAsNumbers(
            device,
            endpoint,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeClusterDescriptorAttributeServerListID));

        AddToBuffer(@"\n");
        AddToBuffer(@"EndPoint %@ [%@]", endpoint, deviceType);
        AddToBuffer(@"\t┃");
        for (NSNumber * cluster in clusters) {
            AddClusterToBuffer(device, endpoint, cluster);
        }
    }

    void AddClusterToBuffer(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster)
    {
        __auto_type * clusterName = MTRClusterNameForID(static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue));
        __auto_type * clusterRevision = ReadAsNumber(
            device,
            endpoint,
            cluster,
            @(MTRAttributeIDTypeGlobalAttributeClusterRevisionID));
        __auto_type * features = GetFeatureMap(device, endpoint);

        AddToBuffer(@"\t┃");
        AddToBuffer(@"\t┣━━ %@ [Revision: %@ | Feature Map: %@ | ID: %@ (%@)]", clusterName, clusterRevision, features, cluster, FormatID(cluster));
        AddCommandsToBuffer(device, endpoint, cluster);
        AddAttributesToBuffer(device, endpoint, cluster);
        AddEventsToBuffer(device, endpoint, cluster);
    }

    void AddCommandsToBuffer(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster)
    {
        __auto_type * commands = ReadAsNumbers(
            device,
            endpoint,
            cluster,
            @(MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID));

        if (commands.count == 0) {
            return;
        }

        AddClusterToBufferMemberHeader(@"Commands");

        NSNumber * longestCommand = @(0);
        for (NSNumber * command in commands) {
            NSString * name = MTRRequestCommandNameForID(
                static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue),
                static_cast<MTRCommandIDType>(command.unsignedLongLongValue));

            longestCommand = @(MAX(longestCommand.unsignedLongLongValue, name.length));
        }

        for (NSNumber * command in commands) {
            NSString * name = MTRRequestCommandNameForID(
                static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue),
                static_cast<MTRCommandIDType>(command.unsignedLongLongValue));
            AddClusterToBufferMemberContent(name, command, longestCommand, command == commands.lastObject);
        }
    }

    void AddAttributesToBuffer(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster)
    {
        __auto_type * attributes = ReadAsNumbers(
            device,
            endpoint,
            cluster,
            @(MTRAttributeIDTypeGlobalAttributeAttributeListID));

        if (attributes.count == 0) {
            return;
        }

        AddClusterToBufferMemberHeader(@"Attributes");

        NSNumber * longestAttribute = @(0);
        for (NSNumber * attribute in attributes) {
            NSString * name = MTRAttributeNameForID(
                static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue),
                static_cast<MTRAttributeIDType>(attribute.unsignedLongLongValue));

            longestAttribute = @(MAX(longestAttribute.unsignedLongLongValue, name.length));
        }

        for (NSNumber * attribute in attributes) {
            NSString * name = MTRAttributeNameForID(
                static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue),
                static_cast<MTRAttributeIDType>(attribute.unsignedLongLongValue));
            AddClusterToBufferMemberContent(name, attribute, longestAttribute, attribute == attributes.lastObject);
        }
    }

    void AddEventsToBuffer(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster)
    {
        __auto_type * events = ReadAsNumbers(
            device,
            endpoint,
            cluster,
            @(0xFFFA));

        if (events.count == 0) {
            return;
        }

        AddClusterToBufferMemberHeader(@"Events");

        NSNumber * longestEvent = @(0);
        for (NSNumber * event in events) {
            NSString * name = MTREventNameForID(
                static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue),
                static_cast<MTREventIDType>(event.unsignedLongLongValue));

            longestEvent = @(MAX(longestEvent.unsignedLongLongValue, name.length));
        }

        for (NSNumber * event in events) {
            NSString * name = MTREventNameForID(
                static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue),
                static_cast<MTREventIDType>(event.unsignedLongLongValue));
            AddClusterToBufferMemberContent(name, event, longestEvent, event == events.lastObject);
        }
    }

    void AddClusterToBufferMemberHeader(NSString * header)
    {
        AddToBuffer(@"\t┃   ┃");
        AddToBuffer(@"\t┃   ┣━ %@", header);
    }

    void AddClusterToBufferMemberContent(NSString * name, NSNumber * value, NSNumber * longest, BOOL isLast)
    {
        NSUInteger padCount = (NSUInteger) MAX(0, (NSInteger) longest.unsignedLongLongValue - (NSInteger) name.length);
        NSString * padding = [@"" stringByPaddingToLength:padCount withString:@"╶" startingAtIndex:0];

        if (isLast) {
            AddToBuffer(@"\t┃   ┃   ┗━━ %@%@ [ID: %@ (%@)]", name, padding, value, FormatID(value));
        } else {
            AddToBuffer(@"\t┃   ┃   ┣━━ %@%@ [ID: %@ (%@)]", name, padding, value, FormatID(value));
        }
    }

    NSString * GetDeviceType(MTRDevice * device, NSNumber * endpoint)
    {
        __auto_type * deviceTypes = ReadAsStructures(
            device,
            endpoint,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID));

        NSString * deviceTypeString = @"";

        for (NSDictionary * value in deviceTypes) {
            NSNumber * deviceTypeID = value[@(0)];
            NSNumber * deviceTypeRevision = value[@(1)];
            __auto_type * deviceType = [MTRDeviceType deviceTypeForID:deviceTypeID];
            deviceTypeString = [deviceTypeString stringByAppendingFormat:@" (%@, Revision: %@) ", deviceType.name, deviceTypeRevision];
        }

        return deviceTypeString;
    }

    NSString * GetFeatureMap(MTRDevice * device, NSNumber * endpoint)
    {
        __auto_type * featureMap = ReadAsNumber(
            device,
            endpoint,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeGlobalAttributeFeatureMapID));

        // Ideally, we would like to return the features as strings, but there is no such methods to get the feature names.
        return [NSString stringWithFormat:@"%@", featureMap];
    }

    NSNumber * ReadResponseAsNumber(NSDictionary * response)
    {
        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRSignedIntegerValueType] && ![responseType isEqual:MTRUnsignedIntegerValueType]) {
            return nil;
        }

        return response[MTRValueKey];
    }

    NSArray<NSNumber *> * ReadResponseAsNumbers(NSDictionary * response)
    {
        NSMutableArray<NSNumber *> * values = [NSMutableArray array];

        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRArrayValueType]) {
            return nil;
        }

        NSArray * responseValues = response[MTRValueKey];
        for (NSDictionary * responseValue in responseValues) {
            __auto_type * value = ReadResponseAsNumber(responseValue[MTRDataKey]);
            [values addObject:value];
        }

        return values;
    }

    NSDictionary * ReadResponseAsStructure(NSDictionary * response)
    {
        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRStructureValueType]) {
            return nil;
        }

        NSMutableDictionary * value = [NSMutableDictionary dictionary];

        NSArray * responseValues = response[MTRValueKey];
        for (NSDictionary * responseValue in responseValues) {
            value[responseValue[MTRContextTagKey]] = ReadResponseAsNumber(responseValue[MTRDataKey]);
        }

        return value;
    }

    NSArray<NSDictionary *> * ReadResponseAsStructures(NSDictionary * response)
    {
        NSMutableArray<NSDictionary *> * values = [NSMutableArray array];

        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRArrayValueType]) {
            return nil;
        }

        NSArray * responseValues = response[MTRValueKey];
        for (NSDictionary * responseValue in responseValues) {
            NSDictionary * value = ReadResponseAsStructure(responseValue[MTRDataKey]);
            [values addObject:value];
        }

        return values;
    }

    NSNumber * ReadAsNumber(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSNumber * attribute)
    {
        __auto_type * response = [device readAttributeWithEndpointID:endpoint clusterID:cluster attributeID:attribute params:nil];
        return ReadResponseAsNumber(response);
    }

    NSArray<NSNumber *> * ReadAsNumbers(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSNumber * attribute)
    {
        __auto_type * response = [device readAttributeWithEndpointID:endpoint clusterID:cluster attributeID:attribute params:nil];
        return ReadResponseAsNumbers(response);
    }

    NSArray<NSDictionary *> * ReadAsStructures(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSNumber * attribute)
    {
        __auto_type * response = [device readAttributeWithEndpointID:endpoint clusterID:cluster attributeID:attribute params:nil];
        return ReadResponseAsStructures(response);
    }

    static inline NSString * FormatID(NSNumber * value)
    {
        unsigned long long v = value.unsignedLongLongValue;

        if (v <= 0xFFFFULL) {
            return [NSString stringWithFormat:@"0x%04llX", v];
        } else if (v <= 0xFFFFFFFFULL) {
            return [NSString stringWithFormat:@"0x%08llX", v];
        } else {
            return [NSString stringWithFormat:@"0x%016llX", v];
        }
    }

    void AddToBuffer(NSString * format, ...) NS_FORMAT_FUNCTION(2, 3)
    {
        va_list args;
        va_start(args, format);
        __auto_type * line = [[NSString alloc] initWithFormat:format arguments:args];
        va_end(args);

        [mLogBuffer appendString:line];
        [mLogBuffer appendString:@"\n"];
    }

    chip::NodeId mNodeId;
    chip::Optional<chip::EndpointId> mEndPointId;
    NSMutableString * mLogBuffer;
};

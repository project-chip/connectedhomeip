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

#import "../common/DeviceDelegate.h"
#import "MTRError_Utils.h"

#include <commands/common/Command.h>

NS_ASSUME_NONNULL_BEGIN

class PrintDeviceCommand : public CHIPCommandBridge {
public:
    PrintDeviceCommand()
        : CHIPCommandBridge("print", "Prints the endpoints, clusters, commands and attributes that are "
                                     "available on a device instance.")
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId, "The Node ID of the device whose details will be printed.");
        AddArgument("endpoint-id", 0, UINT16_MAX, &mEndPointId, "If provided, restrict the output to this single endpoint.");
    }

protected:
    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override
    {
        auto * controller = CurrentCommissioner();
        VerifyOrReturnError(nil != controller, CHIP_ERROR_INCORRECT_STATE);

        auto * device = [MTRDevice deviceWithNodeID:@(mNodeId) controller:controller];
        VerifyOrReturnError(nil != device, CHIP_ERROR_INCORRECT_STATE);

        DeviceWithNodeId(mNodeId);

        if (device.deviceCachePrimed) {
            ReturnErrorOnFailure(PrintDevice(device));
            SetCommandExitStatus(CHIP_NO_ERROR);
        } else {
            auto onDeviceCachePrimedBlock = ^{
                CHIP_ERROR error = PrintDevice(device);

                [device removeDelegate:mDeviceDelegate];
                mDeviceDelegate = nil;
                SetCommandExitStatus(error);
            };

            mDeviceDelegate = [[DeviceDelegate alloc] init];
            mDeviceDelegate.onDeviceCachePrimed = onDeviceCachePrimedBlock;
            auto deviceDelegateDispatchQueue = dispatch_queue_create("com.chip.devicedelegate", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            [device addDelegate:mDeviceDelegate queue:deviceDelegateDispatchQueue];
        }

        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }

private:
    // Error Descriptions
    NSString * const kErrorDomain = @"Error";
    NSInteger const kErrorCode = 0;
    NSString * const kErrorNotAnArray = @"Expected an array, but received a different type.";
    NSString * const kErrorNotANumber = @"Expected a numeric value, but received a different type.";
    NSString * const kErrorNotAStructure = @"Expected a structure, but received a different type.";
    NSString * const kErrorNilArray = @"Expected an array, but received a nil response.";
    NSString * const kErrorNilNumber = @"Expected a numeric value, but received a nil response.";
    NSString * const kErrorNilStructure = @"Expected a structure, but received a nil response.";

    CHIP_ERROR PrintDevice(MTRDevice * device)
    {
        CHIP_ERROR error = CHIP_NO_ERROR;

        NSError * internalError = nil;
        AddDeviceToBuffer(device, &internalError);
        if (nil != internalError) {
            error = MTRErrorToCHIPErrorCode(internalError);
            ChipLogError(chipTool, "Error while reading device configuration: %s", internalError.localizedDescription.UTF8String);
        }

        return error;
    }

    void AddDeviceToBuffer(MTRDevice * device, NSError * _Nonnull __autoreleasing * error)
    {
        mLogBuffer = [NSMutableString string];

        auto * rootEndPointID = @(0);

        auto * endpoints = ReadAsNumbers(
            device,
            rootEndPointID,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeClusterDescriptorAttributePartsListID),
            error);
        VerifyOrReturn(nil == *error);

        AddEndPointToBuffer(device, rootEndPointID, error);
        VerifyOrReturn(nil == *error);

        for (NSNumber * endpoint in endpoints) {
            AddEndPointToBuffer(device, endpoint, error);
            VerifyOrReturn(nil == *error);
        }

        // Everything should be buffered now, print it.
        NSLog(@"%@", mLogBuffer);
    }

    void AddEndPointToBuffer(MTRDevice * device, NSNumber * endpoint, NSError * _Nonnull __autoreleasing * error)
    {
        if (mEndPointId.HasValue() && ![endpoint isEqual:@(mEndPointId.Value())]) {
            return;
        }

        auto * deviceType = GetDeviceType(device, endpoint, error);
        VerifyOrReturn(nil == *error);

        auto * clusters = ReadAsNumbers(
            device,
            endpoint,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeClusterDescriptorAttributeServerListID),
            error);
        VerifyOrReturn(nil == *error);

        AddToBuffer(@"\n");
        AddToBuffer(@"Endpoint %@ [%@]", endpoint, deviceType);
        AddToBuffer(@"\t┃");
        for (NSNumber * cluster in clusters) {
            AddClusterToBuffer(device, endpoint, cluster, error);
            VerifyOrReturn(nil == *error);
        }
    }

    void AddClusterToBuffer(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSError * _Nonnull __autoreleasing * error)
    {
        auto * clusterName = MTRClusterNameForID(static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue));
        auto * clusterRevision = ReadAsNumber(
            device,
            endpoint,
            cluster,
            @(MTRAttributeIDTypeGlobalAttributeClusterRevisionID),
            error);
        VerifyOrReturn(nil == *error);

        auto * features = GetFeatureMap(device, endpoint, error);
        VerifyOrReturn(nil == *error);

        AddToBuffer(@"\t┃");
        AddToBuffer(@"\t┣━━ %@ [Revision: %@ | Feature Map: %@ | ID: %@ (%@)]", clusterName, clusterRevision, features, cluster, FormatID(cluster));
        AddCommandsToBuffer(device, endpoint, cluster, error);
        VerifyOrReturn(nil == *error);

        AddAttributesToBuffer(device, endpoint, cluster, error);
        VerifyOrReturn(nil == *error);
    }

    void AddCommandsToBuffer(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSError * _Nonnull __autoreleasing * error)
    {
        auto * commands = ReadAsNumbers(
            device,
            endpoint,
            cluster,
            @(MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID),
            error);
        VerifyOrReturn(nil == *error);

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

    void AddAttributesToBuffer(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSError * _Nonnull __autoreleasing * error)
    {
        auto * attributes = ReadAsNumbers(
            device,
            endpoint,
            cluster,
            @(MTRAttributeIDTypeGlobalAttributeAttributeListID),
            error);
        VerifyOrReturn(nil == *error);

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

    void AddClusterToBufferMemberHeader(NSString * header)
    {
        AddToBuffer(@"\t┃   ┃");
        AddToBuffer(@"\t┃   ┣━ %@", header);
    }

    void AddClusterToBufferMemberContent(NSString * name, NSNumber * value, NSNumber * longest, BOOL isLast)
    {
        NSUInteger padCount = static_cast<NSUInteger>(MAX(0, static_cast<NSInteger>(longest.unsignedLongLongValue) - static_cast<NSInteger>(name.length)));
        NSString * padding = [@"" stringByPaddingToLength:padCount withString:@"╶" startingAtIndex:0];

        if (isLast) {
            AddToBuffer(@"\t┃   ┃   ┗━━ %@%@ [ID: %@ (%@)]", name, padding, value, FormatID(value));
        } else {
            AddToBuffer(@"\t┃   ┃   ┣━━ %@%@ [ID: %@ (%@)]", name, padding, value, FormatID(value));
        }
    }

    NSString * _Nullable GetDeviceType(MTRDevice * device, NSNumber * endpoint, NSError * _Nonnull __autoreleasing * error)
    {
        auto * deviceTypes = ReadAsStructures(
            device,
            endpoint,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID),
            error);
        VerifyOrReturnValue(nil == *error, nil);

        NSString * deviceTypeString = @"";

        for (NSDictionary * value in deviceTypes) {
            NSNumber * deviceTypeID = value[@(0)];
            NSNumber * deviceTypeRevision = value[@(1)];
            auto * deviceType = [MTRDeviceType deviceTypeForID:deviceTypeID];
            deviceTypeString = [deviceTypeString stringByAppendingFormat:@" (%@, Revision: %@) ", deviceType.name, deviceTypeRevision];
        }

        return deviceTypeString;
    }

    NSString * _Nullable GetFeatureMap(MTRDevice * device, NSNumber * endpoint, NSError * _Nonnull __autoreleasing * error)
    {
        auto * featureMap = ReadAsNumber(
            device,
            endpoint,
            @(MTRClusterIDTypeDescriptorID),
            @(MTRAttributeIDTypeGlobalAttributeFeatureMapID),
            error);
        VerifyOrReturnValue(nil == *error, nil);

        // Ideally, we would like to return the features as strings, but there are no such methods to get the feature names.
        return [NSString stringWithFormat:@"%@", featureMap];
    }

    NSNumber * _Nullable ReadResponseAsNumber(NSDictionary * response, NSError * _Nonnull __autoreleasing * error)
    {
        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRSignedIntegerValueType] && ![responseType isEqual:MTRUnsignedIntegerValueType]) {
            *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:@ { NSLocalizedDescriptionKey : kErrorNotANumber }];
            return nil;
        }

        return response[MTRValueKey];
    }

    NSArray<NSNumber *> * _Nullable ReadResponseAsNumbers(NSDictionary * response, NSError * _Nonnull __autoreleasing * error)
    {
        NSMutableArray<NSNumber *> * values = [NSMutableArray array];

        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRArrayValueType]) {
            *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:@ { NSLocalizedDescriptionKey : kErrorNotAnArray }];
            return nil;
        }

        NSArray * responseValues = response[MTRValueKey];
        for (NSDictionary * responseValue in responseValues) {
            auto * value = ReadResponseAsNumber(responseValue[MTRDataKey], error);
            if (*error) {
                // Bubble the underlying error up to the caller unchanged
                return nil;
            }
            [values addObject:value];
        }

        return values;
    }

    NSDictionary * _Nullable ReadResponseAsStructure(NSDictionary * response, NSError * _Nonnull __autoreleasing * error)
    {
        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRStructureValueType]) {
            *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:@ { NSLocalizedDescriptionKey : kErrorNotAStructure }];
            return nil;
        }

        NSMutableDictionary * value = [NSMutableDictionary dictionary];

        NSArray * responseValues = response[MTRValueKey];
        for (NSDictionary * responseValue in responseValues) {
            auto * numberValue = ReadResponseAsNumber(responseValue[MTRDataKey], error);
            if (*error) {
                // Bubble the underlying error up to the caller unchanged
                return nil;
            }
            value[responseValue[MTRContextTagKey]] = numberValue;
        }

        return value;
    }

    NSArray<NSDictionary *> * _Nullable ReadResponseAsStructures(NSDictionary * response, NSError * _Nonnull __autoreleasing * error)
    {
        NSMutableArray<NSDictionary *> * values = [NSMutableArray array];

        NSString * responseType = response[MTRTypeKey];
        if (![responseType isEqual:MTRArrayValueType]) {
            *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:@ { NSLocalizedDescriptionKey : kErrorNotAnArray }];
            return nil;
        }

        NSArray * responseValues = response[MTRValueKey];
        for (NSDictionary * responseValue in responseValues) {
            auto * value = ReadResponseAsStructure(responseValue[MTRDataKey], error);
            if (*error) {
                // Bubble the underlying error up to the caller unchanged
                return nil;
            }
            [values addObject:value];
        }

        return values;
    }

    NSNumber * _Nullable ReadAsNumber(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSNumber * attribute, NSError * _Nonnull __autoreleasing * error)
    {
        auto * response = [device readAttributeWithEndpointID:endpoint clusterID:cluster attributeID:attribute params:nil];
        if (nil == response) {
            *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:@ { NSLocalizedDescriptionKey : kErrorNilNumber }];
            return nil;
        }

        NSError * innerError = nil;
        auto * value = ReadResponseAsNumber(response, &innerError);
        VerifyOrReturnValue(nil == innerError, nil, WrapError(innerError, endpoint, cluster, attribute, error));

        return value;
    }

    NSArray<NSNumber *> * _Nullable ReadAsNumbers(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSNumber * attribute, NSError * _Nonnull __autoreleasing * error)
    {
        auto * response = [device readAttributeWithEndpointID:endpoint clusterID:cluster attributeID:attribute params:nil];
        if (nil == response) {
            *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:@ { NSLocalizedDescriptionKey : kErrorNilArray }];
            return nil;
        }

        NSError * innerError = nil;
        auto * value = ReadResponseAsNumbers(response, &innerError);
        VerifyOrReturnValue(nil == innerError, nil, WrapError(innerError, endpoint, cluster, attribute, error));

        return value;
    }

    NSArray<NSDictionary *> * _Nullable ReadAsStructures(MTRDevice * device, NSNumber * endpoint, NSNumber * cluster, NSNumber * attribute, NSError * _Nonnull __autoreleasing * error)
    {
        auto * response = [device readAttributeWithEndpointID:endpoint clusterID:cluster attributeID:attribute params:nil];
        if (nil == response) {
            *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:@ { NSLocalizedDescriptionKey : kErrorNilStructure }];
            return nil;
        }

        NSError * innerError = nil;
        auto * value = ReadResponseAsStructures(response, &innerError);
        VerifyOrReturnValue(nil == innerError, nil, WrapError(innerError, endpoint, cluster, attribute, error));

        return value;
    }

    void WrapError(NSError * innerError, NSNumber * endpoint, NSNumber * cluster, NSNumber * attribute, NSError * _Nonnull __autoreleasing * error)
    {
        auto * clusterName = MTRClusterNameForID(static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue));
        auto * attributeName = MTRAttributeNameForID(
            static_cast<MTRClusterIDType>(cluster.unsignedLongLongValue),
            static_cast<MTRAttributeIDType>(attribute.unsignedLongLongValue));
        auto * userInfo = @ { NSLocalizedDescriptionKey : [NSString stringWithFormat:@"Endpoint: %@ - Cluster: %@ - Attribute: %@ : %@", endpoint, clusterName, attributeName, innerError.localizedDescription] };
        *error = [NSError errorWithDomain:kErrorDomain code:kErrorCode userInfo:userInfo];
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
        auto * line = [[NSString alloc] initWithFormat:format arguments:args];
        va_end(args);

        [mLogBuffer appendString:line];
        [mLogBuffer appendString:@"\n"];
    }

    chip::NodeId mNodeId;
    chip::Optional<chip::EndpointId> mEndPointId;
    NSMutableString * mLogBuffer;
    DeviceDelegate * mDeviceDelegate = nil;
};

NS_ASSUME_NONNULL_END

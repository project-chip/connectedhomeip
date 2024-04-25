/*
 *   Copyright (c) 2023 Project CHIP Authors
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
 *
 */

#include "RemoteDataModelLogger.h"

#import "MTRError_Utils.h"
#import <objc/runtime.h>

#include <json/json.h>
#include <lib/support/SafeInt.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>

constexpr char kClusterIdKey[] = "clusterId";
constexpr char kEndpointIdKey[] = "endpointId";
constexpr char kAttributeIdKey[] = "attributeId";
constexpr char kCommandIdKey[] = "commandId";
constexpr char kErrorIdKey[] = "error";
constexpr char kClusterErrorIdKey[] = "clusterError";
constexpr char kValueKey[] = "value";
constexpr char kNodeIdKey[] = "nodeId";
constexpr char kLogContentIdKey[] = "logContent";

constexpr char kBase64Header[] = "base64:";

namespace {
RemoteDataModelLoggerDelegate * gDelegate;

std::string JsonToString(Json::Value & json)
{
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    return writer.write(json);
}

CHIP_ERROR LogError(Json::Value & value, const chip::app::StatusIB & status)
{
    if (status.mClusterStatus.HasValue()) {
        auto statusValue = status.mClusterStatus.Value();
        value[kClusterErrorIdKey] = statusValue;
    }

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    auto statusName = chip::Protocols::InteractionModel::StatusName(status.mStatus);
    value[kErrorIdKey] = statusName;
#else
    auto statusName = status.mStatus;
    value[kErrorIdKey] = chip::to_underlying(statusName);
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

    auto valueStr = JsonToString(value);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR AsJsonValue(id value, Json::Value & jsonValue)
{
    if (value == nil) {
        jsonValue = Json::nullValue;
    } else if ([value isKindOfClass:[NSNumber class]]) {
        if (CFNumberIsFloatType((CFNumberRef) value)) {
            jsonValue = [value doubleValue];
        } else if ([[value stringValue] hasPrefix:@"-"]) {
            jsonValue = [value longLongValue];
        } else {
            jsonValue = [value unsignedLongLongValue];
        }
    } else if ([value isKindOfClass:[NSArray class]]) {
        jsonValue = Json::arrayValue;

        NSArray * array = value;
        for (id element in array) {
            Json::Value jsonElement;
            VerifyOrDie(CHIP_NO_ERROR == AsJsonValue(element, jsonElement));
            jsonValue.append(jsonElement);
        }
    } else if ([value isKindOfClass:[NSDictionary class]]) {
        jsonValue = Json::ValueType::objectValue;

        NSDictionary * dict = value;
        for (id key in dict) {
            Json::Value jsonElement;
            VerifyOrDie(CHIP_NO_ERROR == AsJsonValue([dict objectForKey:key], jsonElement));
            jsonValue[[key UTF8String]] = jsonElement;
        }
    } else if ([value isKindOfClass:[NSData class]]) {
        NSData * data = value;
        data = [data base64EncodedDataWithOptions:0];
        auto base64Str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        auto prefix = [NSString stringWithUTF8String:kBase64Header];
        auto base64PrefixedStr = [prefix stringByAppendingString:base64Str];
        jsonValue = [base64PrefixedStr UTF8String];
    } else if ([value isKindOfClass:[NSString class]]) {
        jsonValue = [value UTF8String];
    } else if ([value isKindOfClass:[NSObject class]]) {
        jsonValue = Json::ValueType::objectValue;

        unsigned int numberOfProperties;
        objc_property_t * properties = class_copyPropertyList([value class], &numberOfProperties);
        for (NSUInteger i = 0; i < numberOfProperties; i++) {
            objc_property_t property = properties[i];
            NSString * key = [[NSString alloc] initWithUTF8String:property_getName(property)];

            Json::Value jsonElement;
            VerifyOrDie(CHIP_NO_ERROR == AsJsonValue([value valueForKey:key], jsonElement));
            jsonValue[[key UTF8String]] = jsonElement;
        }
        free(properties);
    } else {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    return CHIP_NO_ERROR;
}

} // namespace

namespace RemoteDataModelLogger {
CHIP_ERROR LogAttributeAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId, id result)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kEndpointIdKey] = [endpointId unsignedLongLongValue];
    value[kClusterIdKey] = [clusterId unsignedLongLongValue];
    value[kAttributeIdKey] = [attributeId unsignedLongLongValue];

    Json::Value jsonValue;
    VerifyOrDie(CHIP_NO_ERROR == AsJsonValue(result, jsonValue));
    value[kValueKey] = jsonValue;

    auto valueStr = JsonToString(value);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogCommandAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, id result)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kEndpointIdKey] = [endpointId unsignedLongLongValue];
    value[kClusterIdKey] = [clusterId unsignedLongLongValue];
    value[kCommandIdKey] = [commandId unsignedLongLongValue];

    Json::Value jsonValue;
    VerifyOrDie(CHIP_NO_ERROR == AsJsonValue(result, jsonValue));
    value[kValueKey] = jsonValue;

    auto valueStr = JsonToString(value);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogAttributeErrorAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId, NSError * error)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kEndpointIdKey] = [endpointId unsignedLongLongValue];
    value[kClusterIdKey] = [clusterId unsignedLongLongValue];
    value[kAttributeIdKey] = [attributeId unsignedLongLongValue];

    auto err = MTRErrorToCHIPErrorCode(error);
    auto status = chip::app::StatusIB(err);
    return LogError(value, status);
}

CHIP_ERROR LogCommandErrorAsJSON(NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, NSError * error)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kEndpointIdKey] = [endpointId unsignedLongLongValue];
    value[kClusterIdKey] = [clusterId unsignedLongLongValue];
    value[kCommandIdKey] = [commandId unsignedLongLongValue];

    auto err = MTRErrorToCHIPErrorCode(error);
    auto status = chip::app::StatusIB(err);
    return LogError(value, status);
}

CHIP_ERROR LogGetCommissionerNodeId(NSNumber * value)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value rootValue;
    rootValue[kValueKey] = Json::Value();
    rootValue[kValueKey][kNodeIdKey] = [value unsignedLongLongValue];

    auto valueStr = JsonToString(rootValue);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogBdxDownload(NSString * content, NSError * error)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value rootValue;
    rootValue[kValueKey] = Json::Value();

    Json::Value jsonValue;
    VerifyOrDie(CHIP_NO_ERROR == AsJsonValue(content, jsonValue));
    rootValue[kValueKey][kLogContentIdKey] = jsonValue;

    if (error) {
        auto err = MTRErrorToCHIPErrorCode(error);
        auto status = chip::app::StatusIB(err);

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
        auto statusName = chip::Protocols::InteractionModel::StatusName(status.mStatus);
        rootValue[kValueKey][kErrorIdKey] = statusName;
#else
        auto statusName = status.mStatus;
        rootValue[kValueKey][kErrorIdKey] = chip::to_underlying(statusName);
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    }

    auto valueStr = JsonToString(rootValue);
    return gDelegate->LogJSON(valueStr.c_str());
}

void SetDelegate(RemoteDataModelLoggerDelegate * delegate) { gDelegate = delegate; }
}; // namespace RemoteDataModelLogger

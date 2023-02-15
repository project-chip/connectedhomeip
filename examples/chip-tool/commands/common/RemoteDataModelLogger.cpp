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

#include <lib/support/SafeInt.h>
#include <lib/support/jsontlv/TlvJson.h>

constexpr const char * kClusterIdKey      = "clusterId";
constexpr const char * kEndpointIdKey     = "endpointId";
constexpr const char * kAttributeIdKey    = "attributeId";
constexpr const char * kEventIdKey        = "eventId";
constexpr const char * kCommandIdKey      = "commandId";
constexpr const char * kErrorIdKey        = "error";
constexpr const char * kClusterErrorIdKey = "clusterError";
constexpr const char * kValueKey          = "value";
constexpr const char * kNodeIdKey         = "nodeId";

namespace {
RemoteDataModelLoggerDelegate * gDelegate;

CHIP_ERROR LogError(Json::Value & value, const chip::app::StatusIB & status)
{
    if (status.mClusterStatus.HasValue())
    {
        auto statusValue          = status.mClusterStatus.Value();
        value[kClusterErrorIdKey] = statusValue;
    }

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    auto statusName    = chip::Protocols::InteractionModel::StatusName(status.mStatus);
    value[kErrorIdKey] = statusName;
#else
    auto statusName    = status.mStatus;
    value[kErrorIdKey] = chip::to_underlying(statusName);
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

    auto valueStr = chip::JsonToString(value);
    return gDelegate->LogJSON(valueStr.c_str());
}
} // namespace

namespace RemoteDataModelLogger {
CHIP_ERROR LogAttributeAsJSON(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kClusterIdKey]   = path.mClusterId;
    value[kEndpointIdKey]  = path.mEndpointId;
    value[kAttributeIdKey] = path.mAttributeId;

    chip::TLV::TLVReader reader;
    reader.Init(*data);
    ReturnErrorOnFailure(chip::TlvToJson(reader, value));

    auto valueStr = chip::JsonToString(value);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogErrorAsJSON(const chip::app::ConcreteDataAttributePath & path, const chip::app::StatusIB & status)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kClusterIdKey]   = path.mClusterId;
    value[kEndpointIdKey]  = path.mEndpointId;
    value[kAttributeIdKey] = path.mAttributeId;

    return LogError(value, status);
}

CHIP_ERROR LogCommandAsJSON(const chip::app::ConcreteCommandPath & path, chip::TLV::TLVReader * data)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kClusterIdKey]  = path.mClusterId;
    value[kEndpointIdKey] = path.mEndpointId;
    value[kCommandIdKey]  = path.mCommandId;

    chip::TLV::TLVReader reader;
    reader.Init(*data);
    ReturnErrorOnFailure(chip::TlvToJson(reader, value));

    auto valueStr = chip::JsonToString(value);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogErrorAsJSON(const chip::app::ConcreteCommandPath & path, const chip::app::StatusIB & status)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kClusterIdKey]  = path.mClusterId;
    value[kEndpointIdKey] = path.mEndpointId;
    value[kCommandIdKey]  = path.mCommandId;

    return LogError(value, status);
}

CHIP_ERROR LogEventAsJSON(const chip::app::EventHeader & header, chip::TLV::TLVReader * data)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kClusterIdKey]  = header.mPath.mClusterId;
    value[kEndpointIdKey] = header.mPath.mEndpointId;
    value[kEventIdKey]    = header.mPath.mEventId;

    chip::TLV::TLVReader reader;
    reader.Init(*data);
    ReturnErrorOnFailure(chip::TlvToJson(reader, value));

    auto valueStr = chip::JsonToString(value);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogErrorAsJSON(const chip::app::EventHeader & header, const chip::app::StatusIB & status)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    value[kClusterIdKey]  = header.mPath.mClusterId;
    value[kEndpointIdKey] = header.mPath.mEndpointId;
    value[kEventIdKey]    = header.mPath.mEventId;

    return LogError(value, status);
}

CHIP_ERROR LogErrorAsJSON(const CHIP_ERROR & error)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value value;
    chip::app::StatusIB status;
    status.InitFromChipError(error);
    return LogError(value, status);
}

CHIP_ERROR LogGetCommissionerNodeId(chip::NodeId value)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value rootValue;
    rootValue[kValueKey]             = Json::Value();
    rootValue[kValueKey][kNodeIdKey] = value;

    auto valueStr = chip::JsonToString(rootValue);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogDiscoveredNodeData(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    if (!chip::CanCastTo<uint8_t>(nodeData.resolutionData.numIPs))
    {
        ChipLogError(chipTool, "Too many ips.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!chip::CanCastTo<uint64_t>(nodeData.commissionData.rotatingIdLen))
    {
        ChipLogError(chipTool, "Can not convert rotatingId to json format.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    char rotatingId[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
    chip::Encoding::BytesToUppercaseHexString(nodeData.commissionData.rotatingId, nodeData.commissionData.rotatingIdLen, rotatingId,
                                              sizeof(rotatingId));

    Json::Value value;
    value["hostName"]           = nodeData.resolutionData.hostName;
    value["instanceName"]       = nodeData.commissionData.instanceName;
    value["longDiscriminator"]  = nodeData.commissionData.longDiscriminator;
    value["shortDiscriminator"] = ((nodeData.commissionData.longDiscriminator >> 8) & 0x0F);
    value["vendorId"]           = nodeData.commissionData.vendorId;
    value["productId"]          = nodeData.commissionData.productId;
    value["commissioningMode"]  = nodeData.commissionData.commissioningMode;
    value["deviceType"]         = nodeData.commissionData.deviceType;
    value["deviceName"]         = nodeData.commissionData.deviceName;
    value["rotatingId"]         = rotatingId;
    value["rotatingIdLen"]      = static_cast<uint64_t>(nodeData.commissionData.rotatingIdLen);
    value["pairingHint"]        = nodeData.commissionData.pairingHint;
    value["pairingInstruction"] = nodeData.commissionData.pairingInstruction;
    value["supportsTcp"]        = nodeData.resolutionData.supportsTcp;
    value["port"]               = nodeData.resolutionData.port;
    value["numIPs"]             = static_cast<uint8_t>(nodeData.resolutionData.numIPs);

    if (nodeData.resolutionData.mrpRetryIntervalIdle.HasValue())
    {
        value["mrpRetryIntervalIdle"] = nodeData.resolutionData.mrpRetryIntervalIdle.Value().count();
    }

    if (nodeData.resolutionData.mrpRetryIntervalActive.HasValue())
    {
        value["mrpRetryIntervalActive"] = nodeData.resolutionData.mrpRetryIntervalActive.Value().count();
    }

    Json::Value rootValue;
    rootValue[kValueKey] = value;

    auto valueStr = chip::JsonToString(rootValue);
    return gDelegate->LogJSON(valueStr.c_str());
}

void SetDelegate(RemoteDataModelLoggerDelegate * delegate)
{
    gDelegate = delegate;
}
}; // namespace RemoteDataModelLogger

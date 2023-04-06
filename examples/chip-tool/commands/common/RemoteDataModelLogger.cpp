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
constexpr const char * kNOCKey            = "NOC";
constexpr const char * kICACKey           = "ICAC";
constexpr const char * kRCACKey           = "RCAC";
constexpr const char * kIPKKey            = "IPK";

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

CHIP_ERROR LogGetCommissionerRootCertificate(const char * value)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value rootValue;
    rootValue[kValueKey]           = Json::Value();
    rootValue[kValueKey][kRCACKey] = value;

    auto valueStr = chip::JsonToString(rootValue);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogIssueNOCChain(const char * noc, const char * icac, const char * rcac, const char * ipk)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    Json::Value rootValue;
    rootValue[kValueKey]           = Json::Value();
    rootValue[kValueKey][kNOCKey]  = noc;
    rootValue[kValueKey][kICACKey] = icac;
    rootValue[kValueKey][kRCACKey] = rcac;
    rootValue[kValueKey][kIPKKey]  = ipk;

    auto valueStr = chip::JsonToString(rootValue);
    return gDelegate->LogJSON(valueStr.c_str());
}

CHIP_ERROR LogDiscoveredNodeData(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    auto & resolutionData = nodeData.resolutionData;
    auto & commissionData = nodeData.commissionData;

    if (!chip::CanCastTo<uint8_t>(resolutionData.numIPs))
    {
        ChipLogError(chipTool, "Too many ips.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!chip::CanCastTo<uint64_t>(commissionData.rotatingIdLen))
    {
        ChipLogError(chipTool, "Can not convert rotatingId to json format.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    char rotatingId[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
    ReturnErrorOnFailure(chip::Encoding::BytesToUppercaseHexString(commissionData.rotatingId, commissionData.rotatingIdLen,
                                                                   rotatingId, sizeof(rotatingId)));

    Json::Value value;
    value["hostName"]           = resolutionData.hostName;
    value["instanceName"]       = commissionData.instanceName;
    value["longDiscriminator"]  = commissionData.longDiscriminator;
    value["shortDiscriminator"] = ((commissionData.longDiscriminator >> 8) & 0x0F);
    value["vendorId"]           = commissionData.vendorId;
    value["productId"]          = commissionData.productId;
    value["commissioningMode"]  = commissionData.commissioningMode;
    value["deviceType"]         = commissionData.deviceType;
    value["deviceName"]         = commissionData.deviceName;
    value["rotatingId"]         = rotatingId;
    value["rotatingIdLen"]      = static_cast<uint64_t>(commissionData.rotatingIdLen);
    value["pairingHint"]        = commissionData.pairingHint;
    value["pairingInstruction"] = commissionData.pairingInstruction;
    value["supportsTcp"]        = resolutionData.supportsTcp;
    value["port"]               = resolutionData.port;
    value["numIPs"]             = static_cast<uint8_t>(resolutionData.numIPs);

    if (resolutionData.mrpRetryIntervalIdle.HasValue())
    {
        value["mrpRetryIntervalIdle"] = resolutionData.mrpRetryIntervalIdle.Value().count();
    }

    if (resolutionData.mrpRetryIntervalActive.HasValue())
    {
        value["mrpRetryIntervalActive"] = resolutionData.mrpRetryIntervalActive.Value().count();
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

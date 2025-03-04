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
 *
 */

#include "RemoteDataModelLogger.h"

#include <lib/support/SafeInt.h>
#include <lib/support/jsontlv/TlvJson.h>

constexpr char kEventNumberKey[]    = "eventNumber";
constexpr char kDataVersionKey[]    = "dataVersion";
constexpr char kClusterIdKey[]      = "clusterId";
constexpr char kEndpointIdKey[]     = "endpointId";
constexpr char kAttributeIdKey[]    = "attributeId";
constexpr char kEventIdKey[]        = "eventId";
constexpr char kCommandIdKey[]      = "commandId";
constexpr char kErrorIdKey[]        = "error";
constexpr char kClusterErrorIdKey[] = "clusterError";
constexpr char kValueKey[]          = "value";
constexpr char kNodeIdKey[]         = "nodeId";
constexpr char kNOCKey[]            = "NOC";
constexpr char kICACKey[]           = "ICAC";
constexpr char kRCACKey[]           = "RCAC";
constexpr char kIPKKey[]            = "IPK";

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
    if (path.mDataVersion.HasValue())
    {
        value[kDataVersionKey] = path.mDataVersion.Value();
    }

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
    value[kClusterIdKey]   = header.mPath.mClusterId;
    value[kEndpointIdKey]  = header.mPath.mEndpointId;
    value[kEventIdKey]     = header.mPath.mEventId;
    value[kEventNumberKey] = header.mEventNumber;

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
    chip::app::StatusIB status(error);
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

CHIP_ERROR LogDiscoveredNodeData(const chip::Dnssd::CommissionNodeData & nodeData)
{
    VerifyOrReturnError(gDelegate != nullptr, CHIP_NO_ERROR);

    auto & commissionData = nodeData;
    auto & resolutionData = commissionData;

    if (!chip::CanCastTo<uint8_t>(resolutionData.numIPs))
    {
        ChipLogError(NotSpecified, "Too many ips.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!chip::CanCastTo<uint64_t>(commissionData.rotatingIdLen))
    {
        ChipLogError(NotSpecified, "Can not convert rotatingId to json format.");
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
    value["supportsTcpClient"]  = resolutionData.supportsTcpClient;
    value["supportsTcpServer"]  = resolutionData.supportsTcpServer;
    value["port"]               = resolutionData.port;
    value["numIPs"]             = static_cast<uint8_t>(resolutionData.numIPs);

    if (resolutionData.mrpRetryIntervalIdle.has_value())
    {
        value["mrpRetryIntervalIdle"] = resolutionData.mrpRetryIntervalIdle->count();
    }

    if (resolutionData.mrpRetryIntervalActive.has_value())
    {
        value["mrpRetryIntervalActive"] = resolutionData.mrpRetryIntervalActive->count();
    }

    if (resolutionData.mrpRetryActiveThreshold.has_value())
    {
        value["mrpRetryActiveThreshold"] = resolutionData.mrpRetryActiveThreshold->count();
    }

    if (resolutionData.isICDOperatingAsLIT.has_value())
    {
        value["isICDOperatingAsLIT"] = *(resolutionData.isICDOperatingAsLIT);
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

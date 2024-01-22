/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "mpc_attribute_cluster_parser.h"

#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <platform/PlatformManager.h>
#include <stdbool.h>
#include <string.h>
#include <vector>

#include "mpc_attribute_resolver.h"
#include "mpc_attribute_resolver_callbacks.h"
#include "mpc_attribute_resolver_rules.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "mpc_command_sender.hpp"

// Unify Components
#include "sl_log.h"
#include "sl_status.h"

// Includes from other components
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "attribute_resolver_rule.h"
#include "attribute_store.h"
#include "dotdot_mqtt.h"
#include "matter_to_unify_converter.hpp"
#include "zap-types.h"

using namespace std;
using namespace attribute_store;
using namespace chip;
using namespace chip::DeviceLayer;

constexpr char LOG_TAG[] = "mpc_attribute_cluster_parser";

// Local vector where we can add mpc supported cluster list
const static std::vector<chip::ClusterId> mpc_supported_clusters { app::Clusters::OnOff::Id };

typedef sl_status_t (*mpc_resolver_attribute_process_t)(attribute_store_node_t mNode, uint64_t value);

template <typename T>
sl_status_t mpc_parse_decodablelist_attributes(T& listAttr, attribute_store_node_t mNode,
    mpc_resolver_attribute_process_t callback = nullptr)
{
    size_t count = 0;
    listAttr.ComputeSize(&count);
    string attrList;
    int i = 0;
    for (auto ent = listAttr.begin(); ent.Next(); i++) {
        sl_log_info(LOG_TAG, "[%u] %x", i, ent.GetValue());
        auto value = ent.GetValue();
        if (callback)
            callback(mNode, value);
        attrList.append(std::to_string((int)value) + ',');
    }
    // remove excess delimiter ','
    attrList.pop_back();
    return attribute_store_set_reported_string(mNode, attrList.c_str());
}

bool is_cluster_supported(chip::ClusterId clusterID)
{
    return std::find(begin(mpc_supported_clusters), end(mpc_supported_clusters), clusterID) != std::end(mpc_supported_clusters);
}

sl_status_t mpc_server_list_process(attribute_store_node_t mNode, uint64_t value)
{
    using namespace chip::app::Clusters::Globals;
    if (!is_cluster_supported(value))
        return SL_STATUS_FAIL;

    uint32_t typeID;
    auto parentNode = attribute(mNode).parent();
    attribute ep = attribute(parentNode);
    try {
        typeID = (value & 0x0000FFFF) << 16 | (Attributes::AttributeList::Id & 0xFFFF);
        ep.emplace_node(typeID);

        typeID = (value & 0x0000FFFF) << 16 | (Attributes::FeatureMap::Id & 0xFFFF);
        ep.emplace_node(typeID);

        typeID = (value & 0x0000FFFF) << 16 | (Attributes::ClusterRevision::Id & 0xFFFF);
        ep.emplace_node(typeID);

        typeID = (value & 0x0000FFFF) << 16 | (Attributes::GeneratedCommandList::Id & 0xFFFF);
        ep.emplace_node(typeID);

        typeID = (value & 0x0000FFFF) << 16 | (Attributes::AcceptedCommandList::Id & 0xFFFF);
        ep.emplace_node(typeID);
        return SL_STATUS_OK;
    } catch (...) {
        std::exception_ptr p = std::current_exception();
        sl_log_error(LOG_TAG, "Failed to parse server list [%s]", (p ? p.__cxa_exception_type()->name() : "null"));
        return SL_STATUS_FAIL;
    }
}

sl_status_t mpc_parts_list_process(attribute_store_node_t mNode, uint64_t value)
{
    attribute node;
    try {
        auto parentNode = attribute(mNode).parent();
        auto parentOfParentNode = parentNode.parent();
        if ((node = parentOfParentNode.add_node(ATTRIBUTE_ENDPOINT_ID)) && !node.is_valid())
            return SL_STATUS_FAIL;
        node.set_reported<EndpointId>(value);
        return SL_STATUS_OK;
    } catch (...) {
        std::exception_ptr p = std::current_exception();
        sl_log_error(LOG_TAG, "Failed to parse part list [%s]", (p ? p.__cxa_exception_type()->name() : "null"));
        return SL_STATUS_FAIL;
    }
}

sl_status_t mpc_attribute_list_process(attribute_store_node_t mNode, uint64_t value)
{
    uint32_t typeID;
    try {
        auto parentNode = attribute(mNode).parent();
        typeID = attribute(mNode).type() & 0xFFFF0000;
        typeID |= (value & 0xFFFF);
        parentNode.emplace_node(typeID);
        return SL_STATUS_OK;
    } catch (...) {
        std::exception_ptr p = std::current_exception();
        sl_log_error(LOG_TAG, "Failed to parse server list [%s]", (p ? p.__cxa_exception_type()->name() : "null"));
        return SL_STATUS_FAIL;
    }
}

sl_status_t mpc_accepted_command_list_process(attribute_store_node_t mNode, uint64_t value)
{
    try {
        attribute endpoint = attribute(mNode).first_parent(ATTRIBUTE_ENDPOINT_ID);
        auto nodeId = endpoint.first_parent(ATTRIBUTE_NODE_ID);
        auto unId = nodeId.reported<string>();
        // Publish the commavalue
        // uic_mqtt_dotdot_on_off_publish_supported_commands(unId.c_str(), endpoint);
        return SL_STATUS_OK;
    } catch (...) {
        std::exception_ptr p = std::current_exception();
        sl_log_error(LOG_TAG, "Failed to parse server list [%s]", (p ? p.__cxa_exception_type()->name() : "null"));
        return SL_STATUS_FAIL;
    }
}

sl_status_t mpc_on_off_cluster_parser(const chip::app::ConcreteDataAttributePath& path, chip::TLV::TLVReader* data,
    attribute_store_node_t mNode)
{
    sl_status_t reportStatus = SL_STATUS_OK;
    size_t count = 0;
    using namespace chip::app::Clusters;
    OnOff::Attributes::TypeInfo::DecodableType info;
    info.Decode(*data, path);

    switch (path.mAttributeId) {
    case OnOff::Attributes::OnOff::Id: { // type boolean
        auto value = info.onOff;
        sl_log_info(LOG_TAG, "OnOff: entries on read to %x : %u", path.mAttributeId, value);
        attribute(mNode).set_reported<uint8_t>(to_unify(value));
    } break;
    case OnOff::Attributes::GlobalSceneControl::Id: { // type boolean
        auto value = info.globalSceneControl;
        sl_log_info(LOG_TAG, "GlobalSceneControl: entries on read to %x : %u", path.mAttributeId, value);
        attribute(mNode).set_reported<uint8_t>(to_unify(value));
    } break;
    case OnOff::Attributes::OnTime::Id: { // type int16u
        auto value = info.onTime;
        sl_log_info(LOG_TAG, "OnTime: entries on read to %x : %u", path.mAttributeId, value);
        attribute(mNode).set_reported<uint16_t>(to_unify(value));
    } break;
    case OnOff::Attributes::OffWaitTime::Id: { // type int16u
        auto value = info.offWaitTime;
        sl_log_info(LOG_TAG, "OffWaitTime: entries on read to %x : %u", path.mAttributeId, value);
        attribute(mNode).set_reported<uint16_t>(to_unify(value));
    } break;
    case OnOff::Attributes::StartUpOnOff::Id: { // type StartUpOnOffEnum
        chip::app::Clusters::OnOff::StartUpOnOffEnum value;
        if (info.startUpOnOff.IsNull())
            value = chip::app::Clusters::OnOff::StartUpOnOffEnum::kUnknownEnumValue;
        else
            value = info.startUpOnOff.Value();
        OnOffStartUpOnOff unify_value = to_unify<chip::app::Clusters::OnOff::StartUpOnOffEnum, OnOffStartUpOnOff>(value);
        sl_log_info(LOG_TAG, "StartUpOnOff: entries on read to %x : %u", path.mAttributeId, value);
        attribute(mNode).set_reported<uint8_t>(static_cast<uint8_t>(unify_value));
    } break;
    case OnOff::Attributes::GeneratedCommandList::Id: {
        info.generatedCommandList.ComputeSize(&count);
        sl_log_info(LOG_TAG, "GeneratedCommandList: %u entries on read to %u", count, path.mAttributeId);
        reportStatus |= mpc_parse_decodablelist_attributes<OnOff::Attributes::GeneratedCommandList::TypeInfo::DecodableType>(
            info.generatedCommandList, mNode);
    } break;
    case OnOff::Attributes::AcceptedCommandList::Id: {
        info.acceptedCommandList.ComputeSize(&count);
        sl_log_info(LOG_TAG, "AcceptedCommandList: %u entries on read to %u", count, path.mAttributeId);
        reportStatus |= mpc_parse_decodablelist_attributes<OnOff::Attributes::AcceptedCommandList::TypeInfo::DecodableType>(
            info.acceptedCommandList, mNode, mpc_accepted_command_list_process);
    } break;
    case OnOff::Attributes::AttributeList::Id: {
        info.attributeList.ComputeSize(&count);
        sl_log_info(LOG_TAG, "AttributeList: %u entries on read to %u", count, path.mAttributeId);
        reportStatus |= mpc_parse_decodablelist_attributes<OnOff::Attributes::AttributeList::TypeInfo::DecodableType>(
            info.attributeList, mNode, mpc_attribute_list_process);
    } break;
    case OnOff::Attributes::FeatureMap::Id: { // type bitmap32
        auto value = info.featureMap;
        sl_log_info(LOG_TAG, "FeatureMap: entries on read to %x : %u", path.mAttributeId, value);
        attribute(mNode).set_reported<uint32_t>(to_unify(value));
    } break;
    case OnOff::Attributes::ClusterRevision::Id: { // type int16u
        auto value = info.clusterRevision;
        sl_log_info(LOG_TAG, "ClusterRevision: entries on read to %x : %u", path.mAttributeId, value);
        attribute(mNode).set_reported<uint16_t>(to_unify(value));
    } break;
    default:
        break;
    }

    return reportStatus;
}

sl_status_t mpc_descriptor_cluster_parser(const chip::app::ConcreteDataAttributePath& path, chip::TLV::TLVReader* data,
    attribute_store_node_t mNode)
{
    sl_status_t reportStatus = SL_STATUS_OK;
    size_t count = 0;
    using namespace chip::app::Clusters;
    Descriptor::Attributes::TypeInfo::DecodableType info;
    info.Decode(*data, path);

    switch (path.mAttributeId) {
    case Descriptor::Attributes::DeviceTypeList::Id: {
        info.deviceTypeList.ComputeSize(&count);
        string deviceList;
        sl_log_info(LOG_TAG, "%u entries on read to DeviceTypeList[%u]", count, path.mAttributeId);
        int i = 0;
        for (auto ent = info.deviceTypeList.begin(); ent.Next(); i++) {
            sl_log_info(LOG_TAG, "[%u] %x", i, ent.GetValue());
            auto value = (ent.GetValue().deviceType);
            deviceList.append(std::to_string(value) + ',');
        }
        // set last byte to \0 to remove excess delimiter ','
        deviceList[deviceList.size() - 1] = '\0';
        reportStatus = attribute_store_set_reported_string(mNode, deviceList.c_str());
    } break;
    case Descriptor::Attributes::ServerList::Id: {
        info.serverList.ComputeSize(&count);
        sl_log_info(LOG_TAG, "ServerList: %u entries on read to %u", count, path.mAttributeId);
        reportStatus |= mpc_parse_decodablelist_attributes<Descriptor::Attributes::ServerList::TypeInfo::DecodableType>(
            info.serverList, mNode, mpc_server_list_process);
    } break;
    case Descriptor::Attributes::ClientList::Id: {
        info.clientList.ComputeSize(&count);
        sl_log_info(LOG_TAG, "ClientList: %u entries on read to %u", count, path.mAttributeId);
        reportStatus |= mpc_parse_decodablelist_attributes<Descriptor::Attributes::ClientList::TypeInfo::DecodableType>(info.clientList, mNode);
    } break;
    case Descriptor::Attributes::PartsList::Id: {
        info.partsList.ComputeSize(&count);
        sl_log_info(LOG_TAG, "PartsList: %u entries on read to %u", count, path.mAttributeId);
        reportStatus |= mpc_parse_decodablelist_attributes<Descriptor::Attributes::PartsList::TypeInfo::DecodableType>(
            info.partsList, mNode, mpc_parts_list_process);
    } break;
    default:
        break;
    }

    return reportStatus;
}

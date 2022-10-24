/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "bridged_device_basic_info_attribute_translator.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <sstream>
#include <regex>
#include <unordered_map>

#include <app/reporting/reporting.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>

#include <attribute_state_cache.hpp>
#include "matter_device_translator.hpp"
#include "matter_node_state_monitor.hpp"
#include "uic_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "bridge_device_basic_cluster_server"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BridgedDeviceBasic;
using namespace chip::app::Clusters::BridgedDeviceBasic::Attributes;
using namespace unify::matter_bridge;

constexpr uint32_t ATTRIBUTE_ID_INVALID = 0xFFFFFFFF;
// Can not find a map for PartNumber, UniqueID (i.e., they are also optional attributes).
// ProductID, Location and LocalConfigDisabled attributes are not a part of
// BridgedDeviceBasic attribute namespace.
const static std::unordered_map<std::string, uint32_t> basic_information_map {
  {"ManufacturerName", Attributes::VendorName::Id},  // string, string
  {"ManufacturerName",
   Attributes::VendorID::Id},  // string, vendor id {can not map to one to one}
  {"ModelIdentifier", Attributes::ProductName::Id},  // string, string
  {"HWVersion", Attributes::HardwareVersion::Id},    // uint8_t, uint16_t
  {"ModelIdentifier", Attributes::HardwareVersionString::Id},  // string, string
  {"ApplicationVersion", Attributes::SoftwareVersion::Id},  // uint8_t, uint32_t
  {"SWBuildID", Attributes::SoftwareVersionString::Id},     // string , string
  {"DateCode", Attributes::ManufacturingDate::Id},          // string, string
  {"ProductURL", Attributes::ProductURL::Id},               // string, string
  {"ProductLabel", Attributes::ProductLabel::Id},           // string, string
  {"SerialNumber", Attributes::SerialNumber::Id},           //string, string
  {"ApplicationVersion", Attributes::UniqueID::Id}};        // uint8_t, string

BridgedDeviceBasicInfoAttributeAccess::BridgedDeviceBasicInfoAttributeAccess(
  matter_node_state_monitor &node_state_monitor) :
  attribute_translator_interface(node_state_monitor,
                                 chip::app::Clusters::BridgedDeviceBasic::Id, "attr_translator_BridgedDeviceBasic")

{
  //Register the an event listener for reachable state update
  node_state_monitor.register_event_listener(
    BridgedDeviceBasicInfoAttributeAccess::unify_node_reachable_state_update);
}

CHIP_ERROR BridgedDeviceBasicInfoAttributeAccess::Read(
  const ConcreteReadAttributePath &aPath, AttributeValueEncoder &aEncoder)
{
  if (aPath.mClusterId != BridgedDeviceBasic::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }

  ConcreteAttributePath attr_path = ConcreteAttributePath(aPath.mEndpointId,
                                                          aPath.mClusterId,
                                                          aPath.mAttributeId);
  try {
    switch (aPath.mAttributeId) {
      case VendorName::Id: {
        std::string read_vendor_name
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_vendor_name.c_str()));
      }
      case VendorID::Id: {
        uint16_t read_vendor_id
          = attribute_state_cache::get_instance().get<uint16_t>(attr_path);
        return aEncoder.Encode(read_vendor_id);
      }
      case ProductName::Id: {
        std::string read_product_name
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_product_name.c_str()));
      }
      case HardwareVersion::Id: {
        uint16_t read_hardware_version
          = attribute_state_cache::get_instance().get<uint16_t>(attr_path);
        return aEncoder.Encode(read_hardware_version);
      }
      case HardwareVersionString::Id: {
        std::string read_hardware_version_string
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_hardware_version_string.c_str()));
      }
      case SoftwareVersion::Id: {
        uint32_t read_software_version
          = attribute_state_cache::get_instance().get<uint32_t>(attr_path);
        return aEncoder.Encode(read_software_version);
      }
      case SoftwareVersionString::Id: {
        std::string read_software_version_string
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_software_version_string.c_str()));
      }
      case ManufacturingDate::Id: {
        std::string read_manufacturing_date
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_manufacturing_date.c_str()));
      }
      case ProductURL::Id: {
        std::string read_product_url
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_product_url.c_str()));
      }
      case ProductLabel::Id: {
        std::string read_product_label
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_product_label.c_str()));
      }
      case SerialNumber::Id: {
        std::string read_serial_number
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_serial_number.c_str()));
      }
      case NodeLabel::Id: {
        std::string read_node_label
          = attribute_state_cache::get_instance().get<std::string>(attr_path);
        return aEncoder.Encode(
          chip::CharSpan::fromCharString(read_node_label.c_str()));
      }
      case Reachable::Id: {
        Reachable::TypeInfo::Type state
          = attribute_state_cache::get_instance()
              .get<Reachable::TypeInfo::Type>(attr_path);
        return aEncoder.Encode(state);
      }
      case FeatureMap::Id: {
        uint32_t map = 0;
        return aEncoder.Encode(map);
      }
      case ClusterRevision::Id: {
        // Update version if bridged-device-basic.xml version is changed
        uint16_t revision = 1;
        return aEncoder.Encode(revision);
      }
    }
  } catch (const std::out_of_range &e) {
    sl_log_info(
      LOG_TAG,
      "The request attribute Path is not found in the attribute state "
      "container, %s\n",
      e.what());
    return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }
  return CHIP_NO_ERROR;
}

CHIP_ERROR BridgedDeviceBasicInfoAttributeAccess::Write(
  const ConcreteDataAttributePath &aPath, AttributeValueDecoder &aDecoder)
{
  if (aPath.mClusterId != BridgedDeviceBasic::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }

  // Note that only NodeLabel and Location attributes are the only writable attributes.
  // Location attribute is not mapped since we can not find the attribute
  // that matches under BridgedDeviceBasic attributes namespace.
  if (aPath.mAttributeId == NodeLabel::Id) {
    chip::CharSpan node_label_span;
    ReturnErrorOnFailure(aDecoder.Decode(node_label_span));

    nlohmann::json jsn;
    std::string node_label
      = std::string(node_label_span.begin(), node_label_span.end());
    jsn["value"] = node_label;
    // Get unify node unid and endpoint from matter endpoint
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);
    std::string name_topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep"
                             + std::to_string(unify_node->unify_endpoint)
                             + "/NameAndLocation/Attributes/Name/Desired";
    std::string payload_str = jsn.dump();
    uic_mqtt_publish(name_topic.c_str(),
                     payload_str.c_str(),
                     payload_str.length(),
                     true);
    return CHIP_NO_ERROR;
  }
  return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void BridgedDeviceBasicInfoAttributeAccess::reported_updated(
  const bridged_endpoint *ep,
  const std::string &cluster,
  const std::string &attribute,
  const nlohmann::json &unify_value)
{
  chip::EndpointId matter_endpoint = ep->matter_endpoint;
  ConcreteAttributePath attrpath;

  //handler basic cluster attributes
  if (cluster == "Basic") {
    uint32_t attribute_id = map_basic_cluster_attributes(attribute.c_str());
    if (attribute_id == ATTRIBUTE_ID_INVALID) {
      return;
    }
    attrpath = ConcreteAttributePath(matter_endpoint,
                                     BridgedDeviceBasic::Id,
                                     attribute_id);

    switch (attribute_id) {
      case VendorName::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               VendorName::Id,
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
        break;
      }
      case VendorID::Id: {
        // We can find a direct mapping, we just set to unspecified
        // considering that manufacturer name is publish on unify side
        uint16_t unspecified_vendor = chip::VendorId::NotSpecified;
        attribute_state_cache::get_instance().set<uint16_t>(attrpath,
                                                            unspecified_vendor);
        MatterReportingAttributeChangeCallback(
          matter_endpoint,
          BridgedDeviceBasic::Id,
          VendorID::Id,
          ZCL_INT16U_ATTRIBUTE_TYPE,
          reinterpret_cast<uint8_t *>(unspecified_vendor));
        break;
      }
      case ProductName::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               ProductName::Id,
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
        break;
      }
      case HardwareVersion::Id: {
        uint16_t value = unify_value;
        attribute_state_cache::get_instance().set<uint16_t>(attrpath, value);
        MatterReportingAttributeChangeCallback(
          matter_endpoint,
          BridgedDeviceBasic::Id,
          HardwareVersion::Id,
          
          ZCL_INT16U_ATTRIBUTE_TYPE,
          reinterpret_cast<uint8_t *>(value));
        break;
      }
      case HardwareVersionString::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               HardwareVersionString::Id,
                                               
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
        break;
      }
      case SoftwareVersion::Id: {
        uint32_t value = unify_value;
        attribute_state_cache::get_instance().set<uint16_t>(attrpath, value);
        MatterReportingAttributeChangeCallback(
          matter_endpoint,
          BridgedDeviceBasic::Id,
          SoftwareVersion::Id,
          
          ZCL_INT32U_ATTRIBUTE_TYPE,
          reinterpret_cast<uint8_t *>(value));
        break;
      }
      case SoftwareVersionString::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               SoftwareVersionString::Id,
                                               
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
        break;
      }
      case ManufacturingDate::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               ManufacturingDate::Id,
                                               
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
        break;
      }
      case ProductURL::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               ProductURL::Id,
                                               
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
        break;
      }
      case ProductLabel::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               ProductLabel::Id,
                                               
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
        break;
      }
      case SerialNumber::Id: {
        std::string value = unify_value;
        std::vector<uint8_t> value_vec(value.begin(), value.end());
        attribute_state_cache::get_instance().set<std::string>(attrpath, value);
        MatterReportingAttributeChangeCallback(matter_endpoint,
                                               BridgedDeviceBasic::Id,
                                               SerialNumber::Id,
                                               
                                               ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                               &value_vec[0]);
      }
    }
  }

  // Handler dot dot NameAndLocation cluster attribute update for NodeLabel
  // attribute for bridged device basic information cluster mapping
  if (cluster == "NameAndLocation") {
    if (attribute.compare("Name") == 0) {
      // parse the name or location attribute state value
      std::string name = unify_value;
      std::vector<uint8_t> name_vec(name.begin(), name.end());
      attrpath = ConcreteAttributePath(matter_endpoint,
                                       BridgedDeviceBasic::Id,
                                       NodeLabel::Id);
      attribute_state_cache::get_instance().set<std::string>(attrpath, name);
      MatterReportingAttributeChangeCallback(matter_endpoint,
                                             BridgedDeviceBasic::Id,
                                             NodeLabel::Id,
                                             
                                             ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
                                             &name_vec[0]);
    }
  }
}

void BridgedDeviceBasicInfoAttributeAccess::unify_node_reachable_state_update(
  const bridged_endpoint &ep, matter_node_state_monitor::update_t state)
{
  /// A node has changed its reachable state
  if (state == matter_node_state_monitor::update_t::NODE_STATE_CHANGED) {
    ConcreteAttributePath attr_path
      = ConcreteAttributePath(ep.matter_endpoint,
                              BridgedDeviceBasic::Id,
                              Reachable::Id);

    Reachable::TypeInfo::Type node_reachable_state = ep.reachable;
    attribute_state_cache::get_instance().set<Reachable::TypeInfo::Type>(
      attr_path,
      node_reachable_state);


    MatterReportingAttributeChangeCallback(
      ep.matter_endpoint,
      BridgedDeviceBasic::Id,
      Reachable::Id,
      ZCL_BOOLEAN_ATTRIBUTE_TYPE,
      reinterpret_cast<uint8_t *>(&node_reachable_state));

    // update the ReachableChanged Event
    Events::ReachableChanged::Type event {node_reachable_state};
    EventNumber eventNumber;
    if (CHIP_NO_ERROR != LogEvent(event, ep.matter_endpoint, eventNumber)) {
      ChipLogError(Zcl,
                   "ReachableChanged: Failed to record ReachableChanged event");
    }
  }
}

uint32_t BridgedDeviceBasicInfoAttributeAccess::map_basic_cluster_attributes(
  const std::string &dot_dot_basic_attribute) const
{
  static const auto end = basic_information_map.end();
  auto it               = basic_information_map.find(dot_dot_basic_attribute);
  if (it != end) {
    return it->second;
  } else {
    return ATTRIBUTE_ID_INVALID;
  }
}
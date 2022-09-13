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
/****************************************************************************
 * @file
 * @brief Implementation for the OnOff Server Cluster
 ***************************************************************************/
#include <nlohmann/json.hpp>
#include <sstream>
#include <regex>
#include "matter.h"

#include <attribute_state_cache.hpp>
#include "on_off_attribute_translator.hpp"
#include "matter_device_translator.hpp"
#include "uic_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "on_off_cluster_server"

using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;
using namespace chip::app::Clusters::OnOff::Attributes;
using namespace unify::matter_bridge;

CHIP_ERROR
OnOffAttributeAccess::Read(const ConcreteReadAttributePath &aPath,
                           AttributeValueEncoder &aEncoder)
{
  if (aPath.mClusterId != Clusters::OnOff::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }
  return ReadAttributeState(aPath, aEncoder);
}

CHIP_ERROR OnOffAttributeAccess::Write(const ConcreteDataAttributePath &aPath,
                                       AttributeValueDecoder &aDecoder)
{
  if (aPath.mClusterId != Clusters::OnOff::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }
  auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

  if (!unify_node) {
    return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }

  std::string attribute_name;
  nlohmann::json jsn;
  switch (aPath.mAttributeId) {
    case Attributes::OnOff::Id: {
      sl_log_info(LOG_TAG, "OnOff attribute is not writable.\n");
      return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    case Attributes::OnTime::Id: {
      Attributes::OnTime::TypeInfo::DecodableType on_time_payload;
      aDecoder.Decode(on_time_payload);
      jsn["value"]   = on_time_payload;
      attribute_name = string("OnTime");
      break;
    }
    case Attributes::OffWaitTime::Id: {
      Attributes::OffWaitTime::TypeInfo::DecodableType off_wait_time_value;
      aDecoder.Decode(off_wait_time_value);
      jsn["value"]   = off_wait_time_value;
      attribute_name = string("OffWaitTime");
      break;
    }
    case Attributes::StartUpOnOff::Id: {
      OnOffStartUpOnOff start_up_on_off_value{OnOffStartUpOnOff::kOff};
      aDecoder.Decode(start_up_on_off_value);
      jsn["value"]   = start_up_on_off_value;
      attribute_name = string("StartUpOnOff");
      break;
    }
    case Attributes::GlobalSceneControl::Id: {
      Attributes::GlobalSceneControl::TypeInfo::DecodableType
        global_scene_control_value;
      aDecoder.Decode(global_scene_control_value);
      jsn["value"]   = global_scene_control_value;
      attribute_name = string("GlobalSceneControl");
      break;
    }
    default:
      return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }

  if (!attribute_name.empty()) {
    std::string onoff_topic;
    std::string payload_str;
    onoff_topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep"
                  + std::to_string(unify_node->unify_endpoint)
                  + "/OnOff/Attributes/" + attribute_name + "/Desired";
    payload_str = jsn.dump();
    uic_mqtt_publish(onoff_topic.c_str(),
                     payload_str.c_str(),
                     payload_str.length(),
                     true);
  }

  return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffAttributeAccess::ReadAttributeState(
  const ConcreteReadAttributePath &aPath, AttributeValueEncoder &aEncoder)
{
  sl_log_info(LOG_TAG, "read %i\n", aPath.mEndpointId);

  ConcreteAttributePath atrpath = ConcreteAttributePath(aPath.mEndpointId,
                                                        aPath.mClusterId,
                                                        aPath.mAttributeId);
  try {
    switch (aPath.mAttributeId) {
      case Attributes::OnOff::Id: {
        Attributes::OnOff::TypeInfo::Type read_on_off_attr_state
          = attribute_state_cache::get_instance()
              .get<Attributes::OnOff::TypeInfo::Type>(atrpath);
        return aEncoder.Encode(read_on_off_attr_state);
      }
      case Attributes::OnTime::Id: {
        Attributes::OnTime::TypeInfo::Type read_on_time_attr_state
          = attribute_state_cache::get_instance()
              .get<Attributes::OnTime::TypeInfo::Type>(atrpath);
        return aEncoder.Encode(read_on_time_attr_state);
      }
      case Attributes::StartUpOnOff::Id: {
        uint8_t read_start_up_on_off_attr_state
          = attribute_state_cache::get_instance().get<uint8_t>(atrpath);
        return aEncoder.Encode(read_start_up_on_off_attr_state);
      }
      case Attributes::OffWaitTime::Id: {
        Attributes::OffWaitTime::TypeInfo::Type read_off_wait_time_attr_state
          = attribute_state_cache::get_instance()
              .get<Attributes::OffWaitTime::TypeInfo::Type>(atrpath);
        return aEncoder.Encode(read_off_wait_time_attr_state);
      }
      case Attributes::GlobalSceneControl::Id: {
        Attributes::GlobalSceneControl::TypeInfo::Type
          read_global_scene_attr_state
          = attribute_state_cache::get_instance()
              .get<Attributes::GlobalSceneControl::TypeInfo::Type>(atrpath);
        return aEncoder.Encode(read_global_scene_attr_state);
      }
      default:
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
  } catch (const std::out_of_range &e) {
    sl_log_info(
      LOG_TAG,
      "The request attribute Path is not found in the attribute state "
      "contanier, %s\n",
      e.what());
    return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }
}

void OnOffAttributeAccess::reported_updated(const bridged_endpoint *ep,
                                            const std::string &cluster,
                                            const std::string &attribute,
                                            const nlohmann::json &unify_value)
{
  auto cluster_id = device_translator::instance().get_cluster_id(cluster);

  if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OnOff::Id)) {
    return;
  }

  // get attribute id
  auto attribute_id
    = device_translator::instance().get_attribute_id(cluster, attribute);

  if (!attribute_id.has_value()) {
    return;
  }

  chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
  ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint,
                                                         Clusters::OnOff::Id,
                                                         attribute_id.value());

  switch (attribute_id.value()) {
    case Attributes::OnOff::Id: {
      Attributes::OnOff::TypeInfo::Type value = unify_value;
      sl_log_debug(LOG_TAG,
                   "OnOff attribute value is %s",
                   value ? "On" : "Off");
      attribute_state_cache::get_instance()
        .set<Attributes::OnOff::TypeInfo::Type>(attrpath, value);
      MatterReportingAttributeChangeCallback(
        node_matter_endpoint,
        Clusters::OnOff::Id,
        Attributes::OnOff::Id,
        
        ZCL_BOOLEAN_ATTRIBUTE_TYPE,
        reinterpret_cast<uint8_t *>(&value));
      break;
    }
    case Attributes::OnTime::Id: {
      Attributes::OnTime::TypeInfo::Type value = unify_value;
      attribute_state_cache::get_instance()
        .set<Attributes::OnTime::TypeInfo::Type>(attrpath, value);
      MatterReportingAttributeChangeCallback(
        node_matter_endpoint,
        Clusters::OnOff::Id,
        Attributes::OnTime::Id,
        
        ZCL_INT16U_ATTRIBUTE_TYPE,
        reinterpret_cast<uint8_t *>(&value));
      break;
    }
    case Attributes::OffWaitTime::Id: {
      Attributes::OffWaitTime::TypeInfo::Type value = unify_value;
      attribute_state_cache::get_instance()
        .set<Attributes::OffWaitTime::TypeInfo::Type>(attrpath, value);
      MatterReportingAttributeChangeCallback(
        node_matter_endpoint,
        Clusters::OnOff::Id,
        Attributes::OffWaitTime::Id,
        
        ZCL_INT16U_ATTRIBUTE_TYPE,
        reinterpret_cast<uint8_t *>(&value));
      break;
    }
    case Attributes::StartUpOnOff::Id: {
      uint8_t value = unify_value;
      attribute_state_cache::get_instance().set<uint8_t>(attrpath, value);
      MatterReportingAttributeChangeCallback(
        node_matter_endpoint,
        Clusters::OnOff::Id,
        Attributes::StartUpOnOff::Id,
        
        ZCL_INT8U_ATTRIBUTE_TYPE,
        reinterpret_cast<uint8_t *>(&value));
      break;
    }
    case Attributes::GlobalSceneControl::Id: {
      Attributes::GlobalSceneControl::TypeInfo::Type value = unify_value;
      attribute_state_cache::get_instance()
        .set<Attributes::GlobalSceneControl::TypeInfo::Type>(attrpath, value);
      MatterReportingAttributeChangeCallback(
        node_matter_endpoint,
        Clusters::OnOff::Id,
        Attributes::GlobalSceneControl::Id,
        
        ZCL_BOOLEAN_ATTRIBUTE_TYPE,
        reinterpret_cast<uint8_t *>(&value));
      break;
    }
  }
}
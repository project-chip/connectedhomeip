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
#include "identify_attribute_translator.hpp"

#include <nlohmann/json.hpp>
#include <sstream>
#include <regex>
#include "matter.h"

#include <attribute_state_cache.hpp>
#include "matter_device_translator.hpp"
#include "uic_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "identify_cluster_server"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Identify;
using namespace chip::app::Clusters::Identify::Attributes;
using namespace unify::matter_bridge;

CHIP_ERROR IdentifyAttributeAccess::Read(const ConcreteReadAttributePath &aPath,
                                         AttributeValueEncoder &aEncoder)
{
  if (aPath.mClusterId != Clusters::Identify::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }
  if (aPath.mAttributeId == IdentifyTime::Id) {
    ConcreteAttributePath attr_path = ConcreteAttributePath(aPath.mEndpointId,
                                                            aPath.mClusterId,
                                                            aPath.mAttributeId);
    Attributes::IdentifyTime::TypeInfo::Type remaining_time
      = attribute_state_cache::get_instance()
          .get<Attributes::IdentifyTime::TypeInfo::Type>(attr_path);
    return aEncoder.Encode(remaining_time);
  } else {
    return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }
}

CHIP_ERROR
IdentifyAttributeAccess::Write(const ConcreteDataAttributePath &aPath,
                               AttributeValueDecoder &aDecoder)
{
  if (aPath.mClusterId != Clusters::Identify::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }

  auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

  if (!unify_node) {
    return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }

  if (aPath.mAttributeId == IdentifyTime::Id) {
    IdentifyTime::TypeInfo::DecodableType timeout_value;
    aDecoder.Decode(timeout_value);
    nlohmann::json jsn;
    jsn["value"] = timeout_value;
    std::string identify_time_topic
      = "ucl/by-unid/" + unify_node->unify_unid + "/ep"
        + std::to_string(unify_node->unify_endpoint)
        + "/Identify/Attributes/IdentifyTime/Desired";
    std::string payload_str = jsn.dump();
    uic_mqtt_publish(identify_time_topic.c_str(),
                     payload_str.c_str(),
                     payload_str.length(),
                     true);
    return CHIP_NO_ERROR;
  } else {
    return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }
}

void IdentifyAttributeAccess::reported_updated(
  const bridged_endpoint *ep,
  const std::string &cluster,
  const std::string &attribute,
  const nlohmann::json &unify_value)
{
  auto cluster_id = device_translator::instance().get_cluster_id(cluster);
  if (!cluster_id.has_value() || (cluster_id.value() != Identify::Id)) {
    return;
  }
  auto attribute_id
    = device_translator::instance().get_attribute_id(cluster, attribute);

  if (!attribute_id.has_value()) {
    return;
  }

  if (attribute_id.value() == IdentifyTime::Id) {
    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attr_path
      = ConcreteAttributePath(node_matter_endpoint,
                              Identify::Id,
                              attribute_id.value());
    IdentifyTime::TypeInfo::Type value = unify_value;
    attribute_state_cache::get_instance()
      .set<Attributes::IdentifyTime::TypeInfo::Type>(attr_path, value);
    MatterReportingAttributeChangeCallback(node_matter_endpoint,
                                           Clusters::Identify::Id,
                                           Attributes::IdentifyTime::Id,
                                           
                                           ZCL_INT16U_ATTRIBUTE_TYPE,
                                           reinterpret_cast<uint8_t *>(&value));
  }
}
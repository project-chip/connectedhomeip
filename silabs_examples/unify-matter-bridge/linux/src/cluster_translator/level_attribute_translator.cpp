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

#include "level_attribute_translator.hpp"

// Standard library
#include <regex>
#include <string>
#include <typeindex>

// Third party
#include <nlohmann/json.hpp>

// Matter library
#include <app/reporting/reporting.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>

// Application components
#include "matter_device_translator.hpp"
#include "matter_node_state_monitor.hpp"

// Unify components
#include "attribute_state_cache.hpp"
#include "uic_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "level_attribute_cluster_server"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using namespace unify::matter_bridge;

static const unify::matter_bridge::device_translator &device_translator_obj
  = unify::matter_bridge::device_translator::instance();

static const std::unordered_map<std::type_index, uint32_t> zcl_types_map {
  {typeid(uint8_t), ZCL_INT8_U_ATTRIBUTE_ID},
  {typeid(uint16_t), ZCL_INT16_U_ATTRIBUTE_ID},
  {typeid(uint32_t), ZCL_INT32_U_ATTRIBUTE_ID},
  {typeid(uint64_t), ZCL_INT64_U_ATTRIBUTE_ID},
  {typeid(int8_t), ZCL_INT8_S_ATTRIBUTE_ID},
  {typeid(int16_t), ZCL_INT16_S_ATTRIBUTE_ID},
  {typeid(int32_t), ZCL_INT32_S_ATTRIBUTE_ID},
  {typeid(int64_t), ZCL_INT64_S_ATTRIBUTE_ID},
  {typeid(chip::app::DataModel::Nullable<uint8_t>),
   ZCL_NULLABLE_INT8_U_ATTRIBUTE_ID},
  {typeid(chip::app::DataModel::Nullable<uint16_t>),
   ZCL_NULLABLE_INT16_U_ATTRIBUTE_ID},
  {typeid(chip::app::DataModel::Nullable<uint32_t>),
   ZCL_NULLABLE_INT32_U_ATTRIBUTE_ID},
  {typeid(chip::app::DataModel::Nullable<uint64_t>),
   ZCL_NULLABLE_INT64_U_ATTRIBUTE_ID},
};

CHIP_ERROR
LevelAttributeAccess::Read(const ConcreteReadAttributePath &aPath,
                           AttributeValueEncoder &aEncoder)
{
  if (aPath.mClusterId != Clusters::LevelControl::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }
  return ReadAttributeState(aPath, aEncoder);
}

CHIP_ERROR LevelAttributeAccess::ReadAttributeState(
  const ConcreteReadAttributePath &aPath, AttributeValueEncoder &aEncoder)
{
  sl_log_info(LOG_TAG, "read %i\n", aPath.mEndpointId);

  ConcreteAttributePath attribute_path
    = ConcreteAttributePath(aPath.mEndpointId,
                            aPath.mClusterId,
                            aPath.mAttributeId);

  try {
    switch (aPath.mAttributeId) {
      case Attributes::CurrentLevel::Id: {
        Attributes::CurrentLevel::TypeInfo::Type read_current_level_state
          = attribute_state_cache::get_instance()
              .get<Attributes::CurrentLevel::TypeInfo::Type>(attribute_path);
        return aEncoder.Encode(read_current_level_state);
      }
      case Attributes::RemainingTime::Id: {
        Attributes::RemainingTime::TypeInfo::Type read_remaining_time_state
          = attribute_state_cache::get_instance()
              .get<Attributes::RemainingTime::TypeInfo::Type>(attribute_path);
        return aEncoder.Encode(read_remaining_time_state);
      }
      case Attributes::MinLevel::Id: {
        Attributes::MinLevel::TypeInfo::Type read_min_level_state
          = attribute_state_cache::get_instance()
              .get<Attributes::MinLevel::TypeInfo::Type>(attribute_path);
        return aEncoder.Encode(read_min_level_state);
      }
      case Attributes::MaxLevel::Id: {
        Attributes::MaxLevel::TypeInfo::Type read_max_level_state
          = attribute_state_cache::get_instance()
              .get<Attributes::MaxLevel::TypeInfo::Type>(attribute_path);
        return aEncoder.Encode(read_max_level_state);
      }
      case Attributes::CurrentFrequency::Id: {
        Attributes::CurrentFrequency::TypeInfo::Type
          read_current_frequency_state
          = attribute_state_cache::get_instance()
              .get<Attributes::CurrentFrequency::TypeInfo::Type>(
                attribute_path);
        return aEncoder.Encode(read_current_frequency_state);
      }
      case Attributes::MinFrequency::Id: {
        Attributes::MinFrequency::TypeInfo::Type read_min_frequency_state
          = attribute_state_cache::get_instance()
              .get<Attributes::MinFrequency::TypeInfo::Type>(attribute_path);
        return aEncoder.Encode(read_min_frequency_state);
      }
      case Attributes::MaxFrequency::Id: {
        Attributes::MaxFrequency::TypeInfo::Type read_max_frequency_state
          = attribute_state_cache::get_instance()
              .get<Attributes::MaxFrequency::TypeInfo::Type>(attribute_path);
        return aEncoder.Encode(read_max_frequency_state);
      }
      case Attributes::OnOffTransitionTime::Id: {
        Attributes::OnOffTransitionTime::TypeInfo::Type
          read_on_off_transition_time_state
          = attribute_state_cache::get_instance()
              .get<Attributes::OnOffTransitionTime::TypeInfo::Type>(
                attribute_path);
        return aEncoder.Encode(read_on_off_transition_time_state);
      }
      case Attributes::OnLevel::Id: {
        Attributes::OnLevel::TypeInfo::Type::UnderlyingType read_on_level_state
          = attribute_state_cache::get_instance()
              .get<Attributes::OnLevel::TypeInfo::Type::UnderlyingType>(
                attribute_path);
        return aEncoder.Encode(read_on_level_state);
      }
      case Attributes::OnTransitionTime::Id: {
        Attributes::OnTransitionTime::TypeInfo::Type::UnderlyingType
          read_on_transition_time_state
          = attribute_state_cache::get_instance()
              .get<
                Attributes::OnTransitionTime::TypeInfo::Type::UnderlyingType>(
                attribute_path);
        return aEncoder.Encode(read_on_transition_time_state);
      }
      case Attributes::OffTransitionTime::Id: {
        Attributes::OffTransitionTime::TypeInfo::Type::UnderlyingType
          read_off_transition_time_state
          = attribute_state_cache::get_instance()
              .get<
                Attributes::OffTransitionTime::TypeInfo::Type::UnderlyingType>(
                attribute_path);
        return aEncoder.Encode(read_off_transition_time_state);
      }
      case Attributes::DefaultMoveRate::Id: {
        Attributes::DefaultMoveRate::TypeInfo::Type::UnderlyingType
          read_default_move_rate_state
          = attribute_state_cache::get_instance()
              .get<Attributes::DefaultMoveRate::TypeInfo::Type::UnderlyingType>(
                attribute_path);
        return aEncoder.Encode(read_default_move_rate_state);
      }
      case Attributes::Options::Id: {
        Attributes::Options::TypeInfo::Type read_options_state
          = attribute_state_cache::get_instance()
              .get<Attributes::Options::TypeInfo::Type>(attribute_path);
        return aEncoder.Encode(read_options_state);
      }
      case Attributes::StartUpCurrentLevel::Id: {
        Attributes::StartUpCurrentLevel::TypeInfo::Type::UnderlyingType
          read_start_up_current_level_state
          = attribute_state_cache::get_instance()
              .get<Attributes::StartUpCurrentLevel::TypeInfo::Type::
                     UnderlyingType>(attribute_path);
        return aEncoder.Encode(read_start_up_current_level_state);
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

CHIP_ERROR
LevelAttributeAccess::Write(const ConcreteDataAttributePath &aPath,
                            AttributeValueDecoder &aDecoder)
{
  if (aPath.mClusterId != Clusters::LevelControl::Id) {
    return CHIP_ERROR_INVALID_ARGUMENT;
  }
  auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

  if (!unify_node) {
    return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }

  std::string attribute_name;
  nlohmann::json payload;
  switch (aPath.mAttributeId) {
    case Attributes::CurrentLevel::Id: {
      Attributes::CurrentLevel::TypeInfo::DecodableType desired_current_level;
      aDecoder.Decode(desired_current_level);
      payload["value"] = desired_current_level;
      attribute_name   = "CurrentLevel";
      break;
    }
    case Attributes::OnOffTransitionTime::Id: {
      Attributes::OnOffTransitionTime::TypeInfo::DecodableType
        desired_on_off_transition_time;
      aDecoder.Decode(desired_on_off_transition_time);
      payload["value"] = desired_on_off_transition_time;
      attribute_name   = "OnOffTransitionTime";
      break;
    }
    case Attributes::OnLevel::Id: {
      Attributes::OnLevel::TypeInfo::DecodableType::UnderlyingType
        desired_on_level;
      aDecoder.Decode(desired_on_level);
      payload["value"] = desired_on_level;
      attribute_name   = "OnLevel";
      break;
    }
    case Attributes::OnTransitionTime::Id: {
      Attributes::OnTransitionTime::TypeInfo::DecodableType::UnderlyingType
        desired_on_transition_time;
      aDecoder.Decode(desired_on_transition_time);
      payload["value"] = desired_on_transition_time;
      attribute_name   = "OnTransitionTime";
      break;
    }
    case Attributes::OffTransitionTime::Id: {
      Attributes::OffTransitionTime::TypeInfo::DecodableType::UnderlyingType
        desired_off_transition_time;
      aDecoder.Decode(desired_off_transition_time);
      payload["value"] = desired_off_transition_time;
      attribute_name   = "OffTransitionTime";
      break;
    }
    case Attributes::DefaultMoveRate::Id: {
      Attributes::DefaultMoveRate::TypeInfo::DecodableType::UnderlyingType
        desired_default_move_rate;
      aDecoder.Decode(desired_default_move_rate);
      payload["value"] = desired_default_move_rate;
      attribute_name   = "DefaultMoveRate";
      break;
    }
    case Attributes::Options::Id: {
      Attributes::Options::TypeInfo::DecodableType desired_options;
      aDecoder.Decode(desired_options);
      payload["value"] = desired_options;
      attribute_name   = "Options";
      break;
    }
    case Attributes::StartUpCurrentLevel::Id: {
      Attributes::StartUpCurrentLevel::TypeInfo::DecodableType::UnderlyingType
        desired_start_up_current_level;
      aDecoder.Decode(desired_start_up_current_level);
      payload["value"] = desired_start_up_current_level;
      attribute_name   = "StartUpCurrentLevel";
      break;
    }
    default:
      return CHIP_ERROR_NO_MESSAGE_HANDLER;
  }

  if (!attribute_name.empty()) {
    std::string onoff_topic;
    std::string payload_str;
    onoff_topic = "ucl/by-unid/" + unify_node->unify_unid + "/"
                  + std::to_string(unify_node->unify_endpoint)
                  + "/Level/Attributes/" + attribute_name + "/Desired";
    payload_str = payload.dump();
    uic_mqtt_publish(onoff_topic.c_str(),
                     payload_str.c_str(),
                     payload_str.length(),
                     true);
  }

  return CHIP_NO_ERROR;
}

template<typename attribute_underlying_type, typename attribute_type>
bool attribute_translation(chip::ClusterId cluster_id,
                           chip::AttributeId attribute_id,
                           ConcreteAttributePath attribute_path,
                           chip::EndpointId matter_endpoint,
                           nlohmann::json payload)
{
  attribute_underlying_type value = payload;
  attribute_state_cache::get_instance().set<attribute_underlying_type>(
    attribute_path,
    value);
  auto zcl_type = zcl_types_map.at(typeid(attribute_type));

  MatterReportingAttributeChangeCallback(matter_endpoint,
                                         cluster_id,
                                         attribute_id,
                                         zcl_type,
                                         reinterpret_cast<uint8_t *>(&value));
  return true;
}

void LevelAttributeAccess::reported_updated(const bridged_endpoint *ep,
                                            const std::string &cluster,
                                            const std::string &attribute,
                                            const nlohmann::json &unify_value)
{
  auto cluster_id = device_translator::instance().get_cluster_id(cluster);
  if (!cluster_id.has_value() || (cluster_id.value() != LevelControl::Id)) {
    return;
  }
  auto attribute_id
    = device_translator::instance().get_attribute_id(cluster, attribute);

  if (!attribute_id.has_value()) {
    return;
  }

  //handler basic cluster attributes
  chip::EndpointId matter_endpoint = ep->matter_endpoint;
  ConcreteAttributePath attribute_path
    = ConcreteAttributePath(matter_endpoint,
                            LevelControl::Id,
                            attribute_id.value());
  try {
    switch (attribute_id.value()) {
      case Attributes::CurrentLevel::Id: {
        attribute_translation<Attributes::CurrentLevel::TypeInfo::Type,
                              Attributes::CurrentLevel::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
        break;
      }
      case Attributes::RemainingTime::Id: {
        attribute_translation<Attributes::RemainingTime::TypeInfo::Type,
                              Attributes::RemainingTime::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
        break;
      }
      case Attributes::MinLevel::Id: {
        attribute_translation<Attributes::MinLevel::TypeInfo::Type,
                              Attributes::MinLevel::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
        break;
      }
      case Attributes::MaxLevel::Id: {
        attribute_translation<Attributes::MaxLevel::TypeInfo::Type,
                              Attributes::MaxLevel::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
      }
      case Attributes::CurrentFrequency::Id: {
        attribute_translation<Attributes::CurrentFrequency::TypeInfo::Type,
                              Attributes::CurrentFrequency::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
      }
      case Attributes::MinFrequency::Id: {
        attribute_translation<Attributes::MinFrequency::TypeInfo::Type,
                              Attributes::MinFrequency::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
      }
      case Attributes::MaxFrequency::Id: {
        attribute_translation<Attributes::MaxFrequency::TypeInfo::Type,
                              Attributes::MaxFrequency::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
      }
      case Attributes::OnOffTransitionTime::Id: {
        attribute_translation<Attributes::OnOffTransitionTime::TypeInfo::Type,
                              Attributes::OnOffTransitionTime::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
      }
      case Attributes::OnLevel::Id: {
        attribute_translation<
          Attributes::OnLevel::TypeInfo::Type::UnderlyingType,
          Attributes::OnLevel::TypeInfo::Type>(LevelControl::Id,
                                               attribute_id.value(),
                                               attribute_path,
                                               matter_endpoint,
                                               unify_value);
      }
      case Attributes::OnTransitionTime::Id: {
        attribute_translation<
          Attributes::OnTransitionTime::TypeInfo::Type::UnderlyingType,
          Attributes::OnTransitionTime::TypeInfo::Type>(LevelControl::Id,
                                                        attribute_id.value(),
                                                        attribute_path,
                                                        matter_endpoint,
                                                        unify_value);
      }
      case Attributes::OffTransitionTime::Id: {
        attribute_translation<
          Attributes::OffTransitionTime::TypeInfo::Type::UnderlyingType,
          Attributes::OffTransitionTime::TypeInfo::Type>(LevelControl::Id,
                                                         attribute_id.value(),
                                                         attribute_path,
                                                         matter_endpoint,
                                                         unify_value);
      }
      case Attributes::DefaultMoveRate::Id: {
        attribute_translation<
          Attributes::DefaultMoveRate::TypeInfo::Type::UnderlyingType,
          Attributes::DefaultMoveRate::TypeInfo::Type>(LevelControl::Id,
                                                       attribute_id.value(),
                                                       attribute_path,
                                                       matter_endpoint,
                                                       unify_value);
      }
      case Attributes::Options::Id: {
        attribute_translation<Attributes::Options::TypeInfo::Type,
                              Attributes::Options::TypeInfo::Type>(
          LevelControl::Id,
          attribute_id.value(),
          attribute_path,
          matter_endpoint,
          unify_value);
      }
      case Attributes::StartUpCurrentLevel::Id: {
        attribute_translation<
          Attributes::StartUpCurrentLevel::TypeInfo::Type::UnderlyingType,
          Attributes::StartUpCurrentLevel::TypeInfo::Type>(LevelControl::Id,
                                                           attribute_id.value(),
                                                           attribute_path,
                                                           matter_endpoint,
                                                           unify_value);
      }
    }
  } catch (const nlohmann::json::type_error &e) {
    sl_log_info(LOG_TAG,
                "Failed parsing incoming attribute the value of different type "
                "or key is not present, %s\n. Payload: %s",
                e.what(),
                unify_value.dump().c_str());
  }
}
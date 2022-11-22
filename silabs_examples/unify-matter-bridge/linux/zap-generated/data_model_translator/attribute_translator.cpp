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

#define CHIP_USE_ENUM_CLASS_FOR_IM_ENUM

#include "matter.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <type_traits>

#include "app-common/zap-generated/attributes/Accessors.h"
#include "attribute_translator.hpp"
#include "matter_device_translator.hpp"
#include "sl_log.h"
#include "uic_mqtt.h"
#include <attribute_state_cache.hpp>
#define LOG_TAG "attribute_translator"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace unify::matter_bridge;

#include "chip_types_from_json.hpp"
#include "chip_types_to_json.hpp"
#include "unify_accessors.hpp"

// ZCL cluster revision
constexpr uint16_t ZCL_IDENTIFY_REVISION                                = 4;
constexpr uint16_t ZCL_SCENES_REVISION                                  = 4;
constexpr uint16_t ZCL_ON_OFF_REVISION                                  = 4;
constexpr uint16_t ZCL_ON_OFF_SWITCH_CONFIGURATION_REVISION             = 0;
constexpr uint16_t ZCL_LEVEL_CONTROL_REVISION                           = 5;
constexpr uint16_t ZCL_BINARY_INPUT_BASIC_REVISION                      = 0;
constexpr uint16_t ZCL_PULSE_WIDTH_MODULATION_REVISION                  = 0;
constexpr uint16_t ZCL_BASIC_REVISION                                   = 1;
constexpr uint16_t ZCL_OTA_SOFTWARE_UPDATE_PROVIDER_REVISION            = 0;
constexpr uint16_t ZCL_OTA_SOFTWARE_UPDATE_REQUESTOR_REVISION           = 0;
constexpr uint16_t ZCL_LOCALIZATION_CONFIGURATION_REVISION              = 0;
constexpr uint16_t ZCL_TIME_FORMAT_LOCALIZATION_REVISION                = 0;
constexpr uint16_t ZCL_UNIT_LOCALIZATION_REVISION                       = 1;
constexpr uint16_t ZCL_POWER_SOURCE_CONFIGURATION_REVISION              = 1;
constexpr uint16_t ZCL_POWER_SOURCE_REVISION                            = 0;
constexpr uint16_t ZCL_GENERAL_COMMISSIONING_REVISION                   = 0;
constexpr uint16_t ZCL_DIAGNOSTIC_LOGS_REVISION                         = 0;
constexpr uint16_t ZCL_GENERAL_DIAGNOSTICS_REVISION                     = 0;
constexpr uint16_t ZCL_SOFTWARE_DIAGNOSTICS_REVISION                    = 0;
constexpr uint16_t ZCL_THREAD_NETWORK_DIAGNOSTICS_REVISION              = 0;
constexpr uint16_t ZCL_WI_FI_NETWORK_DIAGNOSTICS_REVISION               = 0;
constexpr uint16_t ZCL_TIME_SYNCHRONIZATION_REVISION                    = 0;
constexpr uint16_t ZCL_SWITCH_REVISION                                  = 0;
constexpr uint16_t ZCL_OPERATIONAL_CREDENTIALS_REVISION                 = 0;
constexpr uint16_t ZCL_GROUP_KEY_MANAGEMENT_REVISION                    = 0;
constexpr uint16_t ZCL_FIXED_LABEL_REVISION                             = 0;
constexpr uint16_t ZCL_USER_LABEL_REVISION                              = 0;
constexpr uint16_t ZCL_PROXY_CONFIGURATION_REVISION                     = 0;
constexpr uint16_t ZCL_PROXY_DISCOVERY_REVISION                         = 0;
constexpr uint16_t ZCL_PROXY_VALID_REVISION                             = 0;
constexpr uint16_t ZCL_BOOLEAN_STATE_REVISION                           = 1;
constexpr uint16_t ZCL_MODE_SELECT_REVISION                             = 0;
constexpr uint16_t ZCL_DOOR_LOCK_REVISION                               = 6;
constexpr uint16_t ZCL_WINDOW_COVERING_REVISION                         = 5;
constexpr uint16_t ZCL_BARRIER_CONTROL_REVISION                         = 0;
constexpr uint16_t ZCL_THERMOSTAT_REVISION                              = 5;
constexpr uint16_t ZCL_FAN_CONTROL_REVISION                             = 0;
constexpr uint16_t ZCL_THERMOSTAT_USER_INTERFACE_CONFIGURATION_REVISION = 0;
constexpr uint16_t ZCL_COLOR_CONTROL_REVISION                           = 5;
constexpr uint16_t ZCL_ILLUMINANCE_MEASUREMENT_REVISION                 = 3;
constexpr uint16_t ZCL_TEMPERATURE_MEASUREMENT_REVISION                 = 0;
constexpr uint16_t ZCL_PRESSURE_MEASUREMENT_REVISION                    = 3;
constexpr uint16_t ZCL_FLOW_MEASUREMENT_REVISION                        = 0;
constexpr uint16_t ZCL_RELATIVE_HUMIDITY_MEASUREMENT_REVISION           = 3;
constexpr uint16_t ZCL_OCCUPANCY_SENSING_REVISION                       = 2;
constexpr uint16_t ZCL_WAKE_ON_LAN_REVISION                             = 0;
constexpr uint16_t ZCL_CHANNEL_REVISION                                 = 0;
constexpr uint16_t ZCL_TARGET_NAVIGATOR_REVISION                        = 0;
constexpr uint16_t ZCL_MEDIA_PLAYBACK_REVISION                          = 0;
constexpr uint16_t ZCL_MEDIA_INPUT_REVISION                             = 0;
constexpr uint16_t ZCL_LOW_POWER_REVISION                               = 0;
constexpr uint16_t ZCL_KEYPAD_INPUT_REVISION                            = 0;
constexpr uint16_t ZCL_CONTENT_LAUNCHER_REVISION                        = 0;
constexpr uint16_t ZCL_AUDIO_OUTPUT_REVISION                            = 0;
constexpr uint16_t ZCL_APPLICATION_LAUNCHER_REVISION                    = 0;
constexpr uint16_t ZCL_APPLICATION_BASIC_REVISION                       = 0;
constexpr uint16_t ZCL_ACCOUNT_LOGIN_REVISION                           = 0;
constexpr uint16_t ZCL_ELECTRICAL_MEASUREMENT_REVISION                  = 3;

CHIP_ERROR
IdentifyAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::Identify::Attributes;
    namespace UN = unify::matter_bridge::Identify::Attributes;
    if (aPath.mClusterId != Clusters::Identify::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::IdentifyTime::Id: { // type is int16u
            MN::IdentifyTime::TypeInfo::Type value;
            UN::IdentifyTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::IdentifyType::Id: { // type is enum8
            MN::IdentifyType::TypeInfo::Type value;
            UN::IdentifyType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_IDENTIFY_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR IdentifyAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::Identify;

    if (aPath.mClusterId != Clusters::Identify::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::IdentifyTime::Id: {

        Attributes::IdentifyTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("IdentifyTime");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/Identify/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void IdentifyAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                               const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::Identify::Attributes;
    namespace UN = unify::matter_bridge::Identify::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Identify::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Identify::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::IdentifyTime::Id: {
        using T                = MN::IdentifyTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "IdentifyTime attribute value is %s", unify_value.dump().c_str());
            UN::IdentifyTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Identify::Id, MN::IdentifyTime::Id);
        }
        break;
    }
        // type is enum8
    case MN::IdentifyType::Id: {
        using T                = MN::IdentifyType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "IdentifyType attribute value is %s", unify_value.dump().c_str());
            UN::IdentifyType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Identify::Id, MN::IdentifyType::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Identify::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Identify::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ScenesAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::Scenes::Attributes;
    namespace UN = unify::matter_bridge::Scenes::Attributes;
    if (aPath.mClusterId != Clusters::Scenes::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::SceneCount::Id: { // type is int8u
            MN::SceneCount::TypeInfo::Type value;
            UN::SceneCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentScene::Id: { // type is int8u
            MN::CurrentScene::TypeInfo::Type value;
            UN::CurrentScene::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentGroup::Id: { // type is group_id
            MN::CurrentGroup::TypeInfo::Type value;
            UN::CurrentGroup::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SceneValid::Id: { // type is boolean
            MN::SceneValid::TypeInfo::Type value;
            UN::SceneValid::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NameSupport::Id: { // type is bitmap8
            MN::NameSupport::TypeInfo::Type value;
            UN::NameSupport::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LastConfiguredBy::Id: { // type is node_id
            MN::LastConfiguredBy::TypeInfo::Type value;
            UN::LastConfiguredBy::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_SCENES_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ScenesAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::Scenes;

    if (aPath.mClusterId != Clusters::Scenes::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/Scenes/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ScenesAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                             const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::Scenes::Attributes;
    namespace UN = unify::matter_bridge::Scenes::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Scenes::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath        = ConcreteAttributePath(node_matter_endpoint, Clusters::Scenes::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::SceneCount::Id: {
        using T                = MN::SceneCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SceneCount attribute value is %s", unify_value.dump().c_str());
            UN::SceneCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::SceneCount::Id);
        }
        break;
    }
        // type is int8u
    case MN::CurrentScene::Id: {
        using T                = MN::CurrentScene::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentScene attribute value is %s", unify_value.dump().c_str());
            UN::CurrentScene::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::CurrentScene::Id);
        }
        break;
    }
        // type is group_id
    case MN::CurrentGroup::Id: {
        using T                = MN::CurrentGroup::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentGroup attribute value is %s", unify_value.dump().c_str());
            UN::CurrentGroup::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::CurrentGroup::Id);
        }
        break;
    }
        // type is boolean
    case MN::SceneValid::Id: {
        using T                = MN::SceneValid::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SceneValid attribute value is %s", unify_value.dump().c_str());
            UN::SceneValid::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::SceneValid::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::NameSupport::Id: {
        using T                = MN::NameSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NameSupport attribute value is %s", unify_value.dump().c_str());
            UN::NameSupport::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::NameSupport::Id);
        }
        break;
    }
        // type is node_id
    case MN::LastConfiguredBy::Id: {
        using T                = MN::LastConfiguredBy::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LastConfiguredBy attribute value is %s", unify_value.dump().c_str());
            UN::LastConfiguredBy::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::LastConfiguredBy::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Scenes::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OnOffAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::OnOff::Attributes;
    namespace UN = unify::matter_bridge::OnOff::Attributes;
    if (aPath.mClusterId != Clusters::OnOff::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::OnOff::Id: { // type is boolean
            MN::OnOff::TypeInfo::Type value;
            UN::OnOff::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::GlobalSceneControl::Id: { // type is boolean
            MN::GlobalSceneControl::TypeInfo::Type value;
            UN::GlobalSceneControl::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OnTime::Id: { // type is int16u
            MN::OnTime::TypeInfo::Type value;
            UN::OnTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OffWaitTime::Id: { // type is int16u
            MN::OffWaitTime::TypeInfo::Type value;
            UN::OffWaitTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StartUpOnOff::Id: { // type is OnOffStartUpOnOff
            MN::StartUpOnOff::TypeInfo::Type value;
            UN::StartUpOnOff::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_ON_OFF_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::OnOff;

    if (aPath.mClusterId != Clusters::OnOff::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::OnTime::Id: {

        Attributes::OnTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OnTime");
        break;
    }
    case Attributes::OffWaitTime::Id: {

        Attributes::OffWaitTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OffWaitTime");
        break;
    }
    case Attributes::StartUpOnOff::Id: {

        Attributes::StartUpOnOff::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("StartUpOnOff");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/OnOff/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void OnOffAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                                            const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::OnOff::Attributes;
    namespace UN = unify::matter_bridge::OnOff::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OnOff::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath        = ConcreteAttributePath(node_matter_endpoint, Clusters::OnOff::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is boolean
    case MN::OnOff::Id: {
        using T                = MN::OnOff::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OnOff attribute value is %s", unify_value.dump().c_str());
            UN::OnOff::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OnOff::Id);
        }
        break;
    }
        // type is boolean
    case MN::GlobalSceneControl::Id: {
        using T                = MN::GlobalSceneControl::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "GlobalSceneControl attribute value is %s", unify_value.dump().c_str());
            UN::GlobalSceneControl::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::GlobalSceneControl::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnTime::Id: {
        using T                = MN::OnTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OnTime attribute value is %s", unify_value.dump().c_str());
            UN::OnTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OnTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::OffWaitTime::Id: {
        using T                = MN::OffWaitTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OffWaitTime attribute value is %s", unify_value.dump().c_str());
            UN::OffWaitTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OffWaitTime::Id);
        }
        break;
    }
        // type is OnOffStartUpOnOff
    case MN::StartUpOnOff::Id: {
        using T                = MN::StartUpOnOff::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StartUpOnOff attribute value is %s", unify_value.dump().c_str());
            UN::StartUpOnOff::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::StartUpOnOff::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OnOffSwitchConfigurationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::OnOffSwitchConfiguration::Attributes;
    namespace UN = unify::matter_bridge::OnOffSwitchConfiguration::Attributes;
    if (aPath.mClusterId != Clusters::OnOffSwitchConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::SwitchType::Id: { // type is enum8
            MN::SwitchType::TypeInfo::Type value;
            UN::SwitchType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SwitchActions::Id: { // type is enum8
            MN::SwitchActions::TypeInfo::Type value;
            UN::SwitchActions::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_ON_OFF_SWITCH_CONFIGURATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffSwitchConfigurationAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::OnOffSwitchConfiguration;

    if (aPath.mClusterId != Clusters::OnOffSwitchConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::SwitchActions::Id: {

        Attributes::SwitchActions::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("SwitchActions");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/OnOffSwitchConfiguration/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void OnOffSwitchConfigurationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                               const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::OnOffSwitchConfiguration::Attributes;
    namespace UN = unify::matter_bridge::OnOffSwitchConfiguration::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OnOffSwitchConfiguration::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::OnOffSwitchConfiguration::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is enum8
    case MN::SwitchType::Id: {
        using T                = MN::SwitchType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SwitchType attribute value is %s", unify_value.dump().c_str());
            UN::SwitchType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOffSwitchConfiguration::Id,
                                                   MN::SwitchType::Id);
        }
        break;
    }
        // type is enum8
    case MN::SwitchActions::Id: {
        using T                = MN::SwitchActions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SwitchActions attribute value is %s", unify_value.dump().c_str());
            UN::SwitchActions::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOffSwitchConfiguration::Id,
                                                   MN::SwitchActions::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOffSwitchConfiguration::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOffSwitchConfiguration::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
LevelControlAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::LevelControl::Attributes;
    namespace UN = unify::matter_bridge::LevelControl::Attributes;
    if (aPath.mClusterId != Clusters::LevelControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::CurrentLevel::Id: { // type is int8u
            MN::CurrentLevel::TypeInfo::Type value;
            UN::CurrentLevel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RemainingTime::Id: { // type is int16u
            MN::RemainingTime::TypeInfo::Type value;
            UN::RemainingTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinLevel::Id: { // type is int8u
            MN::MinLevel::TypeInfo::Type value;
            UN::MinLevel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxLevel::Id: { // type is int8u
            MN::MaxLevel::TypeInfo::Type value;
            UN::MaxLevel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentFrequency::Id: { // type is int16u
            MN::CurrentFrequency::TypeInfo::Type value;
            UN::CurrentFrequency::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinFrequency::Id: { // type is int16u
            MN::MinFrequency::TypeInfo::Type value;
            UN::MinFrequency::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxFrequency::Id: { // type is int16u
            MN::MaxFrequency::TypeInfo::Type value;
            UN::MaxFrequency::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Options::Id: { // type is LevelControlOptions
            MN::Options::TypeInfo::Type value;
            UN::Options::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OnOffTransitionTime::Id: { // type is int16u
            MN::OnOffTransitionTime::TypeInfo::Type value;
            UN::OnOffTransitionTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OnLevel::Id: { // type is int8u
            MN::OnLevel::TypeInfo::Type value;
            UN::OnLevel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OnTransitionTime::Id: { // type is int16u
            MN::OnTransitionTime::TypeInfo::Type value;
            UN::OnTransitionTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OffTransitionTime::Id: { // type is int16u
            MN::OffTransitionTime::TypeInfo::Type value;
            UN::OffTransitionTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DefaultMoveRate::Id: { // type is int8u
            MN::DefaultMoveRate::TypeInfo::Type value;
            UN::DefaultMoveRate::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StartUpCurrentLevel::Id: { // type is int8u
            MN::StartUpCurrentLevel::TypeInfo::Type value;
            UN::StartUpCurrentLevel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_LEVEL_CONTROL_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::LevelControl;

    if (aPath.mClusterId != Clusters::LevelControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::Options::Id: {

        Attributes::Options::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Options");
        break;
    }
    case Attributes::OnOffTransitionTime::Id: {

        Attributes::OnOffTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OnOffTransitionTime");
        break;
    }
    case Attributes::OnLevel::Id: {

        Attributes::OnLevel::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OnLevel");
        break;
    }
    case Attributes::OnTransitionTime::Id: {

        Attributes::OnTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OnTransitionTime");
        break;
    }
    case Attributes::OffTransitionTime::Id: {

        Attributes::OffTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OffTransitionTime");
        break;
    }
    case Attributes::DefaultMoveRate::Id: {

        Attributes::DefaultMoveRate::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("DefaultMoveRate");
        break;
    }
    case Attributes::StartUpCurrentLevel::Id: {

        Attributes::StartUpCurrentLevel::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("StartUpCurrentLevel");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/Level/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void LevelControlAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                   const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::LevelControl::Attributes;
    namespace UN = unify::matter_bridge::LevelControl::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::LevelControl::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::LevelControl::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::CurrentLevel::Id: {
        using T                = MN::CurrentLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentLevel attribute value is %s", unify_value.dump().c_str());
            UN::CurrentLevel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::CurrentLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::RemainingTime::Id: {
        using T                = MN::RemainingTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RemainingTime attribute value is %s", unify_value.dump().c_str());
            UN::RemainingTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::RemainingTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinLevel::Id: {
        using T                = MN::MinLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinLevel attribute value is %s", unify_value.dump().c_str());
            UN::MinLevel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MinLevel::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxLevel::Id: {
        using T                = MN::MaxLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxLevel attribute value is %s", unify_value.dump().c_str());
            UN::MaxLevel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MaxLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentFrequency::Id: {
        using T                = MN::CurrentFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentFrequency attribute value is %s", unify_value.dump().c_str());
            UN::CurrentFrequency::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::CurrentFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinFrequency::Id: {
        using T                = MN::MinFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinFrequency attribute value is %s", unify_value.dump().c_str());
            UN::MinFrequency::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MinFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxFrequency::Id: {
        using T                = MN::MaxFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxFrequency attribute value is %s", unify_value.dump().c_str());
            UN::MaxFrequency::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MaxFrequency::Id);
        }
        break;
    }
        // type is LevelControlOptions
    case MN::Options::Id: {
        using T                = MN::Options::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Options attribute value is %s", unify_value.dump().c_str());
            UN::Options::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::Options::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnOffTransitionTime::Id: {
        using T                = MN::OnOffTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OnOffTransitionTime attribute value is %s", unify_value.dump().c_str());
            UN::OnOffTransitionTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnOffTransitionTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::OnLevel::Id: {
        using T                = MN::OnLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OnLevel attribute value is %s", unify_value.dump().c_str());
            UN::OnLevel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnTransitionTime::Id: {
        using T                = MN::OnTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OnTransitionTime attribute value is %s", unify_value.dump().c_str());
            UN::OnTransitionTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnTransitionTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::OffTransitionTime::Id: {
        using T                = MN::OffTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OffTransitionTime attribute value is %s", unify_value.dump().c_str());
            UN::OffTransitionTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OffTransitionTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::DefaultMoveRate::Id: {
        using T                = MN::DefaultMoveRate::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DefaultMoveRate attribute value is %s", unify_value.dump().c_str());
            UN::DefaultMoveRate::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::DefaultMoveRate::Id);
        }
        break;
    }
        // type is int8u
    case MN::StartUpCurrentLevel::Id: {
        using T                = MN::StartUpCurrentLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StartUpCurrentLevel attribute value is %s", unify_value.dump().c_str());
            UN::StartUpCurrentLevel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::StartUpCurrentLevel::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
BinaryInputBasicAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::BinaryInputBasic::Attributes;
    namespace UN = unify::matter_bridge::BinaryInputBasic::Attributes;
    if (aPath.mClusterId != Clusters::BinaryInputBasic::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::ActiveText::Id: { // type is char_string
            MN::ActiveText::TypeInfo::Type value;
            UN::ActiveText::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Description::Id: { // type is char_string
            MN::Description::TypeInfo::Type value;
            UN::Description::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InactiveText::Id: { // type is char_string
            MN::InactiveText::TypeInfo::Type value;
            UN::InactiveText::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OutOfService::Id: { // type is boolean
            MN::OutOfService::TypeInfo::Type value;
            UN::OutOfService::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Polarity::Id: { // type is enum8
            MN::Polarity::TypeInfo::Type value;
            UN::Polarity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PresentValue::Id: { // type is boolean
            MN::PresentValue::TypeInfo::Type value;
            UN::PresentValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Reliability::Id: { // type is enum8
            MN::Reliability::TypeInfo::Type value;
            UN::Reliability::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StatusFlags::Id: { // type is bitmap8
            MN::StatusFlags::TypeInfo::Type value;
            UN::StatusFlags::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ApplicationType::Id: { // type is int32u
            MN::ApplicationType::TypeInfo::Type value;
            UN::ApplicationType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_BINARY_INPUT_BASIC_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BinaryInputBasicAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::BinaryInputBasic;

    if (aPath.mClusterId != Clusters::BinaryInputBasic::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::ActiveText::Id: {

        Attributes::ActiveText::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ActiveText");
        break;
    }
    case Attributes::Description::Id: {

        Attributes::Description::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Description");
        break;
    }
    case Attributes::InactiveText::Id: {

        Attributes::InactiveText::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("InactiveText");
        break;
    }
    case Attributes::OutOfService::Id: {

        Attributes::OutOfService::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OutOfService");
        break;
    }
    case Attributes::PresentValue::Id: {

        Attributes::PresentValue::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PresentValue");
        break;
    }
    case Attributes::Reliability::Id: {

        Attributes::Reliability::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Reliability");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/BinaryInputBasic/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void BinaryInputBasicAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                       const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::BinaryInputBasic::Attributes;
    namespace UN = unify::matter_bridge::BinaryInputBasic::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::BinaryInputBasic::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::BinaryInputBasic::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is char_string
    case MN::ActiveText::Id: {
        using T                = MN::ActiveText::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActiveText attribute value is %s", unify_value.dump().c_str());
            UN::ActiveText::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::ActiveText::Id);
        }
        break;
    }
        // type is char_string
    case MN::Description::Id: {
        using T                = MN::Description::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Description attribute value is %s", unify_value.dump().c_str());
            UN::Description::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::Description::Id);
        }
        break;
    }
        // type is char_string
    case MN::InactiveText::Id: {
        using T                = MN::InactiveText::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InactiveText attribute value is %s", unify_value.dump().c_str());
            UN::InactiveText::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::InactiveText::Id);
        }
        break;
    }
        // type is boolean
    case MN::OutOfService::Id: {
        using T                = MN::OutOfService::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OutOfService attribute value is %s", unify_value.dump().c_str());
            UN::OutOfService::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::OutOfService::Id);
        }
        break;
    }
        // type is enum8
    case MN::Polarity::Id: {
        using T                = MN::Polarity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Polarity attribute value is %s", unify_value.dump().c_str());
            UN::Polarity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::Polarity::Id);
        }
        break;
    }
        // type is boolean
    case MN::PresentValue::Id: {
        using T                = MN::PresentValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PresentValue attribute value is %s", unify_value.dump().c_str());
            UN::PresentValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::PresentValue::Id);
        }
        break;
    }
        // type is enum8
    case MN::Reliability::Id: {
        using T                = MN::Reliability::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Reliability attribute value is %s", unify_value.dump().c_str());
            UN::Reliability::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::Reliability::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::StatusFlags::Id: {
        using T                = MN::StatusFlags::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StatusFlags attribute value is %s", unify_value.dump().c_str());
            UN::StatusFlags::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::StatusFlags::Id);
        }
        break;
    }
        // type is int32u
    case MN::ApplicationType::Id: {
        using T                = MN::ApplicationType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ApplicationType attribute value is %s", unify_value.dump().c_str());
            UN::ApplicationType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::ApplicationType::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BinaryInputBasic::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
PulseWidthModulationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::PulseWidthModulation::Attributes;
    namespace UN = unify::matter_bridge::PulseWidthModulation::Attributes;
    if (aPath.mClusterId != Clusters::PulseWidthModulation::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_PULSE_WIDTH_MODULATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PulseWidthModulationAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::PulseWidthModulation;

    if (aPath.mClusterId != Clusters::PulseWidthModulation::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/PulseWidthModulation/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void PulseWidthModulationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                           const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::PulseWidthModulation::Attributes;
    namespace UN = unify::matter_bridge::PulseWidthModulation::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::PulseWidthModulation::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::PulseWidthModulation::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PulseWidthModulation::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PulseWidthModulation::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
BasicAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::Basic::Attributes;
    namespace UN = unify::matter_bridge::Basic::Attributes;
    if (aPath.mClusterId != Clusters::Basic::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::DataModelRevision::Id: { // type is int16u
            MN::DataModelRevision::TypeInfo::Type value;
            UN::DataModelRevision::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::VendorName::Id: { // type is char_string
            MN::VendorName::TypeInfo::Type value;
            UN::VendorName::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::VendorID::Id: { // type is vendor_id
            MN::VendorID::TypeInfo::Type value;
            UN::VendorID::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ProductName::Id: { // type is char_string
            MN::ProductName::TypeInfo::Type value;
            UN::ProductName::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ProductID::Id: { // type is int16u
            MN::ProductID::TypeInfo::Type value;
            UN::ProductID::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NodeLabel::Id: { // type is char_string
            MN::NodeLabel::TypeInfo::Type value;
            UN::NodeLabel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Location::Id: { // type is char_string
            MN::Location::TypeInfo::Type value;
            UN::Location::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::HardwareVersion::Id: { // type is int16u
            MN::HardwareVersion::TypeInfo::Type value;
            UN::HardwareVersion::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::HardwareVersionString::Id: { // type is char_string
            MN::HardwareVersionString::TypeInfo::Type value;
            UN::HardwareVersionString::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SoftwareVersion::Id: { // type is int32u
            MN::SoftwareVersion::TypeInfo::Type value;
            UN::SoftwareVersion::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SoftwareVersionString::Id: { // type is char_string
            MN::SoftwareVersionString::TypeInfo::Type value;
            UN::SoftwareVersionString::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ManufacturingDate::Id: { // type is char_string
            MN::ManufacturingDate::TypeInfo::Type value;
            UN::ManufacturingDate::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PartNumber::Id: { // type is char_string
            MN::PartNumber::TypeInfo::Type value;
            UN::PartNumber::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ProductURL::Id: { // type is long_char_string
            MN::ProductURL::TypeInfo::Type value;
            UN::ProductURL::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ProductLabel::Id: { // type is char_string
            MN::ProductLabel::TypeInfo::Type value;
            UN::ProductLabel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SerialNumber::Id: { // type is char_string
            MN::SerialNumber::TypeInfo::Type value;
            UN::SerialNumber::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LocalConfigDisabled::Id: { // type is boolean
            MN::LocalConfigDisabled::TypeInfo::Type value;
            UN::LocalConfigDisabled::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Reachable::Id: { // type is boolean
            MN::Reachable::TypeInfo::Type value;
            UN::Reachable::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UniqueID::Id: { // type is char_string
            MN::UniqueID::TypeInfo::Type value;
            UN::UniqueID::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_BASIC_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BasicAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::Basic;

    if (aPath.mClusterId != Clusters::Basic::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::NodeLabel::Id: {

        Attributes::NodeLabel::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("NodeLabel");
        break;
    }
    case Attributes::Location::Id: {

        Attributes::Location::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Location");
        break;
    }
    case Attributes::LocalConfigDisabled::Id: {

        Attributes::LocalConfigDisabled::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("LocalConfigDisabled");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/Basic/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void BasicAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                                            const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::Basic::Attributes;
    namespace UN = unify::matter_bridge::Basic::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Basic::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath        = ConcreteAttributePath(node_matter_endpoint, Clusters::Basic::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::DataModelRevision::Id: {
        using T                = MN::DataModelRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DataModelRevision attribute value is %s", unify_value.dump().c_str());
            UN::DataModelRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::DataModelRevision::Id);
        }
        break;
    }
        // type is char_string
    case MN::VendorName::Id: {
        using T                = MN::VendorName::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "VendorName attribute value is %s", unify_value.dump().c_str());
            UN::VendorName::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::VendorName::Id);
        }
        break;
    }
        // type is vendor_id
    case MN::VendorID::Id: {
        using T                = MN::VendorID::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "VendorID attribute value is %s", unify_value.dump().c_str());
            UN::VendorID::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::VendorID::Id);
        }
        break;
    }
        // type is char_string
    case MN::ProductName::Id: {
        using T                = MN::ProductName::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ProductName attribute value is %s", unify_value.dump().c_str());
            UN::ProductName::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::ProductName::Id);
        }
        break;
    }
        // type is int16u
    case MN::ProductID::Id: {
        using T                = MN::ProductID::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ProductID attribute value is %s", unify_value.dump().c_str());
            UN::ProductID::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::ProductID::Id);
        }
        break;
    }
        // type is char_string
    case MN::NodeLabel::Id: {
        using T                = MN::NodeLabel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NodeLabel attribute value is %s", unify_value.dump().c_str());
            UN::NodeLabel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::NodeLabel::Id);
        }
        break;
    }
        // type is char_string
    case MN::Location::Id: {
        using T                = MN::Location::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Location attribute value is %s", unify_value.dump().c_str());
            UN::Location::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::Location::Id);
        }
        break;
    }
        // type is int16u
    case MN::HardwareVersion::Id: {
        using T                = MN::HardwareVersion::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "HardwareVersion attribute value is %s", unify_value.dump().c_str());
            UN::HardwareVersion::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::HardwareVersion::Id);
        }
        break;
    }
        // type is char_string
    case MN::HardwareVersionString::Id: {
        using T                = MN::HardwareVersionString::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "HardwareVersionString attribute value is %s", unify_value.dump().c_str());
            UN::HardwareVersionString::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::HardwareVersionString::Id);
        }
        break;
    }
        // type is int32u
    case MN::SoftwareVersion::Id: {
        using T                = MN::SoftwareVersion::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SoftwareVersion attribute value is %s", unify_value.dump().c_str());
            UN::SoftwareVersion::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::SoftwareVersion::Id);
        }
        break;
    }
        // type is char_string
    case MN::SoftwareVersionString::Id: {
        using T                = MN::SoftwareVersionString::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SoftwareVersionString attribute value is %s", unify_value.dump().c_str());
            UN::SoftwareVersionString::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::SoftwareVersionString::Id);
        }
        break;
    }
        // type is char_string
    case MN::ManufacturingDate::Id: {
        using T                = MN::ManufacturingDate::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ManufacturingDate attribute value is %s", unify_value.dump().c_str());
            UN::ManufacturingDate::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::ManufacturingDate::Id);
        }
        break;
    }
        // type is char_string
    case MN::PartNumber::Id: {
        using T                = MN::PartNumber::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PartNumber attribute value is %s", unify_value.dump().c_str());
            UN::PartNumber::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::PartNumber::Id);
        }
        break;
    }
        // type is long_char_string
    case MN::ProductURL::Id: {
        using T                = MN::ProductURL::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ProductURL attribute value is %s", unify_value.dump().c_str());
            UN::ProductURL::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::ProductURL::Id);
        }
        break;
    }
        // type is char_string
    case MN::ProductLabel::Id: {
        using T                = MN::ProductLabel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ProductLabel attribute value is %s", unify_value.dump().c_str());
            UN::ProductLabel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::ProductLabel::Id);
        }
        break;
    }
        // type is char_string
    case MN::SerialNumber::Id: {
        using T                = MN::SerialNumber::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SerialNumber attribute value is %s", unify_value.dump().c_str());
            UN::SerialNumber::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::SerialNumber::Id);
        }
        break;
    }
        // type is boolean
    case MN::LocalConfigDisabled::Id: {
        using T                = MN::LocalConfigDisabled::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LocalConfigDisabled attribute value is %s", unify_value.dump().c_str());
            UN::LocalConfigDisabled::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::LocalConfigDisabled::Id);
        }
        break;
    }
        // type is boolean
    case MN::Reachable::Id: {
        using T                = MN::Reachable::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Reachable attribute value is %s", unify_value.dump().c_str());
            UN::Reachable::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::Reachable::Id);
        }
        break;
    }
        // type is char_string
    case MN::UniqueID::Id: {
        using T                = MN::UniqueID::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UniqueID attribute value is %s", unify_value.dump().c_str());
            UN::UniqueID::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::UniqueID::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Basic::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OtaSoftwareUpdateProviderAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::OtaSoftwareUpdateProvider::Attributes;
    namespace UN = unify::matter_bridge::OtaSoftwareUpdateProvider::Attributes;
    if (aPath.mClusterId != Clusters::OtaSoftwareUpdateProvider::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_OTA_SOFTWARE_UPDATE_PROVIDER_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaSoftwareUpdateProviderAttributeAccess::Write(const ConcreteDataAttributePath & aPath,
                                                           AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;

    if (aPath.mClusterId != Clusters::OtaSoftwareUpdateProvider::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/OtaSoftwareUpdateProvider/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void OtaSoftwareUpdateProviderAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                                const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::OtaSoftwareUpdateProvider::Attributes;
    namespace UN = unify::matter_bridge::OtaSoftwareUpdateProvider::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OtaSoftwareUpdateProvider::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::OtaSoftwareUpdateProvider::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OtaSoftwareUpdateProvider::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OtaSoftwareUpdateProvider::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OtaSoftwareUpdateRequestorAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::OtaSoftwareUpdateRequestor::Attributes;
    namespace UN = unify::matter_bridge::OtaSoftwareUpdateRequestor::Attributes;
    if (aPath.mClusterId != Clusters::OtaSoftwareUpdateRequestor::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::UpdatePossible::Id: { // type is boolean
            MN::UpdatePossible::TypeInfo::Type value;
            UN::UpdatePossible::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UpdateState::Id: { // type is OTAUpdateStateEnum
            MN::UpdateState::TypeInfo::Type value;
            UN::UpdateState::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UpdateStateProgress::Id: { // type is int8u
            MN::UpdateStateProgress::TypeInfo::Type value;
            UN::UpdateStateProgress::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_OTA_SOFTWARE_UPDATE_REQUESTOR_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaSoftwareUpdateRequestorAttributeAccess::Write(const ConcreteDataAttributePath & aPath,
                                                            AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor;

    if (aPath.mClusterId != Clusters::OtaSoftwareUpdateRequestor::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::DefaultOtaProviders::Id: {

        Attributes::DefaultOtaProviders::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("DefaultOtaProviders");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/OtaSoftwareUpdateRequestor/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void OtaSoftwareUpdateRequestorAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                                 const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::OtaSoftwareUpdateRequestor::Attributes;
    namespace UN = unify::matter_bridge::OtaSoftwareUpdateRequestor::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OtaSoftwareUpdateRequestor::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is boolean
    case MN::UpdatePossible::Id: {
        using T                = MN::UpdatePossible::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UpdatePossible attribute value is %s", unify_value.dump().c_str());
            UN::UpdatePossible::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OtaSoftwareUpdateRequestor::Id,
                                                   MN::UpdatePossible::Id);
        }
        break;
    }
        // type is OTAUpdateStateEnum
    case MN::UpdateState::Id: {
        using T                = MN::UpdateState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UpdateState attribute value is %s", unify_value.dump().c_str());
            UN::UpdateState::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OtaSoftwareUpdateRequestor::Id,
                                                   MN::UpdateState::Id);
        }
        break;
    }
        // type is int8u
    case MN::UpdateStateProgress::Id: {
        using T                = MN::UpdateStateProgress::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UpdateStateProgress attribute value is %s", unify_value.dump().c_str());
            UN::UpdateStateProgress::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OtaSoftwareUpdateRequestor::Id,
                                                   MN::UpdateStateProgress::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OtaSoftwareUpdateRequestor::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OtaSoftwareUpdateRequestor::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
LocalizationConfigurationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::LocalizationConfiguration::Attributes;
    namespace UN = unify::matter_bridge::LocalizationConfiguration::Attributes;
    if (aPath.mClusterId != Clusters::LocalizationConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::ActiveLocale::Id: { // type is char_string
            MN::ActiveLocale::TypeInfo::Type value;
            UN::ActiveLocale::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_LOCALIZATION_CONFIGURATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LocalizationConfigurationAttributeAccess::Write(const ConcreteDataAttributePath & aPath,
                                                           AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::LocalizationConfiguration;

    if (aPath.mClusterId != Clusters::LocalizationConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::ActiveLocale::Id: {

        Attributes::ActiveLocale::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ActiveLocale");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/LocalizationConfiguration/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void LocalizationConfigurationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                                const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::LocalizationConfiguration::Attributes;
    namespace UN = unify::matter_bridge::LocalizationConfiguration::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::LocalizationConfiguration::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::LocalizationConfiguration::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is char_string
    case MN::ActiveLocale::Id: {
        using T                = MN::ActiveLocale::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActiveLocale attribute value is %s", unify_value.dump().c_str());
            UN::ActiveLocale::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LocalizationConfiguration::Id,
                                                   MN::ActiveLocale::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LocalizationConfiguration::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LocalizationConfiguration::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
TimeFormatLocalizationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::TimeFormatLocalization::Attributes;
    namespace UN = unify::matter_bridge::TimeFormatLocalization::Attributes;
    if (aPath.mClusterId != Clusters::TimeFormatLocalization::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::HourFormat::Id: { // type is HourFormat
            MN::HourFormat::TypeInfo::Type value;
            UN::HourFormat::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActiveCalendarType::Id: { // type is CalendarType
            MN::ActiveCalendarType::TypeInfo::Type value;
            UN::ActiveCalendarType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_TIME_FORMAT_LOCALIZATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeFormatLocalizationAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::TimeFormatLocalization;

    if (aPath.mClusterId != Clusters::TimeFormatLocalization::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::HourFormat::Id: {

        Attributes::HourFormat::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("HourFormat");
        break;
    }
    case Attributes::ActiveCalendarType::Id: {

        Attributes::ActiveCalendarType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ActiveCalendarType");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/TimeFormatLocalization/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void TimeFormatLocalizationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                             const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::TimeFormatLocalization::Attributes;
    namespace UN = unify::matter_bridge::TimeFormatLocalization::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::TimeFormatLocalization::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::TimeFormatLocalization::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is HourFormat
    case MN::HourFormat::Id: {
        using T                = MN::HourFormat::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "HourFormat attribute value is %s", unify_value.dump().c_str());
            UN::HourFormat::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeFormatLocalization::Id, MN::HourFormat::Id);
        }
        break;
    }
        // type is CalendarType
    case MN::ActiveCalendarType::Id: {
        using T                = MN::ActiveCalendarType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActiveCalendarType attribute value is %s", unify_value.dump().c_str());
            UN::ActiveCalendarType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeFormatLocalization::Id,
                                                   MN::ActiveCalendarType::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeFormatLocalization::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeFormatLocalization::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
UnitLocalizationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::UnitLocalization::Attributes;
    namespace UN = unify::matter_bridge::UnitLocalization::Attributes;
    if (aPath.mClusterId != Clusters::UnitLocalization::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::TemperatureUnit::Id: { // type is TempUnit
            MN::TemperatureUnit::TypeInfo::Type value;
            UN::TemperatureUnit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_UNIT_LOCALIZATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR UnitLocalizationAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::UnitLocalization;

    if (aPath.mClusterId != Clusters::UnitLocalization::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::TemperatureUnit::Id: {

        Attributes::TemperatureUnit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("TemperatureUnit");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/UnitLocalization/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void UnitLocalizationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                       const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::UnitLocalization::Attributes;
    namespace UN = unify::matter_bridge::UnitLocalization::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::UnitLocalization::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::UnitLocalization::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is TempUnit
    case MN::TemperatureUnit::Id: {
        using T                = MN::TemperatureUnit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TemperatureUnit attribute value is %s", unify_value.dump().c_str());
            UN::TemperatureUnit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::UnitLocalization::Id, MN::TemperatureUnit::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::UnitLocalization::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::UnitLocalization::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
PowerSourceConfigurationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::PowerSourceConfiguration::Attributes;
    namespace UN = unify::matter_bridge::PowerSourceConfiguration::Attributes;
    if (aPath.mClusterId != Clusters::PowerSourceConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_POWER_SOURCE_CONFIGURATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceConfigurationAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::PowerSourceConfiguration;

    if (aPath.mClusterId != Clusters::PowerSourceConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/PowerSourceConfiguration/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void PowerSourceConfigurationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                               const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::PowerSourceConfiguration::Attributes;
    namespace UN = unify::matter_bridge::PowerSourceConfiguration::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::PowerSourceConfiguration::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::PowerSourceConfiguration::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSourceConfiguration::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSourceConfiguration::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
PowerSourceAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::PowerSource::Attributes;
    namespace UN = unify::matter_bridge::PowerSource::Attributes;
    if (aPath.mClusterId != Clusters::PowerSource::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::Status::Id: { // type is PowerSourceStatus
            MN::Status::TypeInfo::Type value;
            UN::Status::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Order::Id: { // type is int8u
            MN::Order::TypeInfo::Type value;
            UN::Order::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Description::Id: { // type is char_string
            MN::Description::TypeInfo::Type value;
            UN::Description::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiredAssessedInputVoltage::Id: { // type is int32u
            MN::WiredAssessedInputVoltage::TypeInfo::Type value;
            UN::WiredAssessedInputVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiredAssessedInputFrequency::Id: { // type is int16u
            MN::WiredAssessedInputFrequency::TypeInfo::Type value;
            UN::WiredAssessedInputFrequency::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiredCurrentType::Id: { // type is WiredCurrentType
            MN::WiredCurrentType::TypeInfo::Type value;
            UN::WiredCurrentType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiredAssessedCurrent::Id: { // type is int32u
            MN::WiredAssessedCurrent::TypeInfo::Type value;
            UN::WiredAssessedCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiredNominalVoltage::Id: { // type is int32u
            MN::WiredNominalVoltage::TypeInfo::Type value;
            UN::WiredNominalVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiredMaximumCurrent::Id: { // type is int32u
            MN::WiredMaximumCurrent::TypeInfo::Type value;
            UN::WiredMaximumCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiredPresent::Id: { // type is boolean
            MN::WiredPresent::TypeInfo::Type value;
            UN::WiredPresent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatVoltage::Id: { // type is int32u
            MN::BatVoltage::TypeInfo::Type value;
            UN::BatVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatPercentRemaining::Id: { // type is int8u
            MN::BatPercentRemaining::TypeInfo::Type value;
            UN::BatPercentRemaining::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatTimeRemaining::Id: { // type is int32u
            MN::BatTimeRemaining::TypeInfo::Type value;
            UN::BatTimeRemaining::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatChargeLevel::Id: { // type is BatChargeLevel
            MN::BatChargeLevel::TypeInfo::Type value;
            UN::BatChargeLevel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatReplacementNeeded::Id: { // type is boolean
            MN::BatReplacementNeeded::TypeInfo::Type value;
            UN::BatReplacementNeeded::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatReplaceability::Id: { // type is BatReplaceability
            MN::BatReplaceability::TypeInfo::Type value;
            UN::BatReplaceability::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatPresent::Id: { // type is boolean
            MN::BatPresent::TypeInfo::Type value;
            UN::BatPresent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatReplacementDescription::Id: { // type is char_string
            MN::BatReplacementDescription::TypeInfo::Type value;
            UN::BatReplacementDescription::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatCommonDesignation::Id: { // type is int32u
            MN::BatCommonDesignation::TypeInfo::Type value;
            UN::BatCommonDesignation::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatANSIDesignation::Id: { // type is char_string
            MN::BatANSIDesignation::TypeInfo::Type value;
            UN::BatANSIDesignation::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatIECDesignation::Id: { // type is char_string
            MN::BatIECDesignation::TypeInfo::Type value;
            UN::BatIECDesignation::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatApprovedChemistry::Id: { // type is int32u
            MN::BatApprovedChemistry::TypeInfo::Type value;
            UN::BatApprovedChemistry::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatCapacity::Id: { // type is int32u
            MN::BatCapacity::TypeInfo::Type value;
            UN::BatCapacity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatQuantity::Id: { // type is int8u
            MN::BatQuantity::TypeInfo::Type value;
            UN::BatQuantity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatChargeState::Id: { // type is BatChargeState
            MN::BatChargeState::TypeInfo::Type value;
            UN::BatChargeState::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatTimeToFullCharge::Id: { // type is int32u
            MN::BatTimeToFullCharge::TypeInfo::Type value;
            UN::BatTimeToFullCharge::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatFunctionalWhileCharging::Id: { // type is boolean
            MN::BatFunctionalWhileCharging::TypeInfo::Type value;
            UN::BatFunctionalWhileCharging::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BatChargingCurrent::Id: { // type is int32u
            MN::BatChargingCurrent::TypeInfo::Type value;
            UN::BatChargingCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_POWER_SOURCE_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerSourceAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::PowerSource;

    if (aPath.mClusterId != Clusters::PowerSource::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/PowerSource/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void PowerSourceAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                  const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::PowerSource::Attributes;
    namespace UN = unify::matter_bridge::PowerSource::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::PowerSource::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::PowerSource::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is PowerSourceStatus
    case MN::Status::Id: {
        using T                = MN::Status::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Status attribute value is %s", unify_value.dump().c_str());
            UN::Status::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::Status::Id);
        }
        break;
    }
        // type is int8u
    case MN::Order::Id: {
        using T                = MN::Order::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Order attribute value is %s", unify_value.dump().c_str());
            UN::Order::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::Order::Id);
        }
        break;
    }
        // type is char_string
    case MN::Description::Id: {
        using T                = MN::Description::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Description attribute value is %s", unify_value.dump().c_str());
            UN::Description::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::Description::Id);
        }
        break;
    }
        // type is int32u
    case MN::WiredAssessedInputVoltage::Id: {
        using T                = MN::WiredAssessedInputVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiredAssessedInputVoltage attribute value is %s", unify_value.dump().c_str());
            UN::WiredAssessedInputVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id,
                                                   MN::WiredAssessedInputVoltage::Id);
        }
        break;
    }
        // type is int16u
    case MN::WiredAssessedInputFrequency::Id: {
        using T                = MN::WiredAssessedInputFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiredAssessedInputFrequency attribute value is %s", unify_value.dump().c_str());
            UN::WiredAssessedInputFrequency::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id,
                                                   MN::WiredAssessedInputFrequency::Id);
        }
        break;
    }
        // type is WiredCurrentType
    case MN::WiredCurrentType::Id: {
        using T                = MN::WiredCurrentType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiredCurrentType attribute value is %s", unify_value.dump().c_str());
            UN::WiredCurrentType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::WiredCurrentType::Id);
        }
        break;
    }
        // type is int32u
    case MN::WiredAssessedCurrent::Id: {
        using T                = MN::WiredAssessedCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiredAssessedCurrent attribute value is %s", unify_value.dump().c_str());
            UN::WiredAssessedCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::WiredAssessedCurrent::Id);
        }
        break;
    }
        // type is int32u
    case MN::WiredNominalVoltage::Id: {
        using T                = MN::WiredNominalVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiredNominalVoltage attribute value is %s", unify_value.dump().c_str());
            UN::WiredNominalVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::WiredNominalVoltage::Id);
        }
        break;
    }
        // type is int32u
    case MN::WiredMaximumCurrent::Id: {
        using T                = MN::WiredMaximumCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiredMaximumCurrent attribute value is %s", unify_value.dump().c_str());
            UN::WiredMaximumCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::WiredMaximumCurrent::Id);
        }
        break;
    }
        // type is boolean
    case MN::WiredPresent::Id: {
        using T                = MN::WiredPresent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiredPresent attribute value is %s", unify_value.dump().c_str());
            UN::WiredPresent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::WiredPresent::Id);
        }
        break;
    }
        // type is int32u
    case MN::BatVoltage::Id: {
        using T                = MN::BatVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatVoltage attribute value is %s", unify_value.dump().c_str());
            UN::BatVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatVoltage::Id);
        }
        break;
    }
        // type is int8u
    case MN::BatPercentRemaining::Id: {
        using T                = MN::BatPercentRemaining::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatPercentRemaining attribute value is %s", unify_value.dump().c_str());
            UN::BatPercentRemaining::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatPercentRemaining::Id);
        }
        break;
    }
        // type is int32u
    case MN::BatTimeRemaining::Id: {
        using T                = MN::BatTimeRemaining::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatTimeRemaining attribute value is %s", unify_value.dump().c_str());
            UN::BatTimeRemaining::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatTimeRemaining::Id);
        }
        break;
    }
        // type is BatChargeLevel
    case MN::BatChargeLevel::Id: {
        using T                = MN::BatChargeLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatChargeLevel attribute value is %s", unify_value.dump().c_str());
            UN::BatChargeLevel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatChargeLevel::Id);
        }
        break;
    }
        // type is boolean
    case MN::BatReplacementNeeded::Id: {
        using T                = MN::BatReplacementNeeded::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatReplacementNeeded attribute value is %s", unify_value.dump().c_str());
            UN::BatReplacementNeeded::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatReplacementNeeded::Id);
        }
        break;
    }
        // type is BatReplaceability
    case MN::BatReplaceability::Id: {
        using T                = MN::BatReplaceability::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatReplaceability attribute value is %s", unify_value.dump().c_str());
            UN::BatReplaceability::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatReplaceability::Id);
        }
        break;
    }
        // type is boolean
    case MN::BatPresent::Id: {
        using T                = MN::BatPresent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatPresent attribute value is %s", unify_value.dump().c_str());
            UN::BatPresent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatPresent::Id);
        }
        break;
    }
        // type is char_string
    case MN::BatReplacementDescription::Id: {
        using T                = MN::BatReplacementDescription::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatReplacementDescription attribute value is %s", unify_value.dump().c_str());
            UN::BatReplacementDescription::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id,
                                                   MN::BatReplacementDescription::Id);
        }
        break;
    }
        // type is int32u
    case MN::BatCommonDesignation::Id: {
        using T                = MN::BatCommonDesignation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatCommonDesignation attribute value is %s", unify_value.dump().c_str());
            UN::BatCommonDesignation::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatCommonDesignation::Id);
        }
        break;
    }
        // type is char_string
    case MN::BatANSIDesignation::Id: {
        using T                = MN::BatANSIDesignation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatANSIDesignation attribute value is %s", unify_value.dump().c_str());
            UN::BatANSIDesignation::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatANSIDesignation::Id);
        }
        break;
    }
        // type is char_string
    case MN::BatIECDesignation::Id: {
        using T                = MN::BatIECDesignation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatIECDesignation attribute value is %s", unify_value.dump().c_str());
            UN::BatIECDesignation::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatIECDesignation::Id);
        }
        break;
    }
        // type is int32u
    case MN::BatApprovedChemistry::Id: {
        using T                = MN::BatApprovedChemistry::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatApprovedChemistry attribute value is %s", unify_value.dump().c_str());
            UN::BatApprovedChemistry::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatApprovedChemistry::Id);
        }
        break;
    }
        // type is int32u
    case MN::BatCapacity::Id: {
        using T                = MN::BatCapacity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatCapacity attribute value is %s", unify_value.dump().c_str());
            UN::BatCapacity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatCapacity::Id);
        }
        break;
    }
        // type is int8u
    case MN::BatQuantity::Id: {
        using T                = MN::BatQuantity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatQuantity attribute value is %s", unify_value.dump().c_str());
            UN::BatQuantity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatQuantity::Id);
        }
        break;
    }
        // type is BatChargeState
    case MN::BatChargeState::Id: {
        using T                = MN::BatChargeState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatChargeState attribute value is %s", unify_value.dump().c_str());
            UN::BatChargeState::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatChargeState::Id);
        }
        break;
    }
        // type is int32u
    case MN::BatTimeToFullCharge::Id: {
        using T                = MN::BatTimeToFullCharge::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatTimeToFullCharge attribute value is %s", unify_value.dump().c_str());
            UN::BatTimeToFullCharge::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatTimeToFullCharge::Id);
        }
        break;
    }
        // type is boolean
    case MN::BatFunctionalWhileCharging::Id: {
        using T                = MN::BatFunctionalWhileCharging::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatFunctionalWhileCharging attribute value is %s", unify_value.dump().c_str());
            UN::BatFunctionalWhileCharging::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id,
                                                   MN::BatFunctionalWhileCharging::Id);
        }
        break;
    }
        // type is int32u
    case MN::BatChargingCurrent::Id: {
        using T                = MN::BatChargingCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BatChargingCurrent attribute value is %s", unify_value.dump().c_str());
            UN::BatChargingCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::BatChargingCurrent::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PowerSource::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
GeneralCommissioningAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::GeneralCommissioning::Attributes;
    namespace UN = unify::matter_bridge::GeneralCommissioning::Attributes;
    if (aPath.mClusterId != Clusters::GeneralCommissioning::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::Breadcrumb::Id: { // type is int64u
            MN::Breadcrumb::TypeInfo::Type value;
            UN::Breadcrumb::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RegulatoryConfig::Id: { // type is RegulatoryLocationType
            MN::RegulatoryConfig::TypeInfo::Type value;
            UN::RegulatoryConfig::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LocationCapability::Id: { // type is RegulatoryLocationType
            MN::LocationCapability::TypeInfo::Type value;
            UN::LocationCapability::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SupportsConcurrentConnection::Id: { // type is boolean
            MN::SupportsConcurrentConnection::TypeInfo::Type value;
            UN::SupportsConcurrentConnection::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_GENERAL_COMMISSIONING_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GeneralCommissioningAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::GeneralCommissioning;

    if (aPath.mClusterId != Clusters::GeneralCommissioning::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::Breadcrumb::Id: {

        Attributes::Breadcrumb::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Breadcrumb");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/GeneralCommissioning/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void GeneralCommissioningAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                           const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::GeneralCommissioning::Attributes;
    namespace UN = unify::matter_bridge::GeneralCommissioning::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::GeneralCommissioning::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::GeneralCommissioning::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int64u
    case MN::Breadcrumb::Id: {
        using T                = MN::Breadcrumb::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Breadcrumb attribute value is %s", unify_value.dump().c_str());
            UN::Breadcrumb::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralCommissioning::Id, MN::Breadcrumb::Id);
        }
        break;
    }
        // type is RegulatoryLocationType
    case MN::RegulatoryConfig::Id: {
        using T                = MN::RegulatoryConfig::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RegulatoryConfig attribute value is %s", unify_value.dump().c_str());
            UN::RegulatoryConfig::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralCommissioning::Id,
                                                   MN::RegulatoryConfig::Id);
        }
        break;
    }
        // type is RegulatoryLocationType
    case MN::LocationCapability::Id: {
        using T                = MN::LocationCapability::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LocationCapability attribute value is %s", unify_value.dump().c_str());
            UN::LocationCapability::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralCommissioning::Id,
                                                   MN::LocationCapability::Id);
        }
        break;
    }
        // type is boolean
    case MN::SupportsConcurrentConnection::Id: {
        using T                = MN::SupportsConcurrentConnection::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SupportsConcurrentConnection attribute value is %s", unify_value.dump().c_str());
            UN::SupportsConcurrentConnection::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralCommissioning::Id,
                                                   MN::SupportsConcurrentConnection::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralCommissioning::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralCommissioning::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
DiagnosticLogsAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::DiagnosticLogs::Attributes;
    namespace UN = unify::matter_bridge::DiagnosticLogs::Attributes;
    if (aPath.mClusterId != Clusters::DiagnosticLogs::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_DIAGNOSTIC_LOGS_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticLogsAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::DiagnosticLogs;

    if (aPath.mClusterId != Clusters::DiagnosticLogs::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/DiagnosticLogs/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void DiagnosticLogsAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                     const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::DiagnosticLogs::Attributes;
    namespace UN = unify::matter_bridge::DiagnosticLogs::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::DiagnosticLogs::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::DiagnosticLogs::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DiagnosticLogs::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DiagnosticLogs::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
GeneralDiagnosticsAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::GeneralDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::GeneralDiagnostics::Attributes;
    if (aPath.mClusterId != Clusters::GeneralDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::RebootCount::Id: { // type is int16u
            MN::RebootCount::TypeInfo::Type value;
            UN::RebootCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UpTime::Id: { // type is int64u
            MN::UpTime::TypeInfo::Type value;
            UN::UpTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TotalOperationalHours::Id: { // type is int32u
            MN::TotalOperationalHours::TypeInfo::Type value;
            UN::TotalOperationalHours::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BootReasons::Id: { // type is enum8
            MN::BootReasons::TypeInfo::Type value;
            UN::BootReasons::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TestEventTriggersEnabled::Id: { // type is boolean
            MN::TestEventTriggersEnabled::TypeInfo::Type value;
            UN::TestEventTriggersEnabled::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_GENERAL_DIAGNOSTICS_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GeneralDiagnosticsAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::GeneralDiagnostics;

    if (aPath.mClusterId != Clusters::GeneralDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/GeneralDiagnostics/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void GeneralDiagnosticsAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                         const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::GeneralDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::GeneralDiagnostics::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::GeneralDiagnostics::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::GeneralDiagnostics::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::RebootCount::Id: {
        using T                = MN::RebootCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RebootCount attribute value is %s", unify_value.dump().c_str());
            UN::RebootCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralDiagnostics::Id, MN::RebootCount::Id);
        }
        break;
    }
        // type is int64u
    case MN::UpTime::Id: {
        using T                = MN::UpTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UpTime attribute value is %s", unify_value.dump().c_str());
            UN::UpTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralDiagnostics::Id, MN::UpTime::Id);
        }
        break;
    }
        // type is int32u
    case MN::TotalOperationalHours::Id: {
        using T                = MN::TotalOperationalHours::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TotalOperationalHours attribute value is %s", unify_value.dump().c_str());
            UN::TotalOperationalHours::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralDiagnostics::Id,
                                                   MN::TotalOperationalHours::Id);
        }
        break;
    }
        // type is enum8
    case MN::BootReasons::Id: {
        using T                = MN::BootReasons::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BootReasons attribute value is %s", unify_value.dump().c_str());
            UN::BootReasons::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralDiagnostics::Id, MN::BootReasons::Id);
        }
        break;
    }
        // type is boolean
    case MN::TestEventTriggersEnabled::Id: {
        using T                = MN::TestEventTriggersEnabled::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TestEventTriggersEnabled attribute value is %s", unify_value.dump().c_str());
            UN::TestEventTriggersEnabled::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralDiagnostics::Id,
                                                   MN::TestEventTriggersEnabled::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralDiagnostics::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GeneralDiagnostics::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
SoftwareDiagnosticsAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::SoftwareDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::SoftwareDiagnostics::Attributes;
    if (aPath.mClusterId != Clusters::SoftwareDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::CurrentHeapFree::Id: { // type is int64u
            MN::CurrentHeapFree::TypeInfo::Type value;
            UN::CurrentHeapFree::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentHeapUsed::Id: { // type is int64u
            MN::CurrentHeapUsed::TypeInfo::Type value;
            UN::CurrentHeapUsed::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentHeapHighWatermark::Id: { // type is int64u
            MN::CurrentHeapHighWatermark::TypeInfo::Type value;
            UN::CurrentHeapHighWatermark::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_SOFTWARE_DIAGNOSTICS_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SoftwareDiagnosticsAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::SoftwareDiagnostics;

    if (aPath.mClusterId != Clusters::SoftwareDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/SoftwareDiagnostics/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void SoftwareDiagnosticsAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                          const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::SoftwareDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::SoftwareDiagnostics::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::SoftwareDiagnostics::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::SoftwareDiagnostics::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int64u
    case MN::CurrentHeapFree::Id: {
        using T                = MN::CurrentHeapFree::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentHeapFree attribute value is %s", unify_value.dump().c_str());
            UN::CurrentHeapFree::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::SoftwareDiagnostics::Id,
                                                   MN::CurrentHeapFree::Id);
        }
        break;
    }
        // type is int64u
    case MN::CurrentHeapUsed::Id: {
        using T                = MN::CurrentHeapUsed::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentHeapUsed attribute value is %s", unify_value.dump().c_str());
            UN::CurrentHeapUsed::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::SoftwareDiagnostics::Id,
                                                   MN::CurrentHeapUsed::Id);
        }
        break;
    }
        // type is int64u
    case MN::CurrentHeapHighWatermark::Id: {
        using T                = MN::CurrentHeapHighWatermark::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentHeapHighWatermark attribute value is %s", unify_value.dump().c_str());
            UN::CurrentHeapHighWatermark::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::SoftwareDiagnostics::Id,
                                                   MN::CurrentHeapHighWatermark::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::SoftwareDiagnostics::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::SoftwareDiagnostics::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ThreadNetworkDiagnosticsAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::ThreadNetworkDiagnostics::Attributes;
    if (aPath.mClusterId != Clusters::ThreadNetworkDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::Channel::Id: { // type is int16u
            MN::Channel::TypeInfo::Type value;
            UN::Channel::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RoutingRole::Id: { // type is RoutingRole
            MN::RoutingRole::TypeInfo::Type value;
            UN::RoutingRole::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NetworkName::Id: { // type is char_string
            MN::NetworkName::TypeInfo::Type value;
            UN::NetworkName::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PanId::Id: { // type is int16u
            MN::PanId::TypeInfo::Type value;
            UN::PanId::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ExtendedPanId::Id: { // type is int64u
            MN::ExtendedPanId::TypeInfo::Type value;
            UN::ExtendedPanId::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MeshLocalPrefix::Id: { // type is octet_string
            MN::MeshLocalPrefix::TypeInfo::Type value;
            UN::MeshLocalPrefix::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OverrunCount::Id: { // type is int64u
            MN::OverrunCount::TypeInfo::Type value;
            UN::OverrunCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PartitionId::Id: { // type is int32u
            MN::PartitionId::TypeInfo::Type value;
            UN::PartitionId::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Weighting::Id: { // type is int8u
            MN::Weighting::TypeInfo::Type value;
            UN::Weighting::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DataVersion::Id: { // type is int8u
            MN::DataVersion::TypeInfo::Type value;
            UN::DataVersion::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StableDataVersion::Id: { // type is int8u
            MN::StableDataVersion::TypeInfo::Type value;
            UN::StableDataVersion::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LeaderRouterId::Id: { // type is int8u
            MN::LeaderRouterId::TypeInfo::Type value;
            UN::LeaderRouterId::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DetachedRoleCount::Id: { // type is int16u
            MN::DetachedRoleCount::TypeInfo::Type value;
            UN::DetachedRoleCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ChildRoleCount::Id: { // type is int16u
            MN::ChildRoleCount::TypeInfo::Type value;
            UN::ChildRoleCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RouterRoleCount::Id: { // type is int16u
            MN::RouterRoleCount::TypeInfo::Type value;
            UN::RouterRoleCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LeaderRoleCount::Id: { // type is int16u
            MN::LeaderRoleCount::TypeInfo::Type value;
            UN::LeaderRoleCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AttachAttemptCount::Id: { // type is int16u
            MN::AttachAttemptCount::TypeInfo::Type value;
            UN::AttachAttemptCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PartitionIdChangeCount::Id: { // type is int16u
            MN::PartitionIdChangeCount::TypeInfo::Type value;
            UN::PartitionIdChangeCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BetterPartitionAttachAttemptCount::Id: { // type is int16u
            MN::BetterPartitionAttachAttemptCount::TypeInfo::Type value;
            UN::BetterPartitionAttachAttemptCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ParentChangeCount::Id: { // type is int16u
            MN::ParentChangeCount::TypeInfo::Type value;
            UN::ParentChangeCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxTotalCount::Id: { // type is int32u
            MN::TxTotalCount::TypeInfo::Type value;
            UN::TxTotalCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxUnicastCount::Id: { // type is int32u
            MN::TxUnicastCount::TypeInfo::Type value;
            UN::TxUnicastCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxBroadcastCount::Id: { // type is int32u
            MN::TxBroadcastCount::TypeInfo::Type value;
            UN::TxBroadcastCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxAckRequestedCount::Id: { // type is int32u
            MN::TxAckRequestedCount::TypeInfo::Type value;
            UN::TxAckRequestedCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxAckedCount::Id: { // type is int32u
            MN::TxAckedCount::TypeInfo::Type value;
            UN::TxAckedCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxNoAckRequestedCount::Id: { // type is int32u
            MN::TxNoAckRequestedCount::TypeInfo::Type value;
            UN::TxNoAckRequestedCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxDataCount::Id: { // type is int32u
            MN::TxDataCount::TypeInfo::Type value;
            UN::TxDataCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxDataPollCount::Id: { // type is int32u
            MN::TxDataPollCount::TypeInfo::Type value;
            UN::TxDataPollCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxBeaconCount::Id: { // type is int32u
            MN::TxBeaconCount::TypeInfo::Type value;
            UN::TxBeaconCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxBeaconRequestCount::Id: { // type is int32u
            MN::TxBeaconRequestCount::TypeInfo::Type value;
            UN::TxBeaconRequestCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxOtherCount::Id: { // type is int32u
            MN::TxOtherCount::TypeInfo::Type value;
            UN::TxOtherCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxRetryCount::Id: { // type is int32u
            MN::TxRetryCount::TypeInfo::Type value;
            UN::TxRetryCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxDirectMaxRetryExpiryCount::Id: { // type is int32u
            MN::TxDirectMaxRetryExpiryCount::TypeInfo::Type value;
            UN::TxDirectMaxRetryExpiryCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxIndirectMaxRetryExpiryCount::Id: { // type is int32u
            MN::TxIndirectMaxRetryExpiryCount::TypeInfo::Type value;
            UN::TxIndirectMaxRetryExpiryCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxErrCcaCount::Id: { // type is int32u
            MN::TxErrCcaCount::TypeInfo::Type value;
            UN::TxErrCcaCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxErrAbortCount::Id: { // type is int32u
            MN::TxErrAbortCount::TypeInfo::Type value;
            UN::TxErrAbortCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TxErrBusyChannelCount::Id: { // type is int32u
            MN::TxErrBusyChannelCount::TypeInfo::Type value;
            UN::TxErrBusyChannelCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxTotalCount::Id: { // type is int32u
            MN::RxTotalCount::TypeInfo::Type value;
            UN::RxTotalCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxUnicastCount::Id: { // type is int32u
            MN::RxUnicastCount::TypeInfo::Type value;
            UN::RxUnicastCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxBroadcastCount::Id: { // type is int32u
            MN::RxBroadcastCount::TypeInfo::Type value;
            UN::RxBroadcastCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxDataCount::Id: { // type is int32u
            MN::RxDataCount::TypeInfo::Type value;
            UN::RxDataCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxDataPollCount::Id: { // type is int32u
            MN::RxDataPollCount::TypeInfo::Type value;
            UN::RxDataPollCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxBeaconCount::Id: { // type is int32u
            MN::RxBeaconCount::TypeInfo::Type value;
            UN::RxBeaconCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxBeaconRequestCount::Id: { // type is int32u
            MN::RxBeaconRequestCount::TypeInfo::Type value;
            UN::RxBeaconRequestCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxOtherCount::Id: { // type is int32u
            MN::RxOtherCount::TypeInfo::Type value;
            UN::RxOtherCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxAddressFilteredCount::Id: { // type is int32u
            MN::RxAddressFilteredCount::TypeInfo::Type value;
            UN::RxAddressFilteredCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxDestAddrFilteredCount::Id: { // type is int32u
            MN::RxDestAddrFilteredCount::TypeInfo::Type value;
            UN::RxDestAddrFilteredCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxDuplicatedCount::Id: { // type is int32u
            MN::RxDuplicatedCount::TypeInfo::Type value;
            UN::RxDuplicatedCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxErrNoFrameCount::Id: { // type is int32u
            MN::RxErrNoFrameCount::TypeInfo::Type value;
            UN::RxErrNoFrameCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxErrUnknownNeighborCount::Id: { // type is int32u
            MN::RxErrUnknownNeighborCount::TypeInfo::Type value;
            UN::RxErrUnknownNeighborCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxErrInvalidSrcAddrCount::Id: { // type is int32u
            MN::RxErrInvalidSrcAddrCount::TypeInfo::Type value;
            UN::RxErrInvalidSrcAddrCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxErrSecCount::Id: { // type is int32u
            MN::RxErrSecCount::TypeInfo::Type value;
            UN::RxErrSecCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxErrFcsCount::Id: { // type is int32u
            MN::RxErrFcsCount::TypeInfo::Type value;
            UN::RxErrFcsCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RxErrOtherCount::Id: { // type is int32u
            MN::RxErrOtherCount::TypeInfo::Type value;
            UN::RxErrOtherCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActiveTimestamp::Id: { // type is int64u
            MN::ActiveTimestamp::TypeInfo::Type value;
            UN::ActiveTimestamp::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PendingTimestamp::Id: { // type is int64u
            MN::PendingTimestamp::TypeInfo::Type value;
            UN::PendingTimestamp::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Delay::Id: { // type is int32u
            MN::Delay::TypeInfo::Type value;
            UN::Delay::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ChannelPage0Mask::Id: { // type is octet_string
            MN::ChannelPage0Mask::TypeInfo::Type value;
            UN::ChannelPage0Mask::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_THREAD_NETWORK_DIAGNOSTICS_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadNetworkDiagnosticsAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ThreadNetworkDiagnostics;

    if (aPath.mClusterId != Clusters::ThreadNetworkDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ThreadNetworkDiagnostics/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ThreadNetworkDiagnosticsAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                               const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::ThreadNetworkDiagnostics::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ThreadNetworkDiagnostics::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::Channel::Id: {
        using T                = MN::Channel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Channel attribute value is %s", unify_value.dump().c_str());
            UN::Channel::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id, MN::Channel::Id);
        }
        break;
    }
        // type is RoutingRole
    case MN::RoutingRole::Id: {
        using T                = MN::RoutingRole::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RoutingRole attribute value is %s", unify_value.dump().c_str());
            UN::RoutingRole::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RoutingRole::Id);
        }
        break;
    }
        // type is char_string
    case MN::NetworkName::Id: {
        using T                = MN::NetworkName::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NetworkName attribute value is %s", unify_value.dump().c_str());
            UN::NetworkName::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::NetworkName::Id);
        }
        break;
    }
        // type is int16u
    case MN::PanId::Id: {
        using T                = MN::PanId::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PanId attribute value is %s", unify_value.dump().c_str());
            UN::PanId::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id, MN::PanId::Id);
        }
        break;
    }
        // type is int64u
    case MN::ExtendedPanId::Id: {
        using T                = MN::ExtendedPanId::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ExtendedPanId attribute value is %s", unify_value.dump().c_str());
            UN::ExtendedPanId::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::ExtendedPanId::Id);
        }
        break;
    }
        // type is octet_string
    case MN::MeshLocalPrefix::Id: {
        using T                = MN::MeshLocalPrefix::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeshLocalPrefix attribute value is %s", unify_value.dump().c_str());
            UN::MeshLocalPrefix::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::MeshLocalPrefix::Id);
        }
        break;
    }
        // type is int64u
    case MN::OverrunCount::Id: {
        using T                = MN::OverrunCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OverrunCount attribute value is %s", unify_value.dump().c_str());
            UN::OverrunCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::OverrunCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::PartitionId::Id: {
        using T                = MN::PartitionId::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PartitionId attribute value is %s", unify_value.dump().c_str());
            UN::PartitionId::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::PartitionId::Id);
        }
        break;
    }
        // type is int8u
    case MN::Weighting::Id: {
        using T                = MN::Weighting::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Weighting attribute value is %s", unify_value.dump().c_str());
            UN::Weighting::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id, MN::Weighting::Id);
        }
        break;
    }
        // type is int8u
    case MN::DataVersion::Id: {
        using T                = MN::DataVersion::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DataVersion attribute value is %s", unify_value.dump().c_str());
            UN::DataVersion::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::DataVersion::Id);
        }
        break;
    }
        // type is int8u
    case MN::StableDataVersion::Id: {
        using T                = MN::StableDataVersion::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StableDataVersion attribute value is %s", unify_value.dump().c_str());
            UN::StableDataVersion::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::StableDataVersion::Id);
        }
        break;
    }
        // type is int8u
    case MN::LeaderRouterId::Id: {
        using T                = MN::LeaderRouterId::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LeaderRouterId attribute value is %s", unify_value.dump().c_str());
            UN::LeaderRouterId::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::LeaderRouterId::Id);
        }
        break;
    }
        // type is int16u
    case MN::DetachedRoleCount::Id: {
        using T                = MN::DetachedRoleCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DetachedRoleCount attribute value is %s", unify_value.dump().c_str());
            UN::DetachedRoleCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::DetachedRoleCount::Id);
        }
        break;
    }
        // type is int16u
    case MN::ChildRoleCount::Id: {
        using T                = MN::ChildRoleCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ChildRoleCount attribute value is %s", unify_value.dump().c_str());
            UN::ChildRoleCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::ChildRoleCount::Id);
        }
        break;
    }
        // type is int16u
    case MN::RouterRoleCount::Id: {
        using T                = MN::RouterRoleCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RouterRoleCount attribute value is %s", unify_value.dump().c_str());
            UN::RouterRoleCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RouterRoleCount::Id);
        }
        break;
    }
        // type is int16u
    case MN::LeaderRoleCount::Id: {
        using T                = MN::LeaderRoleCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LeaderRoleCount attribute value is %s", unify_value.dump().c_str());
            UN::LeaderRoleCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::LeaderRoleCount::Id);
        }
        break;
    }
        // type is int16u
    case MN::AttachAttemptCount::Id: {
        using T                = MN::AttachAttemptCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AttachAttemptCount attribute value is %s", unify_value.dump().c_str());
            UN::AttachAttemptCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::AttachAttemptCount::Id);
        }
        break;
    }
        // type is int16u
    case MN::PartitionIdChangeCount::Id: {
        using T                = MN::PartitionIdChangeCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PartitionIdChangeCount attribute value is %s", unify_value.dump().c_str());
            UN::PartitionIdChangeCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::PartitionIdChangeCount::Id);
        }
        break;
    }
        // type is int16u
    case MN::BetterPartitionAttachAttemptCount::Id: {
        using T                = MN::BetterPartitionAttachAttemptCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BetterPartitionAttachAttemptCount attribute value is %s", unify_value.dump().c_str());
            UN::BetterPartitionAttachAttemptCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::BetterPartitionAttachAttemptCount::Id);
        }
        break;
    }
        // type is int16u
    case MN::ParentChangeCount::Id: {
        using T                = MN::ParentChangeCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ParentChangeCount attribute value is %s", unify_value.dump().c_str());
            UN::ParentChangeCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::ParentChangeCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxTotalCount::Id: {
        using T                = MN::TxTotalCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxTotalCount attribute value is %s", unify_value.dump().c_str());
            UN::TxTotalCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxTotalCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxUnicastCount::Id: {
        using T                = MN::TxUnicastCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxUnicastCount attribute value is %s", unify_value.dump().c_str());
            UN::TxUnicastCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxUnicastCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxBroadcastCount::Id: {
        using T                = MN::TxBroadcastCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxBroadcastCount attribute value is %s", unify_value.dump().c_str());
            UN::TxBroadcastCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxBroadcastCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxAckRequestedCount::Id: {
        using T                = MN::TxAckRequestedCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxAckRequestedCount attribute value is %s", unify_value.dump().c_str());
            UN::TxAckRequestedCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxAckRequestedCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxAckedCount::Id: {
        using T                = MN::TxAckedCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxAckedCount attribute value is %s", unify_value.dump().c_str());
            UN::TxAckedCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxAckedCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxNoAckRequestedCount::Id: {
        using T                = MN::TxNoAckRequestedCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxNoAckRequestedCount attribute value is %s", unify_value.dump().c_str());
            UN::TxNoAckRequestedCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxNoAckRequestedCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxDataCount::Id: {
        using T                = MN::TxDataCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxDataCount attribute value is %s", unify_value.dump().c_str());
            UN::TxDataCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxDataCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxDataPollCount::Id: {
        using T                = MN::TxDataPollCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxDataPollCount attribute value is %s", unify_value.dump().c_str());
            UN::TxDataPollCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxDataPollCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxBeaconCount::Id: {
        using T                = MN::TxBeaconCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxBeaconCount attribute value is %s", unify_value.dump().c_str());
            UN::TxBeaconCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxBeaconCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxBeaconRequestCount::Id: {
        using T                = MN::TxBeaconRequestCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxBeaconRequestCount attribute value is %s", unify_value.dump().c_str());
            UN::TxBeaconRequestCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxBeaconRequestCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxOtherCount::Id: {
        using T                = MN::TxOtherCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxOtherCount attribute value is %s", unify_value.dump().c_str());
            UN::TxOtherCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxOtherCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxRetryCount::Id: {
        using T                = MN::TxRetryCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxRetryCount attribute value is %s", unify_value.dump().c_str());
            UN::TxRetryCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxRetryCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxDirectMaxRetryExpiryCount::Id: {
        using T                = MN::TxDirectMaxRetryExpiryCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxDirectMaxRetryExpiryCount attribute value is %s", unify_value.dump().c_str());
            UN::TxDirectMaxRetryExpiryCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxDirectMaxRetryExpiryCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxIndirectMaxRetryExpiryCount::Id: {
        using T                = MN::TxIndirectMaxRetryExpiryCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxIndirectMaxRetryExpiryCount attribute value is %s", unify_value.dump().c_str());
            UN::TxIndirectMaxRetryExpiryCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxIndirectMaxRetryExpiryCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxErrCcaCount::Id: {
        using T                = MN::TxErrCcaCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxErrCcaCount attribute value is %s", unify_value.dump().c_str());
            UN::TxErrCcaCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxErrCcaCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxErrAbortCount::Id: {
        using T                = MN::TxErrAbortCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxErrAbortCount attribute value is %s", unify_value.dump().c_str());
            UN::TxErrAbortCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxErrAbortCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::TxErrBusyChannelCount::Id: {
        using T                = MN::TxErrBusyChannelCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TxErrBusyChannelCount attribute value is %s", unify_value.dump().c_str());
            UN::TxErrBusyChannelCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::TxErrBusyChannelCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxTotalCount::Id: {
        using T                = MN::RxTotalCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxTotalCount attribute value is %s", unify_value.dump().c_str());
            UN::RxTotalCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxTotalCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxUnicastCount::Id: {
        using T                = MN::RxUnicastCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxUnicastCount attribute value is %s", unify_value.dump().c_str());
            UN::RxUnicastCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxUnicastCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxBroadcastCount::Id: {
        using T                = MN::RxBroadcastCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxBroadcastCount attribute value is %s", unify_value.dump().c_str());
            UN::RxBroadcastCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxBroadcastCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxDataCount::Id: {
        using T                = MN::RxDataCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxDataCount attribute value is %s", unify_value.dump().c_str());
            UN::RxDataCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxDataCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxDataPollCount::Id: {
        using T                = MN::RxDataPollCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxDataPollCount attribute value is %s", unify_value.dump().c_str());
            UN::RxDataPollCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxDataPollCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxBeaconCount::Id: {
        using T                = MN::RxBeaconCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxBeaconCount attribute value is %s", unify_value.dump().c_str());
            UN::RxBeaconCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxBeaconCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxBeaconRequestCount::Id: {
        using T                = MN::RxBeaconRequestCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxBeaconRequestCount attribute value is %s", unify_value.dump().c_str());
            UN::RxBeaconRequestCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxBeaconRequestCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxOtherCount::Id: {
        using T                = MN::RxOtherCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxOtherCount attribute value is %s", unify_value.dump().c_str());
            UN::RxOtherCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxOtherCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxAddressFilteredCount::Id: {
        using T                = MN::RxAddressFilteredCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxAddressFilteredCount attribute value is %s", unify_value.dump().c_str());
            UN::RxAddressFilteredCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxAddressFilteredCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxDestAddrFilteredCount::Id: {
        using T                = MN::RxDestAddrFilteredCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxDestAddrFilteredCount attribute value is %s", unify_value.dump().c_str());
            UN::RxDestAddrFilteredCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxDestAddrFilteredCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxDuplicatedCount::Id: {
        using T                = MN::RxDuplicatedCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxDuplicatedCount attribute value is %s", unify_value.dump().c_str());
            UN::RxDuplicatedCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxDuplicatedCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxErrNoFrameCount::Id: {
        using T                = MN::RxErrNoFrameCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxErrNoFrameCount attribute value is %s", unify_value.dump().c_str());
            UN::RxErrNoFrameCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxErrNoFrameCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxErrUnknownNeighborCount::Id: {
        using T                = MN::RxErrUnknownNeighborCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxErrUnknownNeighborCount attribute value is %s", unify_value.dump().c_str());
            UN::RxErrUnknownNeighborCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxErrUnknownNeighborCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxErrInvalidSrcAddrCount::Id: {
        using T                = MN::RxErrInvalidSrcAddrCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxErrInvalidSrcAddrCount attribute value is %s", unify_value.dump().c_str());
            UN::RxErrInvalidSrcAddrCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxErrInvalidSrcAddrCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxErrSecCount::Id: {
        using T                = MN::RxErrSecCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxErrSecCount attribute value is %s", unify_value.dump().c_str());
            UN::RxErrSecCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxErrSecCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxErrFcsCount::Id: {
        using T                = MN::RxErrFcsCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxErrFcsCount attribute value is %s", unify_value.dump().c_str());
            UN::RxErrFcsCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxErrFcsCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::RxErrOtherCount::Id: {
        using T                = MN::RxErrOtherCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RxErrOtherCount attribute value is %s", unify_value.dump().c_str());
            UN::RxErrOtherCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::RxErrOtherCount::Id);
        }
        break;
    }
        // type is int64u
    case MN::ActiveTimestamp::Id: {
        using T                = MN::ActiveTimestamp::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActiveTimestamp attribute value is %s", unify_value.dump().c_str());
            UN::ActiveTimestamp::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::ActiveTimestamp::Id);
        }
        break;
    }
        // type is int64u
    case MN::PendingTimestamp::Id: {
        using T                = MN::PendingTimestamp::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PendingTimestamp attribute value is %s", unify_value.dump().c_str());
            UN::PendingTimestamp::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::PendingTimestamp::Id);
        }
        break;
    }
        // type is int32u
    case MN::Delay::Id: {
        using T                = MN::Delay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Delay attribute value is %s", unify_value.dump().c_str());
            UN::Delay::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id, MN::Delay::Id);
        }
        break;
    }
        // type is octet_string
    case MN::ChannelPage0Mask::Id: {
        using T                = MN::ChannelPage0Mask::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ChannelPage0Mask attribute value is %s", unify_value.dump().c_str());
            UN::ChannelPage0Mask::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::ChannelPage0Mask::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThreadNetworkDiagnostics::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
WiFiNetworkDiagnosticsAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::WiFiNetworkDiagnostics::Attributes;
    if (aPath.mClusterId != Clusters::WiFiNetworkDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::Bssid::Id: { // type is octet_string
            MN::Bssid::TypeInfo::Type value;
            UN::Bssid::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SecurityType::Id: { // type is SecurityType
            MN::SecurityType::TypeInfo::Type value;
            UN::SecurityType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WiFiVersion::Id: { // type is WiFiVersionType
            MN::WiFiVersion::TypeInfo::Type value;
            UN::WiFiVersion::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ChannelNumber::Id: { // type is int16u
            MN::ChannelNumber::TypeInfo::Type value;
            UN::ChannelNumber::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Rssi::Id: { // type is int8s
            MN::Rssi::TypeInfo::Type value;
            UN::Rssi::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BeaconLostCount::Id: { // type is int32u
            MN::BeaconLostCount::TypeInfo::Type value;
            UN::BeaconLostCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BeaconRxCount::Id: { // type is int32u
            MN::BeaconRxCount::TypeInfo::Type value;
            UN::BeaconRxCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PacketMulticastRxCount::Id: { // type is int32u
            MN::PacketMulticastRxCount::TypeInfo::Type value;
            UN::PacketMulticastRxCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PacketMulticastTxCount::Id: { // type is int32u
            MN::PacketMulticastTxCount::TypeInfo::Type value;
            UN::PacketMulticastTxCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PacketUnicastRxCount::Id: { // type is int32u
            MN::PacketUnicastRxCount::TypeInfo::Type value;
            UN::PacketUnicastRxCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PacketUnicastTxCount::Id: { // type is int32u
            MN::PacketUnicastTxCount::TypeInfo::Type value;
            UN::PacketUnicastTxCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentMaxRate::Id: { // type is int64u
            MN::CurrentMaxRate::TypeInfo::Type value;
            UN::CurrentMaxRate::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OverrunCount::Id: { // type is int64u
            MN::OverrunCount::TypeInfo::Type value;
            UN::OverrunCount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_WI_FI_NETWORK_DIAGNOSTICS_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiNetworkDiagnosticsAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::WiFiNetworkDiagnostics;

    if (aPath.mClusterId != Clusters::WiFiNetworkDiagnostics::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/WiFiNetworkDiagnostics/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void WiFiNetworkDiagnosticsAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                             const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
    namespace UN = unify::matter_bridge::WiFiNetworkDiagnostics::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::WiFiNetworkDiagnostics::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is octet_string
    case MN::Bssid::Id: {
        using T                = MN::Bssid::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Bssid attribute value is %s", unify_value.dump().c_str());
            UN::Bssid::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id, MN::Bssid::Id);
        }
        break;
    }
        // type is SecurityType
    case MN::SecurityType::Id: {
        using T                = MN::SecurityType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SecurityType attribute value is %s", unify_value.dump().c_str());
            UN::SecurityType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::SecurityType::Id);
        }
        break;
    }
        // type is WiFiVersionType
    case MN::WiFiVersion::Id: {
        using T                = MN::WiFiVersion::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WiFiVersion attribute value is %s", unify_value.dump().c_str());
            UN::WiFiVersion::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id, MN::WiFiVersion::Id);
        }
        break;
    }
        // type is int16u
    case MN::ChannelNumber::Id: {
        using T                = MN::ChannelNumber::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ChannelNumber attribute value is %s", unify_value.dump().c_str());
            UN::ChannelNumber::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::ChannelNumber::Id);
        }
        break;
    }
        // type is int8s
    case MN::Rssi::Id: {
        using T                = MN::Rssi::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Rssi attribute value is %s", unify_value.dump().c_str());
            UN::Rssi::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id, MN::Rssi::Id);
        }
        break;
    }
        // type is int32u
    case MN::BeaconLostCount::Id: {
        using T                = MN::BeaconLostCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BeaconLostCount attribute value is %s", unify_value.dump().c_str());
            UN::BeaconLostCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::BeaconLostCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::BeaconRxCount::Id: {
        using T                = MN::BeaconRxCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BeaconRxCount attribute value is %s", unify_value.dump().c_str());
            UN::BeaconRxCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::BeaconRxCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::PacketMulticastRxCount::Id: {
        using T                = MN::PacketMulticastRxCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PacketMulticastRxCount attribute value is %s", unify_value.dump().c_str());
            UN::PacketMulticastRxCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::PacketMulticastRxCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::PacketMulticastTxCount::Id: {
        using T                = MN::PacketMulticastTxCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PacketMulticastTxCount attribute value is %s", unify_value.dump().c_str());
            UN::PacketMulticastTxCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::PacketMulticastTxCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::PacketUnicastRxCount::Id: {
        using T                = MN::PacketUnicastRxCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PacketUnicastRxCount attribute value is %s", unify_value.dump().c_str());
            UN::PacketUnicastRxCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::PacketUnicastRxCount::Id);
        }
        break;
    }
        // type is int32u
    case MN::PacketUnicastTxCount::Id: {
        using T                = MN::PacketUnicastTxCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PacketUnicastTxCount attribute value is %s", unify_value.dump().c_str());
            UN::PacketUnicastTxCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::PacketUnicastTxCount::Id);
        }
        break;
    }
        // type is int64u
    case MN::CurrentMaxRate::Id: {
        using T                = MN::CurrentMaxRate::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentMaxRate attribute value is %s", unify_value.dump().c_str());
            UN::CurrentMaxRate::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::CurrentMaxRate::Id);
        }
        break;
    }
        // type is int64u
    case MN::OverrunCount::Id: {
        using T                = MN::OverrunCount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OverrunCount attribute value is %s", unify_value.dump().c_str());
            UN::OverrunCount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::OverrunCount::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WiFiNetworkDiagnostics::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
TimeSynchronizationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::TimeSynchronization::Attributes;
    namespace UN = unify::matter_bridge::TimeSynchronization::Attributes;
    if (aPath.mClusterId != Clusters::TimeSynchronization::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::UTCTime::Id: { // type is epoch_us
            MN::UTCTime::TypeInfo::Type value;
            UN::UTCTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Granularity::Id: { // type is GranularityEnum
            MN::Granularity::TypeInfo::Type value;
            UN::Granularity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TimeSource::Id: { // type is TimeSourceEnum
            MN::TimeSource::TypeInfo::Type value;
            UN::TimeSource::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TrustedTimeNodeId::Id: { // type is node_id
            MN::TrustedTimeNodeId::TypeInfo::Type value;
            UN::TrustedTimeNodeId::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DefaultNtp::Id: { // type is char_string
            MN::DefaultNtp::TypeInfo::Type value;
            UN::DefaultNtp::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LocalTime::Id: { // type is epoch_us
            MN::LocalTime::TypeInfo::Type value;
            UN::LocalTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TimeZoneDatabase::Id: { // type is boolean
            MN::TimeZoneDatabase::TypeInfo::Type value;
            UN::TimeZoneDatabase::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NtpServerPort::Id: { // type is int16u
            MN::NtpServerPort::TypeInfo::Type value;
            UN::NtpServerPort::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_TIME_SYNCHRONIZATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TimeSynchronizationAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::TimeSynchronization;

    if (aPath.mClusterId != Clusters::TimeSynchronization::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::TrustedTimeNodeId::Id: {

        Attributes::TrustedTimeNodeId::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("TrustedTimeNodeId");
        break;
    }
    case Attributes::DefaultNtp::Id: {

        Attributes::DefaultNtp::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("DefaultNtp");
        break;
    }
    case Attributes::TimeZone::Id: {

        Attributes::TimeZone::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("TimeZone");
        break;
    }
    case Attributes::DstOffset::Id: {

        Attributes::DstOffset::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("DstOffset");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/TimeSynchronization/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void TimeSynchronizationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                          const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::TimeSynchronization::Attributes;
    namespace UN = unify::matter_bridge::TimeSynchronization::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::TimeSynchronization::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::TimeSynchronization::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is epoch_us
    case MN::UTCTime::Id: {
        using T                = MN::UTCTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UTCTime attribute value is %s", unify_value.dump().c_str());
            UN::UTCTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id, MN::UTCTime::Id);
        }
        break;
    }
        // type is GranularityEnum
    case MN::Granularity::Id: {
        using T                = MN::Granularity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Granularity attribute value is %s", unify_value.dump().c_str());
            UN::Granularity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id, MN::Granularity::Id);
        }
        break;
    }
        // type is TimeSourceEnum
    case MN::TimeSource::Id: {
        using T                = MN::TimeSource::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TimeSource attribute value is %s", unify_value.dump().c_str());
            UN::TimeSource::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id, MN::TimeSource::Id);
        }
        break;
    }
        // type is node_id
    case MN::TrustedTimeNodeId::Id: {
        using T                = MN::TrustedTimeNodeId::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TrustedTimeNodeId attribute value is %s", unify_value.dump().c_str());
            UN::TrustedTimeNodeId::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id,
                                                   MN::TrustedTimeNodeId::Id);
        }
        break;
    }
        // type is char_string
    case MN::DefaultNtp::Id: {
        using T                = MN::DefaultNtp::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DefaultNtp attribute value is %s", unify_value.dump().c_str());
            UN::DefaultNtp::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id, MN::DefaultNtp::Id);
        }
        break;
    }
        // type is epoch_us
    case MN::LocalTime::Id: {
        using T                = MN::LocalTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LocalTime attribute value is %s", unify_value.dump().c_str());
            UN::LocalTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id, MN::LocalTime::Id);
        }
        break;
    }
        // type is boolean
    case MN::TimeZoneDatabase::Id: {
        using T                = MN::TimeZoneDatabase::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TimeZoneDatabase attribute value is %s", unify_value.dump().c_str());
            UN::TimeZoneDatabase::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id,
                                                   MN::TimeZoneDatabase::Id);
        }
        break;
    }
        // type is int16u
    case MN::NtpServerPort::Id: {
        using T                = MN::NtpServerPort::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NtpServerPort attribute value is %s", unify_value.dump().c_str());
            UN::NtpServerPort::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id, MN::NtpServerPort::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TimeSynchronization::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
SwitchAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::Switch::Attributes;
    namespace UN = unify::matter_bridge::Switch::Attributes;
    if (aPath.mClusterId != Clusters::Switch::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::NumberOfPositions::Id: { // type is int8u
            MN::NumberOfPositions::TypeInfo::Type value;
            UN::NumberOfPositions::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentPosition::Id: { // type is int8u
            MN::CurrentPosition::TypeInfo::Type value;
            UN::CurrentPosition::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MultiPressMax::Id: { // type is int8u
            MN::MultiPressMax::TypeInfo::Type value;
            UN::MultiPressMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_SWITCH_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SwitchAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::Switch;

    if (aPath.mClusterId != Clusters::Switch::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/Switch/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void SwitchAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                             const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::Switch::Attributes;
    namespace UN = unify::matter_bridge::Switch::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Switch::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath        = ConcreteAttributePath(node_matter_endpoint, Clusters::Switch::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::NumberOfPositions::Id: {
        using T                = MN::NumberOfPositions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfPositions attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfPositions::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Switch::Id, MN::NumberOfPositions::Id);
        }
        break;
    }
        // type is int8u
    case MN::CurrentPosition::Id: {
        using T                = MN::CurrentPosition::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentPosition attribute value is %s", unify_value.dump().c_str());
            UN::CurrentPosition::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Switch::Id, MN::CurrentPosition::Id);
        }
        break;
    }
        // type is int8u
    case MN::MultiPressMax::Id: {
        using T                = MN::MultiPressMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MultiPressMax attribute value is %s", unify_value.dump().c_str());
            UN::MultiPressMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Switch::Id, MN::MultiPressMax::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Switch::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Switch::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OperationalCredentialsAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::OperationalCredentials::Attributes;
    namespace UN = unify::matter_bridge::OperationalCredentials::Attributes;
    if (aPath.mClusterId != Clusters::OperationalCredentials::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::SupportedFabrics::Id: { // type is int8u
            MN::SupportedFabrics::TypeInfo::Type value;
            UN::SupportedFabrics::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CommissionedFabrics::Id: { // type is int8u
            MN::CommissionedFabrics::TypeInfo::Type value;
            UN::CommissionedFabrics::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentFabricIndex::Id: { // type is int8u
            MN::CurrentFabricIndex::TypeInfo::Type value;
            UN::CurrentFabricIndex::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_OPERATIONAL_CREDENTIALS_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalCredentialsAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::OperationalCredentials;

    if (aPath.mClusterId != Clusters::OperationalCredentials::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/OperationalCredentials/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void OperationalCredentialsAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                             const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::OperationalCredentials::Attributes;
    namespace UN = unify::matter_bridge::OperationalCredentials::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OperationalCredentials::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::OperationalCredentials::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::SupportedFabrics::Id: {
        using T                = MN::SupportedFabrics::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SupportedFabrics attribute value is %s", unify_value.dump().c_str());
            UN::SupportedFabrics::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OperationalCredentials::Id,
                                                   MN::SupportedFabrics::Id);
        }
        break;
    }
        // type is int8u
    case MN::CommissionedFabrics::Id: {
        using T                = MN::CommissionedFabrics::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CommissionedFabrics attribute value is %s", unify_value.dump().c_str());
            UN::CommissionedFabrics::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OperationalCredentials::Id,
                                                   MN::CommissionedFabrics::Id);
        }
        break;
    }
        // type is int8u
    case MN::CurrentFabricIndex::Id: {
        using T                = MN::CurrentFabricIndex::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentFabricIndex attribute value is %s", unify_value.dump().c_str());
            UN::CurrentFabricIndex::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OperationalCredentials::Id,
                                                   MN::CurrentFabricIndex::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OperationalCredentials::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OperationalCredentials::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
GroupKeyManagementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::GroupKeyManagement::Attributes;
    namespace UN = unify::matter_bridge::GroupKeyManagement::Attributes;
    if (aPath.mClusterId != Clusters::GroupKeyManagement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MaxGroupsPerFabric::Id: { // type is int16u
            MN::MaxGroupsPerFabric::TypeInfo::Type value;
            UN::MaxGroupsPerFabric::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxGroupKeysPerFabric::Id: { // type is int16u
            MN::MaxGroupKeysPerFabric::TypeInfo::Type value;
            UN::MaxGroupKeysPerFabric::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_GROUP_KEY_MANAGEMENT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupKeyManagementAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::GroupKeyManagement;

    if (aPath.mClusterId != Clusters::GroupKeyManagement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::GroupKeyMap::Id: {

        Attributes::GroupKeyMap::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("GroupKeyMap");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/GroupKeyManagement/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void GroupKeyManagementAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                         const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::GroupKeyManagement::Attributes;
    namespace UN = unify::matter_bridge::GroupKeyManagement::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::GroupKeyManagement::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::GroupKeyManagement::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::MaxGroupsPerFabric::Id: {
        using T                = MN::MaxGroupsPerFabric::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxGroupsPerFabric attribute value is %s", unify_value.dump().c_str());
            UN::MaxGroupsPerFabric::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GroupKeyManagement::Id,
                                                   MN::MaxGroupsPerFabric::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxGroupKeysPerFabric::Id: {
        using T                = MN::MaxGroupKeysPerFabric::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxGroupKeysPerFabric attribute value is %s", unify_value.dump().c_str());
            UN::MaxGroupKeysPerFabric::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GroupKeyManagement::Id,
                                                   MN::MaxGroupKeysPerFabric::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GroupKeyManagement::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::GroupKeyManagement::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
FixedLabelAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::FixedLabel::Attributes;
    namespace UN = unify::matter_bridge::FixedLabel::Attributes;
    if (aPath.mClusterId != Clusters::FixedLabel::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_FIXED_LABEL_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FixedLabelAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::FixedLabel;

    if (aPath.mClusterId != Clusters::FixedLabel::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/FixedLabel/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void FixedLabelAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                 const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::FixedLabel::Attributes;
    namespace UN = unify::matter_bridge::FixedLabel::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::FixedLabel::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::FixedLabel::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FixedLabel::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FixedLabel::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
UserLabelAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::UserLabel::Attributes;
    namespace UN = unify::matter_bridge::UserLabel::Attributes;
    if (aPath.mClusterId != Clusters::UserLabel::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_USER_LABEL_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR UserLabelAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::UserLabel;

    if (aPath.mClusterId != Clusters::UserLabel::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::LabelList::Id: {

        Attributes::LabelList::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("LabelList");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/UserLabel/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void UserLabelAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::UserLabel::Attributes;
    namespace UN = unify::matter_bridge::UserLabel::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::UserLabel::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::UserLabel::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::UserLabel::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::UserLabel::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ProxyConfigurationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ProxyConfiguration::Attributes;
    namespace UN = unify::matter_bridge::ProxyConfiguration::Attributes;
    if (aPath.mClusterId != Clusters::ProxyConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_PROXY_CONFIGURATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProxyConfigurationAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ProxyConfiguration;

    if (aPath.mClusterId != Clusters::ProxyConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ProxyConfiguration/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ProxyConfigurationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                         const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ProxyConfiguration::Attributes;
    namespace UN = unify::matter_bridge::ProxyConfiguration::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ProxyConfiguration::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ProxyConfiguration::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ProxyConfiguration::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ProxyConfiguration::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ProxyDiscoveryAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ProxyDiscovery::Attributes;
    namespace UN = unify::matter_bridge::ProxyDiscovery::Attributes;
    if (aPath.mClusterId != Clusters::ProxyDiscovery::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_PROXY_DISCOVERY_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProxyDiscoveryAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ProxyDiscovery;

    if (aPath.mClusterId != Clusters::ProxyDiscovery::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ProxyDiscovery/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ProxyDiscoveryAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                     const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ProxyDiscovery::Attributes;
    namespace UN = unify::matter_bridge::ProxyDiscovery::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ProxyDiscovery::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ProxyDiscovery::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ProxyDiscovery::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ProxyDiscovery::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ProxyValidAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ProxyValid::Attributes;
    namespace UN = unify::matter_bridge::ProxyValid::Attributes;
    if (aPath.mClusterId != Clusters::ProxyValid::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_PROXY_VALID_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProxyValidAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ProxyValid;

    if (aPath.mClusterId != Clusters::ProxyValid::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ProxyValid/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ProxyValidAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                 const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ProxyValid::Attributes;
    namespace UN = unify::matter_bridge::ProxyValid::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ProxyValid::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::ProxyValid::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ProxyValid::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ProxyValid::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
BooleanStateAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::BooleanState::Attributes;
    namespace UN = unify::matter_bridge::BooleanState::Attributes;
    if (aPath.mClusterId != Clusters::BooleanState::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::StateValue::Id: { // type is boolean
            MN::StateValue::TypeInfo::Type value;
            UN::StateValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_BOOLEAN_STATE_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BooleanStateAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::BooleanState;

    if (aPath.mClusterId != Clusters::BooleanState::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/BooleanState/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void BooleanStateAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                   const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::BooleanState::Attributes;
    namespace UN = unify::matter_bridge::BooleanState::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::BooleanState::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::BooleanState::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is boolean
    case MN::StateValue::Id: {
        using T                = MN::StateValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StateValue attribute value is %s", unify_value.dump().c_str());
            UN::StateValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BooleanState::Id, MN::StateValue::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BooleanState::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BooleanState::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ModeSelectAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ModeSelect::Attributes;
    namespace UN = unify::matter_bridge::ModeSelect::Attributes;
    if (aPath.mClusterId != Clusters::ModeSelect::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::Description::Id: { // type is char_string
            MN::Description::TypeInfo::Type value;
            UN::Description::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StandardNamespace::Id: { // type is enum16
            MN::StandardNamespace::TypeInfo::Type value;
            UN::StandardNamespace::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentMode::Id: { // type is int8u
            MN::CurrentMode::TypeInfo::Type value;
            UN::CurrentMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StartUpMode::Id: { // type is int8u
            MN::StartUpMode::TypeInfo::Type value;
            UN::StartUpMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OnMode::Id: { // type is int8u
            MN::OnMode::TypeInfo::Type value;
            UN::OnMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_MODE_SELECT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ModeSelectAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ModeSelect;

    if (aPath.mClusterId != Clusters::ModeSelect::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::StartUpMode::Id: {

        Attributes::StartUpMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("StartUpMode");
        break;
    }
    case Attributes::OnMode::Id: {

        Attributes::OnMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OnMode");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ModeSelect/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ModeSelectAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                 const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ModeSelect::Attributes;
    namespace UN = unify::matter_bridge::ModeSelect::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ModeSelect::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::ModeSelect::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is char_string
    case MN::Description::Id: {
        using T                = MN::Description::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Description attribute value is %s", unify_value.dump().c_str());
            UN::Description::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ModeSelect::Id, MN::Description::Id);
        }
        break;
    }
        // type is enum16
    case MN::StandardNamespace::Id: {
        using T                = MN::StandardNamespace::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StandardNamespace attribute value is %s", unify_value.dump().c_str());
            UN::StandardNamespace::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ModeSelect::Id, MN::StandardNamespace::Id);
        }
        break;
    }
        // type is int8u
    case MN::CurrentMode::Id: {
        using T                = MN::CurrentMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentMode attribute value is %s", unify_value.dump().c_str());
            UN::CurrentMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ModeSelect::Id, MN::CurrentMode::Id);
        }
        break;
    }
        // type is int8u
    case MN::StartUpMode::Id: {
        using T                = MN::StartUpMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StartUpMode attribute value is %s", unify_value.dump().c_str());
            UN::StartUpMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ModeSelect::Id, MN::StartUpMode::Id);
        }
        break;
    }
        // type is int8u
    case MN::OnMode::Id: {
        using T                = MN::OnMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OnMode attribute value is %s", unify_value.dump().c_str());
            UN::OnMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ModeSelect::Id, MN::OnMode::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ModeSelect::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ModeSelect::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
DoorLockAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::DoorLock::Attributes;
    namespace UN = unify::matter_bridge::DoorLock::Attributes;
    if (aPath.mClusterId != Clusters::DoorLock::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::LockState::Id: { // type is DlLockState
            MN::LockState::TypeInfo::Type value;
            UN::LockState::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LockType::Id: { // type is DlLockType
            MN::LockType::TypeInfo::Type value;
            UN::LockType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActuatorEnabled::Id: { // type is boolean
            MN::ActuatorEnabled::TypeInfo::Type value;
            UN::ActuatorEnabled::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DoorState::Id: { // type is DlDoorState
            MN::DoorState::TypeInfo::Type value;
            UN::DoorState::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DoorOpenEvents::Id: { // type is int32u
            MN::DoorOpenEvents::TypeInfo::Type value;
            UN::DoorOpenEvents::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DoorClosedEvents::Id: { // type is int32u
            MN::DoorClosedEvents::TypeInfo::Type value;
            UN::DoorClosedEvents::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OpenPeriod::Id: { // type is int16u
            MN::OpenPeriod::TypeInfo::Type value;
            UN::OpenPeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfTotalUsersSupported::Id: { // type is int16u
            MN::NumberOfTotalUsersSupported::TypeInfo::Type value;
            UN::NumberOfTotalUsersSupported::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfPINUsersSupported::Id: { // type is int16u
            MN::NumberOfPINUsersSupported::TypeInfo::Type value;
            UN::NumberOfPINUsersSupported::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfRFIDUsersSupported::Id: { // type is int16u
            MN::NumberOfRFIDUsersSupported::TypeInfo::Type value;
            UN::NumberOfRFIDUsersSupported::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfWeekDaySchedulesSupportedPerUser::Id: { // type is int8u
            MN::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo::Type value;
            UN::NumberOfWeekDaySchedulesSupportedPerUser::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfYearDaySchedulesSupportedPerUser::Id: { // type is int8u
            MN::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo::Type value;
            UN::NumberOfYearDaySchedulesSupportedPerUser::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfHolidaySchedulesSupported::Id: { // type is int8u
            MN::NumberOfHolidaySchedulesSupported::TypeInfo::Type value;
            UN::NumberOfHolidaySchedulesSupported::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxPINCodeLength::Id: { // type is int8u
            MN::MaxPINCodeLength::TypeInfo::Type value;
            UN::MaxPINCodeLength::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinPINCodeLength::Id: { // type is int8u
            MN::MinPINCodeLength::TypeInfo::Type value;
            UN::MinPINCodeLength::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxRFIDCodeLength::Id: { // type is int8u
            MN::MaxRFIDCodeLength::TypeInfo::Type value;
            UN::MaxRFIDCodeLength::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinRFIDCodeLength::Id: { // type is int8u
            MN::MinRFIDCodeLength::TypeInfo::Type value;
            UN::MinRFIDCodeLength::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CredentialRulesSupport::Id: { // type is DlCredentialRuleMask
            MN::CredentialRulesSupport::TypeInfo::Type value;
            UN::CredentialRulesSupport::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfCredentialsSupportedPerUser::Id: { // type is int8u
            MN::NumberOfCredentialsSupportedPerUser::TypeInfo::Type value;
            UN::NumberOfCredentialsSupportedPerUser::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Language::Id: { // type is char_string
            MN::Language::TypeInfo::Type value;
            UN::Language::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LEDSettings::Id: { // type is int8u
            MN::LEDSettings::TypeInfo::Type value;
            UN::LEDSettings::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AutoRelockTime::Id: { // type is int32u
            MN::AutoRelockTime::TypeInfo::Type value;
            UN::AutoRelockTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SoundVolume::Id: { // type is int8u
            MN::SoundVolume::TypeInfo::Type value;
            UN::SoundVolume::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OperatingMode::Id: { // type is DlOperatingMode
            MN::OperatingMode::TypeInfo::Type value;
            UN::OperatingMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SupportedOperatingModes::Id: { // type is DlSupportedOperatingModes
            MN::SupportedOperatingModes::TypeInfo::Type value;
            UN::SupportedOperatingModes::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DefaultConfigurationRegister::Id: { // type is DlDefaultConfigurationRegister
            MN::DefaultConfigurationRegister::TypeInfo::Type value;
            UN::DefaultConfigurationRegister::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EnableLocalProgramming::Id: { // type is boolean
            MN::EnableLocalProgramming::TypeInfo::Type value;
            UN::EnableLocalProgramming::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EnableOneTouchLocking::Id: { // type is boolean
            MN::EnableOneTouchLocking::TypeInfo::Type value;
            UN::EnableOneTouchLocking::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EnableInsideStatusLED::Id: { // type is boolean
            MN::EnableInsideStatusLED::TypeInfo::Type value;
            UN::EnableInsideStatusLED::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EnablePrivacyModeButton::Id: { // type is boolean
            MN::EnablePrivacyModeButton::TypeInfo::Type value;
            UN::EnablePrivacyModeButton::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LocalProgrammingFeatures::Id: { // type is DlLocalProgrammingFeatures
            MN::LocalProgrammingFeatures::TypeInfo::Type value;
            UN::LocalProgrammingFeatures::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WrongCodeEntryLimit::Id: { // type is int8u
            MN::WrongCodeEntryLimit::TypeInfo::Type value;
            UN::WrongCodeEntryLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UserCodeTemporaryDisableTime::Id: { // type is int8u
            MN::UserCodeTemporaryDisableTime::TypeInfo::Type value;
            UN::UserCodeTemporaryDisableTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SendPINOverTheAir::Id: { // type is boolean
            MN::SendPINOverTheAir::TypeInfo::Type value;
            UN::SendPINOverTheAir::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RequirePINforRemoteOperation::Id: { // type is boolean
            MN::RequirePINforRemoteOperation::TypeInfo::Type value;
            UN::RequirePINforRemoteOperation::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ExpiringUserTimeout::Id: { // type is int16u
            MN::ExpiringUserTimeout::TypeInfo::Type value;
            UN::ExpiringUserTimeout::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_DOOR_LOCK_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::DoorLock;

    if (aPath.mClusterId != Clusters::DoorLock::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::DoorOpenEvents::Id: {

        Attributes::DoorOpenEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("DoorOpenEvents");
        break;
    }
    case Attributes::DoorClosedEvents::Id: {

        Attributes::DoorClosedEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("DoorClosedEvents");
        break;
    }
    case Attributes::OpenPeriod::Id: {

        Attributes::OpenPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OpenPeriod");
        break;
    }
    case Attributes::Language::Id: {

        Attributes::Language::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Language");
        break;
    }
    case Attributes::LEDSettings::Id: {

        Attributes::LEDSettings::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("LEDSettings");
        break;
    }
    case Attributes::AutoRelockTime::Id: {

        Attributes::AutoRelockTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("AutoRelockTime");
        break;
    }
    case Attributes::SoundVolume::Id: {

        Attributes::SoundVolume::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("SoundVolume");
        break;
    }
    case Attributes::OperatingMode::Id: {

        Attributes::OperatingMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OperatingMode");
        break;
    }
    case Attributes::EnableLocalProgramming::Id: {

        Attributes::EnableLocalProgramming::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("EnableLocalProgramming");
        break;
    }
    case Attributes::EnableOneTouchLocking::Id: {

        Attributes::EnableOneTouchLocking::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("EnableOneTouchLocking");
        break;
    }
    case Attributes::EnableInsideStatusLED::Id: {

        Attributes::EnableInsideStatusLED::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("EnableInsideStatusLED");
        break;
    }
    case Attributes::EnablePrivacyModeButton::Id: {

        Attributes::EnablePrivacyModeButton::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("EnablePrivacyModeButton");
        break;
    }
    case Attributes::LocalProgrammingFeatures::Id: {

        Attributes::LocalProgrammingFeatures::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("LocalProgrammingFeatures");
        break;
    }
    case Attributes::WrongCodeEntryLimit::Id: {

        Attributes::WrongCodeEntryLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("WrongCodeEntryLimit");
        break;
    }
    case Attributes::UserCodeTemporaryDisableTime::Id: {

        Attributes::UserCodeTemporaryDisableTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("UserCodeTemporaryDisableTime");
        break;
    }
    case Attributes::SendPINOverTheAir::Id: {

        Attributes::SendPINOverTheAir::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("SendPINOverTheAir");
        break;
    }
    case Attributes::RequirePINforRemoteOperation::Id: {

        Attributes::RequirePINforRemoteOperation::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("RequirePINforRemoteOperation");
        break;
    }
    case Attributes::ExpiringUserTimeout::Id: {

        Attributes::ExpiringUserTimeout::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ExpiringUserTimeout");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/DoorLock/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void DoorLockAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                               const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::DoorLock::Attributes;
    namespace UN = unify::matter_bridge::DoorLock::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::DoorLock::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::DoorLock::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is DlLockState
    case MN::LockState::Id: {
        using T                = MN::LockState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LockState attribute value is %s", unify_value.dump().c_str());
            UN::LockState::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LockState::Id);
        }
        break;
    }
        // type is DlLockType
    case MN::LockType::Id: {
        using T                = MN::LockType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LockType attribute value is %s", unify_value.dump().c_str());
            UN::LockType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LockType::Id);
        }
        break;
    }
        // type is boolean
    case MN::ActuatorEnabled::Id: {
        using T                = MN::ActuatorEnabled::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActuatorEnabled attribute value is %s", unify_value.dump().c_str());
            UN::ActuatorEnabled::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::ActuatorEnabled::Id);
        }
        break;
    }
        // type is DlDoorState
    case MN::DoorState::Id: {
        using T                = MN::DoorState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DoorState attribute value is %s", unify_value.dump().c_str());
            UN::DoorState::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorState::Id);
        }
        break;
    }
        // type is int32u
    case MN::DoorOpenEvents::Id: {
        using T                = MN::DoorOpenEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DoorOpenEvents attribute value is %s", unify_value.dump().c_str());
            UN::DoorOpenEvents::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorOpenEvents::Id);
        }
        break;
    }
        // type is int32u
    case MN::DoorClosedEvents::Id: {
        using T                = MN::DoorClosedEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DoorClosedEvents attribute value is %s", unify_value.dump().c_str());
            UN::DoorClosedEvents::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorClosedEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::OpenPeriod::Id: {
        using T                = MN::OpenPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OpenPeriod attribute value is %s", unify_value.dump().c_str());
            UN::OpenPeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::OpenPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfTotalUsersSupported::Id: {
        using T                = MN::NumberOfTotalUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfTotalUsersSupported attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfTotalUsersSupported::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::NumberOfTotalUsersSupported::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfPINUsersSupported::Id: {
        using T                = MN::NumberOfPINUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfPINUsersSupported attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfPINUsersSupported::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::NumberOfPINUsersSupported::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfRFIDUsersSupported::Id: {
        using T                = MN::NumberOfRFIDUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfRFIDUsersSupported attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfRFIDUsersSupported::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::NumberOfRFIDUsersSupported::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfWeekDaySchedulesSupportedPerUser::Id: {
        using T                = MN::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfWeekDaySchedulesSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfWeekDaySchedulesSupportedPerUser::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::NumberOfWeekDaySchedulesSupportedPerUser::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfYearDaySchedulesSupportedPerUser::Id: {
        using T                = MN::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfYearDaySchedulesSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfYearDaySchedulesSupportedPerUser::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::NumberOfYearDaySchedulesSupportedPerUser::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfHolidaySchedulesSupported::Id: {
        using T                = MN::NumberOfHolidaySchedulesSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfHolidaySchedulesSupported attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfHolidaySchedulesSupported::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::NumberOfHolidaySchedulesSupported::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxPINCodeLength::Id: {
        using T                = MN::MaxPINCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxPINCodeLength attribute value is %s", unify_value.dump().c_str());
            UN::MaxPINCodeLength::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MaxPINCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinPINCodeLength::Id: {
        using T                = MN::MinPINCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinPINCodeLength attribute value is %s", unify_value.dump().c_str());
            UN::MinPINCodeLength::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MinPINCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxRFIDCodeLength::Id: {
        using T                = MN::MaxRFIDCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxRFIDCodeLength attribute value is %s", unify_value.dump().c_str());
            UN::MaxRFIDCodeLength::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MaxRFIDCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinRFIDCodeLength::Id: {
        using T                = MN::MinRFIDCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinRFIDCodeLength attribute value is %s", unify_value.dump().c_str());
            UN::MinRFIDCodeLength::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MinRFIDCodeLength::Id);
        }
        break;
    }
        // type is DlCredentialRuleMask
    case MN::CredentialRulesSupport::Id: {
        using T                = MN::CredentialRulesSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CredentialRulesSupport attribute value is %s", unify_value.dump().c_str());
            UN::CredentialRulesSupport::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::CredentialRulesSupport::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfCredentialsSupportedPerUser::Id: {
        using T                = MN::NumberOfCredentialsSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfCredentialsSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfCredentialsSupportedPerUser::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::NumberOfCredentialsSupportedPerUser::Id);
        }
        break;
    }
        // type is char_string
    case MN::Language::Id: {
        using T                = MN::Language::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Language attribute value is %s", unify_value.dump().c_str());
            UN::Language::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::Language::Id);
        }
        break;
    }
        // type is int8u
    case MN::LEDSettings::Id: {
        using T                = MN::LEDSettings::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LEDSettings attribute value is %s", unify_value.dump().c_str());
            UN::LEDSettings::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LEDSettings::Id);
        }
        break;
    }
        // type is int32u
    case MN::AutoRelockTime::Id: {
        using T                = MN::AutoRelockTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AutoRelockTime attribute value is %s", unify_value.dump().c_str());
            UN::AutoRelockTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::AutoRelockTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::SoundVolume::Id: {
        using T                = MN::SoundVolume::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SoundVolume attribute value is %s", unify_value.dump().c_str());
            UN::SoundVolume::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SoundVolume::Id);
        }
        break;
    }
        // type is DlOperatingMode
    case MN::OperatingMode::Id: {
        using T                = MN::OperatingMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OperatingMode attribute value is %s", unify_value.dump().c_str());
            UN::OperatingMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::OperatingMode::Id);
        }
        break;
    }
        // type is DlSupportedOperatingModes
    case MN::SupportedOperatingModes::Id: {
        using T                = MN::SupportedOperatingModes::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SupportedOperatingModes attribute value is %s", unify_value.dump().c_str());
            UN::SupportedOperatingModes::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SupportedOperatingModes::Id);
        }
        break;
    }
        // type is DlDefaultConfigurationRegister
    case MN::DefaultConfigurationRegister::Id: {
        using T                = MN::DefaultConfigurationRegister::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DefaultConfigurationRegister attribute value is %s", unify_value.dump().c_str());
            UN::DefaultConfigurationRegister::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::DefaultConfigurationRegister::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableLocalProgramming::Id: {
        using T                = MN::EnableLocalProgramming::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EnableLocalProgramming attribute value is %s", unify_value.dump().c_str());
            UN::EnableLocalProgramming::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableLocalProgramming::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableOneTouchLocking::Id: {
        using T                = MN::EnableOneTouchLocking::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EnableOneTouchLocking attribute value is %s", unify_value.dump().c_str());
            UN::EnableOneTouchLocking::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableOneTouchLocking::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableInsideStatusLED::Id: {
        using T                = MN::EnableInsideStatusLED::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EnableInsideStatusLED attribute value is %s", unify_value.dump().c_str());
            UN::EnableInsideStatusLED::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableInsideStatusLED::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnablePrivacyModeButton::Id: {
        using T                = MN::EnablePrivacyModeButton::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EnablePrivacyModeButton attribute value is %s", unify_value.dump().c_str());
            UN::EnablePrivacyModeButton::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnablePrivacyModeButton::Id);
        }
        break;
    }
        // type is DlLocalProgrammingFeatures
    case MN::LocalProgrammingFeatures::Id: {
        using T                = MN::LocalProgrammingFeatures::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LocalProgrammingFeatures attribute value is %s", unify_value.dump().c_str());
            UN::LocalProgrammingFeatures::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LocalProgrammingFeatures::Id);
        }
        break;
    }
        // type is int8u
    case MN::WrongCodeEntryLimit::Id: {
        using T                = MN::WrongCodeEntryLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WrongCodeEntryLimit attribute value is %s", unify_value.dump().c_str());
            UN::WrongCodeEntryLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::WrongCodeEntryLimit::Id);
        }
        break;
    }
        // type is int8u
    case MN::UserCodeTemporaryDisableTime::Id: {
        using T                = MN::UserCodeTemporaryDisableTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UserCodeTemporaryDisableTime attribute value is %s", unify_value.dump().c_str());
            UN::UserCodeTemporaryDisableTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::UserCodeTemporaryDisableTime::Id);
        }
        break;
    }
        // type is boolean
    case MN::SendPINOverTheAir::Id: {
        using T                = MN::SendPINOverTheAir::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SendPINOverTheAir attribute value is %s", unify_value.dump().c_str());
            UN::SendPINOverTheAir::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SendPINOverTheAir::Id);
        }
        break;
    }
        // type is boolean
    case MN::RequirePINforRemoteOperation::Id: {
        using T                = MN::RequirePINforRemoteOperation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RequirePINforRemoteOperation attribute value is %s", unify_value.dump().c_str());
            UN::RequirePINforRemoteOperation::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                                                   MN::RequirePINforRemoteOperation::Id);
        }
        break;
    }
        // type is int16u
    case MN::ExpiringUserTimeout::Id: {
        using T                = MN::ExpiringUserTimeout::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ExpiringUserTimeout attribute value is %s", unify_value.dump().c_str());
            UN::ExpiringUserTimeout::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::ExpiringUserTimeout::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
WindowCoveringAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::WindowCovering::Attributes;
    namespace UN = unify::matter_bridge::WindowCovering::Attributes;
    if (aPath.mClusterId != Clusters::WindowCovering::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::Type::Id: { // type is Type
            MN::Type::TypeInfo::Type value;
            UN::Type::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PhysicalClosedLimitLift::Id: { // type is int16u
            MN::PhysicalClosedLimitLift::TypeInfo::Type value;
            UN::PhysicalClosedLimitLift::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PhysicalClosedLimitTilt::Id: { // type is int16u
            MN::PhysicalClosedLimitTilt::TypeInfo::Type value;
            UN::PhysicalClosedLimitTilt::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentPositionLift::Id: { // type is int16u
            MN::CurrentPositionLift::TypeInfo::Type value;
            UN::CurrentPositionLift::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentPositionTilt::Id: { // type is int16u
            MN::CurrentPositionTilt::TypeInfo::Type value;
            UN::CurrentPositionTilt::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfActuationsLift::Id: { // type is int16u
            MN::NumberOfActuationsLift::TypeInfo::Type value;
            UN::NumberOfActuationsLift::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfActuationsTilt::Id: { // type is int16u
            MN::NumberOfActuationsTilt::TypeInfo::Type value;
            UN::NumberOfActuationsTilt::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ConfigStatus::Id: { // type is ConfigStatus
            MN::ConfigStatus::TypeInfo::Type value;
            UN::ConfigStatus::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentPositionLiftPercentage::Id: { // type is Percent
            MN::CurrentPositionLiftPercentage::TypeInfo::Type value;
            UN::CurrentPositionLiftPercentage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentPositionTiltPercentage::Id: { // type is Percent
            MN::CurrentPositionTiltPercentage::TypeInfo::Type value;
            UN::CurrentPositionTiltPercentage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OperationalStatus::Id: { // type is OperationalStatus
            MN::OperationalStatus::TypeInfo::Type value;
            UN::OperationalStatus::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TargetPositionLiftPercent100ths::Id: { // type is Percent100ths
            MN::TargetPositionLiftPercent100ths::TypeInfo::Type value;
            UN::TargetPositionLiftPercent100ths::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TargetPositionTiltPercent100ths::Id: { // type is Percent100ths
            MN::TargetPositionTiltPercent100ths::TypeInfo::Type value;
            UN::TargetPositionTiltPercent100ths::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EndProductType::Id: { // type is EndProductType
            MN::EndProductType::TypeInfo::Type value;
            UN::EndProductType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentPositionLiftPercent100ths::Id: { // type is Percent100ths
            MN::CurrentPositionLiftPercent100ths::TypeInfo::Type value;
            UN::CurrentPositionLiftPercent100ths::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentPositionTiltPercent100ths::Id: { // type is Percent100ths
            MN::CurrentPositionTiltPercent100ths::TypeInfo::Type value;
            UN::CurrentPositionTiltPercent100ths::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstalledOpenLimitLift::Id: { // type is int16u
            MN::InstalledOpenLimitLift::TypeInfo::Type value;
            UN::InstalledOpenLimitLift::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstalledClosedLimitLift::Id: { // type is int16u
            MN::InstalledClosedLimitLift::TypeInfo::Type value;
            UN::InstalledClosedLimitLift::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstalledOpenLimitTilt::Id: { // type is int16u
            MN::InstalledOpenLimitTilt::TypeInfo::Type value;
            UN::InstalledOpenLimitTilt::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstalledClosedLimitTilt::Id: { // type is int16u
            MN::InstalledClosedLimitTilt::TypeInfo::Type value;
            UN::InstalledClosedLimitTilt::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Mode::Id: { // type is Mode
            MN::Mode::TypeInfo::Type value;
            UN::Mode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SafetyStatus::Id: { // type is SafetyStatus
            MN::SafetyStatus::TypeInfo::Type value;
            UN::SafetyStatus::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_WINDOW_COVERING_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WindowCoveringAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::WindowCovering;

    if (aPath.mClusterId != Clusters::WindowCovering::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::Mode::Id: {

        Attributes::Mode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Mode");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/WindowCovering/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void WindowCoveringAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                     const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::WindowCovering::Attributes;
    namespace UN = unify::matter_bridge::WindowCovering::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::WindowCovering::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::WindowCovering::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is Type
    case MN::Type::Id: {
        using T                = MN::Type::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Type attribute value is %s", unify_value.dump().c_str());
            UN::Type::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::Type::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalClosedLimitLift::Id: {
        using T                = MN::PhysicalClosedLimitLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PhysicalClosedLimitLift attribute value is %s", unify_value.dump().c_str());
            UN::PhysicalClosedLimitLift::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::PhysicalClosedLimitLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalClosedLimitTilt::Id: {
        using T                = MN::PhysicalClosedLimitTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PhysicalClosedLimitTilt attribute value is %s", unify_value.dump().c_str());
            UN::PhysicalClosedLimitTilt::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::PhysicalClosedLimitTilt::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentPositionLift::Id: {
        using T                = MN::CurrentPositionLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentPositionLift attribute value is %s", unify_value.dump().c_str());
            UN::CurrentPositionLift::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::CurrentPositionLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentPositionTilt::Id: {
        using T                = MN::CurrentPositionTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentPositionTilt attribute value is %s", unify_value.dump().c_str());
            UN::CurrentPositionTilt::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::CurrentPositionTilt::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfActuationsLift::Id: {
        using T                = MN::NumberOfActuationsLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfActuationsLift attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfActuationsLift::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::NumberOfActuationsLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfActuationsTilt::Id: {
        using T                = MN::NumberOfActuationsTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfActuationsTilt attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfActuationsTilt::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::NumberOfActuationsTilt::Id);
        }
        break;
    }
        // type is ConfigStatus
    case MN::ConfigStatus::Id: {
        std::optional<uint8_t> value = from_json_ConfigStatus(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ConfigStatus attribute value is %s", unify_value.dump().c_str());
            UN::ConfigStatus::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::ConfigStatus::Id);
        }
        break;
    }
        // type is Percent
    case MN::CurrentPositionLiftPercentage::Id: {
        using T                = MN::CurrentPositionLiftPercentage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentPositionLiftPercentage attribute value is %s", unify_value.dump().c_str());
            UN::CurrentPositionLiftPercentage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::CurrentPositionLiftPercentage::Id);
        }
        break;
    }
        // type is Percent
    case MN::CurrentPositionTiltPercentage::Id: {
        using T                = MN::CurrentPositionTiltPercentage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentPositionTiltPercentage attribute value is %s", unify_value.dump().c_str());
            UN::CurrentPositionTiltPercentage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::CurrentPositionTiltPercentage::Id);
        }
        break;
    }
        // type is OperationalStatus
    case MN::OperationalStatus::Id: {
        std::optional<uint8_t> value = from_json_OperationalStatus(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OperationalStatus attribute value is %s", unify_value.dump().c_str());
            UN::OperationalStatus::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::OperationalStatus::Id);
        }
        break;
    }
        // type is Percent100ths
    case MN::TargetPositionLiftPercent100ths::Id: {
        using T                = MN::TargetPositionLiftPercent100ths::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TargetPositionLiftPercent100ths attribute value is %s", unify_value.dump().c_str());
            UN::TargetPositionLiftPercent100ths::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::TargetPositionLiftPercent100ths::Id);
        }
        break;
    }
        // type is Percent100ths
    case MN::TargetPositionTiltPercent100ths::Id: {
        using T                = MN::TargetPositionTiltPercent100ths::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TargetPositionTiltPercent100ths attribute value is %s", unify_value.dump().c_str());
            UN::TargetPositionTiltPercent100ths::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::TargetPositionTiltPercent100ths::Id);
        }
        break;
    }
        // type is EndProductType
    case MN::EndProductType::Id: {
        using T                = MN::EndProductType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EndProductType attribute value is %s", unify_value.dump().c_str());
            UN::EndProductType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::EndProductType::Id);
        }
        break;
    }
        // type is Percent100ths
    case MN::CurrentPositionLiftPercent100ths::Id: {
        using T                = MN::CurrentPositionLiftPercent100ths::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentPositionLiftPercent100ths attribute value is %s", unify_value.dump().c_str());
            UN::CurrentPositionLiftPercent100ths::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::CurrentPositionLiftPercent100ths::Id);
        }
        break;
    }
        // type is Percent100ths
    case MN::CurrentPositionTiltPercent100ths::Id: {
        using T                = MN::CurrentPositionTiltPercent100ths::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentPositionTiltPercent100ths attribute value is %s", unify_value.dump().c_str());
            UN::CurrentPositionTiltPercent100ths::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::CurrentPositionTiltPercent100ths::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstalledOpenLimitLift::Id: {
        using T                = MN::InstalledOpenLimitLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstalledOpenLimitLift attribute value is %s", unify_value.dump().c_str());
            UN::InstalledOpenLimitLift::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::InstalledOpenLimitLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstalledClosedLimitLift::Id: {
        using T                = MN::InstalledClosedLimitLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstalledClosedLimitLift attribute value is %s", unify_value.dump().c_str());
            UN::InstalledClosedLimitLift::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::InstalledClosedLimitLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstalledOpenLimitTilt::Id: {
        using T                = MN::InstalledOpenLimitTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstalledOpenLimitTilt attribute value is %s", unify_value.dump().c_str());
            UN::InstalledOpenLimitTilt::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::InstalledOpenLimitTilt::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstalledClosedLimitTilt::Id: {
        using T                = MN::InstalledClosedLimitTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstalledClosedLimitTilt attribute value is %s", unify_value.dump().c_str());
            UN::InstalledClosedLimitTilt::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                                                   MN::InstalledClosedLimitTilt::Id);
        }
        break;
    }
        // type is Mode
    case MN::Mode::Id: {
        std::optional<uint8_t> value = from_json_Mode(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Mode attribute value is %s", unify_value.dump().c_str());
            UN::Mode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::Mode::Id);
        }
        break;
    }
        // type is SafetyStatus
    case MN::SafetyStatus::Id: {
        std::optional<uint16_t> value = from_json_SafetyStatus(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SafetyStatus attribute value is %s", unify_value.dump().c_str());
            UN::SafetyStatus::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::SafetyStatus::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
BarrierControlAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::BarrierControl::Attributes;
    namespace UN = unify::matter_bridge::BarrierControl::Attributes;
    if (aPath.mClusterId != Clusters::BarrierControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::BarrierMovingState::Id: { // type is enum8
            MN::BarrierMovingState::TypeInfo::Type value;
            UN::BarrierMovingState::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierSafetyStatus::Id: { // type is bitmap16
            MN::BarrierSafetyStatus::TypeInfo::Type value;
            UN::BarrierSafetyStatus::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierCapabilities::Id: { // type is bitmap8
            MN::BarrierCapabilities::TypeInfo::Type value;
            UN::BarrierCapabilities::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierOpenEvents::Id: { // type is int16u
            MN::BarrierOpenEvents::TypeInfo::Type value;
            UN::BarrierOpenEvents::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierCloseEvents::Id: { // type is int16u
            MN::BarrierCloseEvents::TypeInfo::Type value;
            UN::BarrierCloseEvents::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierCommandOpenEvents::Id: { // type is int16u
            MN::BarrierCommandOpenEvents::TypeInfo::Type value;
            UN::BarrierCommandOpenEvents::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierCommandCloseEvents::Id: { // type is int16u
            MN::BarrierCommandCloseEvents::TypeInfo::Type value;
            UN::BarrierCommandCloseEvents::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierOpenPeriod::Id: { // type is int16u
            MN::BarrierOpenPeriod::TypeInfo::Type value;
            UN::BarrierOpenPeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierClosePeriod::Id: { // type is int16u
            MN::BarrierClosePeriod::TypeInfo::Type value;
            UN::BarrierClosePeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::BarrierPosition::Id: { // type is int8u
            MN::BarrierPosition::TypeInfo::Type value;
            UN::BarrierPosition::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_BARRIER_CONTROL_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BarrierControlAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::BarrierControl;

    if (aPath.mClusterId != Clusters::BarrierControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::BarrierOpenEvents::Id: {

        Attributes::BarrierOpenEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("BarrierOpenEvents");
        break;
    }
    case Attributes::BarrierCloseEvents::Id: {

        Attributes::BarrierCloseEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("BarrierCloseEvents");
        break;
    }
    case Attributes::BarrierCommandOpenEvents::Id: {

        Attributes::BarrierCommandOpenEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("BarrierCommandOpenEvents");
        break;
    }
    case Attributes::BarrierCommandCloseEvents::Id: {

        Attributes::BarrierCommandCloseEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("BarrierCommandCloseEvents");
        break;
    }
    case Attributes::BarrierOpenPeriod::Id: {

        Attributes::BarrierOpenPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("BarrierOpenPeriod");
        break;
    }
    case Attributes::BarrierClosePeriod::Id: {

        Attributes::BarrierClosePeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("BarrierClosePeriod");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/BarrierControl/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void BarrierControlAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                     const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::BarrierControl::Attributes;
    namespace UN = unify::matter_bridge::BarrierControl::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::BarrierControl::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::BarrierControl::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is enum8
    case MN::BarrierMovingState::Id: {
        using T                = MN::BarrierMovingState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierMovingState attribute value is %s", unify_value.dump().c_str());
            UN::BarrierMovingState::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierMovingState::Id);
        }
        break;
    }
        // type is bitmap16
    case MN::BarrierSafetyStatus::Id: {
        using T                = MN::BarrierSafetyStatus::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierSafetyStatus attribute value is %s", unify_value.dump().c_str());
            UN::BarrierSafetyStatus::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierSafetyStatus::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::BarrierCapabilities::Id: {
        using T                = MN::BarrierCapabilities::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierCapabilities attribute value is %s", unify_value.dump().c_str());
            UN::BarrierCapabilities::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierCapabilities::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierOpenEvents::Id: {
        using T                = MN::BarrierOpenEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierOpenEvents attribute value is %s", unify_value.dump().c_str());
            UN::BarrierOpenEvents::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierOpenEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierCloseEvents::Id: {
        using T                = MN::BarrierCloseEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierCloseEvents attribute value is %s", unify_value.dump().c_str());
            UN::BarrierCloseEvents::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierCloseEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierCommandOpenEvents::Id: {
        using T                = MN::BarrierCommandOpenEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierCommandOpenEvents attribute value is %s", unify_value.dump().c_str());
            UN::BarrierCommandOpenEvents::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id,
                                                   MN::BarrierCommandOpenEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierCommandCloseEvents::Id: {
        using T                = MN::BarrierCommandCloseEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierCommandCloseEvents attribute value is %s", unify_value.dump().c_str());
            UN::BarrierCommandCloseEvents::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id,
                                                   MN::BarrierCommandCloseEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierOpenPeriod::Id: {
        using T                = MN::BarrierOpenPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierOpenPeriod attribute value is %s", unify_value.dump().c_str());
            UN::BarrierOpenPeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierOpenPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::BarrierClosePeriod::Id: {
        using T                = MN::BarrierClosePeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierClosePeriod attribute value is %s", unify_value.dump().c_str());
            UN::BarrierClosePeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierClosePeriod::Id);
        }
        break;
    }
        // type is int8u
    case MN::BarrierPosition::Id: {
        using T                = MN::BarrierPosition::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "BarrierPosition attribute value is %s", unify_value.dump().c_str());
            UN::BarrierPosition::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::BarrierPosition::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::BarrierControl::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ThermostatAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::Thermostat::Attributes;
    namespace UN = unify::matter_bridge::Thermostat::Attributes;
    if (aPath.mClusterId != Clusters::Thermostat::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::LocalTemperature::Id: { // type is int16s
            MN::LocalTemperature::TypeInfo::Type value;
            UN::LocalTemperature::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OutdoorTemperature::Id: { // type is int16s
            MN::OutdoorTemperature::TypeInfo::Type value;
            UN::OutdoorTemperature::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Occupancy::Id: { // type is bitmap8
            MN::Occupancy::TypeInfo::Type value;
            UN::Occupancy::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AbsMinHeatSetpointLimit::Id: { // type is int16s
            MN::AbsMinHeatSetpointLimit::TypeInfo::Type value;
            UN::AbsMinHeatSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AbsMaxHeatSetpointLimit::Id: { // type is int16s
            MN::AbsMaxHeatSetpointLimit::TypeInfo::Type value;
            UN::AbsMaxHeatSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AbsMinCoolSetpointLimit::Id: { // type is int16s
            MN::AbsMinCoolSetpointLimit::TypeInfo::Type value;
            UN::AbsMinCoolSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AbsMaxCoolSetpointLimit::Id: { // type is int16s
            MN::AbsMaxCoolSetpointLimit::TypeInfo::Type value;
            UN::AbsMaxCoolSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PICoolingDemand::Id: { // type is int8u
            MN::PICoolingDemand::TypeInfo::Type value;
            UN::PICoolingDemand::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PIHeatingDemand::Id: { // type is int8u
            MN::PIHeatingDemand::TypeInfo::Type value;
            UN::PIHeatingDemand::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::HVACSystemTypeConfiguration::Id: { // type is bitmap8
            MN::HVACSystemTypeConfiguration::TypeInfo::Type value;
            UN::HVACSystemTypeConfiguration::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LocalTemperatureCalibration::Id: { // type is int8s
            MN::LocalTemperatureCalibration::TypeInfo::Type value;
            UN::LocalTemperatureCalibration::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OccupiedCoolingSetpoint::Id: { // type is int16s
            MN::OccupiedCoolingSetpoint::TypeInfo::Type value;
            UN::OccupiedCoolingSetpoint::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OccupiedHeatingSetpoint::Id: { // type is int16s
            MN::OccupiedHeatingSetpoint::TypeInfo::Type value;
            UN::OccupiedHeatingSetpoint::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UnoccupiedCoolingSetpoint::Id: { // type is int16s
            MN::UnoccupiedCoolingSetpoint::TypeInfo::Type value;
            UN::UnoccupiedCoolingSetpoint::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UnoccupiedHeatingSetpoint::Id: { // type is int16s
            MN::UnoccupiedHeatingSetpoint::TypeInfo::Type value;
            UN::UnoccupiedHeatingSetpoint::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinHeatSetpointLimit::Id: { // type is int16s
            MN::MinHeatSetpointLimit::TypeInfo::Type value;
            UN::MinHeatSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxHeatSetpointLimit::Id: { // type is int16s
            MN::MaxHeatSetpointLimit::TypeInfo::Type value;
            UN::MaxHeatSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinCoolSetpointLimit::Id: { // type is int16s
            MN::MinCoolSetpointLimit::TypeInfo::Type value;
            UN::MinCoolSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxCoolSetpointLimit::Id: { // type is int16s
            MN::MaxCoolSetpointLimit::TypeInfo::Type value;
            UN::MaxCoolSetpointLimit::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinSetpointDeadBand::Id: { // type is int8s
            MN::MinSetpointDeadBand::TypeInfo::Type value;
            UN::MinSetpointDeadBand::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RemoteSensing::Id: { // type is bitmap8
            MN::RemoteSensing::TypeInfo::Type value;
            UN::RemoteSensing::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ControlSequenceOfOperation::Id: { // type is ThermostatControlSequence
            MN::ControlSequenceOfOperation::TypeInfo::Type value;
            UN::ControlSequenceOfOperation::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SystemMode::Id: { // type is enum8
            MN::SystemMode::TypeInfo::Type value;
            UN::SystemMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ThermostatRunningMode::Id: { // type is enum8
            MN::ThermostatRunningMode::TypeInfo::Type value;
            UN::ThermostatRunningMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StartOfWeek::Id: { // type is enum8
            MN::StartOfWeek::TypeInfo::Type value;
            UN::StartOfWeek::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfWeeklyTransitions::Id: { // type is int8u
            MN::NumberOfWeeklyTransitions::TypeInfo::Type value;
            UN::NumberOfWeeklyTransitions::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfDailyTransitions::Id: { // type is int8u
            MN::NumberOfDailyTransitions::TypeInfo::Type value;
            UN::NumberOfDailyTransitions::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TemperatureSetpointHold::Id: { // type is enum8
            MN::TemperatureSetpointHold::TypeInfo::Type value;
            UN::TemperatureSetpointHold::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TemperatureSetpointHoldDuration::Id: { // type is int16u
            MN::TemperatureSetpointHoldDuration::TypeInfo::Type value;
            UN::TemperatureSetpointHoldDuration::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ThermostatProgrammingOperationMode::Id: { // type is bitmap8
            MN::ThermostatProgrammingOperationMode::TypeInfo::Type value;
            UN::ThermostatProgrammingOperationMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ThermostatRunningState::Id: { // type is bitmap16
            MN::ThermostatRunningState::TypeInfo::Type value;
            UN::ThermostatRunningState::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SetpointChangeSource::Id: { // type is enum8
            MN::SetpointChangeSource::TypeInfo::Type value;
            UN::SetpointChangeSource::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SetpointChangeAmount::Id: { // type is int16s
            MN::SetpointChangeAmount::TypeInfo::Type value;
            UN::SetpointChangeAmount::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SetpointChangeSourceTimestamp::Id: { // type is utc
            MN::SetpointChangeSourceTimestamp::TypeInfo::Type value;
            UN::SetpointChangeSourceTimestamp::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OccupiedSetback::Id: { // type is int8u
            MN::OccupiedSetback::TypeInfo::Type value;
            UN::OccupiedSetback::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OccupiedSetbackMin::Id: { // type is int8u
            MN::OccupiedSetbackMin::TypeInfo::Type value;
            UN::OccupiedSetbackMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OccupiedSetbackMax::Id: { // type is int8u
            MN::OccupiedSetbackMax::TypeInfo::Type value;
            UN::OccupiedSetbackMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UnoccupiedSetback::Id: { // type is int8u
            MN::UnoccupiedSetback::TypeInfo::Type value;
            UN::UnoccupiedSetback::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UnoccupiedSetbackMin::Id: { // type is int8u
            MN::UnoccupiedSetbackMin::TypeInfo::Type value;
            UN::UnoccupiedSetbackMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UnoccupiedSetbackMax::Id: { // type is int8u
            MN::UnoccupiedSetbackMax::TypeInfo::Type value;
            UN::UnoccupiedSetbackMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EmergencyHeatDelta::Id: { // type is int8u
            MN::EmergencyHeatDelta::TypeInfo::Type value;
            UN::EmergencyHeatDelta::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACType::Id: { // type is enum8
            MN::ACType::TypeInfo::Type value;
            UN::ACType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACCapacity::Id: { // type is int16u
            MN::ACCapacity::TypeInfo::Type value;
            UN::ACCapacity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACRefrigerantType::Id: { // type is enum8
            MN::ACRefrigerantType::TypeInfo::Type value;
            UN::ACRefrigerantType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACCompressorType::Id: { // type is enum8
            MN::ACCompressorType::TypeInfo::Type value;
            UN::ACCompressorType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACErrorCode::Id: { // type is bitmap32
            MN::ACErrorCode::TypeInfo::Type value;
            UN::ACErrorCode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACLouverPosition::Id: { // type is enum8
            MN::ACLouverPosition::TypeInfo::Type value;
            UN::ACLouverPosition::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACCoilTemperature::Id: { // type is int16s
            MN::ACCoilTemperature::TypeInfo::Type value;
            UN::ACCoilTemperature::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ACCapacityformat::Id: { // type is enum8
            MN::ACCapacityformat::TypeInfo::Type value;
            UN::ACCapacityformat::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_THERMOSTAT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::Thermostat;

    if (aPath.mClusterId != Clusters::Thermostat::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::HVACSystemTypeConfiguration::Id: {

        Attributes::HVACSystemTypeConfiguration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("HVACSystemTypeConfiguration");
        break;
    }
    case Attributes::LocalTemperatureCalibration::Id: {

        Attributes::LocalTemperatureCalibration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("LocalTemperatureCalibration");
        break;
    }
    case Attributes::OccupiedCoolingSetpoint::Id: {

        Attributes::OccupiedCoolingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OccupiedCoolingSetpoint");
        break;
    }
    case Attributes::OccupiedHeatingSetpoint::Id: {

        Attributes::OccupiedHeatingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OccupiedHeatingSetpoint");
        break;
    }
    case Attributes::UnoccupiedCoolingSetpoint::Id: {

        Attributes::UnoccupiedCoolingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("UnoccupiedCoolingSetpoint");
        break;
    }
    case Attributes::UnoccupiedHeatingSetpoint::Id: {

        Attributes::UnoccupiedHeatingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("UnoccupiedHeatingSetpoint");
        break;
    }
    case Attributes::MinHeatSetpointLimit::Id: {

        Attributes::MinHeatSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("MinHeatSetpointLimit");
        break;
    }
    case Attributes::MaxHeatSetpointLimit::Id: {

        Attributes::MaxHeatSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("MaxHeatSetpointLimit");
        break;
    }
    case Attributes::MinCoolSetpointLimit::Id: {

        Attributes::MinCoolSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("MinCoolSetpointLimit");
        break;
    }
    case Attributes::MaxCoolSetpointLimit::Id: {

        Attributes::MaxCoolSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("MaxCoolSetpointLimit");
        break;
    }
    case Attributes::MinSetpointDeadBand::Id: {

        Attributes::MinSetpointDeadBand::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("MinSetpointDeadBand");
        break;
    }
    case Attributes::RemoteSensing::Id: {

        Attributes::RemoteSensing::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("RemoteSensing");
        break;
    }
    case Attributes::ControlSequenceOfOperation::Id: {

        Attributes::ControlSequenceOfOperation::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ControlSequenceOfOperation");
        break;
    }
    case Attributes::SystemMode::Id: {

        Attributes::SystemMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("SystemMode");
        break;
    }
    case Attributes::TemperatureSetpointHold::Id: {

        Attributes::TemperatureSetpointHold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("TemperatureSetpointHold");
        break;
    }
    case Attributes::TemperatureSetpointHoldDuration::Id: {

        Attributes::TemperatureSetpointHoldDuration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("TemperatureSetpointHoldDuration");
        break;
    }
    case Attributes::ThermostatProgrammingOperationMode::Id: {

        Attributes::ThermostatProgrammingOperationMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ThermostatProgrammingOperationMode");
        break;
    }
    case Attributes::OccupiedSetback::Id: {

        Attributes::OccupiedSetback::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OccupiedSetback");
        break;
    }
    case Attributes::UnoccupiedSetback::Id: {

        Attributes::UnoccupiedSetback::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("UnoccupiedSetback");
        break;
    }
    case Attributes::EmergencyHeatDelta::Id: {

        Attributes::EmergencyHeatDelta::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("EmergencyHeatDelta");
        break;
    }
    case Attributes::ACType::Id: {

        Attributes::ACType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ACType");
        break;
    }
    case Attributes::ACCapacity::Id: {

        Attributes::ACCapacity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ACCapacity");
        break;
    }
    case Attributes::ACRefrigerantType::Id: {

        Attributes::ACRefrigerantType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ACRefrigerantType");
        break;
    }
    case Attributes::ACCompressorType::Id: {

        Attributes::ACCompressorType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ACCompressorType");
        break;
    }
    case Attributes::ACErrorCode::Id: {

        Attributes::ACErrorCode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ACErrorCode");
        break;
    }
    case Attributes::ACLouverPosition::Id: {

        Attributes::ACLouverPosition::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ACLouverPosition");
        break;
    }
    case Attributes::ACCapacityformat::Id: {

        Attributes::ACCapacityformat::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ACCapacityformat");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/Thermostat/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ThermostatAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                 const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::Thermostat::Attributes;
    namespace UN = unify::matter_bridge::Thermostat::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Thermostat::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Thermostat::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16s
    case MN::LocalTemperature::Id: {
        using T                = MN::LocalTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LocalTemperature attribute value is %s", unify_value.dump().c_str());
            UN::LocalTemperature::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::LocalTemperature::Id);
        }
        break;
    }
        // type is int16s
    case MN::OutdoorTemperature::Id: {
        using T                = MN::OutdoorTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OutdoorTemperature attribute value is %s", unify_value.dump().c_str());
            UN::OutdoorTemperature::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OutdoorTemperature::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::Occupancy::Id: {
        std::optional<uint8_t> value = from_json_Occupancy(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Occupancy attribute value is %s", unify_value.dump().c_str());
            UN::Occupancy::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::Occupancy::Id);
        }
        break;
    }
        // type is int16s
    case MN::AbsMinHeatSetpointLimit::Id: {
        using T                = MN::AbsMinHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AbsMinHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::AbsMinHeatSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMinHeatSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::AbsMaxHeatSetpointLimit::Id: {
        using T                = MN::AbsMaxHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AbsMaxHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::AbsMaxHeatSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMaxHeatSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::AbsMinCoolSetpointLimit::Id: {
        using T                = MN::AbsMinCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AbsMinCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::AbsMinCoolSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMinCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::AbsMaxCoolSetpointLimit::Id: {
        using T                = MN::AbsMaxCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AbsMaxCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::AbsMaxCoolSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMaxCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int8u
    case MN::PICoolingDemand::Id: {
        using T                = MN::PICoolingDemand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PICoolingDemand attribute value is %s", unify_value.dump().c_str());
            UN::PICoolingDemand::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::PICoolingDemand::Id);
        }
        break;
    }
        // type is int8u
    case MN::PIHeatingDemand::Id: {
        using T                = MN::PIHeatingDemand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PIHeatingDemand attribute value is %s", unify_value.dump().c_str());
            UN::PIHeatingDemand::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::PIHeatingDemand::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::HVACSystemTypeConfiguration::Id: {
        using T                = MN::HVACSystemTypeConfiguration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "HVACSystemTypeConfiguration attribute value is %s", unify_value.dump().c_str());
            UN::HVACSystemTypeConfiguration::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::HVACSystemTypeConfiguration::Id);
        }
        break;
    }
        // type is int8s
    case MN::LocalTemperatureCalibration::Id: {
        using T                = MN::LocalTemperatureCalibration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LocalTemperatureCalibration attribute value is %s", unify_value.dump().c_str());
            UN::LocalTemperatureCalibration::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::LocalTemperatureCalibration::Id);
        }
        break;
    }
        // type is int16s
    case MN::OccupiedCoolingSetpoint::Id: {
        using T                = MN::OccupiedCoolingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OccupiedCoolingSetpoint attribute value is %s", unify_value.dump().c_str());
            UN::OccupiedCoolingSetpoint::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedCoolingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::OccupiedHeatingSetpoint::Id: {
        using T                = MN::OccupiedHeatingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OccupiedHeatingSetpoint attribute value is %s", unify_value.dump().c_str());
            UN::OccupiedHeatingSetpoint::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedHeatingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::UnoccupiedCoolingSetpoint::Id: {
        using T                = MN::UnoccupiedCoolingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UnoccupiedCoolingSetpoint attribute value is %s", unify_value.dump().c_str());
            UN::UnoccupiedCoolingSetpoint::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::UnoccupiedCoolingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::UnoccupiedHeatingSetpoint::Id: {
        using T                = MN::UnoccupiedHeatingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UnoccupiedHeatingSetpoint attribute value is %s", unify_value.dump().c_str());
            UN::UnoccupiedHeatingSetpoint::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::UnoccupiedHeatingSetpoint::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinHeatSetpointLimit::Id: {
        using T                = MN::MinHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::MinHeatSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinHeatSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxHeatSetpointLimit::Id: {
        using T                = MN::MaxHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::MaxHeatSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MaxHeatSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinCoolSetpointLimit::Id: {
        using T                = MN::MinCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::MinCoolSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxCoolSetpointLimit::Id: {
        using T                = MN::MaxCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            UN::MaxCoolSetpointLimit::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MaxCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int8s
    case MN::MinSetpointDeadBand::Id: {
        using T                = MN::MinSetpointDeadBand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinSetpointDeadBand attribute value is %s", unify_value.dump().c_str());
            UN::MinSetpointDeadBand::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinSetpointDeadBand::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::RemoteSensing::Id: {
        using T                = MN::RemoteSensing::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RemoteSensing attribute value is %s", unify_value.dump().c_str());
            UN::RemoteSensing::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::RemoteSensing::Id);
        }
        break;
    }
        // type is ThermostatControlSequence
    case MN::ControlSequenceOfOperation::Id: {
        using T                = MN::ControlSequenceOfOperation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ControlSequenceOfOperation attribute value is %s", unify_value.dump().c_str());
            UN::ControlSequenceOfOperation::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::ControlSequenceOfOperation::Id);
        }
        break;
    }
        // type is enum8
    case MN::SystemMode::Id: {
        using T                = MN::SystemMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SystemMode attribute value is %s", unify_value.dump().c_str());
            UN::SystemMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SystemMode::Id);
        }
        break;
    }
        // type is enum8
    case MN::ThermostatRunningMode::Id: {
        using T                = MN::ThermostatRunningMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ThermostatRunningMode attribute value is %s", unify_value.dump().c_str());
            UN::ThermostatRunningMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ThermostatRunningMode::Id);
        }
        break;
    }
        // type is enum8
    case MN::StartOfWeek::Id: {
        using T                = MN::StartOfWeek::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StartOfWeek attribute value is %s", unify_value.dump().c_str());
            UN::StartOfWeek::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::StartOfWeek::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfWeeklyTransitions::Id: {
        using T                = MN::NumberOfWeeklyTransitions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfWeeklyTransitions attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfWeeklyTransitions::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::NumberOfWeeklyTransitions::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfDailyTransitions::Id: {
        using T                = MN::NumberOfDailyTransitions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfDailyTransitions attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfDailyTransitions::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::NumberOfDailyTransitions::Id);
        }
        break;
    }
        // type is enum8
    case MN::TemperatureSetpointHold::Id: {
        using T                = MN::TemperatureSetpointHold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TemperatureSetpointHold attribute value is %s", unify_value.dump().c_str());
            UN::TemperatureSetpointHold::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::TemperatureSetpointHold::Id);
        }
        break;
    }
        // type is int16u
    case MN::TemperatureSetpointHoldDuration::Id: {
        using T                = MN::TemperatureSetpointHoldDuration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TemperatureSetpointHoldDuration attribute value is %s", unify_value.dump().c_str());
            UN::TemperatureSetpointHoldDuration::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::TemperatureSetpointHoldDuration::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::ThermostatProgrammingOperationMode::Id: {
        using T                = MN::ThermostatProgrammingOperationMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ThermostatProgrammingOperationMode attribute value is %s", unify_value.dump().c_str());
            UN::ThermostatProgrammingOperationMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::ThermostatProgrammingOperationMode::Id);
        }
        break;
    }
        // type is bitmap16
    case MN::ThermostatRunningState::Id: {
        std::optional<uint16_t> value = from_json_ThermostatRunningState(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ThermostatRunningState attribute value is %s", unify_value.dump().c_str());
            UN::ThermostatRunningState::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ThermostatRunningState::Id);
        }
        break;
    }
        // type is enum8
    case MN::SetpointChangeSource::Id: {
        using T                = MN::SetpointChangeSource::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SetpointChangeSource attribute value is %s", unify_value.dump().c_str());
            UN::SetpointChangeSource::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SetpointChangeSource::Id);
        }
        break;
    }
        // type is int16s
    case MN::SetpointChangeAmount::Id: {
        using T                = MN::SetpointChangeAmount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SetpointChangeAmount attribute value is %s", unify_value.dump().c_str());
            UN::SetpointChangeAmount::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SetpointChangeAmount::Id);
        }
        break;
    }
        // type is utc
    case MN::SetpointChangeSourceTimestamp::Id: {
        using T                = MN::SetpointChangeSourceTimestamp::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SetpointChangeSourceTimestamp attribute value is %s", unify_value.dump().c_str());
            UN::SetpointChangeSourceTimestamp::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                                                   MN::SetpointChangeSourceTimestamp::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetback::Id: {
        using T                = MN::OccupiedSetback::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OccupiedSetback attribute value is %s", unify_value.dump().c_str());
            UN::OccupiedSetback::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetback::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetbackMin::Id: {
        using T                = MN::OccupiedSetbackMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OccupiedSetbackMin attribute value is %s", unify_value.dump().c_str());
            UN::OccupiedSetbackMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetbackMin::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetbackMax::Id: {
        using T                = MN::OccupiedSetbackMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OccupiedSetbackMax attribute value is %s", unify_value.dump().c_str());
            UN::OccupiedSetbackMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetbackMax::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetback::Id: {
        using T                = MN::UnoccupiedSetback::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UnoccupiedSetback attribute value is %s", unify_value.dump().c_str());
            UN::UnoccupiedSetback::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetback::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetbackMin::Id: {
        using T                = MN::UnoccupiedSetbackMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UnoccupiedSetbackMin attribute value is %s", unify_value.dump().c_str());
            UN::UnoccupiedSetbackMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetbackMin::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetbackMax::Id: {
        using T                = MN::UnoccupiedSetbackMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UnoccupiedSetbackMax attribute value is %s", unify_value.dump().c_str());
            UN::UnoccupiedSetbackMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetbackMax::Id);
        }
        break;
    }
        // type is int8u
    case MN::EmergencyHeatDelta::Id: {
        using T                = MN::EmergencyHeatDelta::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EmergencyHeatDelta attribute value is %s", unify_value.dump().c_str());
            UN::EmergencyHeatDelta::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::EmergencyHeatDelta::Id);
        }
        break;
    }
        // type is enum8
    case MN::ACType::Id: {
        using T                = MN::ACType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACType attribute value is %s", unify_value.dump().c_str());
            UN::ACType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACType::Id);
        }
        break;
    }
        // type is int16u
    case MN::ACCapacity::Id: {
        using T                = MN::ACCapacity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACCapacity attribute value is %s", unify_value.dump().c_str());
            UN::ACCapacity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCapacity::Id);
        }
        break;
    }
        // type is enum8
    case MN::ACRefrigerantType::Id: {
        using T                = MN::ACRefrigerantType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACRefrigerantType attribute value is %s", unify_value.dump().c_str());
            UN::ACRefrigerantType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACRefrigerantType::Id);
        }
        break;
    }
        // type is enum8
    case MN::ACCompressorType::Id: {
        using T                = MN::ACCompressorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACCompressorType attribute value is %s", unify_value.dump().c_str());
            UN::ACCompressorType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCompressorType::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::ACErrorCode::Id: {
        using T                = MN::ACErrorCode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACErrorCode attribute value is %s", unify_value.dump().c_str());
            UN::ACErrorCode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACErrorCode::Id);
        }
        break;
    }
        // type is enum8
    case MN::ACLouverPosition::Id: {
        using T                = MN::ACLouverPosition::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACLouverPosition attribute value is %s", unify_value.dump().c_str());
            UN::ACLouverPosition::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACLouverPosition::Id);
        }
        break;
    }
        // type is int16s
    case MN::ACCoilTemperature::Id: {
        using T                = MN::ACCoilTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACCoilTemperature attribute value is %s", unify_value.dump().c_str());
            UN::ACCoilTemperature::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCoilTemperature::Id);
        }
        break;
    }
        // type is enum8
    case MN::ACCapacityformat::Id: {
        using T                = MN::ACCapacityformat::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ACCapacityformat attribute value is %s", unify_value.dump().c_str());
            UN::ACCapacityformat::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCapacityformat::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
FanControlAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::FanControl::Attributes;
    namespace UN = unify::matter_bridge::FanControl::Attributes;
    if (aPath.mClusterId != Clusters::FanControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FanMode::Id: { // type is FanModeType
            MN::FanMode::TypeInfo::Type value;
            UN::FanMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FanModeSequence::Id: { // type is FanModeSequenceType
            MN::FanModeSequence::TypeInfo::Type value;
            UN::FanModeSequence::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PercentSetting::Id: { // type is int8u
            MN::PercentSetting::TypeInfo::Type value;
            UN::PercentSetting::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PercentCurrent::Id: { // type is int8u
            MN::PercentCurrent::TypeInfo::Type value;
            UN::PercentCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SpeedMax::Id: { // type is int8u
            MN::SpeedMax::TypeInfo::Type value;
            UN::SpeedMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SpeedSetting::Id: { // type is int8u
            MN::SpeedSetting::TypeInfo::Type value;
            UN::SpeedSetting::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SpeedCurrent::Id: { // type is int8u
            MN::SpeedCurrent::TypeInfo::Type value;
            UN::SpeedCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RockSupport::Id: { // type is bitmap8
            MN::RockSupport::TypeInfo::Type value;
            UN::RockSupport::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RockSetting::Id: { // type is bitmap8
            MN::RockSetting::TypeInfo::Type value;
            UN::RockSetting::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WindSupport::Id: { // type is bitmap8
            MN::WindSupport::TypeInfo::Type value;
            UN::WindSupport::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WindSetting::Id: { // type is bitmap8
            MN::WindSetting::TypeInfo::Type value;
            UN::WindSetting::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_FAN_CONTROL_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanControlAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::FanControl;

    if (aPath.mClusterId != Clusters::FanControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::FanMode::Id: {

        Attributes::FanMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("FanMode");
        break;
    }
    case Attributes::FanModeSequence::Id: {

        Attributes::FanModeSequence::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("FanModeSequence");
        break;
    }
    case Attributes::PercentSetting::Id: {

        Attributes::PercentSetting::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PercentSetting");
        break;
    }
    case Attributes::SpeedSetting::Id: {

        Attributes::SpeedSetting::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("SpeedSetting");
        break;
    }
    case Attributes::RockSetting::Id: {

        Attributes::RockSetting::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("RockSetting");
        break;
    }
    case Attributes::WindSetting::Id: {

        Attributes::WindSetting::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("WindSetting");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/FanControl/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void FanControlAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                 const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::FanControl::Attributes;
    namespace UN = unify::matter_bridge::FanControl::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::FanControl::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::FanControl::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is FanModeType
    case MN::FanMode::Id: {
        using T                = MN::FanMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FanMode attribute value is %s", unify_value.dump().c_str());
            UN::FanMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::FanMode::Id);
        }
        break;
    }
        // type is FanModeSequenceType
    case MN::FanModeSequence::Id: {
        using T                = MN::FanModeSequence::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FanModeSequence attribute value is %s", unify_value.dump().c_str());
            UN::FanModeSequence::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::FanModeSequence::Id);
        }
        break;
    }
        // type is int8u
    case MN::PercentSetting::Id: {
        using T                = MN::PercentSetting::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PercentSetting attribute value is %s", unify_value.dump().c_str());
            UN::PercentSetting::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::PercentSetting::Id);
        }
        break;
    }
        // type is int8u
    case MN::PercentCurrent::Id: {
        using T                = MN::PercentCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PercentCurrent attribute value is %s", unify_value.dump().c_str());
            UN::PercentCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::PercentCurrent::Id);
        }
        break;
    }
        // type is int8u
    case MN::SpeedMax::Id: {
        using T                = MN::SpeedMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SpeedMax attribute value is %s", unify_value.dump().c_str());
            UN::SpeedMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::SpeedMax::Id);
        }
        break;
    }
        // type is int8u
    case MN::SpeedSetting::Id: {
        using T                = MN::SpeedSetting::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SpeedSetting attribute value is %s", unify_value.dump().c_str());
            UN::SpeedSetting::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::SpeedSetting::Id);
        }
        break;
    }
        // type is int8u
    case MN::SpeedCurrent::Id: {
        using T                = MN::SpeedCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SpeedCurrent attribute value is %s", unify_value.dump().c_str());
            UN::SpeedCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::SpeedCurrent::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::RockSupport::Id: {
        using T                = MN::RockSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RockSupport attribute value is %s", unify_value.dump().c_str());
            UN::RockSupport::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::RockSupport::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::RockSetting::Id: {
        using T                = MN::RockSetting::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RockSetting attribute value is %s", unify_value.dump().c_str());
            UN::RockSetting::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::RockSetting::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::WindSupport::Id: {
        using T                = MN::WindSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WindSupport attribute value is %s", unify_value.dump().c_str());
            UN::WindSupport::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::WindSupport::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::WindSetting::Id: {
        using T                = MN::WindSetting::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WindSetting attribute value is %s", unify_value.dump().c_str());
            UN::WindSetting::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::WindSetting::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FanControl::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ThermostatUserInterfaceConfigurationAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ThermostatUserInterfaceConfiguration::Attributes;
    namespace UN = unify::matter_bridge::ThermostatUserInterfaceConfiguration::Attributes;
    if (aPath.mClusterId != Clusters::ThermostatUserInterfaceConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::TemperatureDisplayMode::Id: { // type is enum8
            MN::TemperatureDisplayMode::TypeInfo::Type value;
            UN::TemperatureDisplayMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::KeypadLockout::Id: { // type is enum8
            MN::KeypadLockout::TypeInfo::Type value;
            UN::KeypadLockout::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ScheduleProgrammingVisibility::Id: { // type is enum8
            MN::ScheduleProgrammingVisibility::TypeInfo::Type value;
            UN::ScheduleProgrammingVisibility::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_THERMOSTAT_USER_INTERFACE_CONFIGURATION_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatUserInterfaceConfigurationAttributeAccess::Write(const ConcreteDataAttributePath & aPath,
                                                                      AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration;

    if (aPath.mClusterId != Clusters::ThermostatUserInterfaceConfiguration::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::TemperatureDisplayMode::Id: {

        Attributes::TemperatureDisplayMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("TemperatureDisplayMode");
        break;
    }
    case Attributes::KeypadLockout::Id: {

        Attributes::KeypadLockout::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("KeypadLockout");
        break;
    }
    case Attributes::ScheduleProgrammingVisibility::Id: {

        Attributes::ScheduleProgrammingVisibility::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ScheduleProgrammingVisibility");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ThermostatUserInterfaceConfiguration/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ThermostatUserInterfaceConfigurationAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                                           const std::string & attribute,
                                                                           const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ThermostatUserInterfaceConfiguration::Attributes;
    namespace UN = unify::matter_bridge::ThermostatUserInterfaceConfiguration::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ThermostatUserInterfaceConfiguration::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is enum8
    case MN::TemperatureDisplayMode::Id: {
        using T                = MN::TemperatureDisplayMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TemperatureDisplayMode attribute value is %s", unify_value.dump().c_str());
            UN::TemperatureDisplayMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                                                   MN::TemperatureDisplayMode::Id);
        }
        break;
    }
        // type is enum8
    case MN::KeypadLockout::Id: {
        using T                = MN::KeypadLockout::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "KeypadLockout attribute value is %s", unify_value.dump().c_str());
            UN::KeypadLockout::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                                                   MN::KeypadLockout::Id);
        }
        break;
    }
        // type is enum8
    case MN::ScheduleProgrammingVisibility::Id: {
        using T                = MN::ScheduleProgrammingVisibility::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ScheduleProgrammingVisibility attribute value is %s", unify_value.dump().c_str());
            UN::ScheduleProgrammingVisibility::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                                                   MN::ScheduleProgrammingVisibility::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ThermostatUserInterfaceConfiguration::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ColorControlAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ColorControl::Attributes;
    namespace UN = unify::matter_bridge::ColorControl::Attributes;
    if (aPath.mClusterId != Clusters::ColorControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::CurrentHue::Id: { // type is int8u
            MN::CurrentHue::TypeInfo::Type value;
            UN::CurrentHue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentSaturation::Id: { // type is int8u
            MN::CurrentSaturation::TypeInfo::Type value;
            UN::CurrentSaturation::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RemainingTime::Id: { // type is int16u
            MN::RemainingTime::TypeInfo::Type value;
            UN::RemainingTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentX::Id: { // type is int16u
            MN::CurrentX::TypeInfo::Type value;
            UN::CurrentX::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentY::Id: { // type is int16u
            MN::CurrentY::TypeInfo::Type value;
            UN::CurrentY::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DriftCompensation::Id: { // type is enum8
            MN::DriftCompensation::TypeInfo::Type value;
            UN::DriftCompensation::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CompensationText::Id: { // type is char_string
            MN::CompensationText::TypeInfo::Type value;
            UN::CompensationText::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorTemperatureMireds::Id: { // type is int16u
            MN::ColorTemperatureMireds::TypeInfo::Type value;
            UN::ColorTemperatureMireds::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorMode::Id: { // type is enum8
            MN::ColorMode::TypeInfo::Type value;
            UN::ColorMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Options::Id: { // type is bitmap8
            MN::Options::TypeInfo::Type value;
            UN::Options::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NumberOfPrimaries::Id: { // type is int8u
            MN::NumberOfPrimaries::TypeInfo::Type value;
            UN::NumberOfPrimaries::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary1X::Id: { // type is int16u
            MN::Primary1X::TypeInfo::Type value;
            UN::Primary1X::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary1Y::Id: { // type is int16u
            MN::Primary1Y::TypeInfo::Type value;
            UN::Primary1Y::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary1Intensity::Id: { // type is int8u
            MN::Primary1Intensity::TypeInfo::Type value;
            UN::Primary1Intensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary2X::Id: { // type is int16u
            MN::Primary2X::TypeInfo::Type value;
            UN::Primary2X::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary2Y::Id: { // type is int16u
            MN::Primary2Y::TypeInfo::Type value;
            UN::Primary2Y::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary2Intensity::Id: { // type is int8u
            MN::Primary2Intensity::TypeInfo::Type value;
            UN::Primary2Intensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary3X::Id: { // type is int16u
            MN::Primary3X::TypeInfo::Type value;
            UN::Primary3X::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary3Y::Id: { // type is int16u
            MN::Primary3Y::TypeInfo::Type value;
            UN::Primary3Y::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary3Intensity::Id: { // type is int8u
            MN::Primary3Intensity::TypeInfo::Type value;
            UN::Primary3Intensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary4X::Id: { // type is int16u
            MN::Primary4X::TypeInfo::Type value;
            UN::Primary4X::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary4Y::Id: { // type is int16u
            MN::Primary4Y::TypeInfo::Type value;
            UN::Primary4Y::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary4Intensity::Id: { // type is int8u
            MN::Primary4Intensity::TypeInfo::Type value;
            UN::Primary4Intensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary5X::Id: { // type is int16u
            MN::Primary5X::TypeInfo::Type value;
            UN::Primary5X::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary5Y::Id: { // type is int16u
            MN::Primary5Y::TypeInfo::Type value;
            UN::Primary5Y::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary5Intensity::Id: { // type is int8u
            MN::Primary5Intensity::TypeInfo::Type value;
            UN::Primary5Intensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary6X::Id: { // type is int16u
            MN::Primary6X::TypeInfo::Type value;
            UN::Primary6X::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary6Y::Id: { // type is int16u
            MN::Primary6Y::TypeInfo::Type value;
            UN::Primary6Y::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Primary6Intensity::Id: { // type is int8u
            MN::Primary6Intensity::TypeInfo::Type value;
            UN::Primary6Intensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WhitePointX::Id: { // type is int16u
            MN::WhitePointX::TypeInfo::Type value;
            UN::WhitePointX::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::WhitePointY::Id: { // type is int16u
            MN::WhitePointY::TypeInfo::Type value;
            UN::WhitePointY::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointRX::Id: { // type is int16u
            MN::ColorPointRX::TypeInfo::Type value;
            UN::ColorPointRX::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointRY::Id: { // type is int16u
            MN::ColorPointRY::TypeInfo::Type value;
            UN::ColorPointRY::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointRIntensity::Id: { // type is int8u
            MN::ColorPointRIntensity::TypeInfo::Type value;
            UN::ColorPointRIntensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointGX::Id: { // type is int16u
            MN::ColorPointGX::TypeInfo::Type value;
            UN::ColorPointGX::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointGY::Id: { // type is int16u
            MN::ColorPointGY::TypeInfo::Type value;
            UN::ColorPointGY::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointGIntensity::Id: { // type is int8u
            MN::ColorPointGIntensity::TypeInfo::Type value;
            UN::ColorPointGIntensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointBX::Id: { // type is int16u
            MN::ColorPointBX::TypeInfo::Type value;
            UN::ColorPointBX::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointBY::Id: { // type is int16u
            MN::ColorPointBY::TypeInfo::Type value;
            UN::ColorPointBY::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorPointBIntensity::Id: { // type is int8u
            MN::ColorPointBIntensity::TypeInfo::Type value;
            UN::ColorPointBIntensity::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EnhancedCurrentHue::Id: { // type is int16u
            MN::EnhancedCurrentHue::TypeInfo::Type value;
            UN::EnhancedCurrentHue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::EnhancedColorMode::Id: { // type is enum8
            MN::EnhancedColorMode::TypeInfo::Type value;
            UN::EnhancedColorMode::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorLoopActive::Id: { // type is int8u
            MN::ColorLoopActive::TypeInfo::Type value;
            UN::ColorLoopActive::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorLoopDirection::Id: { // type is int8u
            MN::ColorLoopDirection::TypeInfo::Type value;
            UN::ColorLoopDirection::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorLoopTime::Id: { // type is int16u
            MN::ColorLoopTime::TypeInfo::Type value;
            UN::ColorLoopTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorLoopStartEnhancedHue::Id: { // type is int16u
            MN::ColorLoopStartEnhancedHue::TypeInfo::Type value;
            UN::ColorLoopStartEnhancedHue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorLoopStoredEnhancedHue::Id: { // type is int16u
            MN::ColorLoopStoredEnhancedHue::TypeInfo::Type value;
            UN::ColorLoopStoredEnhancedHue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorCapabilities::Id: { // type is bitmap16
            MN::ColorCapabilities::TypeInfo::Type value;
            UN::ColorCapabilities::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorTempPhysicalMinMireds::Id: { // type is int16u
            MN::ColorTempPhysicalMinMireds::TypeInfo::Type value;
            UN::ColorTempPhysicalMinMireds::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ColorTempPhysicalMaxMireds::Id: { // type is int16u
            MN::ColorTempPhysicalMaxMireds::TypeInfo::Type value;
            UN::ColorTempPhysicalMaxMireds::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CoupleColorTempToLevelMinMireds::Id: { // type is int16u
            MN::CoupleColorTempToLevelMinMireds::TypeInfo::Type value;
            UN::CoupleColorTempToLevelMinMireds::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StartUpColorTemperatureMireds::Id: { // type is int16u
            MN::StartUpColorTemperatureMireds::TypeInfo::Type value;
            UN::StartUpColorTemperatureMireds::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_COLOR_CONTROL_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ColorControl;

    if (aPath.mClusterId != Clusters::ColorControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::Options::Id: {

        Attributes::Options::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("Options");
        break;
    }
    case Attributes::WhitePointX::Id: {

        Attributes::WhitePointX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("WhitePointX");
        break;
    }
    case Attributes::WhitePointY::Id: {

        Attributes::WhitePointY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("WhitePointY");
        break;
    }
    case Attributes::ColorPointRX::Id: {

        Attributes::ColorPointRX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointRX");
        break;
    }
    case Attributes::ColorPointRY::Id: {

        Attributes::ColorPointRY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointRY");
        break;
    }
    case Attributes::ColorPointRIntensity::Id: {

        Attributes::ColorPointRIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointRIntensity");
        break;
    }
    case Attributes::ColorPointGX::Id: {

        Attributes::ColorPointGX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointGX");
        break;
    }
    case Attributes::ColorPointGY::Id: {

        Attributes::ColorPointGY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointGY");
        break;
    }
    case Attributes::ColorPointGIntensity::Id: {

        Attributes::ColorPointGIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointGIntensity");
        break;
    }
    case Attributes::ColorPointBX::Id: {

        Attributes::ColorPointBX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointBX");
        break;
    }
    case Attributes::ColorPointBY::Id: {

        Attributes::ColorPointBY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointBY");
        break;
    }
    case Attributes::ColorPointBIntensity::Id: {

        Attributes::ColorPointBIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("ColorPointBIntensity");
        break;
    }
    case Attributes::StartUpColorTemperatureMireds::Id: {

        Attributes::StartUpColorTemperatureMireds::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("StartUpColorTemperatureMireds");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ColorControl/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ColorControlAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                   const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ColorControl::Attributes;
    namespace UN = unify::matter_bridge::ColorControl::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ColorControl::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::ColorControl::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::CurrentHue::Id: {
        using T                = MN::CurrentHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentHue attribute value is %s", unify_value.dump().c_str());
            UN::CurrentHue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentHue::Id);
        }
        break;
    }
        // type is int8u
    case MN::CurrentSaturation::Id: {
        using T                = MN::CurrentSaturation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentSaturation attribute value is %s", unify_value.dump().c_str());
            UN::CurrentSaturation::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentSaturation::Id);
        }
        break;
    }
        // type is int16u
    case MN::RemainingTime::Id: {
        using T                = MN::RemainingTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RemainingTime attribute value is %s", unify_value.dump().c_str());
            UN::RemainingTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::RemainingTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentX::Id: {
        using T                = MN::CurrentX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentX attribute value is %s", unify_value.dump().c_str());
            UN::CurrentX::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentX::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentY::Id: {
        using T                = MN::CurrentY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentY attribute value is %s", unify_value.dump().c_str());
            UN::CurrentY::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentY::Id);
        }
        break;
    }
        // type is enum8
    case MN::DriftCompensation::Id: {
        using T                = MN::DriftCompensation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DriftCompensation attribute value is %s", unify_value.dump().c_str());
            UN::DriftCompensation::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::DriftCompensation::Id);
        }
        break;
    }
        // type is char_string
    case MN::CompensationText::Id: {
        using T                = MN::CompensationText::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CompensationText attribute value is %s", unify_value.dump().c_str());
            UN::CompensationText::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CompensationText::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTemperatureMireds::Id: {
        using T                = MN::ColorTemperatureMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorTemperatureMireds attribute value is %s", unify_value.dump().c_str());
            UN::ColorTemperatureMireds::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                                                   MN::ColorTemperatureMireds::Id);
        }
        break;
    }
        // type is enum8
    case MN::ColorMode::Id: {
        using T                = ZclEnumColorMode;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorMode attribute value is %s", unify_value.dump().c_str());
            UN::ColorMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorMode::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::Options::Id: {
        using T                = MN::Options::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Options attribute value is %s", unify_value.dump().c_str());
            UN::Options::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Options::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfPrimaries::Id: {
        using T                = MN::NumberOfPrimaries::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NumberOfPrimaries attribute value is %s", unify_value.dump().c_str());
            UN::NumberOfPrimaries::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::NumberOfPrimaries::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary1X::Id: {
        using T                = MN::Primary1X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary1X attribute value is %s", unify_value.dump().c_str());
            UN::Primary1X::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary1Y::Id: {
        using T                = MN::Primary1Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary1Y attribute value is %s", unify_value.dump().c_str());
            UN::Primary1Y::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary1Intensity::Id: {
        using T                = MN::Primary1Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary1Intensity attribute value is %s", unify_value.dump().c_str());
            UN::Primary1Intensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary2X::Id: {
        using T                = MN::Primary2X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary2X attribute value is %s", unify_value.dump().c_str());
            UN::Primary2X::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary2Y::Id: {
        using T                = MN::Primary2Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary2Y attribute value is %s", unify_value.dump().c_str());
            UN::Primary2Y::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary2Intensity::Id: {
        using T                = MN::Primary2Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary2Intensity attribute value is %s", unify_value.dump().c_str());
            UN::Primary2Intensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary3X::Id: {
        using T                = MN::Primary3X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary3X attribute value is %s", unify_value.dump().c_str());
            UN::Primary3X::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary3Y::Id: {
        using T                = MN::Primary3Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary3Y attribute value is %s", unify_value.dump().c_str());
            UN::Primary3Y::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary3Intensity::Id: {
        using T                = MN::Primary3Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary3Intensity attribute value is %s", unify_value.dump().c_str());
            UN::Primary3Intensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary4X::Id: {
        using T                = MN::Primary4X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary4X attribute value is %s", unify_value.dump().c_str());
            UN::Primary4X::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary4Y::Id: {
        using T                = MN::Primary4Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary4Y attribute value is %s", unify_value.dump().c_str());
            UN::Primary4Y::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary4Intensity::Id: {
        using T                = MN::Primary4Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary4Intensity attribute value is %s", unify_value.dump().c_str());
            UN::Primary4Intensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary5X::Id: {
        using T                = MN::Primary5X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary5X attribute value is %s", unify_value.dump().c_str());
            UN::Primary5X::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary5Y::Id: {
        using T                = MN::Primary5Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary5Y attribute value is %s", unify_value.dump().c_str());
            UN::Primary5Y::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary5Intensity::Id: {
        using T                = MN::Primary5Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary5Intensity attribute value is %s", unify_value.dump().c_str());
            UN::Primary5Intensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary6X::Id: {
        using T                = MN::Primary6X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary6X attribute value is %s", unify_value.dump().c_str());
            UN::Primary6X::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary6Y::Id: {
        using T                = MN::Primary6Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary6Y attribute value is %s", unify_value.dump().c_str());
            UN::Primary6Y::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary6Intensity::Id: {
        using T                = MN::Primary6Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Primary6Intensity attribute value is %s", unify_value.dump().c_str());
            UN::Primary6Intensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::WhitePointX::Id: {
        using T                = MN::WhitePointX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WhitePointX attribute value is %s", unify_value.dump().c_str());
            UN::WhitePointX::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::WhitePointX::Id);
        }
        break;
    }
        // type is int16u
    case MN::WhitePointY::Id: {
        using T                = MN::WhitePointY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "WhitePointY attribute value is %s", unify_value.dump().c_str());
            UN::WhitePointY::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::WhitePointY::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointRX::Id: {
        using T                = MN::ColorPointRX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointRX attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointRX::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointRY::Id: {
        using T                = MN::ColorPointRY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointRY attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointRY::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointRIntensity::Id: {
        using T                = MN::ColorPointRIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointRIntensity attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointRIntensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointGX::Id: {
        using T                = MN::ColorPointGX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointGX attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointGX::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointGY::Id: {
        using T                = MN::ColorPointGY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointGY attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointGY::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointGIntensity::Id: {
        using T                = MN::ColorPointGIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointGIntensity attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointGIntensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointBX::Id: {
        using T                = MN::ColorPointBX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointBX attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointBX::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointBY::Id: {
        using T                = MN::ColorPointBY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointBY attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointBY::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointBIntensity::Id: {
        using T                = MN::ColorPointBIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorPointBIntensity attribute value is %s", unify_value.dump().c_str());
            UN::ColorPointBIntensity::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::EnhancedCurrentHue::Id: {
        using T                = MN::EnhancedCurrentHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EnhancedCurrentHue attribute value is %s", unify_value.dump().c_str());
            UN::EnhancedCurrentHue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::EnhancedCurrentHue::Id);
        }
        break;
    }
        // type is enum8
    case MN::EnhancedColorMode::Id: {
        using T                = ZclEnumEnhancedColorMode;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "EnhancedColorMode attribute value is %s", unify_value.dump().c_str());
            UN::EnhancedColorMode::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::EnhancedColorMode::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorLoopActive::Id: {
        using T                = MN::ColorLoopActive::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorLoopActive attribute value is %s", unify_value.dump().c_str());
            UN::ColorLoopActive::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopActive::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorLoopDirection::Id: {
        using T                = MN::ColorLoopDirection::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorLoopDirection attribute value is %s", unify_value.dump().c_str());
            UN::ColorLoopDirection::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopDirection::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopTime::Id: {
        using T                = MN::ColorLoopTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorLoopTime attribute value is %s", unify_value.dump().c_str());
            UN::ColorLoopTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopStartEnhancedHue::Id: {
        using T                = MN::ColorLoopStartEnhancedHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorLoopStartEnhancedHue attribute value is %s", unify_value.dump().c_str());
            UN::ColorLoopStartEnhancedHue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                                                   MN::ColorLoopStartEnhancedHue::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopStoredEnhancedHue::Id: {
        using T                = MN::ColorLoopStoredEnhancedHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorLoopStoredEnhancedHue attribute value is %s", unify_value.dump().c_str());
            UN::ColorLoopStoredEnhancedHue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                                                   MN::ColorLoopStoredEnhancedHue::Id);
        }
        break;
    }
        // type is bitmap16
    case MN::ColorCapabilities::Id: {
        std::optional<uint16_t> value = from_json_ColorCapabilities(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorCapabilities attribute value is %s", unify_value.dump().c_str());
            UN::ColorCapabilities::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorCapabilities::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTempPhysicalMinMireds::Id: {
        using T                = MN::ColorTempPhysicalMinMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorTempPhysicalMinMireds attribute value is %s", unify_value.dump().c_str());
            UN::ColorTempPhysicalMinMireds::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                                                   MN::ColorTempPhysicalMinMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTempPhysicalMaxMireds::Id: {
        using T                = MN::ColorTempPhysicalMaxMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ColorTempPhysicalMaxMireds attribute value is %s", unify_value.dump().c_str());
            UN::ColorTempPhysicalMaxMireds::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                                                   MN::ColorTempPhysicalMaxMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::CoupleColorTempToLevelMinMireds::Id: {
        using T                = MN::CoupleColorTempToLevelMinMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CoupleColorTempToLevelMinMireds attribute value is %s", unify_value.dump().c_str());
            UN::CoupleColorTempToLevelMinMireds::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                                                   MN::CoupleColorTempToLevelMinMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::StartUpColorTemperatureMireds::Id: {
        using T                = MN::StartUpColorTemperatureMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StartUpColorTemperatureMireds attribute value is %s", unify_value.dump().c_str());
            UN::StartUpColorTemperatureMireds::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                                                   MN::StartUpColorTemperatureMireds::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
IlluminanceMeasurementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::IlluminanceMeasurement::Attributes;
    namespace UN = unify::matter_bridge::IlluminanceMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::IlluminanceMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            UN::MeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            UN::MinMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            UN::MaxMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            UN::Tolerance::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LightSensorType::Id: { // type is enum8
            MN::LightSensorType::TypeInfo::Type value;
            UN::LightSensorType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_ILLUMINANCE_MEASUREMENT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR IlluminanceMeasurementAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::IlluminanceMeasurement;

    if (aPath.mClusterId != Clusters::IlluminanceMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/IlluminanceMeasurement/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void IlluminanceMeasurementAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                             const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::IlluminanceMeasurement::Attributes;
    namespace UN = unify::matter_bridge::IlluminanceMeasurement::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::IlluminanceMeasurement::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T                = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                                                   MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T                = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MinMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                                                   MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T                = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MaxMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                                                   MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T                = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            UN::Tolerance::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is enum8
    case MN::LightSensorType::Id: {
        using T                = MN::LightSensorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LightSensorType attribute value is %s", unify_value.dump().c_str());
            UN::LightSensorType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                                                   MN::LightSensorType::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
TemperatureMeasurementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::TemperatureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::TemperatureMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::TemperatureMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MeasuredValue::Id: { // type is int16s
            MN::MeasuredValue::TypeInfo::Type value;
            UN::MeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinMeasuredValue::Id: { // type is int16s
            MN::MinMeasuredValue::TypeInfo::Type value;
            UN::MinMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxMeasuredValue::Id: { // type is int16s
            MN::MaxMeasuredValue::TypeInfo::Type value;
            UN::MaxMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            UN::Tolerance::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_TEMPERATURE_MEASUREMENT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::TemperatureMeasurement;

    if (aPath.mClusterId != Clusters::TemperatureMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/TemperatureMeasurement/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void TemperatureMeasurementAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                             const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::TemperatureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::TemperatureMeasurement::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::TemperatureMeasurement::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::TemperatureMeasurement::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16s
    case MN::MeasuredValue::Id: {
        using T                = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                                                   MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinMeasuredValue::Id: {
        using T                = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MinMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                                                   MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxMeasuredValue::Id: {
        using T                = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MaxMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                                                   MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T                = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            UN::Tolerance::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
PressureMeasurementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::PressureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::PressureMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::PressureMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MeasuredValue::Id: { // type is int16s
            MN::MeasuredValue::TypeInfo::Type value;
            UN::MeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinMeasuredValue::Id: { // type is int16s
            MN::MinMeasuredValue::TypeInfo::Type value;
            UN::MinMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxMeasuredValue::Id: { // type is int16s
            MN::MaxMeasuredValue::TypeInfo::Type value;
            UN::MaxMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            UN::Tolerance::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ScaledValue::Id: { // type is int16s
            MN::ScaledValue::TypeInfo::Type value;
            UN::ScaledValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinScaledValue::Id: { // type is int16s
            MN::MinScaledValue::TypeInfo::Type value;
            UN::MinScaledValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxScaledValue::Id: { // type is int16s
            MN::MaxScaledValue::TypeInfo::Type value;
            UN::MaxScaledValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ScaledTolerance::Id: { // type is int16u
            MN::ScaledTolerance::TypeInfo::Type value;
            UN::ScaledTolerance::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Scale::Id: { // type is int8s
            MN::Scale::TypeInfo::Type value;
            UN::Scale::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_PRESSURE_MEASUREMENT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PressureMeasurementAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::PressureMeasurement;

    if (aPath.mClusterId != Clusters::PressureMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/PressureMeasurement/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void PressureMeasurementAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                          const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::PressureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::PressureMeasurement::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::PressureMeasurement::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::PressureMeasurement::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16s
    case MN::MeasuredValue::Id: {
        using T                = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinMeasuredValue::Id: {
        using T                = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MinMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                                                   MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxMeasuredValue::Id: {
        using T                = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MaxMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                                                   MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T                = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            UN::Tolerance::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is int16s
    case MN::ScaledValue::Id: {
        using T                = MN::ScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ScaledValue attribute value is %s", unify_value.dump().c_str());
            UN::ScaledValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::ScaledValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinScaledValue::Id: {
        using T                = MN::MinScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinScaledValue attribute value is %s", unify_value.dump().c_str());
            UN::MinScaledValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MinScaledValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxScaledValue::Id: {
        using T                = MN::MaxScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxScaledValue attribute value is %s", unify_value.dump().c_str());
            UN::MaxScaledValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MaxScaledValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::ScaledTolerance::Id: {
        using T                = MN::ScaledTolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ScaledTolerance attribute value is %s", unify_value.dump().c_str());
            UN::ScaledTolerance::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                                                   MN::ScaledTolerance::Id);
        }
        break;
    }
        // type is int8s
    case MN::Scale::Id: {
        using T                = MN::Scale::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Scale attribute value is %s", unify_value.dump().c_str());
            UN::Scale::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::Scale::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
FlowMeasurementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::FlowMeasurement::Attributes;
    namespace UN = unify::matter_bridge::FlowMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::FlowMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            UN::MeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            UN::MinMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            UN::MaxMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            UN::Tolerance::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_FLOW_MEASUREMENT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlowMeasurementAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::FlowMeasurement;

    if (aPath.mClusterId != Clusters::FlowMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/FlowMeasurement/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void FlowMeasurementAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                      const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::FlowMeasurement::Attributes;
    namespace UN = unify::matter_bridge::FlowMeasurement::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::FlowMeasurement::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::FlowMeasurement::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T                = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T                = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MinMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T                = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MaxMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T                = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            UN::Tolerance::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
RelativeHumidityMeasurementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::RelativeHumidityMeasurement::Attributes;
    namespace UN = unify::matter_bridge::RelativeHumidityMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::RelativeHumidityMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            UN::MeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            UN::MinMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            UN::MaxMeasuredValue::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            UN::Tolerance::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_RELATIVE_HUMIDITY_MEASUREMENT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR RelativeHumidityMeasurementAttributeAccess::Write(const ConcreteDataAttributePath & aPath,
                                                             AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::RelativeHumidityMeasurement;

    if (aPath.mClusterId != Clusters::RelativeHumidityMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/RelativeHumidityMeasurement/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void RelativeHumidityMeasurementAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                                  const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::RelativeHumidityMeasurement::Attributes;
    namespace UN = unify::matter_bridge::RelativeHumidityMeasurement::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::RelativeHumidityMeasurement::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T                = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                                                   MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T                = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MinMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                                                   MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T                = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            UN::MaxMeasuredValue::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                                                   MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T                = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            UN::Tolerance::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                                                   MN::Tolerance::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                                                   MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OccupancySensingAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::OccupancySensing::Attributes;
    namespace UN = unify::matter_bridge::OccupancySensing::Attributes;
    if (aPath.mClusterId != Clusters::OccupancySensing::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::Occupancy::Id: { // type is bitmap8
            MN::Occupancy::TypeInfo::Type value;
            UN::Occupancy::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OccupancySensorType::Id: { // type is enum8
            MN::OccupancySensorType::TypeInfo::Type value;
            UN::OccupancySensorType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OccupancySensorTypeBitmap::Id: { // type is bitmap8
            MN::OccupancySensorTypeBitmap::TypeInfo::Type value;
            UN::OccupancySensorTypeBitmap::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PirOccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::PirOccupiedToUnoccupiedDelay::TypeInfo::Type value;
            UN::PirOccupiedToUnoccupiedDelay::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PirUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::PirUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            UN::PirUnoccupiedToOccupiedDelay::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PirUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::PirUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            UN::PirUnoccupiedToOccupiedThreshold::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UltrasonicOccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::Type value;
            UN::UltrasonicOccupiedToUnoccupiedDelay::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UltrasonicUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            UN::UltrasonicUnoccupiedToOccupiedDelay::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::UltrasonicUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            UN::UltrasonicUnoccupiedToOccupiedThreshold::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PhysicalContactOccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::Type value;
            UN::PhysicalContactOccupiedToUnoccupiedDelay::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PhysicalContactUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            UN::PhysicalContactUnoccupiedToOccupiedDelay::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            UN::PhysicalContactUnoccupiedToOccupiedThreshold::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_OCCUPANCY_SENSING_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OccupancySensingAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::OccupancySensing;

    if (aPath.mClusterId != Clusters::OccupancySensing::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::PirOccupiedToUnoccupiedDelay::Id: {

        Attributes::PirOccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PirOccupiedToUnoccupiedDelay");
        break;
    }
    case Attributes::PirUnoccupiedToOccupiedDelay::Id: {

        Attributes::PirUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PirUnoccupiedToOccupiedDelay");
        break;
    }
    case Attributes::PirUnoccupiedToOccupiedThreshold::Id: {

        Attributes::PirUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PirUnoccupiedToOccupiedThreshold");
        break;
    }
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id: {

        Attributes::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("UltrasonicOccupiedToUnoccupiedDelay");
        break;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id: {

        Attributes::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("UltrasonicUnoccupiedToOccupiedDelay");
        break;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id: {

        Attributes::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("UltrasonicUnoccupiedToOccupiedThreshold");
        break;
    }
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id: {

        Attributes::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PhysicalContactOccupiedToUnoccupiedDelay");
        break;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id: {

        Attributes::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PhysicalContactUnoccupiedToOccupiedDelay");
        break;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {

        Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("PhysicalContactUnoccupiedToOccupiedThreshold");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/OccupancySensing/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void OccupancySensingAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                       const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::OccupancySensing::Attributes;
    namespace UN = unify::matter_bridge::OccupancySensing::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OccupancySensing::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::OccupancySensing::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap8
    case MN::Occupancy::Id: {
        std::optional<uint8_t> value = from_json_Occupancy(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Occupancy attribute value is %s", unify_value.dump().c_str());
            UN::Occupancy::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id, MN::Occupancy::Id);
        }
        break;
    }
        // type is enum8
    case MN::OccupancySensorType::Id: {
        using T                = ZclEnumOccupancySensorType;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OccupancySensorType attribute value is %s", unify_value.dump().c_str());
            UN::OccupancySensorType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::OccupancySensorType::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::OccupancySensorTypeBitmap::Id: {
        std::optional<uint8_t> value = from_json_OccupancySensorTypeBitmap(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OccupancySensorTypeBitmap attribute value is %s", unify_value.dump().c_str());
            UN::OccupancySensorTypeBitmap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::OccupancySensorTypeBitmap::Id);
        }
        break;
    }
        // type is int16u
    case MN::PirOccupiedToUnoccupiedDelay::Id: {
        using T                = MN::PirOccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PirOccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            UN::PirOccupiedToUnoccupiedDelay::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::PirOccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::PirUnoccupiedToOccupiedDelay::Id: {
        using T                = MN::PirUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PirUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            UN::PirUnoccupiedToOccupiedDelay::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::PirUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::PirUnoccupiedToOccupiedThreshold::Id: {
        using T                = MN::PirUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PirUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            UN::PirUnoccupiedToOccupiedThreshold::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::PirUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
        // type is int16u
    case MN::UltrasonicOccupiedToUnoccupiedDelay::Id: {
        using T                = MN::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UltrasonicOccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            UN::UltrasonicOccupiedToUnoccupiedDelay::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::UltrasonicOccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::UltrasonicUnoccupiedToOccupiedDelay::Id: {
        using T                = MN::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UltrasonicUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            UN::UltrasonicUnoccupiedToOccupiedDelay::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::UltrasonicUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::UltrasonicUnoccupiedToOccupiedThreshold::Id: {
        using T                = MN::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "UltrasonicUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            UN::UltrasonicUnoccupiedToOccupiedThreshold::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::UltrasonicUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalContactOccupiedToUnoccupiedDelay::Id: {
        using T                = MN::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PhysicalContactOccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            UN::PhysicalContactOccupiedToUnoccupiedDelay::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::PhysicalContactOccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalContactUnoccupiedToOccupiedDelay::Id: {
        using T                = MN::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PhysicalContactUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            UN::PhysicalContactUnoccupiedToOccupiedDelay::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::PhysicalContactUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {
        using T                = MN::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PhysicalContactUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            UN::PhysicalContactUnoccupiedToOccupiedThreshold::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                                                   MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
WakeOnLanAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::WakeOnLan::Attributes;
    namespace UN = unify::matter_bridge::WakeOnLan::Attributes;
    if (aPath.mClusterId != Clusters::WakeOnLan::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MACAddress::Id: { // type is char_string
            MN::MACAddress::TypeInfo::Type value;
            UN::MACAddress::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_WAKE_ON_LAN_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WakeOnLanAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::WakeOnLan;

    if (aPath.mClusterId != Clusters::WakeOnLan::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/WakeOnLan/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void WakeOnLanAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::WakeOnLan::Attributes;
    namespace UN = unify::matter_bridge::WakeOnLan::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::WakeOnLan::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::WakeOnLan::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is char_string
    case MN::MACAddress::Id: {
        using T                = MN::MACAddress::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MACAddress attribute value is %s", unify_value.dump().c_str());
            UN::MACAddress::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WakeOnLan::Id, MN::MACAddress::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WakeOnLan::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WakeOnLan::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ChannelAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::Channel::Attributes;
    namespace UN = unify::matter_bridge::Channel::Attributes;
    if (aPath.mClusterId != Clusters::Channel::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_CHANNEL_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChannelAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::Channel;

    if (aPath.mClusterId != Clusters::Channel::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/Channel/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ChannelAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                              const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::Channel::Attributes;
    namespace UN = unify::matter_bridge::Channel::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Channel::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Channel::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Channel::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Channel::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
TargetNavigatorAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::TargetNavigator::Attributes;
    namespace UN = unify::matter_bridge::TargetNavigator::Attributes;
    if (aPath.mClusterId != Clusters::TargetNavigator::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::CurrentTarget::Id: { // type is int8u
            MN::CurrentTarget::TypeInfo::Type value;
            UN::CurrentTarget::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_TARGET_NAVIGATOR_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TargetNavigatorAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::TargetNavigator;

    if (aPath.mClusterId != Clusters::TargetNavigator::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/TargetNavigator/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void TargetNavigatorAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                      const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::TargetNavigator::Attributes;
    namespace UN = unify::matter_bridge::TargetNavigator::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::TargetNavigator::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::TargetNavigator::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::CurrentTarget::Id: {
        using T                = MN::CurrentTarget::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentTarget attribute value is %s", unify_value.dump().c_str());
            UN::CurrentTarget::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TargetNavigator::Id, MN::CurrentTarget::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TargetNavigator::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TargetNavigator::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
MediaPlaybackAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::MediaPlayback::Attributes;
    namespace UN = unify::matter_bridge::MediaPlayback::Attributes;
    if (aPath.mClusterId != Clusters::MediaPlayback::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::CurrentState::Id: { // type is PlaybackStateEnum
            MN::CurrentState::TypeInfo::Type value;
            UN::CurrentState::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::StartTime::Id: { // type is epoch_us
            MN::StartTime::TypeInfo::Type value;
            UN::StartTime::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Duration::Id: { // type is int64u
            MN::Duration::TypeInfo::Type value;
            UN::Duration::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PlaybackSpeed::Id: { // type is single
            MN::PlaybackSpeed::TypeInfo::Type value;
            UN::PlaybackSpeed::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SeekRangeEnd::Id: { // type is int64u
            MN::SeekRangeEnd::TypeInfo::Type value;
            UN::SeekRangeEnd::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::SeekRangeStart::Id: { // type is int64u
            MN::SeekRangeStart::TypeInfo::Type value;
            UN::SeekRangeStart::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_MEDIA_PLAYBACK_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaPlaybackAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::MediaPlayback;

    if (aPath.mClusterId != Clusters::MediaPlayback::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/MediaPlayback/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void MediaPlaybackAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                    const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::MediaPlayback::Attributes;
    namespace UN = unify::matter_bridge::MediaPlayback::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::MediaPlayback::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::MediaPlayback::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is PlaybackStateEnum
    case MN::CurrentState::Id: {
        using T                = MN::CurrentState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentState attribute value is %s", unify_value.dump().c_str());
            UN::CurrentState::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::CurrentState::Id);
        }
        break;
    }
        // type is epoch_us
    case MN::StartTime::Id: {
        using T                = MN::StartTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "StartTime attribute value is %s", unify_value.dump().c_str());
            UN::StartTime::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::StartTime::Id);
        }
        break;
    }
        // type is int64u
    case MN::Duration::Id: {
        using T                = MN::Duration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Duration attribute value is %s", unify_value.dump().c_str());
            UN::Duration::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::Duration::Id);
        }
        break;
    }
        // type is single
    case MN::PlaybackSpeed::Id: {
        using T                = MN::PlaybackSpeed::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PlaybackSpeed attribute value is %s", unify_value.dump().c_str());
            UN::PlaybackSpeed::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::PlaybackSpeed::Id);
        }
        break;
    }
        // type is int64u
    case MN::SeekRangeEnd::Id: {
        using T                = MN::SeekRangeEnd::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SeekRangeEnd attribute value is %s", unify_value.dump().c_str());
            UN::SeekRangeEnd::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::SeekRangeEnd::Id);
        }
        break;
    }
        // type is int64u
    case MN::SeekRangeStart::Id: {
        using T                = MN::SeekRangeStart::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SeekRangeStart attribute value is %s", unify_value.dump().c_str());
            UN::SeekRangeStart::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::SeekRangeStart::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaPlayback::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
MediaInputAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::MediaInput::Attributes;
    namespace UN = unify::matter_bridge::MediaInput::Attributes;
    if (aPath.mClusterId != Clusters::MediaInput::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::CurrentInput::Id: { // type is int8u
            MN::CurrentInput::TypeInfo::Type value;
            UN::CurrentInput::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_MEDIA_INPUT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaInputAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::MediaInput;

    if (aPath.mClusterId != Clusters::MediaInput::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/MediaInput/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void MediaInputAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                 const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::MediaInput::Attributes;
    namespace UN = unify::matter_bridge::MediaInput::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::MediaInput::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::MediaInput::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::CurrentInput::Id: {
        using T                = MN::CurrentInput::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentInput attribute value is %s", unify_value.dump().c_str());
            UN::CurrentInput::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaInput::Id, MN::CurrentInput::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaInput::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::MediaInput::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
LowPowerAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::LowPower::Attributes;
    namespace UN = unify::matter_bridge::LowPower::Attributes;
    if (aPath.mClusterId != Clusters::LowPower::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_LOW_POWER_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LowPowerAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::LowPower;

    if (aPath.mClusterId != Clusters::LowPower::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/LowPower/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void LowPowerAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                               const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::LowPower::Attributes;
    namespace UN = unify::matter_bridge::LowPower::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::LowPower::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::LowPower::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LowPower::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LowPower::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
KeypadInputAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::KeypadInput::Attributes;
    namespace UN = unify::matter_bridge::KeypadInput::Attributes;
    if (aPath.mClusterId != Clusters::KeypadInput::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_KEYPAD_INPUT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeypadInputAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::KeypadInput;

    if (aPath.mClusterId != Clusters::KeypadInput::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/KeypadInput/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void KeypadInputAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                  const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::KeypadInput::Attributes;
    namespace UN = unify::matter_bridge::KeypadInput::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::KeypadInput::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::KeypadInput::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::KeypadInput::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::KeypadInput::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ContentLauncherAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ContentLauncher::Attributes;
    namespace UN = unify::matter_bridge::ContentLauncher::Attributes;
    if (aPath.mClusterId != Clusters::ContentLauncher::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::SupportedStreamingProtocols::Id: { // type is bitmap32
            MN::SupportedStreamingProtocols::TypeInfo::Type value;
            UN::SupportedStreamingProtocols::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_CONTENT_LAUNCHER_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ContentLauncherAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ContentLauncher;

    if (aPath.mClusterId != Clusters::ContentLauncher::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::SupportedStreamingProtocols::Id: {

        Attributes::SupportedStreamingProtocols::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("SupportedStreamingProtocols");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ContentLauncher/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ContentLauncherAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                      const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ContentLauncher::Attributes;
    namespace UN = unify::matter_bridge::ContentLauncher::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ContentLauncher::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ContentLauncher::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::SupportedStreamingProtocols::Id: {
        using T                = MN::SupportedStreamingProtocols::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "SupportedStreamingProtocols attribute value is %s", unify_value.dump().c_str());
            UN::SupportedStreamingProtocols::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ContentLauncher::Id,
                                                   MN::SupportedStreamingProtocols::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ContentLauncher::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ContentLauncher::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
AudioOutputAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::AudioOutput::Attributes;
    namespace UN = unify::matter_bridge::AudioOutput::Attributes;
    if (aPath.mClusterId != Clusters::AudioOutput::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::CurrentOutput::Id: { // type is int8u
            MN::CurrentOutput::TypeInfo::Type value;
            UN::CurrentOutput::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_AUDIO_OUTPUT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AudioOutputAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::AudioOutput;

    if (aPath.mClusterId != Clusters::AudioOutput::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/AudioOutput/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void AudioOutputAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                  const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::AudioOutput::Attributes;
    namespace UN = unify::matter_bridge::AudioOutput::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::AudioOutput::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::AudioOutput::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is int8u
    case MN::CurrentOutput::Id: {
        using T                = MN::CurrentOutput::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentOutput attribute value is %s", unify_value.dump().c_str());
            UN::CurrentOutput::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::AudioOutput::Id, MN::CurrentOutput::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::AudioOutput::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::AudioOutput::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ApplicationLauncherAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ApplicationLauncher::Attributes;
    namespace UN = unify::matter_bridge::ApplicationLauncher::Attributes;
    if (aPath.mClusterId != Clusters::ApplicationLauncher::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_APPLICATION_LAUNCHER_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplicationLauncherAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ApplicationLauncher;

    if (aPath.mClusterId != Clusters::ApplicationLauncher::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::CurrentApp::Id: {

        Attributes::CurrentApp::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("CurrentApp");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ApplicationLauncher/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ApplicationLauncherAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                          const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ApplicationLauncher::Attributes;
    namespace UN = unify::matter_bridge::ApplicationLauncher::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ApplicationLauncher::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ApplicationLauncher::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationLauncher::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationLauncher::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ApplicationBasicAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ApplicationBasic::Attributes;
    namespace UN = unify::matter_bridge::ApplicationBasic::Attributes;
    if (aPath.mClusterId != Clusters::ApplicationBasic::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::VendorName::Id: { // type is char_string
            MN::VendorName::TypeInfo::Type value;
            UN::VendorName::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::VendorID::Id: { // type is vendor_id
            MN::VendorID::TypeInfo::Type value;
            UN::VendorID::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ApplicationName::Id: { // type is char_string
            MN::ApplicationName::TypeInfo::Type value;
            UN::ApplicationName::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ProductID::Id: { // type is int16u
            MN::ProductID::TypeInfo::Type value;
            UN::ProductID::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Status::Id: { // type is ApplicationStatusEnum
            MN::Status::TypeInfo::Type value;
            UN::Status::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ApplicationVersion::Id: { // type is char_string
            MN::ApplicationVersion::TypeInfo::Type value;
            UN::ApplicationVersion::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_APPLICATION_BASIC_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplicationBasicAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ApplicationBasic;

    if (aPath.mClusterId != Clusters::ApplicationBasic::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ApplicationBasic/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ApplicationBasicAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                       const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ApplicationBasic::Attributes;
    namespace UN = unify::matter_bridge::ApplicationBasic::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ApplicationBasic::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ApplicationBasic::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is char_string
    case MN::VendorName::Id: {
        using T                = MN::VendorName::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "VendorName attribute value is %s", unify_value.dump().c_str());
            UN::VendorName::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id, MN::VendorName::Id);
        }
        break;
    }
        // type is vendor_id
    case MN::VendorID::Id: {
        using T                = MN::VendorID::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "VendorID attribute value is %s", unify_value.dump().c_str());
            UN::VendorID::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id, MN::VendorID::Id);
        }
        break;
    }
        // type is char_string
    case MN::ApplicationName::Id: {
        using T                = MN::ApplicationName::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ApplicationName attribute value is %s", unify_value.dump().c_str());
            UN::ApplicationName::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id, MN::ApplicationName::Id);
        }
        break;
    }
        // type is int16u
    case MN::ProductID::Id: {
        using T                = MN::ProductID::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ProductID attribute value is %s", unify_value.dump().c_str());
            UN::ProductID::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id, MN::ProductID::Id);
        }
        break;
    }
        // type is ApplicationStatusEnum
    case MN::Status::Id: {
        using T                = MN::Status::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Status attribute value is %s", unify_value.dump().c_str());
            UN::Status::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id, MN::Status::Id);
        }
        break;
    }
        // type is char_string
    case MN::ApplicationVersion::Id: {
        using T                = MN::ApplicationVersion::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ApplicationVersion attribute value is %s", unify_value.dump().c_str());
            UN::ApplicationVersion::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id,
                                                   MN::ApplicationVersion::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ApplicationBasic::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
AccountLoginAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::AccountLogin::Attributes;
    namespace UN = unify::matter_bridge::AccountLogin::Attributes;
    if (aPath.mClusterId != Clusters::AccountLogin::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_ACCOUNT_LOGIN_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR AccountLoginAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::AccountLogin;

    if (aPath.mClusterId != Clusters::AccountLogin::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/AccountLogin/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void AccountLoginAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                   const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::AccountLogin::Attributes;
    namespace UN = unify::matter_bridge::AccountLogin::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::AccountLogin::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::AccountLogin::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::AccountLogin::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::AccountLogin::Id, MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ElectricalMeasurementAttributeAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    namespace MN = chip::app::Clusters::ElectricalMeasurement::Attributes;
    namespace UN = unify::matter_bridge::ElectricalMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::ElectricalMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    try
    {
        switch (aPath.mAttributeId)
        {
        case MN::MeasurementType::Id: { // type is bitmap32
            MN::MeasurementType::TypeInfo::Type value;
            UN::MeasurementType::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcVoltage::Id: { // type is int16s
            MN::DcVoltage::TypeInfo::Type value;
            UN::DcVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcVoltageMin::Id: { // type is int16s
            MN::DcVoltageMin::TypeInfo::Type value;
            UN::DcVoltageMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcVoltageMax::Id: { // type is int16s
            MN::DcVoltageMax::TypeInfo::Type value;
            UN::DcVoltageMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcCurrent::Id: { // type is int16s
            MN::DcCurrent::TypeInfo::Type value;
            UN::DcCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcCurrentMin::Id: { // type is int16s
            MN::DcCurrentMin::TypeInfo::Type value;
            UN::DcCurrentMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcCurrentMax::Id: { // type is int16s
            MN::DcCurrentMax::TypeInfo::Type value;
            UN::DcCurrentMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcPower::Id: { // type is int16s
            MN::DcPower::TypeInfo::Type value;
            UN::DcPower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcPowerMin::Id: { // type is int16s
            MN::DcPowerMin::TypeInfo::Type value;
            UN::DcPowerMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcPowerMax::Id: { // type is int16s
            MN::DcPowerMax::TypeInfo::Type value;
            UN::DcPowerMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcVoltageMultiplier::Id: { // type is int16u
            MN::DcVoltageMultiplier::TypeInfo::Type value;
            UN::DcVoltageMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcVoltageDivisor::Id: { // type is int16u
            MN::DcVoltageDivisor::TypeInfo::Type value;
            UN::DcVoltageDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcCurrentMultiplier::Id: { // type is int16u
            MN::DcCurrentMultiplier::TypeInfo::Type value;
            UN::DcCurrentMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcCurrentDivisor::Id: { // type is int16u
            MN::DcCurrentDivisor::TypeInfo::Type value;
            UN::DcCurrentDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcPowerMultiplier::Id: { // type is int16u
            MN::DcPowerMultiplier::TypeInfo::Type value;
            UN::DcPowerMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::DcPowerDivisor::Id: { // type is int16u
            MN::DcPowerDivisor::TypeInfo::Type value;
            UN::DcPowerDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcFrequency::Id: { // type is int16u
            MN::AcFrequency::TypeInfo::Type value;
            UN::AcFrequency::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcFrequencyMin::Id: { // type is int16u
            MN::AcFrequencyMin::TypeInfo::Type value;
            UN::AcFrequencyMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcFrequencyMax::Id: { // type is int16u
            MN::AcFrequencyMax::TypeInfo::Type value;
            UN::AcFrequencyMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::NeutralCurrent::Id: { // type is int16u
            MN::NeutralCurrent::TypeInfo::Type value;
            UN::NeutralCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TotalActivePower::Id: { // type is int32s
            MN::TotalActivePower::TypeInfo::Type value;
            UN::TotalActivePower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TotalReactivePower::Id: { // type is int32s
            MN::TotalReactivePower::TypeInfo::Type value;
            UN::TotalReactivePower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::TotalApparentPower::Id: { // type is int32u
            MN::TotalApparentPower::TypeInfo::Type value;
            UN::TotalApparentPower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Measured1stHarmonicCurrent::Id: { // type is int16s
            MN::Measured1stHarmonicCurrent::TypeInfo::Type value;
            UN::Measured1stHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Measured3rdHarmonicCurrent::Id: { // type is int16s
            MN::Measured3rdHarmonicCurrent::TypeInfo::Type value;
            UN::Measured3rdHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Measured5thHarmonicCurrent::Id: { // type is int16s
            MN::Measured5thHarmonicCurrent::TypeInfo::Type value;
            UN::Measured5thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Measured7thHarmonicCurrent::Id: { // type is int16s
            MN::Measured7thHarmonicCurrent::TypeInfo::Type value;
            UN::Measured7thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Measured9thHarmonicCurrent::Id: { // type is int16s
            MN::Measured9thHarmonicCurrent::TypeInfo::Type value;
            UN::Measured9thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::Measured11thHarmonicCurrent::Id: { // type is int16s
            MN::Measured11thHarmonicCurrent::TypeInfo::Type value;
            UN::Measured11thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MeasuredPhase1stHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase1stHarmonicCurrent::TypeInfo::Type value;
            UN::MeasuredPhase1stHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MeasuredPhase3rdHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase3rdHarmonicCurrent::TypeInfo::Type value;
            UN::MeasuredPhase3rdHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MeasuredPhase5thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase5thHarmonicCurrent::TypeInfo::Type value;
            UN::MeasuredPhase5thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MeasuredPhase7thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase7thHarmonicCurrent::TypeInfo::Type value;
            UN::MeasuredPhase7thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MeasuredPhase9thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase9thHarmonicCurrent::TypeInfo::Type value;
            UN::MeasuredPhase9thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::MeasuredPhase11thHarmonicCurrent::Id: { // type is int16s
            MN::MeasuredPhase11thHarmonicCurrent::TypeInfo::Type value;
            UN::MeasuredPhase11thHarmonicCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcFrequencyMultiplier::Id: { // type is int16u
            MN::AcFrequencyMultiplier::TypeInfo::Type value;
            UN::AcFrequencyMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcFrequencyDivisor::Id: { // type is int16u
            MN::AcFrequencyDivisor::TypeInfo::Type value;
            UN::AcFrequencyDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PowerMultiplier::Id: { // type is int32u
            MN::PowerMultiplier::TypeInfo::Type value;
            UN::PowerMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PowerDivisor::Id: { // type is int32u
            MN::PowerDivisor::TypeInfo::Type value;
            UN::PowerDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::HarmonicCurrentMultiplier::Id: { // type is int8s
            MN::HarmonicCurrentMultiplier::TypeInfo::Type value;
            UN::HarmonicCurrentMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PhaseHarmonicCurrentMultiplier::Id: { // type is int8s
            MN::PhaseHarmonicCurrentMultiplier::TypeInfo::Type value;
            UN::PhaseHarmonicCurrentMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstantaneousVoltage::Id: { // type is int16s
            MN::InstantaneousVoltage::TypeInfo::Type value;
            UN::InstantaneousVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstantaneousLineCurrent::Id: { // type is int16u
            MN::InstantaneousLineCurrent::TypeInfo::Type value;
            UN::InstantaneousLineCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstantaneousActiveCurrent::Id: { // type is int16s
            MN::InstantaneousActiveCurrent::TypeInfo::Type value;
            UN::InstantaneousActiveCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstantaneousReactiveCurrent::Id: { // type is int16s
            MN::InstantaneousReactiveCurrent::TypeInfo::Type value;
            UN::InstantaneousReactiveCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::InstantaneousPower::Id: { // type is int16s
            MN::InstantaneousPower::TypeInfo::Type value;
            UN::InstantaneousPower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltage::Id: { // type is int16u
            MN::RmsVoltage::TypeInfo::Type value;
            UN::RmsVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageMin::Id: { // type is int16u
            MN::RmsVoltageMin::TypeInfo::Type value;
            UN::RmsVoltageMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageMax::Id: { // type is int16u
            MN::RmsVoltageMax::TypeInfo::Type value;
            UN::RmsVoltageMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrent::Id: { // type is int16u
            MN::RmsCurrent::TypeInfo::Type value;
            UN::RmsCurrent::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentMin::Id: { // type is int16u
            MN::RmsCurrentMin::TypeInfo::Type value;
            UN::RmsCurrentMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentMax::Id: { // type is int16u
            MN::RmsCurrentMax::TypeInfo::Type value;
            UN::RmsCurrentMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePower::Id: { // type is int16s
            MN::ActivePower::TypeInfo::Type value;
            UN::ActivePower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerMin::Id: { // type is int16s
            MN::ActivePowerMin::TypeInfo::Type value;
            UN::ActivePowerMin::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerMax::Id: { // type is int16s
            MN::ActivePowerMax::TypeInfo::Type value;
            UN::ActivePowerMax::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ReactivePower::Id: { // type is int16s
            MN::ReactivePower::TypeInfo::Type value;
            UN::ReactivePower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ApparentPower::Id: { // type is int16u
            MN::ApparentPower::TypeInfo::Type value;
            UN::ApparentPower::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PowerFactor::Id: { // type is int8s
            MN::PowerFactor::TypeInfo::Type value;
            UN::PowerFactor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsVoltageMeasurementPeriod::Id: { // type is int16u
            MN::AverageRmsVoltageMeasurementPeriod::TypeInfo::Type value;
            UN::AverageRmsVoltageMeasurementPeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsUnderVoltageCounter::Id: { // type is int16u
            MN::AverageRmsUnderVoltageCounter::TypeInfo::Type value;
            UN::AverageRmsUnderVoltageCounter::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeOverVoltagePeriod::Id: { // type is int16u
            MN::RmsExtremeOverVoltagePeriod::TypeInfo::Type value;
            UN::RmsExtremeOverVoltagePeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeUnderVoltagePeriod::Id: { // type is int16u
            MN::RmsExtremeUnderVoltagePeriod::TypeInfo::Type value;
            UN::RmsExtremeUnderVoltagePeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSagPeriod::Id: { // type is int16u
            MN::RmsVoltageSagPeriod::TypeInfo::Type value;
            UN::RmsVoltageSagPeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSwellPeriod::Id: { // type is int16u
            MN::RmsVoltageSwellPeriod::TypeInfo::Type value;
            UN::RmsVoltageSwellPeriod::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcVoltageMultiplier::Id: { // type is int16u
            MN::AcVoltageMultiplier::TypeInfo::Type value;
            UN::AcVoltageMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcVoltageDivisor::Id: { // type is int16u
            MN::AcVoltageDivisor::TypeInfo::Type value;
            UN::AcVoltageDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcCurrentMultiplier::Id: { // type is int16u
            MN::AcCurrentMultiplier::TypeInfo::Type value;
            UN::AcCurrentMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcCurrentDivisor::Id: { // type is int16u
            MN::AcCurrentDivisor::TypeInfo::Type value;
            UN::AcCurrentDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcPowerMultiplier::Id: { // type is int16u
            MN::AcPowerMultiplier::TypeInfo::Type value;
            UN::AcPowerMultiplier::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcPowerDivisor::Id: { // type is int16u
            MN::AcPowerDivisor::TypeInfo::Type value;
            UN::AcPowerDivisor::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::OverloadAlarmsMask::Id: { // type is bitmap8
            MN::OverloadAlarmsMask::TypeInfo::Type value;
            UN::OverloadAlarmsMask::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::VoltageOverload::Id: { // type is int16s
            MN::VoltageOverload::TypeInfo::Type value;
            UN::VoltageOverload::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::CurrentOverload::Id: { // type is int16s
            MN::CurrentOverload::TypeInfo::Type value;
            UN::CurrentOverload::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcOverloadAlarmsMask::Id: { // type is bitmap16
            MN::AcOverloadAlarmsMask::TypeInfo::Type value;
            UN::AcOverloadAlarmsMask::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcVoltageOverload::Id: { // type is int16s
            MN::AcVoltageOverload::TypeInfo::Type value;
            UN::AcVoltageOverload::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcCurrentOverload::Id: { // type is int16s
            MN::AcCurrentOverload::TypeInfo::Type value;
            UN::AcCurrentOverload::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcActivePowerOverload::Id: { // type is int16s
            MN::AcActivePowerOverload::TypeInfo::Type value;
            UN::AcActivePowerOverload::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AcReactivePowerOverload::Id: { // type is int16s
            MN::AcReactivePowerOverload::TypeInfo::Type value;
            UN::AcReactivePowerOverload::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsOverVoltage::Id: { // type is int16s
            MN::AverageRmsOverVoltage::TypeInfo::Type value;
            UN::AverageRmsOverVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsUnderVoltage::Id: { // type is int16s
            MN::AverageRmsUnderVoltage::TypeInfo::Type value;
            UN::AverageRmsUnderVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeOverVoltage::Id: { // type is int16s
            MN::RmsExtremeOverVoltage::TypeInfo::Type value;
            UN::RmsExtremeOverVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeUnderVoltage::Id: { // type is int16s
            MN::RmsExtremeUnderVoltage::TypeInfo::Type value;
            UN::RmsExtremeUnderVoltage::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSag::Id: { // type is int16s
            MN::RmsVoltageSag::TypeInfo::Type value;
            UN::RmsVoltageSag::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSwell::Id: { // type is int16s
            MN::RmsVoltageSwell::TypeInfo::Type value;
            UN::RmsVoltageSwell::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LineCurrentPhaseB::Id: { // type is int16u
            MN::LineCurrentPhaseB::TypeInfo::Type value;
            UN::LineCurrentPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActiveCurrentPhaseB::Id: { // type is int16s
            MN::ActiveCurrentPhaseB::TypeInfo::Type value;
            UN::ActiveCurrentPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ReactiveCurrentPhaseB::Id: { // type is int16s
            MN::ReactiveCurrentPhaseB::TypeInfo::Type value;
            UN::ReactiveCurrentPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltagePhaseB::Id: { // type is int16u
            MN::RmsVoltagePhaseB::TypeInfo::Type value;
            UN::RmsVoltagePhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageMinPhaseB::Id: { // type is int16u
            MN::RmsVoltageMinPhaseB::TypeInfo::Type value;
            UN::RmsVoltageMinPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageMaxPhaseB::Id: { // type is int16u
            MN::RmsVoltageMaxPhaseB::TypeInfo::Type value;
            UN::RmsVoltageMaxPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentPhaseB::Id: { // type is int16u
            MN::RmsCurrentPhaseB::TypeInfo::Type value;
            UN::RmsCurrentPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentMinPhaseB::Id: { // type is int16u
            MN::RmsCurrentMinPhaseB::TypeInfo::Type value;
            UN::RmsCurrentMinPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentMaxPhaseB::Id: { // type is int16u
            MN::RmsCurrentMaxPhaseB::TypeInfo::Type value;
            UN::RmsCurrentMaxPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerPhaseB::Id: { // type is int16s
            MN::ActivePowerPhaseB::TypeInfo::Type value;
            UN::ActivePowerPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerMinPhaseB::Id: { // type is int16s
            MN::ActivePowerMinPhaseB::TypeInfo::Type value;
            UN::ActivePowerMinPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerMaxPhaseB::Id: { // type is int16s
            MN::ActivePowerMaxPhaseB::TypeInfo::Type value;
            UN::ActivePowerMaxPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ReactivePowerPhaseB::Id: { // type is int16s
            MN::ReactivePowerPhaseB::TypeInfo::Type value;
            UN::ReactivePowerPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ApparentPowerPhaseB::Id: { // type is int16u
            MN::ApparentPowerPhaseB::TypeInfo::Type value;
            UN::ApparentPowerPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PowerFactorPhaseB::Id: { // type is int8s
            MN::PowerFactorPhaseB::TypeInfo::Type value;
            UN::PowerFactorPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsVoltageMeasurementPeriodPhaseB::Id: { // type is int16u
            MN::AverageRmsVoltageMeasurementPeriodPhaseB::TypeInfo::Type value;
            UN::AverageRmsVoltageMeasurementPeriodPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsOverVoltageCounterPhaseB::Id: { // type is int16u
            MN::AverageRmsOverVoltageCounterPhaseB::TypeInfo::Type value;
            UN::AverageRmsOverVoltageCounterPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsUnderVoltageCounterPhaseB::Id: { // type is int16u
            MN::AverageRmsUnderVoltageCounterPhaseB::TypeInfo::Type value;
            UN::AverageRmsUnderVoltageCounterPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeOverVoltagePeriodPhaseB::Id: { // type is int16u
            MN::RmsExtremeOverVoltagePeriodPhaseB::TypeInfo::Type value;
            UN::RmsExtremeOverVoltagePeriodPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeUnderVoltagePeriodPhaseB::Id: { // type is int16u
            MN::RmsExtremeUnderVoltagePeriodPhaseB::TypeInfo::Type value;
            UN::RmsExtremeUnderVoltagePeriodPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSagPeriodPhaseB::Id: { // type is int16u
            MN::RmsVoltageSagPeriodPhaseB::TypeInfo::Type value;
            UN::RmsVoltageSagPeriodPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSwellPeriodPhaseB::Id: { // type is int16u
            MN::RmsVoltageSwellPeriodPhaseB::TypeInfo::Type value;
            UN::RmsVoltageSwellPeriodPhaseB::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::LineCurrentPhaseC::Id: { // type is int16u
            MN::LineCurrentPhaseC::TypeInfo::Type value;
            UN::LineCurrentPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActiveCurrentPhaseC::Id: { // type is int16s
            MN::ActiveCurrentPhaseC::TypeInfo::Type value;
            UN::ActiveCurrentPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ReactiveCurrentPhaseC::Id: { // type is int16s
            MN::ReactiveCurrentPhaseC::TypeInfo::Type value;
            UN::ReactiveCurrentPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltagePhaseC::Id: { // type is int16u
            MN::RmsVoltagePhaseC::TypeInfo::Type value;
            UN::RmsVoltagePhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageMinPhaseC::Id: { // type is int16u
            MN::RmsVoltageMinPhaseC::TypeInfo::Type value;
            UN::RmsVoltageMinPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageMaxPhaseC::Id: { // type is int16u
            MN::RmsVoltageMaxPhaseC::TypeInfo::Type value;
            UN::RmsVoltageMaxPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentPhaseC::Id: { // type is int16u
            MN::RmsCurrentPhaseC::TypeInfo::Type value;
            UN::RmsCurrentPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentMinPhaseC::Id: { // type is int16u
            MN::RmsCurrentMinPhaseC::TypeInfo::Type value;
            UN::RmsCurrentMinPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsCurrentMaxPhaseC::Id: { // type is int16u
            MN::RmsCurrentMaxPhaseC::TypeInfo::Type value;
            UN::RmsCurrentMaxPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerPhaseC::Id: { // type is int16s
            MN::ActivePowerPhaseC::TypeInfo::Type value;
            UN::ActivePowerPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerMinPhaseC::Id: { // type is int16s
            MN::ActivePowerMinPhaseC::TypeInfo::Type value;
            UN::ActivePowerMinPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ActivePowerMaxPhaseC::Id: { // type is int16s
            MN::ActivePowerMaxPhaseC::TypeInfo::Type value;
            UN::ActivePowerMaxPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ReactivePowerPhaseC::Id: { // type is int16s
            MN::ReactivePowerPhaseC::TypeInfo::Type value;
            UN::ReactivePowerPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::ApparentPowerPhaseC::Id: { // type is int16u
            MN::ApparentPowerPhaseC::TypeInfo::Type value;
            UN::ApparentPowerPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::PowerFactorPhaseC::Id: { // type is int8s
            MN::PowerFactorPhaseC::TypeInfo::Type value;
            UN::PowerFactorPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsVoltageMeasurementPeriodPhaseC::Id: { // type is int16u
            MN::AverageRmsVoltageMeasurementPeriodPhaseC::TypeInfo::Type value;
            UN::AverageRmsVoltageMeasurementPeriodPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsOverVoltageCounterPhaseC::Id: { // type is int16u
            MN::AverageRmsOverVoltageCounterPhaseC::TypeInfo::Type value;
            UN::AverageRmsOverVoltageCounterPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::AverageRmsUnderVoltageCounterPhaseC::Id: { // type is int16u
            MN::AverageRmsUnderVoltageCounterPhaseC::TypeInfo::Type value;
            UN::AverageRmsUnderVoltageCounterPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeOverVoltagePeriodPhaseC::Id: { // type is int16u
            MN::RmsExtremeOverVoltagePeriodPhaseC::TypeInfo::Type value;
            UN::RmsExtremeOverVoltagePeriodPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsExtremeUnderVoltagePeriodPhaseC::Id: { // type is int16u
            MN::RmsExtremeUnderVoltagePeriodPhaseC::TypeInfo::Type value;
            UN::RmsExtremeUnderVoltagePeriodPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSagPeriodPhaseC::Id: { // type is int16u
            MN::RmsVoltageSagPeriodPhaseC::TypeInfo::Type value;
            UN::RmsVoltageSagPeriodPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::RmsVoltageSwellPeriodPhaseC::Id: { // type is int16u
            MN::RmsVoltageSwellPeriodPhaseC::TypeInfo::Type value;
            UN::RmsVoltageSwellPeriodPhaseC::Get(atr_path, value);
            return aEncoder.Encode(value);
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            value = get_feature_map_settings(aPath);
            return aEncoder.Encode(value);
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            value = ZCL_ELECTRICAL_MEASUREMENT_REVISION;
            return aEncoder.Encode(value);
        }
        }
    } catch (const std::out_of_range & e)
    {
        sl_log_info(LOG_TAG,
                    "The request attribute Path for endpoint [%i] is not found in the attribute state "
                    "container: %s\n",
                    atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalMeasurementAttributeAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    using namespace chip::app::Clusters::ElectricalMeasurement;

    if (aPath.mClusterId != Clusters::ElectricalMeasurement::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    std::string attribute_name;
    nlohmann::json jsn;

    switch (aPath.mAttributeId)
    {
    case Attributes::AverageRmsVoltageMeasurementPeriod::Id: {

        Attributes::AverageRmsVoltageMeasurementPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("AverageRmsVoltageMeasurementPeriod");
        break;
    }
    case Attributes::AverageRmsUnderVoltageCounter::Id: {

        Attributes::AverageRmsUnderVoltageCounter::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("AverageRmsUnderVoltageCounter");
        break;
    }
    case Attributes::RmsExtremeOverVoltagePeriod::Id: {

        Attributes::RmsExtremeOverVoltagePeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("RmsExtremeOverVoltagePeriod");
        break;
    }
    case Attributes::RmsExtremeUnderVoltagePeriod::Id: {

        Attributes::RmsExtremeUnderVoltagePeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("RmsExtremeUnderVoltagePeriod");
        break;
    }
    case Attributes::RmsVoltageSagPeriod::Id: {

        Attributes::RmsVoltageSagPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("RmsVoltageSagPeriod");
        break;
    }
    case Attributes::RmsVoltageSwellPeriod::Id: {

        Attributes::RmsVoltageSwellPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("RmsVoltageSwellPeriod");
        break;
    }
    case Attributes::OverloadAlarmsMask::Id: {

        Attributes::OverloadAlarmsMask::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("OverloadAlarmsMask");
        break;
    }
    case Attributes::AcOverloadAlarmsMask::Id: {

        Attributes::AcOverloadAlarmsMask::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["value"]   = to_json(value);
        attribute_name = std::string("AcOverloadAlarmsMask");
        break;
    }
    }

    if (!attribute_name.empty())
    {
        std::string payload_str;
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) +
            "/ElectricalMeasurement/Attributes/" + attribute_name + "/Desired";
        payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    return CHIP_NO_ERROR;
}

void ElectricalMeasurementAttributeAccess::reported_updated(const bridged_endpoint * ep, const std::string & cluster,
                                                            const std::string & attribute, const nlohmann::json & unify_value)
{
    namespace MN = chip::app::Clusters::ElectricalMeasurement::Attributes;
    namespace UN = unify::matter_bridge::ElectricalMeasurement::Attributes;

    auto cluster_id = device_translator::instance().get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ElectricalMeasurement::Id))
    {
        return;
    }

    // get attribute id
    auto attribute_id = device_translator::instance().get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value())
    {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath =
        ConcreteAttributePath(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, attribute_id.value());
    switch (attribute_id.value())
    {
    // type is bitmap32
    case MN::MeasurementType::Id: {
        using T                = MN::MeasurementType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasurementType attribute value is %s", unify_value.dump().c_str());
            UN::MeasurementType::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::MeasurementType::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcVoltage::Id: {
        using T                = MN::DcVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcVoltage attribute value is %s", unify_value.dump().c_str());
            UN::DcVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcVoltageMin::Id: {
        using T                = MN::DcVoltageMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcVoltageMin attribute value is %s", unify_value.dump().c_str());
            UN::DcVoltageMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcVoltageMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcVoltageMax::Id: {
        using T                = MN::DcVoltageMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcVoltageMax attribute value is %s", unify_value.dump().c_str());
            UN::DcVoltageMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcVoltageMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcCurrent::Id: {
        using T                = MN::DcCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcCurrent attribute value is %s", unify_value.dump().c_str());
            UN::DcCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcCurrentMin::Id: {
        using T                = MN::DcCurrentMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcCurrentMin attribute value is %s", unify_value.dump().c_str());
            UN::DcCurrentMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcCurrentMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcCurrentMax::Id: {
        using T                = MN::DcCurrentMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcCurrentMax attribute value is %s", unify_value.dump().c_str());
            UN::DcCurrentMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcCurrentMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcPower::Id: {
        using T                = MN::DcPower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcPower attribute value is %s", unify_value.dump().c_str());
            UN::DcPower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcPower::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcPowerMin::Id: {
        using T                = MN::DcPowerMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcPowerMin attribute value is %s", unify_value.dump().c_str());
            UN::DcPowerMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcPowerMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::DcPowerMax::Id: {
        using T                = MN::DcPowerMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcPowerMax attribute value is %s", unify_value.dump().c_str());
            UN::DcPowerMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::DcPowerMax::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcVoltageMultiplier::Id: {
        using T                = MN::DcVoltageMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcVoltageMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::DcVoltageMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::DcVoltageMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcVoltageDivisor::Id: {
        using T                = MN::DcVoltageDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcVoltageDivisor attribute value is %s", unify_value.dump().c_str());
            UN::DcVoltageDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::DcVoltageDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcCurrentMultiplier::Id: {
        using T                = MN::DcCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::DcCurrentMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::DcCurrentMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcCurrentDivisor::Id: {
        using T                = MN::DcCurrentDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcCurrentDivisor attribute value is %s", unify_value.dump().c_str());
            UN::DcCurrentDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::DcCurrentDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcPowerMultiplier::Id: {
        using T                = MN::DcPowerMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcPowerMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::DcPowerMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::DcPowerMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::DcPowerDivisor::Id: {
        using T                = MN::DcPowerDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "DcPowerDivisor attribute value is %s", unify_value.dump().c_str());
            UN::DcPowerDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::DcPowerDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequency::Id: {
        using T                = MN::AcFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcFrequency attribute value is %s", unify_value.dump().c_str());
            UN::AcFrequency::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::AcFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyMin::Id: {
        using T                = MN::AcFrequencyMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcFrequencyMin attribute value is %s", unify_value.dump().c_str());
            UN::AcFrequencyMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcFrequencyMin::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyMax::Id: {
        using T                = MN::AcFrequencyMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcFrequencyMax attribute value is %s", unify_value.dump().c_str());
            UN::AcFrequencyMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcFrequencyMax::Id);
        }
        break;
    }
        // type is int16u
    case MN::NeutralCurrent::Id: {
        using T                = MN::NeutralCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "NeutralCurrent attribute value is %s", unify_value.dump().c_str());
            UN::NeutralCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::NeutralCurrent::Id);
        }
        break;
    }
        // type is int32s
    case MN::TotalActivePower::Id: {
        using T                = MN::TotalActivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TotalActivePower attribute value is %s", unify_value.dump().c_str());
            UN::TotalActivePower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::TotalActivePower::Id);
        }
        break;
    }
        // type is int32s
    case MN::TotalReactivePower::Id: {
        using T                = MN::TotalReactivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TotalReactivePower attribute value is %s", unify_value.dump().c_str());
            UN::TotalReactivePower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::TotalReactivePower::Id);
        }
        break;
    }
        // type is int32u
    case MN::TotalApparentPower::Id: {
        using T                = MN::TotalApparentPower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "TotalApparentPower attribute value is %s", unify_value.dump().c_str());
            UN::TotalApparentPower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::TotalApparentPower::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured1stHarmonicCurrent::Id: {
        using T                = MN::Measured1stHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Measured1stHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::Measured1stHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::Measured1stHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured3rdHarmonicCurrent::Id: {
        using T                = MN::Measured3rdHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Measured3rdHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::Measured3rdHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::Measured3rdHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured5thHarmonicCurrent::Id: {
        using T                = MN::Measured5thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Measured5thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::Measured5thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::Measured5thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured7thHarmonicCurrent::Id: {
        using T                = MN::Measured7thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Measured7thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::Measured7thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::Measured7thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured9thHarmonicCurrent::Id: {
        using T                = MN::Measured9thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Measured9thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::Measured9thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::Measured9thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::Measured11thHarmonicCurrent::Id: {
        using T                = MN::Measured11thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "Measured11thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::Measured11thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::Measured11thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase1stHarmonicCurrent::Id: {
        using T                = MN::MeasuredPhase1stHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredPhase1stHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredPhase1stHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::MeasuredPhase1stHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase3rdHarmonicCurrent::Id: {
        using T                = MN::MeasuredPhase3rdHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredPhase3rdHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredPhase3rdHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::MeasuredPhase3rdHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase5thHarmonicCurrent::Id: {
        using T                = MN::MeasuredPhase5thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredPhase5thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredPhase5thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::MeasuredPhase5thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase7thHarmonicCurrent::Id: {
        using T                = MN::MeasuredPhase7thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredPhase7thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredPhase7thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::MeasuredPhase7thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase9thHarmonicCurrent::Id: {
        using T                = MN::MeasuredPhase9thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredPhase9thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredPhase9thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::MeasuredPhase9thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::MeasuredPhase11thHarmonicCurrent::Id: {
        using T                = MN::MeasuredPhase11thHarmonicCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "MeasuredPhase11thHarmonicCurrent attribute value is %s", unify_value.dump().c_str());
            UN::MeasuredPhase11thHarmonicCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::MeasuredPhase11thHarmonicCurrent::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyMultiplier::Id: {
        using T                = MN::AcFrequencyMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcFrequencyMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::AcFrequencyMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcFrequencyMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcFrequencyDivisor::Id: {
        using T                = MN::AcFrequencyDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcFrequencyDivisor attribute value is %s", unify_value.dump().c_str());
            UN::AcFrequencyDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcFrequencyDivisor::Id);
        }
        break;
    }
        // type is int32u
    case MN::PowerMultiplier::Id: {
        using T                = MN::PowerMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PowerMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::PowerMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::PowerMultiplier::Id);
        }
        break;
    }
        // type is int32u
    case MN::PowerDivisor::Id: {
        using T                = MN::PowerDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PowerDivisor attribute value is %s", unify_value.dump().c_str());
            UN::PowerDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::PowerDivisor::Id);
        }
        break;
    }
        // type is int8s
    case MN::HarmonicCurrentMultiplier::Id: {
        using T                = MN::HarmonicCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "HarmonicCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::HarmonicCurrentMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::HarmonicCurrentMultiplier::Id);
        }
        break;
    }
        // type is int8s
    case MN::PhaseHarmonicCurrentMultiplier::Id: {
        using T                = MN::PhaseHarmonicCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PhaseHarmonicCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::PhaseHarmonicCurrentMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::PhaseHarmonicCurrentMultiplier::Id);
        }
        break;
    }
        // type is int16s
    case MN::InstantaneousVoltage::Id: {
        using T                = MN::InstantaneousVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstantaneousVoltage attribute value is %s", unify_value.dump().c_str());
            UN::InstantaneousVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::InstantaneousVoltage::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstantaneousLineCurrent::Id: {
        using T                = MN::InstantaneousLineCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstantaneousLineCurrent attribute value is %s", unify_value.dump().c_str());
            UN::InstantaneousLineCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::InstantaneousLineCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::InstantaneousActiveCurrent::Id: {
        using T                = MN::InstantaneousActiveCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstantaneousActiveCurrent attribute value is %s", unify_value.dump().c_str());
            UN::InstantaneousActiveCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::InstantaneousActiveCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::InstantaneousReactiveCurrent::Id: {
        using T                = MN::InstantaneousReactiveCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstantaneousReactiveCurrent attribute value is %s", unify_value.dump().c_str());
            UN::InstantaneousReactiveCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::InstantaneousReactiveCurrent::Id);
        }
        break;
    }
        // type is int16s
    case MN::InstantaneousPower::Id: {
        using T                = MN::InstantaneousPower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "InstantaneousPower attribute value is %s", unify_value.dump().c_str());
            UN::InstantaneousPower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::InstantaneousPower::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltage::Id: {
        using T                = MN::RmsVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltage attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::RmsVoltage::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMin::Id: {
        using T                = MN::RmsVoltageMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageMin attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageMin::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMax::Id: {
        using T                = MN::RmsVoltageMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageMax attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageMax::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrent::Id: {
        using T                = MN::RmsCurrent::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrent attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrent::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::RmsCurrent::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMin::Id: {
        using T                = MN::RmsCurrentMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentMin attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentMin::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMax::Id: {
        using T                = MN::RmsCurrentMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentMax attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePower::Id: {
        using T                = MN::ActivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePower attribute value is %s", unify_value.dump().c_str());
            UN::ActivePower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::ActivePower::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMin::Id: {
        using T                = MN::ActivePowerMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerMin attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerMin::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerMin::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMax::Id: {
        using T                = MN::ActivePowerMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerMax attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerMax::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerMax::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactivePower::Id: {
        using T                = MN::ReactivePower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ReactivePower attribute value is %s", unify_value.dump().c_str());
            UN::ReactivePower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ReactivePower::Id);
        }
        break;
    }
        // type is int16u
    case MN::ApparentPower::Id: {
        using T                = MN::ApparentPower::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ApparentPower attribute value is %s", unify_value.dump().c_str());
            UN::ApparentPower::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ApparentPower::Id);
        }
        break;
    }
        // type is int8s
    case MN::PowerFactor::Id: {
        using T                = MN::PowerFactor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PowerFactor attribute value is %s", unify_value.dump().c_str());
            UN::PowerFactor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::PowerFactor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsVoltageMeasurementPeriod::Id: {
        using T                = MN::AverageRmsVoltageMeasurementPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsVoltageMeasurementPeriod attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsVoltageMeasurementPeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsVoltageMeasurementPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsUnderVoltageCounter::Id: {
        using T                = MN::AverageRmsUnderVoltageCounter::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltageCounter attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsUnderVoltageCounter::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsUnderVoltageCounter::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeOverVoltagePeriod::Id: {
        using T                = MN::RmsExtremeOverVoltagePeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltagePeriod attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeOverVoltagePeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeOverVoltagePeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeUnderVoltagePeriod::Id: {
        using T                = MN::RmsExtremeUnderVoltagePeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltagePeriod attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeUnderVoltagePeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeUnderVoltagePeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSagPeriod::Id: {
        using T                = MN::RmsVoltageSagPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSagPeriod attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSagPeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSagPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSwellPeriod::Id: {
        using T                = MN::RmsVoltageSwellPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSwellPeriod attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSwellPeriod::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSwellPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcVoltageMultiplier::Id: {
        using T                = MN::AcVoltageMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcVoltageMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::AcVoltageMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcVoltageMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcVoltageDivisor::Id: {
        using T                = MN::AcVoltageDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcVoltageDivisor attribute value is %s", unify_value.dump().c_str());
            UN::AcVoltageDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcVoltageDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcCurrentMultiplier::Id: {
        using T                = MN::AcCurrentMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcCurrentMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::AcCurrentMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcCurrentMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcCurrentDivisor::Id: {
        using T                = MN::AcCurrentDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcCurrentDivisor attribute value is %s", unify_value.dump().c_str());
            UN::AcCurrentDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcCurrentDivisor::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcPowerMultiplier::Id: {
        using T                = MN::AcPowerMultiplier::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcPowerMultiplier attribute value is %s", unify_value.dump().c_str());
            UN::AcPowerMultiplier::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcPowerMultiplier::Id);
        }
        break;
    }
        // type is int16u
    case MN::AcPowerDivisor::Id: {
        using T                = MN::AcPowerDivisor::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcPowerDivisor attribute value is %s", unify_value.dump().c_str());
            UN::AcPowerDivisor::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcPowerDivisor::Id);
        }
        break;
    }
        // type is bitmap8
    case MN::OverloadAlarmsMask::Id: {
        using T                = MN::OverloadAlarmsMask::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "OverloadAlarmsMask attribute value is %s", unify_value.dump().c_str());
            UN::OverloadAlarmsMask::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::OverloadAlarmsMask::Id);
        }
        break;
    }
        // type is int16s
    case MN::VoltageOverload::Id: {
        using T                = MN::VoltageOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "VoltageOverload attribute value is %s", unify_value.dump().c_str());
            UN::VoltageOverload::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::VoltageOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::CurrentOverload::Id: {
        using T                = MN::CurrentOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "CurrentOverload attribute value is %s", unify_value.dump().c_str());
            UN::CurrentOverload::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::CurrentOverload::Id);
        }
        break;
    }
        // type is bitmap16
    case MN::AcOverloadAlarmsMask::Id: {
        using T                = MN::AcOverloadAlarmsMask::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcOverloadAlarmsMask attribute value is %s", unify_value.dump().c_str());
            UN::AcOverloadAlarmsMask::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcOverloadAlarmsMask::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcVoltageOverload::Id: {
        using T                = MN::AcVoltageOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcVoltageOverload attribute value is %s", unify_value.dump().c_str());
            UN::AcVoltageOverload::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcVoltageOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcCurrentOverload::Id: {
        using T                = MN::AcCurrentOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcCurrentOverload attribute value is %s", unify_value.dump().c_str());
            UN::AcCurrentOverload::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcCurrentOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcActivePowerOverload::Id: {
        using T                = MN::AcActivePowerOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcActivePowerOverload attribute value is %s", unify_value.dump().c_str());
            UN::AcActivePowerOverload::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcActivePowerOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AcReactivePowerOverload::Id: {
        using T                = MN::AcReactivePowerOverload::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AcReactivePowerOverload attribute value is %s", unify_value.dump().c_str());
            UN::AcReactivePowerOverload::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AcReactivePowerOverload::Id);
        }
        break;
    }
        // type is int16s
    case MN::AverageRmsOverVoltage::Id: {
        using T                = MN::AverageRmsOverVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsOverVoltage attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsOverVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsOverVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::AverageRmsUnderVoltage::Id: {
        using T                = MN::AverageRmsUnderVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltage attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsUnderVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsUnderVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsExtremeOverVoltage::Id: {
        using T                = MN::RmsExtremeOverVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltage attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeOverVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeOverVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsExtremeUnderVoltage::Id: {
        using T                = MN::RmsExtremeUnderVoltage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltage attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeUnderVoltage::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeUnderVoltage::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsVoltageSag::Id: {
        using T                = MN::RmsVoltageSag::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSag attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSag::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSag::Id);
        }
        break;
    }
        // type is int16s
    case MN::RmsVoltageSwell::Id: {
        using T                = MN::RmsVoltageSwell::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSwell attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSwell::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSwell::Id);
        }
        break;
    }
        // type is int16u
    case MN::LineCurrentPhaseB::Id: {
        using T                = MN::LineCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LineCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::LineCurrentPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::LineCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActiveCurrentPhaseB::Id: {
        using T                = MN::ActiveCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActiveCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::ActiveCurrentPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActiveCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactiveCurrentPhaseB::Id: {
        using T                = MN::ReactiveCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ReactiveCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::ReactiveCurrentPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ReactiveCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltagePhaseB::Id: {
        using T                = MN::RmsVoltagePhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltagePhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltagePhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltagePhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMinPhaseB::Id: {
        using T                = MN::RmsVoltageMinPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageMinPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageMinPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageMinPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMaxPhaseB::Id: {
        using T                = MN::RmsVoltageMaxPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageMaxPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageMaxPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageMaxPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentPhaseB::Id: {
        using T                = MN::RmsCurrentPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMinPhaseB::Id: {
        using T                = MN::RmsCurrentMinPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentMinPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentMinPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentMinPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMaxPhaseB::Id: {
        using T                = MN::RmsCurrentMaxPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentMaxPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentMaxPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentMaxPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerPhaseB::Id: {
        using T                = MN::ActivePowerPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMinPhaseB::Id: {
        using T                = MN::ActivePowerMinPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerMinPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerMinPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerMinPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMaxPhaseB::Id: {
        using T                = MN::ActivePowerMaxPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerMaxPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerMaxPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerMaxPhaseB::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactivePowerPhaseB::Id: {
        using T                = MN::ReactivePowerPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ReactivePowerPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::ReactivePowerPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ReactivePowerPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::ApparentPowerPhaseB::Id: {
        using T                = MN::ApparentPowerPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ApparentPowerPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::ApparentPowerPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ApparentPowerPhaseB::Id);
        }
        break;
    }
        // type is int8s
    case MN::PowerFactorPhaseB::Id: {
        using T                = MN::PowerFactorPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PowerFactorPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::PowerFactorPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::PowerFactorPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsVoltageMeasurementPeriodPhaseB::Id: {
        using T                = MN::AverageRmsVoltageMeasurementPeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsVoltageMeasurementPeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsVoltageMeasurementPeriodPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsVoltageMeasurementPeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsOverVoltageCounterPhaseB::Id: {
        using T                = MN::AverageRmsOverVoltageCounterPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsOverVoltageCounterPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsOverVoltageCounterPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsOverVoltageCounterPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsUnderVoltageCounterPhaseB::Id: {
        using T                = MN::AverageRmsUnderVoltageCounterPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltageCounterPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsUnderVoltageCounterPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsUnderVoltageCounterPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeOverVoltagePeriodPhaseB::Id: {
        using T                = MN::RmsExtremeOverVoltagePeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltagePeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeOverVoltagePeriodPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeOverVoltagePeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeUnderVoltagePeriodPhaseB::Id: {
        using T                = MN::RmsExtremeUnderVoltagePeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltagePeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeUnderVoltagePeriodPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeUnderVoltagePeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSagPeriodPhaseB::Id: {
        using T                = MN::RmsVoltageSagPeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSagPeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSagPeriodPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSagPeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSwellPeriodPhaseB::Id: {
        using T                = MN::RmsVoltageSwellPeriodPhaseB::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSwellPeriodPhaseB attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSwellPeriodPhaseB::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSwellPeriodPhaseB::Id);
        }
        break;
    }
        // type is int16u
    case MN::LineCurrentPhaseC::Id: {
        using T                = MN::LineCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "LineCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::LineCurrentPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::LineCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActiveCurrentPhaseC::Id: {
        using T                = MN::ActiveCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActiveCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::ActiveCurrentPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActiveCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactiveCurrentPhaseC::Id: {
        using T                = MN::ReactiveCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ReactiveCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::ReactiveCurrentPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ReactiveCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltagePhaseC::Id: {
        using T                = MN::RmsVoltagePhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltagePhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltagePhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltagePhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMinPhaseC::Id: {
        using T                = MN::RmsVoltageMinPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageMinPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageMinPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageMinPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageMaxPhaseC::Id: {
        using T                = MN::RmsVoltageMaxPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageMaxPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageMaxPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageMaxPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentPhaseC::Id: {
        using T                = MN::RmsCurrentPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMinPhaseC::Id: {
        using T                = MN::RmsCurrentMinPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentMinPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentMinPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentMinPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsCurrentMaxPhaseC::Id: {
        using T                = MN::RmsCurrentMaxPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsCurrentMaxPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsCurrentMaxPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsCurrentMaxPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerPhaseC::Id: {
        using T                = MN::ActivePowerPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMinPhaseC::Id: {
        using T                = MN::ActivePowerMinPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerMinPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerMinPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerMinPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ActivePowerMaxPhaseC::Id: {
        using T                = MN::ActivePowerMaxPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ActivePowerMaxPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::ActivePowerMaxPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ActivePowerMaxPhaseC::Id);
        }
        break;
    }
        // type is int16s
    case MN::ReactivePowerPhaseC::Id: {
        using T                = MN::ReactivePowerPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ReactivePowerPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::ReactivePowerPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ReactivePowerPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::ApparentPowerPhaseC::Id: {
        using T                = MN::ApparentPowerPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ApparentPowerPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::ApparentPowerPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ApparentPowerPhaseC::Id);
        }
        break;
    }
        // type is int8s
    case MN::PowerFactorPhaseC::Id: {
        using T                = MN::PowerFactorPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "PowerFactorPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::PowerFactorPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::PowerFactorPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsVoltageMeasurementPeriodPhaseC::Id: {
        using T                = MN::AverageRmsVoltageMeasurementPeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsVoltageMeasurementPeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsVoltageMeasurementPeriodPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsVoltageMeasurementPeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsOverVoltageCounterPhaseC::Id: {
        using T                = MN::AverageRmsOverVoltageCounterPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsOverVoltageCounterPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsOverVoltageCounterPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsOverVoltageCounterPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::AverageRmsUnderVoltageCounterPhaseC::Id: {
        using T                = MN::AverageRmsUnderVoltageCounterPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "AverageRmsUnderVoltageCounterPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::AverageRmsUnderVoltageCounterPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::AverageRmsUnderVoltageCounterPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeOverVoltagePeriodPhaseC::Id: {
        using T                = MN::RmsExtremeOverVoltagePeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeOverVoltagePeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeOverVoltagePeriodPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeOverVoltagePeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsExtremeUnderVoltagePeriodPhaseC::Id: {
        using T                = MN::RmsExtremeUnderVoltagePeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsExtremeUnderVoltagePeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsExtremeUnderVoltagePeriodPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsExtremeUnderVoltagePeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSagPeriodPhaseC::Id: {
        using T                = MN::RmsVoltageSagPeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSagPeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSagPeriodPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSagPeriodPhaseC::Id);
        }
        break;
    }
        // type is int16u
    case MN::RmsVoltageSwellPeriodPhaseC::Id: {
        using T                = MN::RmsVoltageSwellPeriodPhaseC::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "RmsVoltageSwellPeriodPhaseC attribute value is %s", unify_value.dump().c_str());
            UN::RmsVoltageSwellPeriodPhaseC::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::RmsVoltageSwellPeriodPhaseC::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T                = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            UN::FeatureMap::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id, MN::FeatureMap::Id);
        }
        break;
    }
        // type is int16u
    case MN::ClusterRevision::Id: {
        using T                = MN::ClusterRevision::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value())
        {
            sl_log_debug(LOG_TAG, "ClusterRevision attribute value is %s", unify_value.dump().c_str());
            UN::ClusterRevision::Set(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ElectricalMeasurement::Id,
                                                   MN::ClusterRevision::Id);
        }
        break;
    }
    }
}

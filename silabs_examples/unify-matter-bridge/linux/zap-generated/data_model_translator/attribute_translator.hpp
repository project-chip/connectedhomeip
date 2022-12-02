/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file on_off_attribute_translator.h
 * @ingroup components
 *
 * @brief  OnOff cluster attribute state transition handler
 *
 * @{
 */

#include "attribute_translator_interface.hpp"

namespace unify::matter_bridge {

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Identify cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - identify type
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class IdentifyAttributeAccess : public attribute_translator_interface
{
public:
    IdentifyAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::Identify::Id,
                                       "attr_translator_Identify"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Identify" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the On/Off cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class OnOffAttributeAccess : public attribute_translator_interface
{
public:
    OnOffAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::OnOff::Id, "attr_translator_OnOff"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "OnOff" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Level Control cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class LevelControlAttributeAccess : public attribute_translator_interface
{
public:
    LevelControlAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::LevelControl::Id,
                                       "attr_translator_LevelControl"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Level" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Door Lock cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - CredentialRulesSupport (optional)
/// - NumberOfCredentialsSupportedPerUser (optional)
/// - LocalProgrammingFeatures (optional)
/// - ExpiringUserTimeout (optional)
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class DoorLockAttributeAccess : public attribute_translator_interface
{
public:
    DoorLockAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::DoorLock::Id,
                                       "attr_translator_DoorLock"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "DoorLock" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Barrier Control cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class BarrierControlAttributeAccess : public attribute_translator_interface
{
public:
    BarrierControlAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::BarrierControl::Id,
                                       "attr_translator_BarrierControl"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "BarrierControl" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Thermostat cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class ThermostatAttributeAccess : public attribute_translator_interface
{
public:
    ThermostatAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::Thermostat::Id,
                                       "attr_translator_Thermostat"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Thermostat" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Fan Control cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - percent setting
/// - percent current
/// - speed max (optional)
/// - speed setting (optional)
/// - speed current (optional)
/// - rock support (optional)
/// - rock setting (optional)
/// - wind support (optional)
/// - wind setting (optional)
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class FanControlAttributeAccess : public attribute_translator_interface
{
public:
    FanControlAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::FanControl::Id,
                                       "attr_translator_FanControl"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "FanControl" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Thermostat User Interface Configuration cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class ThermostatUserInterfaceConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    ThermostatUserInterfaceConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt,
                                       chip::app::Clusters::ThermostatUserInterfaceConfiguration::Id,
                                       "attr_translator_ThermostatUserInterfaceConfiguration"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "ThermostatUserInterfaceConfiguration" });
    }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Color Control cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class ColorControlAttributeAccess : public attribute_translator_interface
{
public:
    ColorControlAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ColorControl::Id,
                                       "attr_translator_ColorControl"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ColorControl" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Illuminance Measurement cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class IlluminanceMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    IlluminanceMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::IlluminanceMeasurement::Id,
                                       "attr_translator_IlluminanceMeasurement"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "IlluminanceMeasurement" });
    }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Temperature Measurement cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class TemperatureMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    TemperatureMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::TemperatureMeasurement::Id,
                                       "attr_translator_TemperatureMeasurement"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "TemperatureMeasurement" });
    }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Pressure Measurement cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class PressureMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    PressureMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::PressureMeasurement::Id,
                                       "attr_translator_PressureMeasurement"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "PressureMeasurement" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Relative Humidity Measurement cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class RelativeHumidityMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    RelativeHumidityMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::RelativeHumidityMeasurement::Id,
                                       "attr_translator_RelativeHumidityMeasurement"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "RelativityHumidity" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Occupancy Sensing cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class OccupancySensingAttributeAccess : public attribute_translator_interface
{
public:
    OccupancySensingAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::OccupancySensing::Id,
                                       "attr_translator_OccupancySensing"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "OccupancySensing" }); }
    UnifyMqtt & m_unify_mqtt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// Attribute Handler for the Electrical Measurement cluster
/// Please note that the following attributes are not defined in UCL, and they must be handled
/// by separate code
/// - instantaneous voltage (optional)
/// - instantaneous power (optional)
/// - GeneratedCommandList
/// - AcceptedCommandList
/// - AttributeList
/// - FeatureMap
/// - ClusterRevision
///
class ElectricalMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    ElectricalMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ElectricalMeasurement::Id,
                                       "attr_translator_ElectricalMeasurement"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "ElectricalMeasurement" });
    }
    UnifyMqtt & m_unify_mqtt;
};
} // namespace unify::matter_bridge
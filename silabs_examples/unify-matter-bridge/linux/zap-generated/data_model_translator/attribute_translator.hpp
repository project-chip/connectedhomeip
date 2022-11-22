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
class ScenesAttributeAccess : public attribute_translator_interface
{
public:
    ScenesAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::Scenes::Id, "attr_translator_Scenes"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Scenes" }); }

    UnifyMqtt & m_unify_mqtt;
};
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
class OnOffSwitchConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    OnOffSwitchConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::OnOffSwitchConfiguration::Id,
                                       "attr_translator_OnOffSwitchConfiguration"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "OnOffSwitchConfiguration" });
    }

    UnifyMqtt & m_unify_mqtt;
};
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
class BinaryInputBasicAttributeAccess : public attribute_translator_interface
{
public:
    BinaryInputBasicAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::BinaryInputBasic::Id,
                                       "attr_translator_BinaryInputBasic"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "BinaryInputBasic" }); }

    UnifyMqtt & m_unify_mqtt;
};
class PulseWidthModulationAttributeAccess : public attribute_translator_interface
{
public:
    PulseWidthModulationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::PulseWidthModulation::Id,
                                       "attr_translator_PulseWidthModulation"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "PulseWidthModulation" }); }

    UnifyMqtt & m_unify_mqtt;
};
class BasicAttributeAccess : public attribute_translator_interface
{
public:
    BasicAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::Basic::Id, "attr_translator_Basic"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Basic" }); }

    UnifyMqtt & m_unify_mqtt;
};
class OtaSoftwareUpdateProviderAttributeAccess : public attribute_translator_interface
{
public:
    OtaSoftwareUpdateProviderAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::OtaSoftwareUpdateProvider::Id,
                                       "attr_translator_OtaSoftwareUpdateProvider"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "OtaSoftwareUpdateProvider" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class OtaSoftwareUpdateRequestorAttributeAccess : public attribute_translator_interface
{
public:
    OtaSoftwareUpdateRequestorAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::OtaSoftwareUpdateRequestor::Id,
                                       "attr_translator_OtaSoftwareUpdateRequestor"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "OtaSoftwareUpdateRequestor" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class LocalizationConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    LocalizationConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::LocalizationConfiguration::Id,
                                       "attr_translator_LocalizationConfiguration"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "LocalizationConfiguration" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class TimeFormatLocalizationAttributeAccess : public attribute_translator_interface
{
public:
    TimeFormatLocalizationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::TimeFormatLocalization::Id,
                                       "attr_translator_TimeFormatLocalization"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "TimeFormatLocalization" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class UnitLocalizationAttributeAccess : public attribute_translator_interface
{
public:
    UnitLocalizationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::UnitLocalization::Id,
                                       "attr_translator_UnitLocalization"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "UnitLocalization" }); }

    UnifyMqtt & m_unify_mqtt;
};
class PowerSourceConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    PowerSourceConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::PowerSourceConfiguration::Id,
                                       "attr_translator_PowerSourceConfiguration"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "PowerSourceConfiguration" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class PowerSourceAttributeAccess : public attribute_translator_interface
{
public:
    PowerSourceAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::PowerSource::Id,
                                       "attr_translator_PowerSource"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "PowerSource" }); }

    UnifyMqtt & m_unify_mqtt;
};
class GeneralCommissioningAttributeAccess : public attribute_translator_interface
{
public:
    GeneralCommissioningAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::GeneralCommissioning::Id,
                                       "attr_translator_GeneralCommissioning"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "GeneralCommissioning" }); }

    UnifyMqtt & m_unify_mqtt;
};
class DiagnosticLogsAttributeAccess : public attribute_translator_interface
{
public:
    DiagnosticLogsAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::DiagnosticLogs::Id,
                                       "attr_translator_DiagnosticLogs"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "DiagnosticLogs" }); }

    UnifyMqtt & m_unify_mqtt;
};
class GeneralDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    GeneralDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::GeneralDiagnostics::Id,
                                       "attr_translator_GeneralDiagnostics"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "GeneralDiagnostics" }); }

    UnifyMqtt & m_unify_mqtt;
};
class SoftwareDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    SoftwareDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::SoftwareDiagnostics::Id,
                                       "attr_translator_SoftwareDiagnostics"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "SoftwareDiagnostics" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ThreadNetworkDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    ThreadNetworkDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ThreadNetworkDiagnostics::Id,
                                       "attr_translator_ThreadNetworkDiagnostics"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "ThreadNetworkDiagnostics" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class WiFiNetworkDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    WiFiNetworkDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::WiFiNetworkDiagnostics::Id,
                                       "attr_translator_WiFiNetworkDiagnostics"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "WiFiNetworkDiagnostics" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class TimeSynchronizationAttributeAccess : public attribute_translator_interface
{
public:
    TimeSynchronizationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::TimeSynchronization::Id,
                                       "attr_translator_TimeSynchronization"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "TimeSynchronization" }); }

    UnifyMqtt & m_unify_mqtt;
};
class SwitchAttributeAccess : public attribute_translator_interface
{
public:
    SwitchAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::Switch::Id, "attr_translator_Switch"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Switch" }); }

    UnifyMqtt & m_unify_mqtt;
};
class OperationalCredentialsAttributeAccess : public attribute_translator_interface
{
public:
    OperationalCredentialsAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::OperationalCredentials::Id,
                                       "attr_translator_OperationalCredentials"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "OperationalCredentials" });
    }

    UnifyMqtt & m_unify_mqtt;
};
class GroupKeyManagementAttributeAccess : public attribute_translator_interface
{
public:
    GroupKeyManagementAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::GroupKeyManagement::Id,
                                       "attr_translator_GroupKeyManagement"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "GroupKeyManagement" }); }

    UnifyMqtt & m_unify_mqtt;
};
class FixedLabelAttributeAccess : public attribute_translator_interface
{
public:
    FixedLabelAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::FixedLabel::Id,
                                       "attr_translator_FixedLabel"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "FixedLabel" }); }

    UnifyMqtt & m_unify_mqtt;
};
class UserLabelAttributeAccess : public attribute_translator_interface
{
public:
    UserLabelAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::UserLabel::Id,
                                       "attr_translator_UserLabel"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "UserLabel" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ProxyConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    ProxyConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ProxyConfiguration::Id,
                                       "attr_translator_ProxyConfiguration"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ProxyConfiguration" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ProxyDiscoveryAttributeAccess : public attribute_translator_interface
{
public:
    ProxyDiscoveryAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ProxyDiscovery::Id,
                                       "attr_translator_ProxyDiscovery"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ProxyDiscovery" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ProxyValidAttributeAccess : public attribute_translator_interface
{
public:
    ProxyValidAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ProxyValid::Id,
                                       "attr_translator_ProxyValid"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ProxyValid" }); }

    UnifyMqtt & m_unify_mqtt;
};
class BooleanStateAttributeAccess : public attribute_translator_interface
{
public:
    BooleanStateAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::BooleanState::Id,
                                       "attr_translator_BooleanState"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "BooleanState" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ModeSelectAttributeAccess : public attribute_translator_interface
{
public:
    ModeSelectAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ModeSelect::Id,
                                       "attr_translator_ModeSelect"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ModeSelect" }); }

    UnifyMqtt & m_unify_mqtt;
};
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
class WindowCoveringAttributeAccess : public attribute_translator_interface
{
public:
    WindowCoveringAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::WindowCovering::Id,
                                       "attr_translator_WindowCovering"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "WindowCovering" }); }

    UnifyMqtt & m_unify_mqtt;
};
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
class FlowMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    FlowMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::FlowMeasurement::Id,
                                       "attr_translator_FlowMeasurement"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "FlowMeasurement" }); }

    UnifyMqtt & m_unify_mqtt;
};
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

    std::vector<const char *> unify_cluster_names() const override
    {
        return std::vector<const char *>({ "RelativeHumidityMeasurement" });
    }

    UnifyMqtt & m_unify_mqtt;
};
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
class WakeOnLanAttributeAccess : public attribute_translator_interface
{
public:
    WakeOnLanAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::WakeOnLan::Id,
                                       "attr_translator_WakeOnLan"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "WakeOnLan" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ChannelAttributeAccess : public attribute_translator_interface
{
public:
    ChannelAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::Channel::Id, "attr_translator_Channel"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Channel" }); }

    UnifyMqtt & m_unify_mqtt;
};
class TargetNavigatorAttributeAccess : public attribute_translator_interface
{
public:
    TargetNavigatorAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::TargetNavigator::Id,
                                       "attr_translator_TargetNavigator"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "TargetNavigator" }); }

    UnifyMqtt & m_unify_mqtt;
};
class MediaPlaybackAttributeAccess : public attribute_translator_interface
{
public:
    MediaPlaybackAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::MediaPlayback::Id,
                                       "attr_translator_MediaPlayback"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "MediaPlayback" }); }

    UnifyMqtt & m_unify_mqtt;
};
class MediaInputAttributeAccess : public attribute_translator_interface
{
public:
    MediaInputAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::MediaInput::Id,
                                       "attr_translator_MediaInput"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "MediaInput" }); }

    UnifyMqtt & m_unify_mqtt;
};
class LowPowerAttributeAccess : public attribute_translator_interface
{
public:
    LowPowerAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::LowPower::Id,
                                       "attr_translator_LowPower"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "LowPower" }); }

    UnifyMqtt & m_unify_mqtt;
};
class KeypadInputAttributeAccess : public attribute_translator_interface
{
public:
    KeypadInputAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::KeypadInput::Id,
                                       "attr_translator_KeypadInput"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "KeypadInput" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ContentLauncherAttributeAccess : public attribute_translator_interface
{
public:
    ContentLauncherAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ContentLauncher::Id,
                                       "attr_translator_ContentLauncher"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ContentLauncher" }); }

    UnifyMqtt & m_unify_mqtt;
};
class AudioOutputAttributeAccess : public attribute_translator_interface
{
public:
    AudioOutputAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::AudioOutput::Id,
                                       "attr_translator_AudioOutput"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "AudioOutput" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ApplicationLauncherAttributeAccess : public attribute_translator_interface
{
public:
    ApplicationLauncherAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ApplicationLauncher::Id,
                                       "attr_translator_ApplicationLauncher"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ApplicationLauncher" }); }

    UnifyMqtt & m_unify_mqtt;
};
class ApplicationBasicAttributeAccess : public attribute_translator_interface
{
public:
    ApplicationBasicAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::ApplicationBasic::Id,
                                       "attr_translator_ApplicationBasic"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ApplicationBasic" }); }

    UnifyMqtt & m_unify_mqtt;
};
class AccountLoginAttributeAccess : public attribute_translator_interface
{
public:
    AccountLoginAttributeAccess(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt) :
        attribute_translator_interface(node_state_monitor, unify_mqtt, chip::app::Clusters::AccountLogin::Id,
                                       "attr_translator_AccountLogin"),
        m_unify_mqtt(unify_mqtt)
    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "AccountLogin" }); }

    UnifyMqtt & m_unify_mqtt;
};
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
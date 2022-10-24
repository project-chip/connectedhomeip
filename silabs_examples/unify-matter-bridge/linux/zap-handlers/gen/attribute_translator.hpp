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
    IdentifyAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::Identify::Id, "attr_translator_Identify")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Identify" }); }
};
class ScenesAttributeAccess : public attribute_translator_interface
{
public:
    ScenesAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::Scenes::Id, "attr_translator_Scenes")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Scenes" }); }
};
class OnOffAttributeAccess : public attribute_translator_interface
{
public:
    OnOffAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::OnOff::Id, "attr_translator_OnOff")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "OnOff" }); }
};
class OnOffSwitchConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    OnOffSwitchConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::OnOffSwitchConfiguration::Id,
                                       "attr_translator_OnOffSwitchConfiguration")

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
};
class LevelControlAttributeAccess : public attribute_translator_interface
{
public:
    LevelControlAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::LevelControl::Id, "attr_translator_LevelControl")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Level" }); }
};
class BinaryInputBasicAttributeAccess : public attribute_translator_interface
{
public:
    BinaryInputBasicAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::BinaryInputBasic::Id,
                                       "attr_translator_BinaryInputBasic")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "BinaryInputBasic" }); }
};
class PulseWidthModulationAttributeAccess : public attribute_translator_interface
{
public:
    PulseWidthModulationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::PulseWidthModulation::Id,
                                       "attr_translator_PulseWidthModulation")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "PulseWidthModulation" }); }
};
class BasicAttributeAccess : public attribute_translator_interface
{
public:
    BasicAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::Basic::Id, "attr_translator_Basic")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Basic" }); }
};
class OtaSoftwareUpdateProviderAttributeAccess : public attribute_translator_interface
{
public:
    OtaSoftwareUpdateProviderAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::OtaSoftwareUpdateProvider::Id,
                                       "attr_translator_OtaSoftwareUpdateProvider")

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
};
class OtaSoftwareUpdateRequestorAttributeAccess : public attribute_translator_interface
{
public:
    OtaSoftwareUpdateRequestorAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::OtaSoftwareUpdateRequestor::Id,
                                       "attr_translator_OtaSoftwareUpdateRequestor")

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
};
class LocalizationConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    LocalizationConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::LocalizationConfiguration::Id,
                                       "attr_translator_LocalizationConfiguration")

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
};
class TimeFormatLocalizationAttributeAccess : public attribute_translator_interface
{
public:
    TimeFormatLocalizationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::TimeFormatLocalization::Id,
                                       "attr_translator_TimeFormatLocalization")

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
};
class UnitLocalizationAttributeAccess : public attribute_translator_interface
{
public:
    UnitLocalizationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::UnitLocalization::Id,
                                       "attr_translator_UnitLocalization")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "UnitLocalization" }); }
};
class PowerSourceConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    PowerSourceConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::PowerSourceConfiguration::Id,
                                       "attr_translator_PowerSourceConfiguration")

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
};
class PowerSourceAttributeAccess : public attribute_translator_interface
{
public:
    PowerSourceAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::PowerSource::Id, "attr_translator_PowerSource")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "PowerSource" }); }
};
class GeneralCommissioningAttributeAccess : public attribute_translator_interface
{
public:
    GeneralCommissioningAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::GeneralCommissioning::Id,
                                       "attr_translator_GeneralCommissioning")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "GeneralCommissioning" }); }
};
class DiagnosticLogsAttributeAccess : public attribute_translator_interface
{
public:
    DiagnosticLogsAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::DiagnosticLogs::Id,
                                       "attr_translator_DiagnosticLogs")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "DiagnosticLogs" }); }
};
class GeneralDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    GeneralDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::GeneralDiagnostics::Id,
                                       "attr_translator_GeneralDiagnostics")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "GeneralDiagnostics" }); }
};
class SoftwareDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    SoftwareDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::SoftwareDiagnostics::Id,
                                       "attr_translator_SoftwareDiagnostics")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "SoftwareDiagnostics" }); }
};
class ThreadNetworkDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    ThreadNetworkDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ThreadNetworkDiagnostics::Id,
                                       "attr_translator_ThreadNetworkDiagnostics")

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
};
class WiFiNetworkDiagnosticsAttributeAccess : public attribute_translator_interface
{
public:
    WiFiNetworkDiagnosticsAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::WiFiNetworkDiagnostics::Id,
                                       "attr_translator_WiFiNetworkDiagnostics")

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
};
class TimeSynchronizationAttributeAccess : public attribute_translator_interface
{
public:
    TimeSynchronizationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::TimeSynchronization::Id,
                                       "attr_translator_TimeSynchronization")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "TimeSynchronization" }); }
};
class SwitchAttributeAccess : public attribute_translator_interface
{
public:
    SwitchAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::Switch::Id, "attr_translator_Switch")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Switch" }); }
};
class OperationalCredentialsAttributeAccess : public attribute_translator_interface
{
public:
    OperationalCredentialsAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::OperationalCredentials::Id,
                                       "attr_translator_OperationalCredentials")

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
};
class GroupKeyManagementAttributeAccess : public attribute_translator_interface
{
public:
    GroupKeyManagementAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::GroupKeyManagement::Id,
                                       "attr_translator_GroupKeyManagement")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "GroupKeyManagement" }); }
};
class FixedLabelAttributeAccess : public attribute_translator_interface
{
public:
    FixedLabelAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::FixedLabel::Id, "attr_translator_FixedLabel")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "FixedLabel" }); }
};
class UserLabelAttributeAccess : public attribute_translator_interface
{
public:
    UserLabelAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::UserLabel::Id, "attr_translator_UserLabel")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "UserLabel" }); }
};
class ProxyConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    ProxyConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ProxyConfiguration::Id,
                                       "attr_translator_ProxyConfiguration")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ProxyConfiguration" }); }
};
class ProxyDiscoveryAttributeAccess : public attribute_translator_interface
{
public:
    ProxyDiscoveryAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ProxyDiscovery::Id,
                                       "attr_translator_ProxyDiscovery")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ProxyDiscovery" }); }
};
class ProxyValidAttributeAccess : public attribute_translator_interface
{
public:
    ProxyValidAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ProxyValid::Id, "attr_translator_ProxyValid")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ProxyValid" }); }
};
class BooleanStateAttributeAccess : public attribute_translator_interface
{
public:
    BooleanStateAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::BooleanState::Id, "attr_translator_BooleanState")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "BooleanState" }); }
};
class ModeSelectAttributeAccess : public attribute_translator_interface
{
public:
    ModeSelectAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ModeSelect::Id, "attr_translator_ModeSelect")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ModeSelect" }); }
};
class DoorLockAttributeAccess : public attribute_translator_interface
{
public:
    DoorLockAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::DoorLock::Id, "attr_translator_DoorLock")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "DoorLock" }); }
};
class WindowCoveringAttributeAccess : public attribute_translator_interface
{
public:
    WindowCoveringAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::WindowCovering::Id,
                                       "attr_translator_WindowCovering")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "WindowCovering" }); }
};
class BarrierControlAttributeAccess : public attribute_translator_interface
{
public:
    BarrierControlAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::BarrierControl::Id,
                                       "attr_translator_BarrierControl")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "BarrierControl" }); }
};
class ThermostatAttributeAccess : public attribute_translator_interface
{
public:
    ThermostatAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::Thermostat::Id, "attr_translator_Thermostat")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Thermostat" }); }
};
class FanControlAttributeAccess : public attribute_translator_interface
{
public:
    FanControlAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::FanControl::Id, "attr_translator_FanControl")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "FanControl" }); }
};
class ThermostatUserInterfaceConfigurationAttributeAccess : public attribute_translator_interface
{
public:
    ThermostatUserInterfaceConfigurationAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ThermostatUserInterfaceConfiguration::Id,
                                       "attr_translator_ThermostatUserInterfaceConfiguration")

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
};
class ColorControlAttributeAccess : public attribute_translator_interface
{
public:
    ColorControlAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ColorControl::Id, "attr_translator_ColorControl")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ColorControl" }); }
};
class IlluminanceMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    IlluminanceMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::IlluminanceMeasurement::Id,
                                       "attr_translator_IlluminanceMeasurement")

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
};
class TemperatureMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    TemperatureMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::TemperatureMeasurement::Id,
                                       "attr_translator_TemperatureMeasurement")

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
};
class PressureMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    PressureMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::PressureMeasurement::Id,
                                       "attr_translator_PressureMeasurement")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "PressureMeasurement" }); }
};
class FlowMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    FlowMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::FlowMeasurement::Id,
                                       "attr_translator_FlowMeasurement")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "FlowMeasurement" }); }
};
class RelativeHumidityMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    RelativeHumidityMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::RelativeHumidityMeasurement::Id,
                                       "attr_translator_RelativeHumidityMeasurement")

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
};
class OccupancySensingAttributeAccess : public attribute_translator_interface
{
public:
    OccupancySensingAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::OccupancySensing::Id,
                                       "attr_translator_OccupancySensing")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "OccupancySensing" }); }
};
class WakeOnLanAttributeAccess : public attribute_translator_interface
{
public:
    WakeOnLanAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::WakeOnLan::Id, "attr_translator_WakeOnLan")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "WakeOnLan" }); }
};
class ChannelAttributeAccess : public attribute_translator_interface
{
public:
    ChannelAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::Channel::Id, "attr_translator_Channel")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "Channel" }); }
};
class TargetNavigatorAttributeAccess : public attribute_translator_interface
{
public:
    TargetNavigatorAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::TargetNavigator::Id,
                                       "attr_translator_TargetNavigator")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "TargetNavigator" }); }
};
class MediaPlaybackAttributeAccess : public attribute_translator_interface
{
public:
    MediaPlaybackAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::MediaPlayback::Id, "attr_translator_MediaPlayback")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "MediaPlayback" }); }
};
class MediaInputAttributeAccess : public attribute_translator_interface
{
public:
    MediaInputAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::MediaInput::Id, "attr_translator_MediaInput")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "MediaInput" }); }
};
class LowPowerAttributeAccess : public attribute_translator_interface
{
public:
    LowPowerAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::LowPower::Id, "attr_translator_LowPower")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "LowPower" }); }
};
class KeypadInputAttributeAccess : public attribute_translator_interface
{
public:
    KeypadInputAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::KeypadInput::Id, "attr_translator_KeypadInput")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "KeypadInput" }); }
};
class ContentLauncherAttributeAccess : public attribute_translator_interface
{
public:
    ContentLauncherAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ContentLauncher::Id,
                                       "attr_translator_ContentLauncher")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ContentLauncher" }); }
};
class AudioOutputAttributeAccess : public attribute_translator_interface
{
public:
    AudioOutputAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::AudioOutput::Id, "attr_translator_AudioOutput")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "AudioOutput" }); }
};
class ApplicationLauncherAttributeAccess : public attribute_translator_interface
{
public:
    ApplicationLauncherAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ApplicationLauncher::Id,
                                       "attr_translator_ApplicationLauncher")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ApplicationLauncher" }); }
};
class ApplicationBasicAttributeAccess : public attribute_translator_interface
{
public:
    ApplicationBasicAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ApplicationBasic::Id,
                                       "attr_translator_ApplicationBasic")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "ApplicationBasic" }); }
};
class AccountLoginAttributeAccess : public attribute_translator_interface
{
public:
    AccountLoginAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::AccountLogin::Id, "attr_translator_AccountLogin")

    {}

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

private:
    void reported_updated(const bridged_endpoint * ep, const std::string & cluster, const std::string & attribute,
                          const nlohmann::json & unify_value) override;

    std::vector<const char *> unify_cluster_names() const override { return std::vector<const char *>({ "AccountLogin" }); }
};
class ElectricalMeasurementAttributeAccess : public attribute_translator_interface
{
public:
    ElectricalMeasurementAttributeAccess(matter_node_state_monitor & node_state_monitor) :
        attribute_translator_interface(node_state_monitor, chip::app::Clusters::ElectricalMeasurement::Id,
                                       "attr_translator_ElectricalMeasurement")

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
};
} // namespace unify::matter_bridge
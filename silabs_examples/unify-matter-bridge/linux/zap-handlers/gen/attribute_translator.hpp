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

namespace unify::matter_bridge
{

class IdentifyAttributeAccess : public attribute_translator_interface
{
  public:
  IdentifyAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Identify::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Identify" });}
};


class GroupsAttributeAccess : public attribute_translator_interface
{
  public:
  GroupsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Groups::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Groups" });}
};


class ScenesAttributeAccess : public attribute_translator_interface
{
  public:
  ScenesAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Scenes::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Scenes" });}
};


class OnOffAttributeAccess : public attribute_translator_interface
{
  public:
  OnOffAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::OnOff::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "OnOff" });}
};


class OnOffSwitchConfigurationAttributeAccess : public attribute_translator_interface
{
  public:
  OnOffSwitchConfigurationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::OnOffSwitchConfiguration::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "OnOffSwitchConfiguration" });}
};


class LevelControlAttributeAccess : public attribute_translator_interface
{
  public:
  LevelControlAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::LevelControl::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "LevelControl" });}
};


class BinaryInputBasicAttributeAccess : public attribute_translator_interface
{
  public:
  BinaryInputBasicAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::BinaryInputBasic::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "BinaryInputBasic" });}
};


class PulseWidthModulationAttributeAccess : public attribute_translator_interface
{
  public:
  PulseWidthModulationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::PulseWidthModulation::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "PulseWidthModulation" });}
};


class DescriptorAttributeAccess : public attribute_translator_interface
{
  public:
  DescriptorAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Descriptor::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Descriptor" });}
};


class BindingAttributeAccess : public attribute_translator_interface
{
  public:
  BindingAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Binding::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Binding" });}
};


class AccessControlAttributeAccess : public attribute_translator_interface
{
  public:
  AccessControlAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::AccessControl::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "AccessControl" });}
};


class ActionsAttributeAccess : public attribute_translator_interface
{
  public:
  ActionsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Actions::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Actions" });}
};


class BasicAttributeAccess : public attribute_translator_interface
{
  public:
  BasicAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Basic::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Basic" });}
};


class OtaSoftwareUpdateProviderAttributeAccess : public attribute_translator_interface
{
  public:
  OtaSoftwareUpdateProviderAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::OtaSoftwareUpdateProvider::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "OtaSoftwareUpdateProvider" });}
};


class OtaSoftwareUpdateRequestorAttributeAccess : public attribute_translator_interface
{
  public:
  OtaSoftwareUpdateRequestorAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::OtaSoftwareUpdateRequestor::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "OtaSoftwareUpdateRequestor" });}
};


class LocalizationConfigurationAttributeAccess : public attribute_translator_interface
{
  public:
  LocalizationConfigurationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::LocalizationConfiguration::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "LocalizationConfiguration" });}
};


class TimeFormatLocalizationAttributeAccess : public attribute_translator_interface
{
  public:
  TimeFormatLocalizationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::TimeFormatLocalization::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "TimeFormatLocalization" });}
};


class UnitLocalizationAttributeAccess : public attribute_translator_interface
{
  public:
  UnitLocalizationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::UnitLocalization::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "UnitLocalization" });}
};


class PowerSourceConfigurationAttributeAccess : public attribute_translator_interface
{
  public:
  PowerSourceConfigurationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::PowerSourceConfiguration::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "PowerSourceConfiguration" });}
};


class PowerSourceAttributeAccess : public attribute_translator_interface
{
  public:
  PowerSourceAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::PowerSource::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "PowerSource" });}
};


class GeneralCommissioningAttributeAccess : public attribute_translator_interface
{
  public:
  GeneralCommissioningAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::GeneralCommissioning::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "GeneralCommissioning" });}
};


class NetworkCommissioningAttributeAccess : public attribute_translator_interface
{
  public:
  NetworkCommissioningAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::NetworkCommissioning::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "NetworkCommissioning" });}
};


class DiagnosticLogsAttributeAccess : public attribute_translator_interface
{
  public:
  DiagnosticLogsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::DiagnosticLogs::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "DiagnosticLogs" });}
};


class GeneralDiagnosticsAttributeAccess : public attribute_translator_interface
{
  public:
  GeneralDiagnosticsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::GeneralDiagnostics::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "GeneralDiagnostics" });}
};


class SoftwareDiagnosticsAttributeAccess : public attribute_translator_interface
{
  public:
  SoftwareDiagnosticsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::SoftwareDiagnostics::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "SoftwareDiagnostics" });}
};


class ThreadNetworkDiagnosticsAttributeAccess : public attribute_translator_interface
{
  public:
  ThreadNetworkDiagnosticsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ThreadNetworkDiagnostics::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ThreadNetworkDiagnostics" });}
};


class WiFiNetworkDiagnosticsAttributeAccess : public attribute_translator_interface
{
  public:
  WiFiNetworkDiagnosticsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::WiFiNetworkDiagnostics::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "WiFiNetworkDiagnostics" });}
};


class EthernetNetworkDiagnosticsAttributeAccess : public attribute_translator_interface
{
  public:
  EthernetNetworkDiagnosticsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::EthernetNetworkDiagnostics::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "EthernetNetworkDiagnostics" });}
};


class TimeSynchronizationAttributeAccess : public attribute_translator_interface
{
  public:
  TimeSynchronizationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::TimeSynchronization::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "TimeSynchronization" });}
};


class BridgedDeviceBasicAttributeAccess : public attribute_translator_interface
{
  public:
  BridgedDeviceBasicAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::BridgedDeviceBasic::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "BridgedDeviceBasic" });}
};


class SwitchAttributeAccess : public attribute_translator_interface
{
  public:
  SwitchAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Switch::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Switch" });}
};


class AdministratorCommissioningAttributeAccess : public attribute_translator_interface
{
  public:
  AdministratorCommissioningAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::AdministratorCommissioning::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "AdministratorCommissioning" });}
};


class OperationalCredentialsAttributeAccess : public attribute_translator_interface
{
  public:
  OperationalCredentialsAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::OperationalCredentials::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "OperationalCredentials" });}
};


class GroupKeyManagementAttributeAccess : public attribute_translator_interface
{
  public:
  GroupKeyManagementAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::GroupKeyManagement::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "GroupKeyManagement" });}
};


class FixedLabelAttributeAccess : public attribute_translator_interface
{
  public:
  FixedLabelAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::FixedLabel::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "FixedLabel" });}
};


class UserLabelAttributeAccess : public attribute_translator_interface
{
  public:
  UserLabelAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::UserLabel::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "UserLabel" });}
};


class ProxyConfigurationAttributeAccess : public attribute_translator_interface
{
  public:
  ProxyConfigurationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ProxyConfiguration::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ProxyConfiguration" });}
};


class ProxyDiscoveryAttributeAccess : public attribute_translator_interface
{
  public:
  ProxyDiscoveryAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ProxyDiscovery::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ProxyDiscovery" });}
};


class ProxyValidAttributeAccess : public attribute_translator_interface
{
  public:
  ProxyValidAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ProxyValid::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ProxyValid" });}
};


class BooleanStateAttributeAccess : public attribute_translator_interface
{
  public:
  BooleanStateAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::BooleanState::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "BooleanState" });}
};


class ModeSelectAttributeAccess : public attribute_translator_interface
{
  public:
  ModeSelectAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ModeSelect::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ModeSelect" });}
};


class DoorLockAttributeAccess : public attribute_translator_interface
{
  public:
  DoorLockAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::DoorLock::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "DoorLock" });}
};


class WindowCoveringAttributeAccess : public attribute_translator_interface
{
  public:
  WindowCoveringAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::WindowCovering::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "WindowCovering" });}
};


class BarrierControlAttributeAccess : public attribute_translator_interface
{
  public:
  BarrierControlAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::BarrierControl::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "BarrierControl" });}
};


class PumpConfigurationAndControlAttributeAccess : public attribute_translator_interface
{
  public:
  PumpConfigurationAndControlAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::PumpConfigurationAndControl::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "PumpConfigurationAndControl" });}
};


class ThermostatAttributeAccess : public attribute_translator_interface
{
  public:
  ThermostatAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Thermostat::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Thermostat" });}
};


class FanControlAttributeAccess : public attribute_translator_interface
{
  public:
  FanControlAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::FanControl::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "FanControl" });}
};


class ThermostatUserInterfaceConfigurationAttributeAccess : public attribute_translator_interface
{
  public:
  ThermostatUserInterfaceConfigurationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ThermostatUserInterfaceConfiguration::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ThermostatUserInterfaceConfiguration" });}
};


class ColorControlAttributeAccess : public attribute_translator_interface
{
  public:
  ColorControlAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ColorControl::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ColorControl" });}
};


class BallastConfigurationAttributeAccess : public attribute_translator_interface
{
  public:
  BallastConfigurationAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::BallastConfiguration::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "BallastConfiguration" });}
};


class IlluminanceMeasurementAttributeAccess : public attribute_translator_interface
{
  public:
  IlluminanceMeasurementAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::IlluminanceMeasurement::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "IlluminanceMeasurement" });}
};


class TemperatureMeasurementAttributeAccess : public attribute_translator_interface
{
  public:
  TemperatureMeasurementAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::TemperatureMeasurement::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "TemperatureMeasurement" });}
};


class PressureMeasurementAttributeAccess : public attribute_translator_interface
{
  public:
  PressureMeasurementAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::PressureMeasurement::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "PressureMeasurement" });}
};


class FlowMeasurementAttributeAccess : public attribute_translator_interface
{
  public:
  FlowMeasurementAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::FlowMeasurement::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "FlowMeasurement" });}
};


class RelativeHumidityMeasurementAttributeAccess : public attribute_translator_interface
{
  public:
  RelativeHumidityMeasurementAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::RelativeHumidityMeasurement::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "RelativeHumidityMeasurement" });}
};


class OccupancySensingAttributeAccess : public attribute_translator_interface
{
  public:
  OccupancySensingAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::OccupancySensing::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "OccupancySensing" });}
};


class WakeOnLanAttributeAccess : public attribute_translator_interface
{
  public:
  WakeOnLanAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::WakeOnLan::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "WakeOnLan" });}
};


class ChannelAttributeAccess : public attribute_translator_interface
{
  public:
  ChannelAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::Channel::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "Channel" });}
};


class TargetNavigatorAttributeAccess : public attribute_translator_interface
{
  public:
  TargetNavigatorAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::TargetNavigator::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "TargetNavigator" });}
};


class MediaPlaybackAttributeAccess : public attribute_translator_interface
{
  public:
  MediaPlaybackAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::MediaPlayback::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "MediaPlayback" });}
};


class MediaInputAttributeAccess : public attribute_translator_interface
{
  public:
  MediaInputAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::MediaInput::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "MediaInput" });}
};


class LowPowerAttributeAccess : public attribute_translator_interface
{
  public:
  LowPowerAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::LowPower::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "LowPower" });}
};


class KeypadInputAttributeAccess : public attribute_translator_interface
{
  public:
  KeypadInputAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::KeypadInput::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "KeypadInput" });}
};


class ContentLauncherAttributeAccess : public attribute_translator_interface
{
  public:
  ContentLauncherAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ContentLauncher::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ContentLauncher" });}
};


class AudioOutputAttributeAccess : public attribute_translator_interface
{
  public:
  AudioOutputAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::AudioOutput::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "AudioOutput" });}
};


class ApplicationLauncherAttributeAccess : public attribute_translator_interface
{
  public:
  ApplicationLauncherAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ApplicationLauncher::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ApplicationLauncher" });}
};


class ApplicationBasicAttributeAccess : public attribute_translator_interface
{
  public:
  ApplicationBasicAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ApplicationBasic::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ApplicationBasic" });}
};


class AccountLoginAttributeAccess : public attribute_translator_interface
{
  public:
  AccountLoginAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::AccountLogin::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "AccountLogin" });}
};


class ElectricalMeasurementAttributeAccess : public attribute_translator_interface
{
  public:
  ElectricalMeasurementAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::ElectricalMeasurement::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "ElectricalMeasurement" });}
};


class TestClusterAttributeAccess : public attribute_translator_interface
{
  public:
  TestClusterAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::TestCluster::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "TestCluster" });}
};


class FaultInjectionAttributeAccess : public attribute_translator_interface
{
  public:
  FaultInjectionAttributeAccess(matter_node_state_monitor &node_state_monitor) :
    attribute_translator_interface(node_state_monitor,
                                   chip::app::Clusters::FaultInjection::Id)
  {}

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath &aPath,
                  chip::app::AttributeValueEncoder &aEncoder) override;
  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath &aPath,
                   chip::app::AttributeValueDecoder &aDecoder) override;

  private:
  void reported_updated(const bridged_endpoint *ep,
                        const std::string& cluster,
                        const std::string& attribute,
                        const nlohmann::json &unify_value) override;

   std::vector<const char *> unify_cluster_names() const override {return std::vector<const char *>({ "FaultInjection" });}
};

}  // namespace unify::matter_bridge
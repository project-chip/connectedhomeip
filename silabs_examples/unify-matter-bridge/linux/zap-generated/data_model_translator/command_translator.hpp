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

/**
 * @defgroup command_translator
 * @brief cluster command translator for matter interface
 *
 * Translate cluster commands from the matter protocol to unify
 * dotdot data model.
 *
 * @{
 */

#ifndef COMMAND_TRANSLATOR_HPP
#define COMMAND_TRANSLATOR_HPP

#pragma once
#include "command_translator_interface.hpp"

namespace unify::matter_bridge {

class IdentifyClusterCommandHandler : public command_translator_interface
{
public:
    IdentifyClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                  group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Identify::Id, "Identify", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ScenesClusterCommandHandler : public command_translator_interface
{
public:
    ScenesClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Scenes::Id, "Scenes", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OnOffClusterCommandHandler : public command_translator_interface
{
public:
    OnOffClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                               group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OnOff::Id, "OnOff", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OnOffSwitchConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    OnOffSwitchConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                  group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OnOffSwitchConfiguration::Id,
                                     "OnOffSwitchConfiguration", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class LevelControlClusterCommandHandler : public command_translator_interface
{
public:
    LevelControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                      group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::LevelControl::Id, "Level", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BinaryInputBasicClusterCommandHandler : public command_translator_interface
{
public:
    BinaryInputBasicClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                          group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BinaryInputBasic::Id, "BinaryInputBasic", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PulseWidthModulationClusterCommandHandler : public command_translator_interface
{
public:
    PulseWidthModulationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                              group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PulseWidthModulation::Id, "PulseWidthModulation",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BasicClusterCommandHandler : public command_translator_interface
{
public:
    BasicClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                               group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Basic::Id, "Basic", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OtaSoftwareUpdateProviderClusterCommandHandler : public command_translator_interface
{
public:
    OtaSoftwareUpdateProviderClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                   group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OtaSoftwareUpdateProvider::Id,
                                     "OtaSoftwareUpdateProvider", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OtaSoftwareUpdateRequestorClusterCommandHandler : public command_translator_interface
{
public:
    OtaSoftwareUpdateRequestorClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                    group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OtaSoftwareUpdateRequestor::Id,
                                     "OtaSoftwareUpdateRequestor", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class LocalizationConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    LocalizationConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                   group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::LocalizationConfiguration::Id,
                                     "LocalizationConfiguration", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TimeFormatLocalizationClusterCommandHandler : public command_translator_interface
{
public:
    TimeFormatLocalizationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TimeFormatLocalization::Id, "TimeFormatLocalization",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class UnitLocalizationClusterCommandHandler : public command_translator_interface
{
public:
    UnitLocalizationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                          group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::UnitLocalization::Id, "UnitLocalization", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PowerSourceConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    PowerSourceConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                  group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PowerSourceConfiguration::Id,
                                     "PowerSourceConfiguration", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PowerSourceClusterCommandHandler : public command_translator_interface
{
public:
    PowerSourceClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                     group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PowerSource::Id, "PowerSource", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class GeneralCommissioningClusterCommandHandler : public command_translator_interface
{
public:
    GeneralCommissioningClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                              group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::GeneralCommissioning::Id, "GeneralCommissioning",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class DiagnosticLogsClusterCommandHandler : public command_translator_interface
{
public:
    DiagnosticLogsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                        group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::DiagnosticLogs::Id, "DiagnosticLogs", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class GeneralDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    GeneralDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                            group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::GeneralDiagnostics::Id, "GeneralDiagnostics",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class SoftwareDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    SoftwareDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                             group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::SoftwareDiagnostics::Id, "SoftwareDiagnostics",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ThreadNetworkDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    ThreadNetworkDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                  group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ThreadNetworkDiagnostics::Id,
                                     "ThreadNetworkDiagnostics", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class WiFiNetworkDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    WiFiNetworkDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::WiFiNetworkDiagnostics::Id, "WiFiNetworkDiagnostics",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TimeSynchronizationClusterCommandHandler : public command_translator_interface
{
public:
    TimeSynchronizationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                             group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TimeSynchronization::Id, "TimeSynchronization",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class SwitchClusterCommandHandler : public command_translator_interface
{
public:
    SwitchClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Switch::Id, "Switch", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OperationalCredentialsClusterCommandHandler : public command_translator_interface
{
public:
    OperationalCredentialsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OperationalCredentials::Id, "OperationalCredentials",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class GroupKeyManagementClusterCommandHandler : public command_translator_interface
{
public:
    GroupKeyManagementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                            group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::GroupKeyManagement::Id, "GroupKeyManagement",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class FixedLabelClusterCommandHandler : public command_translator_interface
{
public:
    FixedLabelClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                    group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::FixedLabel::Id, "FixedLabel", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class UserLabelClusterCommandHandler : public command_translator_interface
{
public:
    UserLabelClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                   group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::UserLabel::Id, "UserLabel", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ProxyConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    ProxyConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                            group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ProxyConfiguration::Id, "ProxyConfiguration",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ProxyDiscoveryClusterCommandHandler : public command_translator_interface
{
public:
    ProxyDiscoveryClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                        group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ProxyDiscovery::Id, "ProxyDiscovery", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ProxyValidClusterCommandHandler : public command_translator_interface
{
public:
    ProxyValidClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                    group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ProxyValid::Id, "ProxyValid", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BooleanStateClusterCommandHandler : public command_translator_interface
{
public:
    BooleanStateClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                      group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BooleanState::Id, "BooleanState", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ModeSelectClusterCommandHandler : public command_translator_interface
{
public:
    ModeSelectClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                    group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ModeSelect::Id, "ModeSelect", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class DoorLockClusterCommandHandler : public command_translator_interface
{
public:
    DoorLockClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                  group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::DoorLock::Id, "DoorLock", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class WindowCoveringClusterCommandHandler : public command_translator_interface
{
public:
    WindowCoveringClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                        group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::WindowCovering::Id, "WindowCovering", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BarrierControlClusterCommandHandler : public command_translator_interface
{
public:
    BarrierControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                        group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BarrierControl::Id, "BarrierControl", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ThermostatClusterCommandHandler : public command_translator_interface
{
public:
    ThermostatClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                    group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Thermostat::Id, "Thermostat", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class FanControlClusterCommandHandler : public command_translator_interface
{
public:
    FanControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                    group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::FanControl::Id, "FanControl", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ThermostatUserInterfaceConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    ThermostatUserInterfaceConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor,
                                                              UnifyMqtt & unify_mqtt, group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ThermostatUserInterfaceConfiguration::Id,
                                     "ThermostatUserInterfaceConfiguration", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ColorControlClusterCommandHandler : public command_translator_interface
{
public:
    ColorControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                      group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ColorControl::Id, "ColorControl", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class IlluminanceMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    IlluminanceMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::IlluminanceMeasurement::Id, "IlluminanceMeasurement",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TemperatureMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    TemperatureMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TemperatureMeasurement::Id, "TemperatureMeasurement",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PressureMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    PressureMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                             group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PressureMeasurement::Id, "PressureMeasurement",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class FlowMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    FlowMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                         group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::FlowMeasurement::Id, "FlowMeasurement", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class RelativeHumidityMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    RelativeHumidityMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                                     group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::RelativeHumidityMeasurement::Id,
                                     "RelativeHumidityMeasurement", unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OccupancySensingClusterCommandHandler : public command_translator_interface
{
public:
    OccupancySensingClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                          group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OccupancySensing::Id, "OccupancySensing", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class WakeOnLanClusterCommandHandler : public command_translator_interface
{
public:
    WakeOnLanClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                   group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::WakeOnLan::Id, "WakeOnLan", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ChannelClusterCommandHandler : public command_translator_interface
{
public:
    ChannelClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                 group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Channel::Id, "Channel", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TargetNavigatorClusterCommandHandler : public command_translator_interface
{
public:
    TargetNavigatorClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                         group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TargetNavigator::Id, "TargetNavigator", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class MediaPlaybackClusterCommandHandler : public command_translator_interface
{
public:
    MediaPlaybackClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                       group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::MediaPlayback::Id, "MediaPlayback", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class MediaInputClusterCommandHandler : public command_translator_interface
{
public:
    MediaInputClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                    group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::MediaInput::Id, "MediaInput", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class LowPowerClusterCommandHandler : public command_translator_interface
{
public:
    LowPowerClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                  group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::LowPower::Id, "LowPower", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class KeypadInputClusterCommandHandler : public command_translator_interface
{
public:
    KeypadInputClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                     group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::KeypadInput::Id, "KeypadInput", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ContentLauncherClusterCommandHandler : public command_translator_interface
{
public:
    ContentLauncherClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                         group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ContentLauncher::Id, "ContentLauncher", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class AudioOutputClusterCommandHandler : public command_translator_interface
{
public:
    AudioOutputClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                     group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::AudioOutput::Id, "AudioOutput", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ApplicationLauncherClusterCommandHandler : public command_translator_interface
{
public:
    ApplicationLauncherClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                             group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ApplicationLauncher::Id, "ApplicationLauncher",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ApplicationBasicClusterCommandHandler : public command_translator_interface
{
public:
    ApplicationBasicClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                          group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ApplicationBasic::Id, "ApplicationBasic", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class AccountLoginClusterCommandHandler : public command_translator_interface
{
public:
    AccountLoginClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                      group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::AccountLogin::Id, "AccountLogin", unify_mqtt,
                                     group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ElectricalMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    ElectricalMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                                               group_translator & group_translator_m) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ElectricalMeasurement::Id, "ElectricalMeasurement",
                                     unify_mqtt, group_translator_m)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};

} // namespace unify::matter_bridge

#endif // COMMAND_TRANSLATOR_HPP
/** @} end command_translator */

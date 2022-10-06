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
    IdentifyClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Identify::Id, "Identify", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ScenesClusterCommandHandler : public command_translator_interface
{
public:
    ScenesClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Scenes::Id, "Scenes", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OnOffClusterCommandHandler : public command_translator_interface
{
public:
    OnOffClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OnOff::Id, "OnOff", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OnOffSwitchConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    OnOffSwitchConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OnOffSwitchConfiguration::Id,
                                     "OnOffSwitchConfiguration", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class LevelControlClusterCommandHandler : public command_translator_interface
{
public:
    LevelControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::LevelControl::Id, "Level", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BinaryInputBasicClusterCommandHandler : public command_translator_interface
{
public:
    BinaryInputBasicClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BinaryInputBasic::Id, "BinaryInputBasic", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PulseWidthModulationClusterCommandHandler : public command_translator_interface
{
public:
    PulseWidthModulationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PulseWidthModulation::Id, "PulseWidthModulation",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ActionsClusterCommandHandler : public command_translator_interface
{
public:
    ActionsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Actions::Id, "Actions", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BasicClusterCommandHandler : public command_translator_interface
{
public:
    BasicClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Basic::Id, "Basic", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OtaSoftwareUpdateProviderClusterCommandHandler : public command_translator_interface
{
public:
    OtaSoftwareUpdateProviderClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OtaSoftwareUpdateProvider::Id,
                                     "OtaSoftwareUpdateProvider", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OtaSoftwareUpdateRequestorClusterCommandHandler : public command_translator_interface
{
public:
    OtaSoftwareUpdateRequestorClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OtaSoftwareUpdateRequestor::Id,
                                     "OtaSoftwareUpdateRequestor", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class LocalizationConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    LocalizationConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::LocalizationConfiguration::Id,
                                     "LocalizationConfiguration", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TimeFormatLocalizationClusterCommandHandler : public command_translator_interface
{
public:
    TimeFormatLocalizationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TimeFormatLocalization::Id, "TimeFormatLocalization",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class UnitLocalizationClusterCommandHandler : public command_translator_interface
{
public:
    UnitLocalizationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::UnitLocalization::Id, "UnitLocalization", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PowerSourceConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    PowerSourceConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PowerSourceConfiguration::Id,
                                     "PowerSourceConfiguration", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PowerSourceClusterCommandHandler : public command_translator_interface
{
public:
    PowerSourceClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PowerSource::Id, "PowerSource", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class GeneralCommissioningClusterCommandHandler : public command_translator_interface
{
public:
    GeneralCommissioningClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::GeneralCommissioning::Id, "GeneralCommissioning",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class DiagnosticLogsClusterCommandHandler : public command_translator_interface
{
public:
    DiagnosticLogsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::DiagnosticLogs::Id, "DiagnosticLogs", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class GeneralDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    GeneralDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::GeneralDiagnostics::Id, "GeneralDiagnostics",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class SoftwareDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    SoftwareDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::SoftwareDiagnostics::Id, "SoftwareDiagnostics",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ThreadNetworkDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    ThreadNetworkDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ThreadNetworkDiagnostics::Id,
                                     "ThreadNetworkDiagnostics", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class WiFiNetworkDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    WiFiNetworkDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::WiFiNetworkDiagnostics::Id, "WiFiNetworkDiagnostics",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class EthernetNetworkDiagnosticsClusterCommandHandler : public command_translator_interface
{
public:
    EthernetNetworkDiagnosticsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::EthernetNetworkDiagnostics::Id,
                                     "EthernetNetworkDiagnostics", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TimeSynchronizationClusterCommandHandler : public command_translator_interface
{
public:
    TimeSynchronizationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TimeSynchronization::Id, "TimeSynchronization",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BridgedDeviceBasicClusterCommandHandler : public command_translator_interface
{
public:
    BridgedDeviceBasicClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BridgedDeviceBasic::Id, "BridgedDeviceBasic",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class SwitchClusterCommandHandler : public command_translator_interface
{
public:
    SwitchClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Switch::Id, "Switch", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class AdministratorCommissioningClusterCommandHandler : public command_translator_interface
{
public:
    AdministratorCommissioningClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::AdministratorCommissioning::Id,
                                     "AdministratorCommissioning", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OperationalCredentialsClusterCommandHandler : public command_translator_interface
{
public:
    OperationalCredentialsClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OperationalCredentials::Id, "OperationalCredentials",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class GroupKeyManagementClusterCommandHandler : public command_translator_interface
{
public:
    GroupKeyManagementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::GroupKeyManagement::Id, "GroupKeyManagement",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class FixedLabelClusterCommandHandler : public command_translator_interface
{
public:
    FixedLabelClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::FixedLabel::Id, "FixedLabel", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class UserLabelClusterCommandHandler : public command_translator_interface
{
public:
    UserLabelClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::UserLabel::Id, "UserLabel", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ProxyConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    ProxyConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ProxyConfiguration::Id, "ProxyConfiguration",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ProxyDiscoveryClusterCommandHandler : public command_translator_interface
{
public:
    ProxyDiscoveryClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ProxyDiscovery::Id, "ProxyDiscovery", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ProxyValidClusterCommandHandler : public command_translator_interface
{
public:
    ProxyValidClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ProxyValid::Id, "ProxyValid", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BooleanStateClusterCommandHandler : public command_translator_interface
{
public:
    BooleanStateClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BooleanState::Id, "BooleanState", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ModeSelectClusterCommandHandler : public command_translator_interface
{
public:
    ModeSelectClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ModeSelect::Id, "ModeSelect", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class DoorLockClusterCommandHandler : public command_translator_interface
{
public:
    DoorLockClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::DoorLock::Id, "DoorLock", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class WindowCoveringClusterCommandHandler : public command_translator_interface
{
public:
    WindowCoveringClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::WindowCovering::Id, "WindowCovering", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BarrierControlClusterCommandHandler : public command_translator_interface
{
public:
    BarrierControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BarrierControl::Id, "BarrierControl", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PumpConfigurationAndControlClusterCommandHandler : public command_translator_interface
{
public:
    PumpConfigurationAndControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PumpConfigurationAndControl::Id,
                                     "PumpConfigurationAndControl", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ThermostatClusterCommandHandler : public command_translator_interface
{
public:
    ThermostatClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Thermostat::Id, "Thermostat", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class FanControlClusterCommandHandler : public command_translator_interface
{
public:
    FanControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::FanControl::Id, "FanControl", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ThermostatUserInterfaceConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    ThermostatUserInterfaceConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor,
                                                              UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ThermostatUserInterfaceConfiguration::Id,
                                     "ThermostatUserInterfaceConfiguration", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ColorControlClusterCommandHandler : public command_translator_interface
{
public:
    ColorControlClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ColorControl::Id, "ColorControl", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class BallastConfigurationClusterCommandHandler : public command_translator_interface
{
public:
    BallastConfigurationClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::BallastConfiguration::Id, "BallastConfiguration",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class IlluminanceMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    IlluminanceMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::IlluminanceMeasurement::Id, "IlluminanceMeasurement",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TemperatureMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    TemperatureMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TemperatureMeasurement::Id, "TemperatureMeasurement",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class PressureMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    PressureMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::PressureMeasurement::Id, "PressureMeasurement",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class FlowMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    FlowMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::FlowMeasurement::Id, "FlowMeasurement", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class RelativeHumidityMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    RelativeHumidityMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::RelativeHumidityMeasurement::Id,
                                     "RelativeHumidityMeasurement", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class OccupancySensingClusterCommandHandler : public command_translator_interface
{
public:
    OccupancySensingClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::OccupancySensing::Id, "OccupancySensing", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class WakeOnLanClusterCommandHandler : public command_translator_interface
{
public:
    WakeOnLanClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::WakeOnLan::Id, "WakeOnLan", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ChannelClusterCommandHandler : public command_translator_interface
{
public:
    ChannelClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Channel::Id, "Channel", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class TargetNavigatorClusterCommandHandler : public command_translator_interface
{
public:
    TargetNavigatorClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::TargetNavigator::Id, "TargetNavigator", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class MediaPlaybackClusterCommandHandler : public command_translator_interface
{
public:
    MediaPlaybackClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::MediaPlayback::Id, "MediaPlayback", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class MediaInputClusterCommandHandler : public command_translator_interface
{
public:
    MediaInputClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::MediaInput::Id, "MediaInput", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class LowPowerClusterCommandHandler : public command_translator_interface
{
public:
    LowPowerClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::LowPower::Id, "LowPower", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class KeypadInputClusterCommandHandler : public command_translator_interface
{
public:
    KeypadInputClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::KeypadInput::Id, "KeypadInput", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ContentLauncherClusterCommandHandler : public command_translator_interface
{
public:
    ContentLauncherClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ContentLauncher::Id, "ContentLauncher", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class AudioOutputClusterCommandHandler : public command_translator_interface
{
public:
    AudioOutputClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::AudioOutput::Id, "AudioOutput", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ApplicationLauncherClusterCommandHandler : public command_translator_interface
{
public:
    ApplicationLauncherClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ApplicationLauncher::Id, "ApplicationLauncher",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ApplicationBasicClusterCommandHandler : public command_translator_interface
{
public:
    ApplicationBasicClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ApplicationBasic::Id, "ApplicationBasic", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class AccountLoginClusterCommandHandler : public command_translator_interface
{
public:
    AccountLoginClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::AccountLogin::Id, "AccountLogin", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class ElectricalMeasurementClusterCommandHandler : public command_translator_interface
{
public:
    ElectricalMeasurementClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::ElectricalMeasurement::Id, "ElectricalMeasurement",
                                     uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};
class FaultInjectionClusterCommandHandler : public command_translator_interface
{
public:
    FaultInjectionClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UicMqtt & uic_mqtt) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::FaultInjection::Id, "FaultInjection", uic_mqtt)
    {}
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & HandlerContext) override;
};

} // namespace unify::matter_bridge

#endif // COMMAND_TRANSLATOR_HPP
/** @} end command_translator */

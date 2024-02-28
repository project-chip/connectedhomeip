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

class IdentifyClusterCommandHandler : public command_translator_interface {
public:
    IdentifyClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::Identify::Id, "Identify", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            64,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class GroupsClusterCommandHandler : public command_translator_interface {
public:
    GroupsClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::Groups::Id, "Groups", unify_mqtt, group_translator_m,
              dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
            2,
            3,
            4,
            5,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class OnOffClusterCommandHandler : public command_translator_interface {
public:
    OnOffClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::OnOff::Id, "OnOff", unify_mqtt, group_translator_m,
              dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
            2,
            64,
            65,
            66,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class LevelControlClusterCommandHandler : public command_translator_interface {
public:
    LevelControlClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::LevelControl::Id, "Level", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class DoorLockClusterCommandHandler : public command_translator_interface {
public:
    DoorLockClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::DoorLock::Id, "DoorLock", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    ///
    /// The command SetAliroReaderConfig is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_SetAliroReaderConfig(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::SetAliroReaderConfig::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command ClearAliroReaderConfig is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_ClearAliroReaderConfig(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::ClearAliroReaderConfig::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
            3,
            11,
            12,
            13,
            14,
            15,
            16,
            17,
            18,
            19,
            26,
            27,
            29,
            34,
            36,
            38,
            39,
            40,
            41,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>({ "DoorLock" }); }
    void command_response(const bridged_endpoint* ep, const std::string& cluster, const std::string& cmd_response,
        const nlohmann::json& unify_value, CommandHandler::Handle& cmd_handle) override;
};
class BarrierControlClusterCommandHandler : public command_translator_interface {
public:
    BarrierControlClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::BarrierControl::Id, "BarrierControl", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class ThermostatClusterCommandHandler : public command_translator_interface {
public:
    ThermostatClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::Thermostat::Id, "Thermostat", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    ///
    /// The command SetActiveScheduleRequest is not defined in UCL and must be manually handled
    ///
    virtual void
    Invoke_SetActiveScheduleRequest(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::Thermostat::Commands::SetActiveScheduleRequest::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command SetActivePresetRequest is not defined in UCL and must be manually handled
    ///
    virtual void
    Invoke_SetActivePresetRequest(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::Thermostat::Commands::SetActivePresetRequest::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command StartPresetsSchedulesEditRequest is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_StartPresetsSchedulesEditRequest(
        CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::Thermostat::Commands::StartPresetsSchedulesEditRequest::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command CancelPresetsSchedulesEditRequest is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_CancelPresetsSchedulesEditRequest(
        CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::Thermostat::Commands::CancelPresetsSchedulesEditRequest::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command CommitPresetsSchedulesRequest is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_CommitPresetsSchedulesRequest(
        CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::Thermostat::Commands::CommitPresetsSchedulesRequest::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command CancelSetActivePresetRequest is not defined in UCL and must be manually handled
    ///
    virtual void
    Invoke_CancelSetActivePresetRequest(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::Thermostat::Commands::CancelSetActivePresetRequest::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command SetTemperatureSetpointHoldPolicy is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_SetTemperatureSetpointHoldPolicy(
        CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::Thermostat::Commands::SetTemperatureSetpointHoldPolicy::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
            2,
            3,
            5,
            6,
            7,
            8,
            9,
            10,
            11,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class FanControlClusterCommandHandler : public command_translator_interface {
public:
    FanControlClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::FanControl::Id, "FanControl", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    ///
    /// The command Step is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_Step(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::FanControl::Commands::Step::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class ThermostatUserInterfaceConfigurationClusterCommandHandler : public command_translator_interface {
public:
    ThermostatUserInterfaceConfigurationClusterCommandHandler(matter_node_state_monitor& node_state_monitor,
        UnifyMqtt& unify_mqtt, group_translator& group_translator_m,
        device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::ThermostatUserInterfaceConfiguration::Id,
              "ThermostatUserInterfaceConfiguration", unify_mqtt, group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {};
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class ColorControlClusterCommandHandler : public command_translator_interface {
public:
    ColorControlClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::ColorControl::Id, "ColorControl", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8,
            9,
            10,
            64,
            65,
            66,
            67,
            68,
            71,
            75,
            76,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class IlluminanceMeasurementClusterCommandHandler : public command_translator_interface {
public:
    IlluminanceMeasurementClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::IlluminanceMeasurement::Id, "IlluminanceMeasurement",
              unify_mqtt, group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {};
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class TemperatureMeasurementClusterCommandHandler : public command_translator_interface {
public:
    TemperatureMeasurementClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::TemperatureMeasurement::Id, "TemperatureMeasurement",
              unify_mqtt, group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {};
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class PressureMeasurementClusterCommandHandler : public command_translator_interface {
public:
    PressureMeasurementClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::PressureMeasurement::Id, "PressureMeasurement",
              unify_mqtt, group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {};
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class FlowMeasurementClusterCommandHandler : public command_translator_interface {
public:
    FlowMeasurementClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::FlowMeasurement::Id, "FlowMeasurement", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {};
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class RelativeHumidityMeasurementClusterCommandHandler : public command_translator_interface {
public:
    RelativeHumidityMeasurementClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::RelativeHumidityMeasurement::Id, "RelativityHumidity",
              unify_mqtt, group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {};
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class OccupancySensingClusterCommandHandler : public command_translator_interface {
public:
    OccupancySensingClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::OccupancySensing::Id, "OccupancySensing", unify_mqtt,
              group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {};
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
class ElectricalMeasurementClusterCommandHandler : public command_translator_interface {
public:
    ElectricalMeasurementClusterCommandHandler(matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m, device_translator& dev_translator)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::ElectricalMeasurement::Id, "ElectricalMeasurement",
              unify_mqtt, group_translator_m, dev_translator)
        , m_dev_translator(dev_translator)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    virtual CHIP_ERROR EnumerateAcceptedCommands(const chip::app::ConcreteClusterPath& cluster, CommandIdCallback callback,
        void* context) override
    {
        const chip::CommandId all_commands[] = {
            0,
            1,
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }

private:
    device_translator& m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};

} // namespace unify::matter_bridge

#endif // COMMAND_TRANSLATOR_HPP
/** @} end command_translator */

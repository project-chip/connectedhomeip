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
    IdentifyClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::Identify::Id, "Identify", unify_mqtt,
            group_translator_m)
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
};
class GroupsClusterCommandHandler : public command_translator_interface {
public:
    GroupsClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::Groups::Id, "Groups", unify_mqtt, group_translator_m)
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
};
class ScenesClusterCommandHandler : public command_translator_interface {
public:
    ScenesClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::Scenes::Id, "Scenes", unify_mqtt, group_translator_m)
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
};
class OnOffClusterCommandHandler : public command_translator_interface {
public:
    OnOffClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::OnOff::Id, "OnOff", unify_mqtt, group_translator_m)
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
};
class LevelControlClusterCommandHandler : public command_translator_interface {
public:
    LevelControlClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::LevelControl::Id, "Level", unify_mqtt,
            group_translator_m)
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
};
class DoorLockClusterCommandHandler : public command_translator_interface {
public:
    DoorLockClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::DoorLock::Id, "DoorLock", unify_mqtt,
            group_translator_m)
    {
    }
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext& HandlerContext) override;

    ///
    /// The command SetUser is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_SetUser(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command GetUser is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_GetUser(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command ClearUser is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_ClearUser(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command SetCredential is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_SetCredential(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command GetCredentialStatus is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_GetCredentialStatus(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command ClearCredential is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_ClearCredential(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType data)
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, chip::Protocols::InteractionModel::Status::UnsupportedCommand);
        ctxt.SetCommandHandled();
    };
    ///
    /// The command UnboltDoor is not defined in UCL and must be manually handled
    ///
    virtual void Invoke_UnboltDoor(CommandHandlerInterface::HandlerContext& ctxt,
        chip::app::Clusters::DoorLock::Commands::UnboltDoor::DecodableType data)
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
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }
};
class BarrierControlClusterCommandHandler : public command_translator_interface {
public:
    BarrierControlClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::BarrierControl::Id, "BarrierControl", unify_mqtt,
            group_translator_m)
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
};
class ThermostatClusterCommandHandler : public command_translator_interface {
public:
    ThermostatClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::Thermostat::Id, "Thermostat", unify_mqtt,
            group_translator_m)
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
        };
        for (const auto& cmd : all_commands) {
            if (callback(cmd, context) != chip::Loop::Continue) {
                break;
            }
        }

        return CHIP_NO_ERROR;
    }
};
class FanControlClusterCommandHandler : public command_translator_interface {
public:
    FanControlClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::FanControl::Id, "FanControl", unify_mqtt,
            group_translator_m)
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
};
class ThermostatUserInterfaceConfigurationClusterCommandHandler : public command_translator_interface {
public:
    ThermostatUserInterfaceConfigurationClusterCommandHandler(const matter_node_state_monitor& node_state_monitor,
        UnifyMqtt& unify_mqtt, group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::ThermostatUserInterfaceConfiguration::Id,
            "ThermostatUserInterfaceConfiguration", unify_mqtt, group_translator_m)
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
};
class ColorControlClusterCommandHandler : public command_translator_interface {
public:
    ColorControlClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::ColorControl::Id, "ColorControl", unify_mqtt,
            group_translator_m)
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
};
class IlluminanceMeasurementClusterCommandHandler : public command_translator_interface {
public:
    IlluminanceMeasurementClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::IlluminanceMeasurement::Id, "IlluminanceMeasurement",
            unify_mqtt, group_translator_m)
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
};
class TemperatureMeasurementClusterCommandHandler : public command_translator_interface {
public:
    TemperatureMeasurementClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::TemperatureMeasurement::Id, "TemperatureMeasurement",
            unify_mqtt, group_translator_m)
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
};
class PressureMeasurementClusterCommandHandler : public command_translator_interface {
public:
    PressureMeasurementClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::PressureMeasurement::Id, "PressureMeasurement",
            unify_mqtt, group_translator_m)
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
};
class FlowMeasurementClusterCommandHandler : public command_translator_interface {
public:
    FlowMeasurementClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::FlowMeasurement::Id, "FlowMeasurement", unify_mqtt,
            group_translator_m)
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
};
class RelativeHumidityMeasurementClusterCommandHandler : public command_translator_interface {
public:
    RelativeHumidityMeasurementClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::RelativeHumidityMeasurement::Id, "RelativityHumidity",
            unify_mqtt, group_translator_m)
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
};
class OccupancySensingClusterCommandHandler : public command_translator_interface {
public:
    OccupancySensingClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::OccupancySensing::Id, "OccupancySensing", unify_mqtt,
            group_translator_m)
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
};
class ElectricalMeasurementClusterCommandHandler : public command_translator_interface {
public:
    ElectricalMeasurementClusterCommandHandler(const matter_node_state_monitor& node_state_monitor, UnifyMqtt& unify_mqtt,
        group_translator& group_translator_m)
        : command_translator_interface(node_state_monitor, chip::app::Clusters::ElectricalMeasurement::Id, "ElectricalMeasurement",
            unify_mqtt, group_translator_m)
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
};

} // namespace unify::matter_bridge

#endif // COMMAND_TRANSLATOR_HPP
/** @} end command_translator */

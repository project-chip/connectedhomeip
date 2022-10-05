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

#ifndef COMMAND_TRANSLATOR_INTERFACE_HPP
#define COMMAND_TRANSLATOR_INTERFACE_HPP

#include "matter.h"

#include "group_translator.hpp"
#include "matter_node_state_monitor.hpp"
#include "sl_log.h"
#include "uic_mqtt_wrapper.hpp"

namespace unify::matter_bridge {

/**
 * @brief Base class for handling translation of commands
 *
 * This class hold base functionality for command translators, such
 * at registering the translator with the chip::app framework.
 *
 */
class command_translator_interface : public chip::app::CommandHandlerInterface
{
public:
    command_translator_interface(const matter_node_state_monitor & node_state_monitor, chip::ClusterId id, const char * name,
                                 UicMqtt & uic_mqtt) :
        chip::app::CommandHandlerInterface(chip::Optional<chip::EndpointId>::Missing(), id),
        m_node_state_monitor(node_state_monitor), cluster_name(name), m_uic_mqtt(uic_mqtt)
    {
        chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this);
    }

    /**
     * @brief Send a unify command
     *
     * This function sends a unify command, either as a Single cast or a group
     * cast message, depending on the context.
     *
     * @param ctxt
     * @param cmd
     * @param payload
     */
    void send_unify_mqtt_cmd(const CommandHandlerInterface::HandlerContext & ctxt, const std::string & cmd,
                             const nlohmann::json & payload) const
    {
        std::string topic;
        auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
        if (!unify_node)
        {
            return;
        }

        if (ctxt.mCommandHandler.GetExchangeContext() != nullptr)
        {
            if (ctxt.mCommandHandler.GetExchangeContext()->IsGroupExchangeContext())
            {
                auto matter_group_id =
                    ctxt.mCommandHandler.GetExchangeContext()->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
                auto unify_group_id = group_translator::instance().get_unify_group({ matter_group_id });
                if (unify_group_id)
                {
                    topic = "ucl/by-group/" + std::to_string(unify_group_id.value());
                }
                else
                {
                    return;
                }
            }
            else
            {
                topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint);
            }
        }
        else
        {
            topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint);
        }

        topic = topic + "/" + std::string(cluster_name) + "/Commands/" + cmd;

        std::string msg = payload.dump();
        sl_log_debug("command_translator_interface", "--- send_unify_mqtt_cmd %s -> %s ---", topic.c_str(), msg.c_str());
        m_uic_mqtt.Publish(topic, msg, true);
    }

    template <typename T>
    void send_unify_mqtt_command_with_callbacks(const CommandHandlerInterface::HandlerContext & ctxt, T command_data,
                                                void (*send_command_callback)(const char *, uint8_t, T),
                                                void (*send_group_command_callback)(unify_group_t, T)) const
    {
        auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
        if (!unify_node)
        {
            return;
        }

        sl_log_debug("command_translator_interface", "Sending translated command from Matter to Unify to node %s",
                     unify_node->unify_unid.c_str());
        if (ctxt.mCommandHandler.GetExchangeContext()->IsGroupExchangeContext())
        {
            auto matter_group_id =
                ctxt.mCommandHandler.GetExchangeContext()->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
            auto unify_group_id = group_translator::instance().get_unify_group({ matter_group_id });
            if (unify_group_id)
            {
                send_group_command_callback(unify_group_id.value(), command_data);
            }
            else
            {
                return;
            }
        }

        send_command_callback(unify_node->unify_unid.c_str(), unify_node->unify_endpoint, command_data);
    }

protected:
    const matter_node_state_monitor & m_node_state_monitor;
    const char * cluster_name;
    UicMqtt & m_uic_mqtt;
};

} // namespace unify::matter_bridge

#endif // COMMAND_TRANSLATOR_INTERFACE_HPP

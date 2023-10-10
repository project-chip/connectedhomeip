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
#include "unify_mqtt_wrapper.hpp"
#include "cluster_emulator.hpp"
#include <regex>
#include <string>
#include <chrono>

#define HANDLER_TIMEOUT std::chrono::seconds(10)

using namespace chip;
using namespace chip::app;
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
    command_translator_interface(matter_node_state_monitor & node_state_monitor, chip::ClusterId id, const char * name,
                                 UnifyMqtt & unify_mqtt, group_translator & group_translator, device_translator & dev_translator) :
        chip::app::CommandHandlerInterface(chip::Optional<chip::EndpointId>::Missing(), id),
        m_node_state_monitor(node_state_monitor), cluster_name(name), m_unify_mqtt(unify_mqtt), m_group_translator(group_translator), m_dev_translator(dev_translator)
    {
        chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this);
        // Register the an event listener for subscriptions
        auto f = [&](const bridged_endpoint & ep, matter_node_state_monitor::update_t update) {
            commands_response_subscription(ep, update);
        };
        node_state_monitor.register_event_listener(f);
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
                             const nlohmann::json & payload, const std::string & cmd_response = "")
    {
        std::string topic;
        std::string cmd_response_key;
        auto unify_node = m_node_state_monitor.bridged_endpoint(ctxt.mRequestPath.mEndpointId);
        if (!unify_node)
        {
            return;
        }
        
        // At present we can only handle one command per topic, so rejecting duplicate command requests here
        if (!cmd_response.empty())
        {
            // Clear timedout handlers
            for (auto it = cmd_responsestruct_map.begin(); it != cmd_responsestruct_map.end();) {
                if (ishandler_timedout(it->second.timeout))
                {
                    it = cmd_responsestruct_map.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            cmd_response_key =  "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/" 
                                + std::string(cluster_name) + "/GeneratedCommands/" + cmd_response;
            
            if (cmd_responsestruct_map.count(cmd_response_key) > 0)
            {
                // We have a command processing in the backend, reject all incoming commands.
                sl_log_warning("command_translator_interface",
                                 "%s Command is in processing for unid %s endpoint %u , rejecting other incoming commands\n",
                                 cmd.c_str(),unify_node->unify_unid,unify_node->unify_endpoint);
                ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Busy);
                return;
            }
        }    

        if (ctxt.mCommandHandler.GetExchangeContext() != nullptr)
        {
            if (ctxt.mCommandHandler.GetExchangeContext()->IsGroupExchangeContext())
            {
                auto matter_group_id =
                    ctxt.mCommandHandler.GetExchangeContext()->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
                chip::FabricIndex fabric_index = ctxt.mCommandHandler.GetExchangeContext()->GetSessionHandle()->GetFabricIndex();
                auto unify_group_id            = m_group_translator.get_unify_group({ matter_group_id, fabric_index });
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
        std::string msg;
        if (payload.empty())
        {
            msg = "{}";
        }
        else
        {
            msg = payload.dump();
        }
        sl_log_debug("command_translator_interface", "--- send_unify_mqtt_cmd %s -> %s ---", topic.c_str(), msg.c_str());
        m_unify_mqtt.Publish(topic, msg, true);
        
        if (!cmd_response.empty())
        {
            sl_log_debug("command_translator_interface", "--- adding cmd_response_key to map %s ",
                        cmd_response_key.c_str());
            CommandHandler::Handle cmd_response_handle(&ctxt.mCommandHandler);
            commandInfo cmd_response_info;
            cmd_response_info.handler = std::move(cmd_response_handle);
            cmd_response_info.timeout = std::chrono::system_clock::now();
            cmd_responsestruct_map[cmd_response_key] = std::move(cmd_response_info);
        }
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
            auto unify_group_id = m_group_translator.get_unify_group({ matter_group_id });
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

private:
    void on_mqtt_message_cb(const char * topic, const char * message, const size_t message_length)
    {
        std::regex rgx("ucl/by-unid"
                       "/([^/]*)"   // UNID
                       "/ep([^/]*)" // Endpoint
                       "/([^/]*)"   // Cluster
                       "/GeneratedCommands"
                       "/([^/]*)"); // Command response
        std::smatch match;
        std::string topic_str(topic);
        if (!std::regex_search(topic_str, match, rgx))
        {
            return;
        }
        const std::string & unid        = match.str(1);
        const std::string & endpoint_id = match.str(2);
        const std::string & cluster     = match.str(3);
        const std::string & cmd_response   = match.str(4);
        
        auto unify_node = m_node_state_monitor.bridged_endpoint(unid, std::stoi(endpoint_id));
        // In Matter Bridge Endpoint 0 is dedicated to the root node (bridge app)
        // So unify bridged node will not be assigned endpoint 0.
        if (!unify_node)
        {
            sl_log_warning("command_translator_interface", "The bridged node is not assigned a matter dynamic endpoint\n");
            return;
        }

        std::string msg(message, message_length);
        if (!cmd_response.empty() && !cluster.empty() && !msg.empty())
        {    
            try
            {
                commandInfo& commandHandleRef = cmd_responsestruct_map.at(topic);
                if(ishandler_timedout(commandHandleRef.timeout)){
                    cmd_responsestruct_map.erase(topic);
                    sl_log_warning("command_translator_interface", "Response recieved for %s,but handler has been timedout,ignoring response\n",topic);
                    return;
                }    
                
                nlohmann::json jsn = nlohmann::json::parse(msg);
                // At present only handling the status type of command response.
                command_response(unify_node, cluster, cmd_response, jsn, commandHandleRef.handler);
            }  catch ( const std::out_of_range& e) {
                sl_log_warning("command_translator_interface", "command handle not found for repsonse %s\n",topic);
            }  catch (const nlohmann::json::parse_error & e)
            {
                sl_log_warning("command_translator_interface",
                               "Error parsing Command %s response for %s/ep%s , \
                               as the payload does not appear to be correct JSON, %s\n",
                               cmd_response.c_str(), unid.c_str(), endpoint_id.c_str(),
                               e.what());
            } catch (const nlohmann::json::type_error & e)
            {
                sl_log_warning("command_translator_interface",
                               "Error parsing command %s response for %s/ep%s , \
                               as the value is of different type or key is unknown to us, %s\n%s\n",
                               cmd_response.c_str(), unid.c_str(), endpoint_id.c_str(),
                               e.what(), msg.c_str());
            }
        }
        else
        {
            sl_log_warning("command_translator_interface", "Unknown command response [%s]", cmd_response.c_str());
        }
    }

    void commands_response_subscription(const bridged_endpoint & ep, matter_node_state_monitor::update_t update)
    {
        sl_log_debug("command_translator_interface","Matter endpoint: %d", ep.matter_endpoint);
        for (const auto & unify_cluster : unify_cluster_names())
        {
            std::string topic = "ucl/by-unid/" + ep.unify_unid + "/ep" + std::to_string(ep.unify_endpoint) + "/" 
                                + unify_cluster + "/GeneratedCommands/+";

            if (update == matter_node_state_monitor::update_t::NODE_ADDED)
            {
                m_unify_mqtt.SubscribeEx(topic.c_str(), command_translator_interface::on_mqtt_message_c_cb, this);
            }
            else if (update == matter_node_state_monitor::update_t::NODE_DELETED)
            {
                m_unify_mqtt.UnsubscribeEx(topic.c_str(), command_translator_interface::on_mqtt_message_c_cb, this);
            }
        }
    }

    static void on_mqtt_message_c_cb(const char * topic, const char * message, const size_t message_length, void * user)
    {
        command_translator_interface * instance = static_cast<command_translator_interface *>(user);
        if (instance)
        {
            instance->on_mqtt_message_cb(topic, message, message_length);
        }
    }

    bool ishandler_timedout(const std::chrono::system_clock::time_point& startTimestamp) {
        auto currentTimestamp = std::chrono::system_clock::now();
        std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds>(currentTimestamp - startTimestamp);
        if (duration > HANDLER_TIMEOUT)
        {
            return true;
        }
        else
        {
            return false;
        }
    }


protected:
    struct commandInfo {
        CommandHandler::Handle handler;
        std::chrono::system_clock::time_point timeout;
    };

    std::unordered_map<std::string, commandInfo> cmd_responsestruct_map;
    matter_node_state_monitor & m_node_state_monitor;
    const char * cluster_name;
    UnifyMqtt & m_unify_mqtt;
    group_translator & m_group_translator;
    device_translator & m_dev_translator;

     /**
     * @brief List of cluster names which this translator will be using.
     *
     * @return std::vector<const char *>
     */
    virtual std::vector<const char *> unify_cluster_names() const = 0;
    
    /**
     * @brief Called when a Command response is recieved
     *
     * @param ep
     * @param cluster
     * @param command
     * @param value
     */
    virtual void command_response(const bridged_endpoint * ep, const std::string & cluster, const std::string & cmd_response,
                                  const nlohmann::json & value, CommandHandler::Handle &cmd_handle) {};
};

} // namespace unify::matter_bridge

#endif // COMMAND_TRANSLATOR_INTERFACE_HPP

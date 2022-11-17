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

#ifndef MOCK_UNIFY_MQTT_HPP
#define MOCK_UNIFY_MQTT_HPP

#include "unify_mqtt_wrapper.hpp"

namespace unify::matter_bridge {
namespace Test {

class MockUnifyMqtt : public unify::matter_bridge::UnifyMqtt
{
public:
    // Overwriting publish function and saving specifics of the function
    void Publish(std::string topic, std::string payload, bool retain) override
    {
        publish_topic   = topic;
        publish_payload = payload;
        ++nNumerUicMqttPublishCall;
    }
    int nNumerUicMqttPublishCall = 0;
    std::string publish_topic;
    std::string publish_payload;

    // Overwriting subscribe function and saving specifics of the function
    void SubscribeEx(std::string topic,
                     void (*callback)(const char * topic, const char * message, const size_t message_length, void * user),
                     void * user) override
    {
        subscribe_topic = topic;
        subscribeCB     = callback;
        ++nNumerUicMqttSubscribeCall;
    }
    int nNumerUicMqttSubscribeCall = 0;
    std::string subscribe_topic;
    std::function<void(const char * topic, const char * message, const size_t message_length, void * user)> subscribeCB;

    // Overwriting unsubscribe function and saving specifics of the function
    void UnsubscribeEx(std::string topic,
                       void (*callback)(const char * topic, const char * message, const size_t message_length, void * user),
                       void * user) override
    {
        unsubscribe_topic = topic;
        ++nNumerUicMqttUnsubscribeCall;
    }
    int nNumerUicMqttUnsubscribeCall = 0;
    std::string unsubscribe_topic;
};

} // namespace Test
} // namespace unify::matter_bridge

#endif

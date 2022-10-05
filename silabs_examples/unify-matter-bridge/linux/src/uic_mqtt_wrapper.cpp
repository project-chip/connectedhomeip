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
#include "uic_mqtt.h"
#include "uic_mqtt_wrapper.hpp"

using namespace unify::matter_bridge;

void UicMqtt::Publish(std::string topic, std::string message, bool retain)
{
    uic_mqtt_publish(topic.c_str(), message.c_str(), message.size(), retain);
}
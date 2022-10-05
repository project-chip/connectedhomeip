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
 * @defgroup uic_mqtt_wrapper
 * @brief This file defines a C++ wrapper around the uic_mqtt module.
 * @{
 */

#ifndef UIC_MQTT_WRAPPER_HPP
#define UIC_MQTT_WRAPPER_HPP

#ifdef __cplusplus

#include <stdbool.h>
#include <stddef.h>
#include <string>

namespace unify::matter_bridge {

class UicMqtt
{
public:
    UicMqtt(){};
    virtual void Publish(std::string topic, std::string message, bool retain);
    virtual ~UicMqtt(){};
};

} // namespace unify::matter_bridge

#endif
#endif // UIC_MQTT_WRAPPER_HPP
/** @} end uic_mqtt_wrapper */

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
 * @file matter_bridge_config.h
 * @addtogroup matter_bridge_config Configuration Extension
 * @ingroup components
 *
 * @brief Add the Matter Bridge-specific fixtures to the Unify \ref config system.
 *
 * @{
 */

#if !defined(MATTER_BRIDGE_QR_CODE_PUBLISHER_H)
#define MATTER_BRIDGE_QR_CODE_PUBLISHER_H

#include "unify_mqtt_wrapper.hpp"
#include "platform/PlatformManager.h"
namespace unify::matter_bridge {

  /**
   * @brief Unify Matter Bridge QR code publisher
   * 
   * The role of this component is to publish the paring code on the mqtt broker, such that a 
   * user interface can display the paring code for a matter commissioner to scan. 
   *
   * This component will automatically detect whe the commissioning window has been opened.
   * 
   */
  class QRCodePublisher {
  public:
    QRCodePublisher(UnifyMqtt& unify_mqtt );

    /**
     * @brief Publish the QRCode
     * 
     */
    void publish();

    /**
     * @brief Unretain the qr code
     * 
     */
    void unretain();

  private:
    static void device_event(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t ptr);
    std::string qrcode();

    UnifyMqtt & m_unify_mqtt;
  };

}

#endif
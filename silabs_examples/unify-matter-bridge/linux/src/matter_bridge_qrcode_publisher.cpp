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

#include "matter_bridge_qrcode_publisher.hpp"

#include "uic_mqtt.h"
#include <lib/support/Span.h>
#include "app/server/Server.h"
#include "app/server/OnboardingCodesUtil.h"
#include "setup_payload/QRCodeSetupPayloadGenerator.h"
#include "Options.h"

namespace unify::matter_bridge {

QRCodePublisher::QRCodePublisher(UnifyMqtt& unify_mqtt ) : m_unify_mqtt(unify_mqtt) {
  chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(QRCodePublisher::device_event, reinterpret_cast<intptr_t>(this));
}

void QRCodePublisher::device_event(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t ptr) {  
  
  switch(event->Type) {
    case chip::DeviceLayer::DeviceEventType::kDnssdPlatformInitialized:
      if(chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen()) {
        QRCodePublisher* qr = reinterpret_cast<QRCodePublisher*>(ptr);
        qr->publish();
      }
      break;
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
      reinterpret_cast<QRCodePublisher*>(ptr)->unretain();
      break;
    default:
      break;
  }
}

void QRCodePublisher::publish() {
  auto qrCode_str = qrcode();
  if(!qrCode_str.empty()) {
    m_unify_mqtt.Publish(
      "ucl/SmartStart/CommissionableDevice/"+qrCode_str,
      "{ \"QRCode\" : \""+qrCode_str+ "\" } ",
      true
    );
  }
}

void QRCodePublisher::unretain() {
  auto qrCode_str = qrcode();
  if(!qrCode_str.empty()) {
    m_unify_mqtt.Publish("ucl/SmartStart/CommissionableDevice/"+qrCode_str,"",false);
  }
}

std::string QRCodePublisher::qrcode() {
  char payloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
  chip::MutableCharSpan qrCode(payloadBuffer);
  LinuxDeviceOptions::GetInstance().payload.commissioningFlow = chip::CommissioningFlow::kStandard;
  if( GetQRCode(qrCode,LinuxDeviceOptions::GetInstance().payload)== CHIP_NO_ERROR) {
    return std::string(qrCode.begin(),qrCode.size());
  } else {
    return "";
  }
}
}
/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2022 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef MQTT_TRANSPORT_H
#define MQTT_TRANSPORT_H

#include "mqtt_opts.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "mqtt.h"
#include "altcp_tls.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"

#define SIGNAL_TRANSINTF_MBEDTLS_RX 0x80
#define SIGNAL_TRANSINTF_RX 0x01
#define SIGNAL_TRANSINTF_TX 0x00
#define SIGNAL_TRANSINTF_TX_ACK 0x02
#define SIGNAL_TRANSINTF_CONN_CLOSE 0x04

typedef void (*dic_connect_cb)(err_t);
typedef struct MQTT_Transport_t MQTT_Transport_t;
void transport_process_mbedtls_rx(MQTT_Transport_t *client);
MQTT_Transport_t * MQTT_Transport_Init(mqtt_transport_intf_t *trans,mqtt_client_t* mqtt_client,EventGroupHandle_t dicEvents);
err_t MQTT_Transport_SSLConfigure(MQTT_Transport_t * transP, const u8_t *ca, size_t ca_len, const u8_t *privkey, size_t privkey_len,
                                        const u8_t *privkey_pass, size_t privkey_pass_len,
                                        const u8_t *cert, size_t cert_len);
err_t MQTT_Transport_Connect(MQTT_Transport_t* client,const char *host, u16_t port, dic_connect_cb dic_con_cb);

#endif //MQTT_TRANSPORT_H
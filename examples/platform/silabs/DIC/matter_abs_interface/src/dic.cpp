/**
 * @file
 * @brief Matter abstraction layer for Direct Internet Connectivity.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#include "dic.h"
#include "dic_config.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "silabs_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "MQTT_transport.h"
#include "mqtt.h"

#ifdef __cplusplus
}
#endif

#include <stdbool.h> //for bool
#include <string.h>    // for std::string

#define DIC_TOPIC_PREFIX "silabs/matter/"

static TaskHandle_t dicTask;
static mqtt_client_t *mqtt_client;
static EventGroupHandle_t dicEvents = NULL;
static MQTT_Transport_t *transport = NULL;
static mqtt_transport_intf_t trans;
static bool end_loop;
static bool init_complete;
dic_subscribe_cb gSubsCB = NULL;

static void mqtt_request_cb(void *arg, mqtt_err_t err)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
  (void)client_info;
  SILABS_LOG("DIC: MQTT sub request callback %d", (int)err);
}

dic_err_t dic_mqtt_subscribe(dic_incoming_data_cb_t data_cb, const char * topic, uint8_t qos){
    mqtt_set_inpub_callback(mqtt_client,
      NULL,
      data_cb,
      NULL);
    int mret;
    if((mret = mqtt_subscribe(mqtt_client, topic,qos, mqtt_request_cb, NULL)) != ERR_OK){
        SILABS_LOG("DIC: MQTT subscribe failed %d",mret);
        return DIC_ERR_FAIL;
    }
    return DIC_OK;
}

static void dic_mqtt_conn_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  (void)client;
  (void)arg;

  SILABS_LOG("DIC: MQTT connection status: %u", status);
  if (status != MQTT_CONNECT_ACCEPTED)
  {
    if (dicTask) end_loop = true;
  }
  else
  {
    SILABS_LOG("DIC: MQTT connection accepted successfully");
    if ( gSubsCB != NULL)
      gSubsCB(); // Subscribe callback
  }
}

void dic_tcp_connect_cb(err_t err)
{
  SILABS_LOG("connection callback new");
  if (err == ERR_OK)
  {
    struct mqtt_connect_client_info_t connect_info;
    mqtt_err_t mret;
    /* Connect to MQTT broker/cloud */
    memset(&connect_info, 0, sizeof(connect_info));

    connect_info.client_id = DIC_CLIENT_ID;
    connect_info.client_user = DIC_CLIENT_USER;
    connect_info.client_pass = DIC_CLIENT_PASS;
    connect_info.keep_alive = DIC_KEEP_ALIVE;

    if ((mret = mqtt_client_connect(mqtt_client, (void *)&trans, dic_mqtt_conn_cb, NULL, &connect_info)) != ERR_OK)
    {
      SILABS_LOG("MQTT Connection failed %d", mret);
      init_complete = false;
      goto DIC_error;
    }
    init_complete = true;
    return;
  }
  init_complete = false;
DIC_error:
  vTaskDelete(dicTask);
  vEventGroupDelete(dicEvents);
  dicTask = NULL;
  return;
}

static void dic_task_fn(void *args)
{
  /* get MQTT client handle */
  err_t ret;
  gSubsCB = reinterpret_cast<void (*)()>(args);
  mqtt_client = mqtt_client_new();
  if (!mqtt_client)
  {
    SILABS_LOG("Failed to create mqtt client");
    goto DIC_error;
  }

  memset(&trans, 0x00, sizeof(mqtt_transport_intf_t));

  /* Get transport handle*/
  transport = MQTT_Transport_Init(&trans, mqtt_client, dicEvents);
  if (!transport)
  {
    SILABS_LOG("Failed to create mqtt transport");
    goto DIC_error;
  }

   /* set SSL configuration for TLS transport connection*/
  if (0 != MQTT_Transport_SSLConfigure(transport, (uint8_t *)DIC_SERVER_CA_CERT, strlen(DIC_SERVER_CA_CERT)+1,
                                      (uint8_t *)DIC_DEVICE_KEY, strlen(DIC_DEVICE_KEY)+1, NULL, 0,                             \
                                      (uint8_t *)DIC_DEVICE_CERT, strlen(DIC_DEVICE_CERT)+1))
  {
    SILABS_LOG("Failed to configure SSL to mqtt transport");
    goto DIC_error;
  }

  if ((ret = MQTT_Transport_Connect(transport, DIC_SERVER_HOST, DIC_SERVER_PORT, dic_tcp_connect_cb)) != ERR_OK)
  {
    SILABS_LOG("Transport Connection failed %d", ret);
    goto DIC_error;
  }

  while (!end_loop)
  {
    EventBits_t event;
    event = xEventGroupWaitBits(dicEvents, SIGNAL_TRANSINTF_RX | SIGNAL_TRANSINTF_TX_ACK | SIGNAL_TRANSINTF_CONN_CLOSE | SIGNAL_TRANSINTF_MBEDTLS_RX, 1, 0, portMAX_DELAY);
    if (event & SIGNAL_TRANSINTF_CONN_CLOSE)
    {
      mqtt_close(mqtt_client, MQTT_CONNECT_DISCONNECTED);
      end_loop = true;
    }
    else
    {
      if (event & SIGNAL_TRANSINTF_RX)
        mqtt_process(mqtt_client, SIGNAL_TRANSINTF_TX);
      else if (event & SIGNAL_TRANSINTF_TX_ACK)
        mqtt_process(mqtt_client, SIGNAL_TRANSINTF_TX_ACK);
      if (event & SIGNAL_TRANSINTF_MBEDTLS_RX)
        transport_process_mbedtls_rx(transport);
    }
  }
  init_complete = false;

DIC_error:
  vTaskDelete(dicTask);
  dicTask = NULL;
  vEventGroupDelete(dicEvents);
  return;
}

void dic_pub_resp_cb(void *arg, mqtt_err_t err)
{
  (void)arg;
  SILABS_LOG("dic publish data %s", err != MQTT_ERR_OK ? "failed!" : "successful!");
}

dic_err_t DIC_Init(dic_subscribe_cb subs_cb)
{
  if (dicTask)
  {
    SILABS_LOG("DIC already initialized, dic task is running");
    return DIC_OK;
  }

  /* Create events group used to receive events from transport layer*/
  dicEvents = xEventGroupCreate();
  if (!dicEvents)
  {
    SILABS_LOG("Failed to create DIC event groups");
    return DIC_ERR_FAIL;
  }

  if ((pdPASS != xTaskCreate(dic_task_fn, DIC_TASK_NAME, DIC_TASK_STACK_SIZE, (void*)subs_cb, DIC_TASK_PRIORITY, &dicTask)) ||
      !dicTask)
  {
    SILABS_LOG("Failed to create DIC Task");
    vEventGroupDelete(dicEvents);
    return DIC_ERR_MEM;
  }

  /* TODO: Register data and pub callback and subscribe if cloud to app messages are expected*/
  return DIC_OK;
}

dic_err_t DIC_SendMsg(const char *subject, const char *content)
{
  if (subject == nullptr || content == nullptr)
  {
    SILABS_LOG("null args passed to DIC_SendMsg()");
    return DIC_ERR_INVAL;
  }
  if (!init_complete)
  {
    SILABS_LOG("Err: DIC not in valid state!");
    return DIC_ERR_CONN;
  }
  dic_buff_t buffValue;
  buffValue.dataP = (uint8_t *)content;
  buffValue.dataLen = strlen(content);
  if (MQTT_ERR_OK != mqtt_publish(mqtt_client, subject, buffValue.dataP, buffValue.dataLen, MQTT_QOS_0, 0, dic_pub_resp_cb, NULL))
  {
    SILABS_LOG("Err: failed request publish!");
    return DIC_ERR_PUBLISH;
  }
  return DIC_OK;
}
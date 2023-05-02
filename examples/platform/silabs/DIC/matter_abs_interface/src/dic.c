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
#include "MQTT_transport.h"
#include "mqtt.h"
#ifdef SIWX_917
#include "siwx917_utils.h"
#else
#include "efr32_utils.h"
#endif

#include <stdbool.h> //for bool
#include <string.h>  // for memset

#define DIC_TOPIC_PREFIX "silabs/matter/"

static TaskHandle_t dicTask;
static mqtt_client_t *mqtt_client;
static EventGroupHandle_t dicEvents = NULL;
static MQTT_Transport_t *transport = NULL;
static mqtt_transport_intf_t trans;
static bool end_loop;
static bool init_complete;

static void dic_mqtt_conn_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  (void)client;
  (void)arg;

  SILABS_LOG("DIC: mqtt connection status: %u", status);
  if (status != MQTT_CONNECT_ACCEPTED)
  {
    if (dicTask) end_loop = true;
  }
}

void dic_tcp_connect_cb(err_t err){
  SILABS_LOG("connection callback  new");
  if(err == ERR_OK){
      struct mqtt_connect_client_info_t connect_info;
      mqtt_err_t mret;
        /* Connect to MQTT broker/cloud */
      memset(&connect_info,0,sizeof(connect_info));
      connect_info.client_id = DIC_CLIENT_ID;
      connect_info.keep_alive = DIC_KEEP_ALIVE;
      connect_info.client_user = DIC_CLIENT_USER;
      connect_info.client_pass = DIC_CLIENT_PASS;

      if ((mret = mqtt_client_connect(mqtt_client,(void*)&trans,dic_mqtt_conn_cb,NULL,&connect_info)) != ERR_OK){
        SILABS_LOG("MQTT Connection failed %d",mret);
        init_complete = false;
        goto DIC_error;
      }
      init_complete = true;
      return;
  }
  init_complete = false;
DIC_error:
  vTaskDelete(dicTask);
  dicTask = NULL;
  return;
}

static void dic_task_fn(void *args)
{
  /* get MQTT client handle */
  err_t ret;

  vTaskDelay(10000);

  mqtt_client = mqtt_client_new();
  if (!mqtt_client)
  {
    SILABS_LOG("Failed to create mqtt client");
    goto DIC_error;
  }

  memset(&trans, 0x00, sizeof(mqtt_transport_intf_t));

  /* Get transport handle*/
  transport = MQTT_Transport_Init(&trans,mqtt_client,dicEvents);
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

  /* establish transport connection with remote */

  if ((ret = MQTT_Transport_Connect(transport, DIC_SERVER_HOST, DIC_SERVER_PORT, dic_tcp_connect_cb)) != ERR_OK){
    SILABS_LOG("Transport Connection failed %d",ret);
    goto DIC_error;
  }

  (void)args;
  while (!end_loop)
    {
        EventBits_t event;
        event = xEventGroupWaitBits(dicEvents, SIGNAL_TRANSINTF_RX | SIGNAL_TRANSINTF_TX_ACK | SIGNAL_TRANSINTF_CONN_CLOSE | SIGNAL_TRANSINTF_MBEDTLS_RX, 1, 0, portMAX_DELAY);
        if (event & SIGNAL_TRANSINTF_CONN_CLOSE)
        {
            //mqtt_close(client, MQTT_CONNECT_DISCONNECTED);
            mqtt_close(mqtt_client, MQTT_CONNECT_DISCONNECTED);
            end_loop = true;
        }
        else
        {
            if (event & SIGNAL_TRANSINTF_RX)
                mqtt_process(mqtt_client, 0);
            else if (event & 2)
               mqtt_process(mqtt_client, SIGNAL_TRANSINTF_TX_ACK);
            if (event & SIGNAL_TRANSINTF_MBEDTLS_RX)
                transport_process_mbedtls_rx(transport);
        }
    }
  init_complete = false;
  dicTask = NULL;
  vTaskDelete(NULL);

DIC_error:
  vTaskDelete(dicTask);
  dicTask = NULL;
  return;
}

void dic_pub_resp_cb(void *arg, mqtt_err_t err)
{
  (void)arg;
  SILABS_LOG("dic publish data %s", err != MQTT_ERR_OK ? "failed!" : "successful!");
}

dic_err_t DIC_Init()
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
    goto DIC_error;
  }
  if ((pdPASS != xTaskCreate(dic_task_fn, DIC_TASK_NAME, DIC_TASK_STACK_SIZE, NULL, 1, &dicTask))
            || !dicTask)
  {
    SILABS_LOG("Failed to create DIC Task");
    return DIC_ERR_MEM;
  }
  /* TODO: Register data and pub callback and subscribe if cloud to app messages are expected*/
  return DIC_OK;
DIC_error:
vTaskDelete(dicTask);
  dicTask = NULL;
  return DIC_ERR_FAIL;
}

dic_err_t DIC_SendMsg(const char *subject, const char *content)
{
  dic_buff_t buffValue;
  buffValue.dataP =(uint8_t *) content;
  buffValue.dataLen = strlen(content);
  if (!subject || !content )
  {
    SILABS_LOG("null args passed to DIC_SendMsg()");
    return DIC_ERR_INVAL;
  }
  if(!init_complete)
  {
    SILABS_LOG("Err: DIC not in valid state!");
    return DIC_ERR_FAIL;
  }
  if (MQTT_ERR_OK != mqtt_publish(mqtt_client, subject, buffValue.dataP, buffValue.dataLen, 0, 0, dic_pub_resp_cb, NULL))
  {
    SILABS_LOG("Err: failed request publish!");
    return DIC_ERR_FAIL;
  }
  return DIC_OK;
}

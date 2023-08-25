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

#include "silabs_utils.h"
#include "dic.h"
#include "dic_config.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "lib/core/CHIPError.h"
#include "dic_nvm_cert.h"

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
mqtt_client_t *mqtt_client;
static EventGroupHandle_t dicEvents = NULL;
MQTT_Transport_t *transport = NULL;
static mqtt_transport_intf_t trans;
static bool end_loop;
static bool init_complete;
dic_subscribe_cb gSubsCB = NULL;
static void dic_mqtt_subscribe_cb(void *arg, mqtt_err_t err)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
  (void)client_info;
  SILABS_LOG("MQTT sub request callback %d", (int)err);
}

dic_err_t dic_mqtt_subscribe(mqtt_client_t *client, mqtt_incoming_publish_cb_t publish_cb, mqtt_incoming_data_cb_t data_cb, const char * topic, uint8_t qos){
    mqtt_set_inpub_callback(mqtt_client,
      publish_cb,
      data_cb,
      NULL);
    int mret;
    if((mret = mqtt_subscribe(mqtt_client, topic,qos, dic_mqtt_subscribe_cb, NULL)) != ERR_OK){
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

    char clientID[DIC_CLIENTID_LENGTH] = {0};
    size_t length = 0;

    if (DICGetClientId(clientID, DIC_CLIENTID_LENGTH, &length) != CHIP_NO_ERROR)
    {
      goto DIC_error;
    }

    connect_info.client_id = clientID;
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
  char ca_cert_buf[DIC_CA_CERT_LENGTH] = {0};
  char cert_buf[DIC_DEV_CERT_LENGTH] = {0};
  char key_buf[DIC_DEV_KEY_LENGTH] = {0};
  char hostname[DIC_HOSTNAME_LENGTH] = {0};
  size_t ca_cert_length = 0;
  size_t cert_length = 0;
  size_t Key_Length = 0;
  size_t len = 0;
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

  if (DICGetCACertificate(ca_cert_buf, DIC_CA_CERT_LENGTH, &ca_cert_length) != CHIP_NO_ERROR)
  {
    goto DIC_error;
  }

  if (DICGetDeviceCertificate(cert_buf, DIC_DEV_CERT_LENGTH, &cert_length) != CHIP_NO_ERROR)
  {
    goto DIC_error;
  }

  if (DICGetDevicePrivKey(key_buf, DIC_DEV_KEY_LENGTH, &Key_Length) != CHIP_NO_ERROR)
  {
    goto DIC_error;
  }

  if (DICGetHostname(hostname, DIC_HOSTNAME_LENGTH, & len) != CHIP_NO_ERROR)
  {
    goto DIC_error;
  }

  /* set SSL configuration for TLS transport connection*/
  if (ERR_OK != MQTT_Transport_SSLConfigure(transport, (const u8_t *)ca_cert_buf, ca_cert_length,
                                       (const u8_t *)key_buf, Key_Length , NULL, 0,                             \
                                       (const u8_t *)cert_buf, cert_length))
  {
    SILABS_LOG("Failed to configure SSL to mqtt transport");
    goto DIC_error;
  }

  if ((ret = MQTT_Transport_Connect(transport, hostname, DIC_SERVER_PORT, dic_tcp_connect_cb)) != ERR_OK)
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

dic_err_t dic_init(dic_subscribe_cb subs_cb)
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

dic_err_t dic_sendmsg(const char *subject, const char *content)
{
  if (subject == nullptr || content == nullptr)
  {
    SILABS_LOG("null args passed to dic_sendmsg()");
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

#ifdef ENABLE_AWS_OTA_FEAT

struct sub_cb_info sub_info;
int dic_init_status(){
  if(init_complete){
    return 1;
  }
  else return 0;
}

static void dic_aws_ota_mqtt_incoming_data_cb(void *arg, const char *topic, u16_t topic_len, const u8_t *data, u16_t len, u8_t flags)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
  (void)client_info;
  u8_t buff[1500] = {0};
  memcpy(buff,data,len);
  SILABS_LOG("incoming data is data cb: payloadlen %d, flags %d\n", (int)len, (int)flags);
  SILABS_LOG("mqtt_incoming_data_cb topic len %d tpoic_len=%d", strlen(topic),topic_len);
  sub_info.cb(topic, topic_len, (char *)buff, len);
}
static void dic_aws_ota_mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  const struct mqtt_connect_client_info_t* client_info = (const struct mqtt_connect_client_info_t*)arg;
  (void)client_info;

  SILABS_LOG("MQTT client publish cb: topic %s, len %d\n",
          topic, (int)tot_len);
}

dic_err_t dic_aws_ota_publish(const char * const topic, const char * message, uint32_t message_len, uint8_t qos)
{
  if(!init_complete)
  {
    SILABS_LOG("Err: DIC not in valid state!");
    return DIC_ERR_FAIL;
  }
  if (MQTT_ERR_OK != mqtt_publish(mqtt_client, topic, message, message_len, qos, 0, dic_pub_resp_cb, NULL))
  {
    SILABS_LOG("Err: failed request publish!");
    return DIC_ERR_FAIL;
  }else{
    SILABS_LOG("Published the data %s to topic %s", message,topic);
  }
  return DIC_OK;
}

dic_err_t dic_aws_ota_unsubscribe(const char * topic)
{
  if(!init_complete)
  {
    SILABS_LOG("Err: DIC not in valid state!");
    return DIC_ERR_FAIL;
  }

  if (MQTT_ERR_OK != mqtt_unsubscribe(mqtt_client, topic, dic_mqtt_subscribe_cb, NULL))
  {
    SILABS_LOG("Err: failed request unsubscribe!");
    return DIC_ERR_FAIL;
  }
return DIC_OK;
}

dic_err_t dic_aws_ota_subscribe(const char * topic, uint8_t qos, callback_t subscribe_cb)
{
  if(!init_complete)
  {
    SILABS_LOG("Err: DIC not in valid state!");
    return DIC_ERR_FAIL;
  }

  sub_info.sub_topic = (char*)topic;
  sub_info.cb = subscribe_cb;
  if (DIC_OK != dic_mqtt_subscribe(mqtt_client, dic_aws_ota_mqtt_incoming_publish_cb, dic_aws_ota_mqtt_incoming_data_cb, topic, qos))
  {
    SILABS_LOG("Err: failed request subscribe!");
    return DIC_ERR_FAIL;
  }
  return DIC_OK;
}

dic_err_t dic_aws_ota_process(){
  if(!init_complete)
  {
    SILABS_LOG("Err: DIC not in valid state!");
    return DIC_ERR_FAIL;
  }
  mqtt_process(mqtt_client, 0);
  
return DIC_OK;
}

dic_err_t dic_aws_ota_close(){
  if(!init_complete)
  {
    SILABS_LOG("Err: DIC not in valid state!");
    return DIC_ERR_FAIL;
  }

  mqtt_close(mqtt_client, MQTT_CONNECT_DISCONNECTED);
  
return DIC_OK;
}
#endif
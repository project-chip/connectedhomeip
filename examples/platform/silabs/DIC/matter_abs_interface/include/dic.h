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

#ifndef __DIC_H
#define __DIC_H
#ifdef __cplusplus
extern "C" {
#endif
#include "mqtt.h"
#include "stdint.h"
typedef enum {
	DIC_OK = 0,
	DIC_ERR_INVAL,
	DIC_ERR_MEM,
	DIC_ERR_FAIL,
	DIC_ERR_CONN,
	DIC_ERR_PUBLISH,
} dic_err_t;

#define MQTT_QOS_0 0

typedef struct {
	uint8_t *dataP;
	uint16_t dataLen;
} dic_buff_t;

typedef void (* dic_subscribe_cb)(void);

dic_err_t dic_init(dic_subscribe_cb subs_cb);

dic_err_t dic_mqtt_subscribe(mqtt_client_t *client, mqtt_incoming_publish_cb_t publish_cb, mqtt_incoming_data_cb_t data_cb, const char * topic, uint8_t qos);

dic_err_t dic_sendmsg(const char *subject, const char *content);


#ifdef ENABLE_AWS_OTA_FEAT

#define AWS_OTA_TASK_STACK_SIZE 1024
#define AWS_OTA_TASK_PRIORITY 1

typedef void (*callback_t)(const char * sub_topic, uint16_t top_len, const void *pload, uint16_t pLoadLength);
struct sub_cb_info{
  char *sub_topic;
  callback_t cb;
};

int dic_init_status(void);

dic_err_t dic_aws_ota_publish(const char * const topic, const char * message, uint32_t message_len, uint8_t qos);

dic_err_t dic_aws_ota_unsubscribe(const char * topic);

dic_err_t dic_aws_ota_subscribe(const char * topic, uint8_t qos, callback_t subscribe_cb);

dic_err_t dic_aws_ota_process();

dic_err_t dic_aws_ota_close();
#endif

#ifdef __cplusplus
}
#endif
#endif //__DIC_H
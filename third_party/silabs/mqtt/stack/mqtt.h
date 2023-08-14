/**
 * @file
 * MQTT client
 */

/*
 * Copyright (c) 2016 Erik Andersson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Erik Andersson
 *
 */
#ifndef MQTT_H
#define MQTT_H

#include "mqtt_opts.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef uint8_t mqtt_err_t;
typedef uint32_t mqtt_mem_size_t;

typedef struct mqtt_client_t mqtt_client_t;
typedef struct mqtt_transport_intf_t mqtt_transport_intf_t;

#define MQTT_MIN(x , y)  (((x) < (y)) ? (x) : (y))
#define MQTT_ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

#ifdef ERR_FAIL
#undef ERR_FAIL
#endif
#define ERR_FAIL -1

#define MQTT_PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); fflush(NULL); abort();} while(0)

#define MQTT_PLATFORM_ERROR(message) MQTT_PLATFORM_ASSERT(message)

/* if "expression" isn't true, then print "message" and execute "handler" expression */
#define MQTT_ERROR(message, expression, handler) do { if (!(expression)) { \
  MQTT_PLATFORM_ERROR(message); handler;}} while(0)

#define MQTT_ASSERT(message, assertion) do { if (!(assertion)) { \
  MQTT_PLATFORM_ASSERT(message); }} while(0)

/* Flags for "apiflags" parameter in tcp_write */
#define MQTT_WRITE_FLAG_COPY 0x01
#define MQTT_WRITE_FLAG_MORE 0x02

/*--------------------------------------------------------------------------------------------------------------------- */
/* Output ring buffer */

#define MQTT_RINGBUF_IDX_MASK ((MQTT_OUTPUT_RINGBUF_SIZE) - 1)

/** Add single item to ring buffer */
#define mqtt_ringbuf_put(rb, item) ((rb)->buf)[(rb)->put++ & MQTT_RINGBUF_IDX_MASK] = (item)

/** Return number of bytes in ring buffer */
#define mqtt_ringbuf_len(rb) ((uint16_t)((rb)->put - (rb)->get))

/** Return number of bytes free in ring buffer */
#define mqtt_ringbuf_free(rb) (MQTT_OUTPUT_RINGBUF_SIZE - mqtt_ringbuf_len(rb))

/** Return number of bytes possible to read without wrapping around */
#define mqtt_ringbuf_linear_read_length(rb) MQTT_MIN(mqtt_ringbuf_len(rb), (MQTT_OUTPUT_RINGBUF_SIZE - ((rb)->get & MQTT_RINGBUF_IDX_MASK)))

/** Return pointer to ring buffer get position */
#define mqtt_ringbuf_get_ptr(rb) (&(rb)->buf[(rb)->get & MQTT_RINGBUF_IDX_MASK])

#define mqtt_ringbuf_advance_get_idx(rb, len) ((rb)->get += (len))


/**
 * MQTT client connection states
 */
enum {
  TCP_DISCONNECTED,
  TCP_CONNECTING,
  MQTT_CONNECTING,
  MQTT_CONNECTED
};

enum {
  /** No error, everything OK. */
  MQTT_ERR_OK = 0,
  /** Out of memory error.     */
  MQTT_ERR_MEM = -1,
  /** Timeout.                 */
  MQTT_ERR_TIMEOUT = -3,
  /** Illegal value.           */
  MQTT_ERR_VAL = -6,
  /** Conn already established.*/
  MQTT_ERR_ISCONN = -10,
  /** Not connected.           */
  MQTT_ERR_CONN = -11,
  /** Connection aborted.      */
  MQTT_ERR_ABRT = -13,
  /** Illegal argument.        */
  MQTT_ERR_ARG = -16,

};
/** @ingroup mqtt
 * Default MQTT port */
#define MQTT_PORT 1883

/*---------------------------------------------------------------------------------------------- */
/* Connection with server */

/**
 * @ingroup mqtt
 * Client information and connection parameters */
struct mqtt_connect_client_info_t {
  /** Client identifier, must be set by caller */
  const char *client_id;
  /** User name and password, set to NULL if not used */
  const char* client_user;
  const char* client_pass;
  /** keep alive time in seconds, 0 to disable keep alive functionality*/
  uint16_t keep_alive;
  /** will topic, set to NULL if will is not to be used,
      will_msg, will_qos and will retain are then ignored */
  const char* will_topic;
  const char* will_msg;
  uint8_t will_qos;
  uint8_t will_retain;
};

/**
 * @ingroup mqtt
 * Connection status codes */
typedef enum
{
  MQTT_CONNECT_ACCEPTED                 = 0,
  MQTT_CONNECT_REFUSED_PROTOCOL_VERSION = 1,
  MQTT_CONNECT_REFUSED_IDENTIFIER       = 2,
  MQTT_CONNECT_REFUSED_SERVER           = 3,
  MQTT_CONNECT_REFUSED_USERNAME_PASS    = 4,
  MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_  = 5,
  MQTT_CONNECT_DISCONNECTED             = 256,
  MQTT_CONNECT_TIMEOUT                  = 257
} mqtt_connection_status_t;

/**
 * @ingroup mqtt
 * Function prototype for mqtt connection status callback. Called when
 * client has connected to the server after initiating a mqtt connection attempt by
 * calling mqtt_connect() or when connection is closed by server or an error
 *
 * @param client MQTT client itself
 * @param arg Additional argument to pass to the callback function
 * @param status Connect result code or disconnection notification @see mqtt_connection_status_t
 *
 */
typedef void (*mqtt_connection_cb_t)(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);


/**
 * @ingroup mqtt
 * Data callback flags */
enum {
  /** Flag set when last fragment of data arrives in data callback */
  MQTT_DATA_FLAG_LAST = 1
};

/**
 * @ingroup mqtt
 * Function prototype for MQTT incoming publish data callback function. Called when data
 * arrives to a subscribed topic @see mqtt_subscribe
 *
 * @param arg Additional argument to pass to the callback function
 * @param data User data, pointed object, data may not be referenced after callback return,
          NULL is passed when all publish data are delivered
 * @param len Length of publish data fragment
 * @param flags MQTT_DATA_FLAG_LAST set when this call contains the last part of data from publish message
 *
 */
typedef void (*mqtt_incoming_data_cb_t)(void *arg, const char *topic, const uint8_t *data, uint16_t len, uint8_t flags);


/**
 * @ingroup mqtt
 * Function prototype for MQTT incoming publish function. Called when an incoming publish
 * arrives to a subscribed topic @see mqtt_subscribe
 *
 * @param arg Additional argument to pass to the callback function
 * @param topic Zero terminated Topic text string, topic may not be referenced after callback return
 * @param tot_len Total length of publish data, if set to 0 (no publish payload) data callback will not be invoked
 */
typedef void (*mqtt_incoming_publish_cb_t)(void *arg, const char *topic, uint32_t tot_len);


/**
 * @ingroup mqtt
 * Function prototype for mqtt request callback. Called when a subscribe, unsubscribe
 * or publish request has completed
 * @param arg Pointer to user data supplied when invoking request
 * @param err ERR_OK on success
 *            ERR_TIMEOUT if no response was received within timeout,
 *            ERR_ABRT if (un)subscribe was denied
 */
typedef void (*mqtt_request_cb_t)(void *arg, mqtt_err_t err);


/**
 * Pending request item, binds application callback to pending server requests
 */
struct mqtt_request_t
{
  /** Next item in list, NULL means this is the last in chain,
      next pointing at itself means request is unallocated */
  struct mqtt_request_t *next;
  /** Callback to upper layer */
  mqtt_request_cb_t cb;
  void *arg;
  /** MQTT packet identifier */
  uint16_t pkt_id;
  /** Expire time relative to element before this  */
  uint16_t timeout_diff;
};

/** Ring buffer */
struct mqtt_ringbuf_t {
  uint16_t put;
  uint16_t get;
  uint8_t buf[MQTT_OUTPUT_RINGBUF_SIZE];
};

typedef uint16_t (*mqtt_ti_getsndLen)(void *conn);
typedef int8_t (*mqtt_send_to_network)(void *conn, void *buff, uint16_t len, uint8_t flags, uint8_t isFinal);
typedef uint16_t (*mqtt_ti_getrcvLen)(void *conn);
typedef int8_t (*mqtt_recv_from_network)(void *conn, void * buf, uint16_t len, uint16_t offset);
typedef void (*mqtt_close_connection)(void *conn);

typedef void (*timer_callback)(void *arg);
typedef void (*mqtt_timer_start)(uint32_t msecs, timer_callback handler, void *arg);
typedef void (*mqtt_timer_stop)(timer_callback handler, void *arg);

struct mqtt_transport_intf_t {
  void *connCtxt;
  mqtt_send_to_network send_to_network;
  mqtt_ti_getsndLen get_send_len;
  mqtt_recv_from_network recv_from_network;
  mqtt_ti_getrcvLen get_recv_len;
  mqtt_close_connection close_connection;
  mqtt_timer_start timer_start;
  mqtt_timer_stop timer_stop;
  void *app_recv;
};

/** MQTT client */
struct mqtt_client_t
{
  /** Timers and timeouts */
  uint16_t cyclic_tick;
  uint16_t keep_alive;
  uint16_t server_watchdog;
  /** Packet identifier generator*/
  uint16_t pkt_id_seq;
  /** Packet identifier of pending incoming publish */
  uint16_t inpub_pkt_id;
  /** Connection state */
  uint8_t conn_state;
  mqtt_transport_intf_t *conn;
  /** Connection callback */
  void *connect_arg;
  mqtt_connection_cb_t connect_cb;
  /** Pending requests to server */
  struct mqtt_request_t *pend_req_queue;
  struct mqtt_request_t req_list[MQTT_REQ_MAX_IN_FLIGHT];
  void *inpub_arg;
  /** Incoming data callback */
  mqtt_incoming_data_cb_t data_cb;
  mqtt_incoming_publish_cb_t pub_cb;
  /** Input */
  uint32_t msg_idx;
  uint8_t rx_buffer[MQTT_VAR_HEADER_BUFFER_LEN];
  /** Output ring-buffer */
  struct mqtt_ringbuf_t output;
};


/** Connect to server */
mqtt_err_t mqtt_client_connect(mqtt_client_t *client, void *conn, mqtt_connection_cb_t cb, void *arg,
                   const struct mqtt_connect_client_info_t *client_info);

/** Disconnect from server */
void mqtt_disconnect(mqtt_client_t *client);

/** Create new client */
mqtt_client_t *mqtt_client_new(void);

/** Check connection status */
uint8_t mqtt_client_is_connected(mqtt_client_t *client);

/** Set callback to call for incoming publish */
void mqtt_set_inpub_callback(mqtt_client_t *client, mqtt_incoming_publish_cb_t,
                             mqtt_incoming_data_cb_t data_cb, void *arg);

/** Common function for subscribe and unsubscribe */
mqtt_err_t mqtt_sub_unsub(mqtt_client_t *client, const char *topic, uint8_t qos, mqtt_request_cb_t cb, void *arg, uint8_t sub);

/** @ingroup mqtt
 *Subscribe to topic */
#define mqtt_subscribe(client, topic, qos, cb, arg) mqtt_sub_unsub(client, topic, qos, cb, arg, 1)
/** @ingroup mqtt
 *  Unsubscribe to topic */
#define mqtt_unsubscribe(client, topic, cb, arg) mqtt_sub_unsub(client, topic, 0, cb, arg, 0)


/** Publish data to topic */
mqtt_err_t mqtt_publish(mqtt_client_t *client, const char *topic, const void *payload, uint16_t payload_length, uint8_t qos, uint8_t retain,
                                    mqtt_request_cb_t cb, void *arg);

void *mqtt_mem_calloc(mqtt_mem_size_t count, mqtt_mem_size_t size);
void mqtt_close(mqtt_client_t *client, mqtt_connection_status_t reason);

void mqtt_process(mqtt_client_t *client, uint8_t isTx);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_H */
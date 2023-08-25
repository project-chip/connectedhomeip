/**
 * @file
 * MQTT client
 *
 * @defgroup mqtt MQTT client
 * @ingroup apps
 * @verbinclude mqtt_client.txt
 */

/*
 * Copyright (c) 2016 Erik Andersson <erian747@gmail.com>
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
 * This file is part of the lwIP TCP/IP stack
 *
 * Author: Erik Andersson <erian747@gmail.com>
 *
 *
 * @todo:
 * - Handle large outgoing payloads for PUBLISH messages
 * - Fix restriction of a single topic in each (UN)SUBSCRIBE message (protocol has support for multiple topics)
 * - Add support for legacy MQTT protocol version
 *
 * Please coordinate changes and requests with Erik Andersson
 * Erik Andersson <erian747@gmail.com>
 *
 */
#include "mqtt.h"
#include <string.h>

/**
 * MQTT_DEBUG: Default is off.
 */
#if !defined MQTT_DEBUG || defined __DOXYGEN__
#define MQTT_DEBUG                  0x00
#endif
#if !defined MQTT_ASSERT_CORE_LOCKED || defined __DOXYGEN__
#define MQTT_ASSERT_CORE_LOCKED()
#endif
extern void *mem_calloc(mqtt_mem_size_t count, mqtt_mem_size_t size);
extern void  mem_free(void *mem);

mqtt_connection_status_t
mqtt_parse_incoming(mqtt_client_t *client);
void
mqtt_output_send(struct mqtt_ringbuf_t *rb, mqtt_transport_intf_t *trans);

#ifdef MQTT_DEBUG
#include "silabs_utils.h"
void silabsLog(const char * aFormat, ...);
#define MQTT_DEBUGF(x) silabsLog x;
#else
#define MQTT_DEBUGF(x)
#endif

/**
 * MQTT control message types
 */
enum mqtt_message_type {
  MQTT_MSG_TYPE_CONNECT = 1,
  MQTT_MSG_TYPE_CONNACK = 2,
  MQTT_MSG_TYPE_PUBLISH = 3,
  MQTT_MSG_TYPE_PUBACK = 4,
  MQTT_MSG_TYPE_PUBREC = 5,
  MQTT_MSG_TYPE_PUBREL = 6,
  MQTT_MSG_TYPE_PUBCOMP = 7,
  MQTT_MSG_TYPE_SUBSCRIBE = 8,
  MQTT_MSG_TYPE_SUBACK = 9,
  MQTT_MSG_TYPE_UNSUBSCRIBE = 10,
  MQTT_MSG_TYPE_UNSUBACK = 11,
  MQTT_MSG_TYPE_PINGREQ = 12,
  MQTT_MSG_TYPE_PINGRESP = 13,
  MQTT_MSG_TYPE_DISCONNECT = 14
};

/** Helpers to extract control packet type and qos from first byte in fixed header */
#define MQTT_CTL_PACKET_TYPE(fixed_hdr_byte0) ((fixed_hdr_byte0 & 0xf0) >> 4)
#define MQTT_CTL_PACKET_QOS(fixed_hdr_byte0) ((fixed_hdr_byte0 & 0x6) >> 1)

/**
 * MQTT connect flags, only used in CONNECT message
 */
enum mqtt_connect_flag {
  MQTT_CONNECT_FLAG_USERNAME = 1 << 7,
  MQTT_CONNECT_FLAG_PASSWORD = 1 << 6,
  MQTT_CONNECT_FLAG_WILL_RETAIN = 1 << 5,
  MQTT_CONNECT_FLAG_WILL = 1 << 2,
  MQTT_CONNECT_FLAG_CLEAN_SESSION = 1 << 1
};


static void mqtt_cyclic_timer(void *arg);


static const char *const mqtt_message_type_str[15] = {
  "UNDEFINED",
  "CONNECT",
  "CONNACK",
  "PUBLISH",
  "PUBACK",
  "PUBREC",
  "PUBREL",
  "PUBCOMP",
  "SUBSCRIBE",
  "SUBACK",
  "UNSUBSCRIBE",
  "UNSUBACK",
  "PINGREQ",
  "PINGRESP",
  "DISCONNECT"
};

/**
 * Message type value to string
 * @param msg_type see enum mqtt_message_type
 *
 * @return Control message type text string
 */
static const char *
mqtt_msg_type_to_str(uint8_t msg_type)
{
  if (msg_type >= MQTT_ARRAYSIZE(mqtt_message_type_str)) {
    msg_type = 0;
  }
  return mqtt_message_type_str[msg_type];
}


/**
 * Generate MQTT packet identifier
 * @param client MQTT client
 * @return New packet identifier, range 1 to 65535
 */
static uint16_t
msg_generate_packet_id(mqtt_client_t *client)
{
  client->pkt_id_seq++;
  if (client->pkt_id_seq == 0) {
    client->pkt_id_seq++;
  }
  return client->pkt_id_seq;
}

/**
 * Try send as many bytes as possible from output ring buffer
 * @param rb Output ring buffer
 * @param tpcb TCP connection handle
 */


/*--------------------------------------------------------------------------------------------------------------------- */
/* Request queue */

/**
 * Create request item
 * @param r_objs Pointer to request objects
 * @param r_objs_len Number of array entries
 * @param pkt_id Packet identifier of request
 * @param cb Packet callback to call when requests lifetime ends
 * @param arg Parameter following callback
 * @return Request or NULL if failed to create
 */
static struct mqtt_request_t *
mqtt_create_request(struct mqtt_request_t *r_objs, size_t r_objs_len, uint16_t pkt_id, mqtt_request_cb_t cb, void *arg)
{
  struct mqtt_request_t *r = NULL;
  uint8_t n;
  MQTT_ASSERT("mqtt_create_request: r_objs != NULL", r_objs != NULL);
  for (n = 0; n < r_objs_len; n++) {
    /* Item point to itself if not in use */
    if (r_objs[n].next == &r_objs[n]) {
      r = &r_objs[n];
      r->next = NULL;
      r->cb = cb;
      r->arg = arg;
      r->pkt_id = pkt_id;
      break;
    }
  }
  return r;
}


/**
 * Append request to pending request queue
 * @param tail Pointer to request queue tail pointer
 * @param r Request to append
 */
static void
mqtt_append_request(struct mqtt_request_t **tail, struct mqtt_request_t *r)
{
  struct mqtt_request_t *head = NULL;
  int16_t time_before = 0;
  struct mqtt_request_t *iter;

  MQTT_ASSERT("mqtt_append_request: tail != NULL", tail != NULL);

  /* Iterate trough queue to find head, and count total timeout time */
  for (iter = *tail; iter != NULL; iter = iter->next) {
    time_before += iter->timeout_diff;
    head = iter;
  }

  MQTT_ASSERT("mqtt_append_request: time_before <= MQTT_REQ_TIMEOUT", time_before <= MQTT_REQ_TIMEOUT);
  r->timeout_diff = MQTT_REQ_TIMEOUT - time_before;
  if (head == NULL) {
    *tail = r;
  } else {
    head->next = r;
  }
}


/**
 * Delete request item
 * @param r Request item to delete
 */
void
mqtt_delete_request(struct mqtt_request_t *r)
{
  if (r != NULL) {
    r->next = r;
  }
}

/**
 * Remove a request item with a specific packet identifier from request queue
 * @param tail Pointer to request queue tail pointer
 * @param pkt_id Packet identifier of request to take
 * @return Request item if found, NULL if not
 */
struct mqtt_request_t *
mqtt_take_request(struct mqtt_request_t **tail, uint16_t pkt_id)
{
  struct mqtt_request_t *iter = NULL, *prev = NULL;
  MQTT_ASSERT("mqtt_take_request: tail != NULL", tail != NULL);
  /* Search all request for pkt_id */
  for (iter = *tail; iter != NULL; iter = iter->next) {
    if (iter->pkt_id == pkt_id) {
      break;
    }
    prev = iter;
  }

  /* If request was found */
  if (iter != NULL) {
    /* unchain */
    if (prev == NULL) {
      *tail = iter->next;
    } else {
      prev->next = iter->next;
    }
    /* If exists, add remaining timeout time for the request to next */
    if (iter->next != NULL) {
      iter->next->timeout_diff += iter->timeout_diff;
    }
    iter->next = NULL;
  }
  return iter;
}

/**
 * Handle requests timeout
 * @param tail Pointer to request queue tail pointer
 * @param t Time since last call in seconds
 */
static void
mqtt_request_time_elapsed(struct mqtt_request_t **tail, uint8_t t)
{
  struct mqtt_request_t *r;
  MQTT_ASSERT("mqtt_request_time_elapsed: tail != NULL", tail != NULL);
  r = *tail;
  while (t > 0 && r != NULL) {
    if (t >= r->timeout_diff) {
      t -= (uint8_t)r->timeout_diff;
      /* Unchain */
      *tail = r->next;
      /* Notify upper layer about timeout */
      if (r->cb != NULL) {
        r->cb(r->arg, MQTT_ERR_TIMEOUT);
      }
      mqtt_delete_request(r);
      /* Tail might be be modified in callback, so re-read it in every iteration */
      r = *(struct mqtt_request_t *const volatile *)tail;
    } else {
      r->timeout_diff -= t;
      t = 0;
    }
  }
}

/**
 * Free all request items
 * @param tail Pointer to request queue tail pointer
 */
static void
mqtt_clear_requests(struct mqtt_request_t **tail)
{
  struct mqtt_request_t *iter, *next;
  MQTT_ASSERT("mqtt_clear_requests: tail != NULL", tail != NULL);
  for (iter = *tail; iter != NULL; iter = next) {
    next = iter->next;
    mqtt_delete_request(iter);
  }
  *tail = NULL;
}

/**
 * Initialize all request items
 * @param r_objs Pointer to request objects
 * @param r_objs_len Number of array entries
 */
static void
mqtt_init_requests(struct mqtt_request_t *r_objs, size_t r_objs_len)
{
  uint8_t n;
  MQTT_ASSERT("mqtt_init_requests: r_objs != NULL", r_objs != NULL);
  for (n = 0; n < r_objs_len; n++) {
    /* Item pointing to itself indicates unused */
    r_objs[n].next = &r_objs[n];
  }
}

/*--------------------------------------------------------------------------------------------------------------------- */
/* Output message build helpers */

static void
mqtt_output_append_uint8(struct mqtt_ringbuf_t *rb, uint8_t value)
{
  mqtt_ringbuf_put(rb, value);
}

static
void mqtt_output_append_uint16(struct mqtt_ringbuf_t *rb, uint16_t value)
{
  mqtt_ringbuf_put(rb, value >> 8);
  mqtt_ringbuf_put(rb, value & 0xff);
}

static void
mqtt_output_append_buf(struct mqtt_ringbuf_t *rb, const void *data, uint16_t length)
{
  uint16_t n;
  for (n = 0; n < length; n++) {
    mqtt_ringbuf_put(rb, ((const uint8_t *)data)[n]);
  }
}

static void
mqtt_output_append_string(struct mqtt_ringbuf_t *rb, const char *str, uint16_t length)
{
  uint16_t n;
  mqtt_ringbuf_put(rb, length >> 8);
  mqtt_ringbuf_put(rb, length & 0xff);
  for (n = 0; n < length; n++) {
    mqtt_ringbuf_put(rb, str[n]);
  }
}

/**
 * Append fixed header
 * @param rb Output ring buffer
 * @param msg_type see enum mqtt_message_type
 * @param fdup MQTT DUP flag
 * @param fqos MQTT QoS field
 * @param fretain MQTT retain flag
 * @param r_length Remaining length after fixed header
 */

static void
mqtt_output_append_fixed_header(struct mqtt_ringbuf_t *rb, uint8_t msg_type, uint8_t fdup,
                                uint8_t fqos, uint8_t fretain, uint16_t r_length)
{
  /* Start with control byte */
  mqtt_output_append_uint8(rb, (((msg_type & 0x0f) << 4) | ((fdup & 1) << 3) | ((fqos & 3) << 1) | (fretain & 1)));
  /* Encode remaining length field */
  do {
    mqtt_output_append_uint8(rb, (r_length & 0x7f) | (r_length >= 128 ? 0x80 : 0));
    r_length >>= 7;
  } while (r_length > 0);
}

/**
 * Check output buffer space
 * @param rb Output ring buffer
 * @param r_length Remaining length after fixed header
 * @return 1 if message will fit, 0 if not enough buffer space
 */
static uint8_t
mqtt_output_check_space(struct mqtt_ringbuf_t *rb, uint16_t r_length)
{
  /* Start with length of type byte + remaining length */
  uint16_t total_len = 1 + r_length;

  MQTT_ASSERT("mqtt_output_check_space: rb != NULL", rb != NULL);

  /* Calculate number of required bytes to contain the remaining bytes field and add to total*/
  do {
    total_len++;
    r_length >>= 7;
  } while (r_length > 0);

  return (total_len <= mqtt_ringbuf_free(rb));
}


/**
 * Close connection to server
 * @param client MQTT client
 * @param reason Reason for disconnection
 */
void
mqtt_close(mqtt_client_t *client, mqtt_connection_status_t reason)
{
  MQTT_ASSERT("mqtt_close: client != NULL", client != NULL);

  client->conn->close_connection(client->conn->connCtxt);

  /* Remove all pending requests */
  mqtt_clear_requests(&client->pend_req_queue);
  /* Stop cyclic timer */
  client->conn->timer_stop(mqtt_cyclic_timer, client);
  client->conn = NULL;

  /* Notify upper layer of disconnection if changed state */
  if (client->conn_state != TCP_DISCONNECTED) {

    client->conn_state = TCP_DISCONNECTED;
    if (client->connect_cb != NULL) {
      client->connect_cb(client, client->connect_arg, reason);
    }
  }
}

static mqtt_err_t mqtt_process_rx(mqtt_client_t *client)
{
  mqtt_connection_status_t res;

  if (MQTT_CONNECT_ACCEPTED != (res = mqtt_parse_incoming(client))) {
    MQTT_DEBUGF (("mqtt_parse - %u", res));
    mqtt_close(client, res);
    return res;
  }
  /* If keep alive functionality is used */
  if (client->keep_alive != 0) {
    /* Reset server alive watchdog */
    client->server_watchdog = 0;
  }
  return MQTT_CONNECT_ACCEPTED;
}

static void mqtt_process_tx(mqtt_client_t *client, uint8_t isTxDone)
{
  struct mqtt_request_t *r;

  if (client->conn_state != MQTT_CONNECTED) return;

  if (isTxDone)
  {
    client->cyclic_tick = 0;
    client->server_watchdog = 0;

    /* QoS 0 publish has no response from server, so call its callbacks here */
    while ((r = mqtt_take_request(&client->pend_req_queue, 0)) != NULL) {
      MQTT_DEBUGF(("mqtt_tcp_sent_cb: Calling QoS 0 publish complete callback\n"));
      if (r->cb != NULL) {
        r->cb(r->arg, MQTT_ERR_OK);
      }
      mqtt_delete_request(r);
    }
  }
  /* Try send any remaining buffers from output queue */
  mqtt_output_send(&client->output, client->conn);
}

/**
 * Process async events such as receive response or send pending request on buffer availability
 * @param client mqtt client handle
*/
void mqtt_process(mqtt_client_t *client, uint8_t isTxDone)
{
  if (!isTxDone && (MQTT_CONNECT_ACCEPTED != mqtt_process_rx(client)))
    return;
  mqtt_process_tx(client, isTxDone);
  return;
}

/**
 * Try send as many bytes as possible from output ring buffer
 * @param rb Output ring buffer
 * @param tpcb TCP connection handle
 */
void
mqtt_output_send(struct mqtt_ringbuf_t *rb, mqtt_transport_intf_t *trans)
{
  mqtt_err_t err;
  uint8_t wrap = 0;
  uint16_t ringbuf_lin_len = mqtt_ringbuf_linear_read_length(rb);
  uint16_t send_len = 0;
  MQTT_ASSERT("mqtt_output_send: trans != NULL", trans != NULL);
  MQTT_ASSERT("mqtt_output_send: trans->send_to_nw != NULL", trans->send_to_network != NULL);

  if (trans->get_send_len)
  {
    send_len = trans->get_send_len(trans->connCtxt);
  }
  if (send_len == 0 || ringbuf_lin_len == 0) {
    return;
  }

  MQTT_DEBUGF(("mqtt_output_send: tcp_sndbuf: %d bytes, ringbuf_linear_available: %d, get %d, put %d\n",send_len, ringbuf_lin_len, ((rb)->get & MQTT_RINGBUF_IDX_MASK), ((rb)->put & MQTT_RINGBUF_IDX_MASK)));

  if (send_len > ringbuf_lin_len) {
    /* Space in TCP output buffer is larger than available in ring buffer linear portion */
    send_len = ringbuf_lin_len;
    /* Wrap around if more data in ring buffer after linear portion */
    wrap = (mqtt_ringbuf_len(rb) > ringbuf_lin_len);
  }
  err = trans->send_to_network(trans->connCtxt, mqtt_ringbuf_get_ptr(rb), send_len,
          MQTT_WRITE_FLAG_COPY | (wrap ? MQTT_WRITE_FLAG_MORE : 0), !wrap);
  if ((err == MQTT_ERR_OK) && wrap) {
    mqtt_ringbuf_advance_get_idx(rb, send_len);
    /* Use the lesser one of ring buffer linear length and TCP send buffer size */
    if (trans->get_send_len)
      send_len = trans->get_send_len(trans->connCtxt);
    else
      send_len = 0;
    send_len = MQTT_MIN(send_len, mqtt_ringbuf_linear_read_length(rb));
    err = trans->send_to_network(trans->connCtxt, mqtt_ringbuf_get_ptr(rb), send_len, MQTT_WRITE_FLAG_COPY, 1);
  }
  if (err == MQTT_ERR_OK) {
    mqtt_ringbuf_advance_get_idx(rb, send_len);
  } else {
    MQTT_DEBUGF(("mqtt_output_send: Send failed with err %d \n", err));
  }
}

/**
 * Interval timer, called every MQTT_CYCLIC_TIMER_INTERVAL seconds in MQTT_CONNECTING and MQTT_CONNECTED states
 * @param arg MQTT client
 */
void
mqtt_cyclic_timer(void *arg)
{
  uint8_t restart_timer = 1;
  mqtt_client_t *client = (mqtt_client_t *)arg;
  MQTT_ASSERT("mqtt_cyclic_timer: client != NULL", client != NULL);

  if (client->conn_state == MQTT_CONNECTING) {
    client->cyclic_tick++;
    if ((client->cyclic_tick * MQTT_CYCLIC_TIMER_INTERVAL) >= MQTT_CONNECT_TIMOUT) {
      MQTT_DEBUGF(("mqtt_cyclic_timer: CONNECT attempt to server timed out\n"));
      /* Disconnect TCP */
      mqtt_close(client, MQTT_CONNECT_TIMEOUT);
      restart_timer = 0;
    }
  } else if (client->conn_state == MQTT_CONNECTED) {
    /* Handle timeout for pending requests */
    mqtt_request_time_elapsed(&client->pend_req_queue, MQTT_CYCLIC_TIMER_INTERVAL);

    /* keep_alive > 0 means keep alive functionality shall be used */
    if (client->keep_alive > 0) {

      client->server_watchdog++;
      /* If reception from server has been idle for 1.5*keep_alive time, server is considered unresponsive */
      if ((client->server_watchdog * MQTT_CYCLIC_TIMER_INTERVAL) > (client->keep_alive + client->keep_alive / 2)) {
        MQTT_DEBUGF( ("mqtt_cyclic_timer: Server incoming keep-alive timeout\n"));
        mqtt_close(client, MQTT_CONNECT_TIMEOUT);
        restart_timer = 0;
      }

      /* If time for a keep alive message to be sent, transmission has been idle for keep_alive time */
      if ((client->cyclic_tick * MQTT_CYCLIC_TIMER_INTERVAL) >= client->keep_alive) {
        MQTT_DEBUGF(("mqtt_cyclic_timer: Sending keep-alive message to server\n"));
        if (mqtt_output_check_space(&client->output, 0) != 0) {
          mqtt_output_append_fixed_header(&client->output, MQTT_MSG_TYPE_PINGREQ, 0, 0, 0, 0);
          client->cyclic_tick = 0;
        }
      } else {
        client->cyclic_tick++;
      }
    }
  } else {
    MQTT_DEBUGF(("mqtt_cyclic_timer: Timer should not be running in state %d\n", client->conn_state));
    restart_timer = 0;
  }
  if (restart_timer) {
    client->conn->timer_start(MQTT_CYCLIC_TIMER_INTERVAL*1000, mqtt_cyclic_timer, arg);
  }
}


/**
 * Send PUBACK, PUBREC or PUBREL response message
 * @param client MQTT client
 * @param msg PUBACK, PUBREC or PUBREL
 * @param pkt_id Packet identifier
 * @param qos QoS value
 * @return MQTT_ERR_OK if successful, MQTT_ERR_MEM if out of memory
 */
static mqtt_err_t
pub_ack_rec_rel_response(mqtt_client_t *client, uint8_t msg, uint16_t pkt_id, uint8_t qos)
{
  mqtt_err_t err = MQTT_ERR_OK;
  if (mqtt_output_check_space(&client->output, 2)) {
    mqtt_output_append_fixed_header(&client->output, msg, 0, qos, 0, 2);
    mqtt_output_append_uint16(&client->output, pkt_id);
    mqtt_output_send(&client->output, client->conn);
  } else {
    MQTT_DEBUGF(("pub_ack_rec_rel_response: OOM creating response: %s with pkt_id: %d\n",mqtt_msg_type_to_str(msg), pkt_id));
    err = MQTT_ERR_MEM;
  }
  return err;
}

/**
 * Subscribe response from server
 * @param r Matching request
 * @param result Result code from server
 */
static void
mqtt_incomming_suback(struct mqtt_request_t *r, uint8_t result)
{
  if (r->cb != NULL) {
    r->cb(r->arg, result < 3 ? MQTT_ERR_OK : MQTT_ERR_ABRT);
  }
}


/**
 * Complete MQTT message received or buffer full
 * @param client MQTT client
 * @param fixed_hdr_idx header index
 * @param length length received part
 * @param remaining_length Remaining length of complete message
 */
mqtt_connection_status_t
mqtt_message_received(mqtt_client_t *client, uint8_t fixed_hdr_idx, uint16_t length, uint32_t remaining_length)
{
  mqtt_connection_status_t res = MQTT_CONNECT_ACCEPTED;

  uint8_t *var_hdr_payload = client->rx_buffer + fixed_hdr_idx;
  size_t var_hdr_payload_bufsize = sizeof(client->rx_buffer) - fixed_hdr_idx;

  /* Control packet type */
  uint8_t pkt_type = MQTT_CTL_PACKET_TYPE(client->rx_buffer[0]);
  uint16_t pkt_id = 0;

  MQTT_ASSERT("client->msg_idx < MQTT_VAR_HEADER_BUFFER_LEN", client->msg_idx < MQTT_VAR_HEADER_BUFFER_LEN);
  MQTT_ASSERT("fixed_hdr_idx <= client->msg_idx", fixed_hdr_idx <= client->msg_idx);
  MQTT_ERROR("buffer length mismatch", fixed_hdr_idx + length <= MQTT_VAR_HEADER_BUFFER_LEN,
             return MQTT_CONNECT_DISCONNECTED);

  if (pkt_type == MQTT_MSG_TYPE_CONNACK) {
    if (client->conn_state == MQTT_CONNECTING) {
      if (length < 2) {
        MQTT_DEBUGF(( "mqtt_message_received: Received short CONNACK message\n"));
        goto out_disconnect;
      }
      /* Get result code from CONNACK */
      res = (mqtt_connection_status_t)var_hdr_payload[1];
      MQTT_DEBUGF(("mqtt_message_received: Connect response code %d\n", res));
      if (res == MQTT_CONNECT_ACCEPTED) {
        /* Reset cyclic_tick when changing to connected state */
        client->cyclic_tick = 0;
        client->conn_state = MQTT_CONNECTED;
        /* Notify upper layer */
        if (client->connect_cb != 0) {
          client->connect_cb(client, client->connect_arg, res);
        }
      }
    } else {
      MQTT_DEBUGF(("mqtt_message_received: Received CONNACK in connected state\n"));
    }
  } else if (pkt_type == MQTT_MSG_TYPE_PINGRESP) {
    MQTT_DEBUGF(( "mqtt_message_received: Received PINGRESP from server\n"));

  } else if (pkt_type == MQTT_MSG_TYPE_PUBLISH) {
    uint16_t payload_offset = 0;
    uint16_t payload_length = length;
    uint8_t qos = MQTT_CTL_PACKET_QOS(client->rx_buffer[0]);
    uint8_t *topic;
    uint16_t topic_len;
    if (client->msg_idx <= MQTT_VAR_HEADER_BUFFER_LEN) {
      /* Should have topic and pkt id*/
      uint16_t after_topic;
      uint8_t bkp;

      uint16_t qos_len = (qos ? 2U : 0U);
      if (length < 2 + qos_len) {
        MQTT_DEBUGF(( "mqtt_message_received: Received short PUBLISH packet\n"));
        goto out_disconnect;
      }
      topic_len = var_hdr_payload[0];
      topic_len = (topic_len << 8) + (uint16_t)(var_hdr_payload[1]);
      if ((topic_len > length - (2 + qos_len)) ||
          (topic_len > var_hdr_payload_bufsize - (2 + qos_len))) {
        MQTT_DEBUGF(( "mqtt_message_received: Received short PUBLISH packet (topic)\n"));
        goto out_disconnect;
      }

      topic = var_hdr_payload + 2;
      after_topic = 2 + topic_len;
      /* Check buffer length, add one byte even for QoS 0 so that zero termination will fit */
      if ((after_topic + (qos ? 2U : 1U)) > var_hdr_payload_bufsize) {
        MQTT_DEBUGF(("mqtt_message_received: Receive buffer can not fit topic + pkt_id\n"));
        goto out_disconnect;
      }

      /* id for QoS 1 and 2 */
      if (qos > 0) {
        if (length < after_topic + 2U) {
          MQTT_DEBUGF(( "mqtt_message_received: Received short PUBLISH packet (after_topic)\n"));
          goto out_disconnect;
        }
        client->inpub_pkt_id = ((uint16_t)var_hdr_payload[after_topic] << 8) + (uint16_t)var_hdr_payload[after_topic + 1];
        after_topic += 2;
      } else {
        client->inpub_pkt_id = 0;
      }
      /* Take backup of byte after topic */
      bkp = topic[topic_len];
      /* Zero terminate string */
      topic[topic_len] = 0;
      /* Payload data remaining in receive buffer */
      payload_length = length - after_topic;
      payload_offset = after_topic;

      MQTT_DEBUGF(("mqtt_incomming_publish: Received message with QoS %d at topic: %s, payload length %u\n",
                                     qos, topic, remaining_length + payload_length));
      if (client->pub_cb != NULL) {
        client->pub_cb(client->inpub_arg, (const char *)topic, remaining_length + payload_length);
      }
      /* Restore byte after topic */
      topic[topic_len] = bkp;
    }
    if (payload_length > 0 || remaining_length == 0) {
      if (length < (size_t)(payload_offset + payload_length)) {
        MQTT_DEBUGF(( "mqtt_message_received: Received short packet (payload)\n"));
        goto out_disconnect;
      }
      client->data_cb(client->inpub_arg, (const char *)topic, topic_len, var_hdr_payload + payload_offset, payload_length, remaining_length == 0 ? MQTT_DATA_FLAG_LAST : 0);
      /* Reply if QoS > 0 */
      if (remaining_length == 0 && qos > 0) {
        /* Send PUBACK for QoS 1 or PUBREC for QoS 2 */
        uint8_t resp_msg = (qos == 1) ? MQTT_MSG_TYPE_PUBACK : MQTT_MSG_TYPE_PUBREC;
        MQTT_DEBUGF(("mqtt_incomming_publish: Sending publish response: %s with pkt_id: %d\n",
                                       mqtt_msg_type_to_str(resp_msg), client->inpub_pkt_id));
        pub_ack_rec_rel_response(client, resp_msg, client->inpub_pkt_id, 0);
      }
    }
  } else {
    /* Get packet identifier */
    pkt_id = (uint16_t)var_hdr_payload[0] << 8;
    pkt_id |= (uint16_t)var_hdr_payload[1];
    if (pkt_id == 0) {
      MQTT_DEBUGF(("mqtt_message_received: Got message with illegal packet identifier: 0\n"));
      goto out_disconnect;
    }
    if (pkt_type == MQTT_MSG_TYPE_PUBREC) {
      MQTT_DEBUGF(("mqtt_message_received: PUBREC, sending PUBREL with pkt_id: %d\n", pkt_id));
      pub_ack_rec_rel_response(client, MQTT_MSG_TYPE_PUBREL, pkt_id, 1);

    } else if (pkt_type == MQTT_MSG_TYPE_PUBREL) {
      MQTT_DEBUGF(("mqtt_message_received: PUBREL, sending PUBCOMP response with pkt_id: %d\n", pkt_id));
      pub_ack_rec_rel_response(client, MQTT_MSG_TYPE_PUBCOMP, pkt_id, 0);

    } else if (pkt_type == MQTT_MSG_TYPE_SUBACK || pkt_type == MQTT_MSG_TYPE_UNSUBACK ||
               pkt_type == MQTT_MSG_TYPE_PUBCOMP || pkt_type == MQTT_MSG_TYPE_PUBACK) {
      struct mqtt_request_t *r = mqtt_take_request(&client->pend_req_queue, pkt_id);
      if (r != NULL) {
        MQTT_DEBUGF(("mqtt_message_received: %s response with id %d\n", mqtt_msg_type_to_str(pkt_type), pkt_id));
        if (pkt_type == MQTT_MSG_TYPE_SUBACK) {
          if (length < 3) {
            MQTT_DEBUGF(("mqtt_message_received: To small SUBACK packet\n"));
            goto out_disconnect;
          } else {
            mqtt_incomming_suback(r, var_hdr_payload[2]);
          }
        } else if (r->cb != NULL) {
          r->cb(r->arg, MQTT_ERR_OK);
        }
        mqtt_delete_request(r);
      } else {
         MQTT_DEBUGF(( "mqtt_message_received: Received %s reply, with wrong pkt_id: %d\n", mqtt_msg_type_to_str(pkt_type), pkt_id));
      }
    } else {
      MQTT_DEBUGF(( "mqtt_message_received: Received unknown message type: %d\n", pkt_type));
      goto out_disconnect;
    }
  }
  return res;
out_disconnect:
  return MQTT_CONNECT_DISCONNECTED;
}


/**
 * MQTT incoming message parser
 * @param client MQTT client
 * @param p PBUF chain of received data
 * @return Connection status
 */
mqtt_connection_status_t
mqtt_parse_incoming(mqtt_client_t *client)
{
  uint16_t in_offset = 0, tot_len = 0;
  uint32_t msg_rem_len = 0;
  uint8_t fixed_hdr_idx = 0;
  uint8_t b = 0;
  mqtt_transport_intf_t *conn = client->conn;

  MQTT_ASSERT("mqtt_parse_incoming(), conn != NULL", client->conn != NULL);
  MQTT_ASSERT("mqtt_parse_incoming(), conn->recv_cb != NULL", conn->recv_from_network != NULL);
  MQTT_ASSERT("mqtt_parse_incoming(), conn->recv_len_cb != NULL", conn->get_recv_len != NULL);
  tot_len = conn->get_recv_len(conn->connCtxt);
  while (tot_len > in_offset) {
    /* We ALWAYS parse the header here first. Even if the header was not
       included in this segment, we re-parse it here by buffering it in
       client->rx_buffer. client->msg_idx keeps track of this. */
    if ((fixed_hdr_idx < 2) || ((b & 0x80) != 0)) {

      if (fixed_hdr_idx < client->msg_idx) {
        /* parse header from old pbuf (buffered in client->rx_buffer) */
        b = client->rx_buffer[fixed_hdr_idx];
      } else {
        /* parse header from this pbuf and save it in client->rx_buffer in case
           it comes in segmented */
        conn->recv_from_network(conn->connCtxt, &b, 1, in_offset++);
        client->rx_buffer[client->msg_idx++] = b;
      }
      fixed_hdr_idx++;

      if (fixed_hdr_idx >= 2) {
        /* fixed header contains at least 2 bytes but can contain more, depending on
           'remaining length'. All bytes but the last of this have 0x80 set to
           indicate more bytes are coming. */
        msg_rem_len |= (uint32_t)(b & 0x7f) << ((fixed_hdr_idx - 2) * 7);
        if ((b & 0x80) == 0) {
          /* fixed header is done */
          MQTT_DEBUGF(("mqtt_parse_incoming: Remaining length after fixed header: %u\n", msg_rem_len));
          if (msg_rem_len == 0) {
            /* Complete message with no extra headers of payload received */
            mqtt_message_received(client, fixed_hdr_idx, 0, 0);
            client->msg_idx = 0;
            fixed_hdr_idx = 0;
          } else {
            /* Bytes remaining in message (changes remaining length if this is
               not the first segment of this message) */
            msg_rem_len = (msg_rem_len + fixed_hdr_idx) - client->msg_idx;
          }
        }
      }
    } else {
      /* Fixed header has been parsed, parse variable header */
      uint16_t cpy_len, cpy_start, buffer_space;

      cpy_start = (client->msg_idx - fixed_hdr_idx) % (MQTT_VAR_HEADER_BUFFER_LEN - fixed_hdr_idx) + fixed_hdr_idx;

      /* Allow to copy the lesser one of available length in input data or bytes remaining in message */
      cpy_len = (uint16_t)MQTT_MIN((uint16_t)(tot_len - in_offset), msg_rem_len);

      /* Limit to available space in buffer */
      buffer_space = MQTT_VAR_HEADER_BUFFER_LEN - cpy_start;
      if (cpy_len > buffer_space) {
        cpy_len = buffer_space;
      }
      conn->recv_from_network(conn->connCtxt, client->rx_buffer + cpy_start, cpy_len, in_offset);

      /* Advance get and put indexes  */
      client->msg_idx += cpy_len;
      in_offset += cpy_len;
      msg_rem_len -= cpy_len;

      MQTT_DEBUGF(("mqtt_parse_incoming: msg_idx: %u, cpy_len: %u, remaining %u\n", client->msg_idx, cpy_len, msg_rem_len));
      if ((msg_rem_len == 0) || (cpy_len == buffer_space)) {
        /* Whole message received or buffer is full */
        mqtt_connection_status_t res = mqtt_message_received(client, fixed_hdr_idx, (cpy_start + cpy_len) - fixed_hdr_idx, msg_rem_len);
        if (res != MQTT_CONNECT_ACCEPTED) {
          return res;
        }
        if (msg_rem_len == 0) {
          /* Reset parser state */
          client->msg_idx = 0;
          /* msg_tot_len = 0; */
          fixed_hdr_idx = 0;
        }
      }
    }
  }
  return MQTT_CONNECT_ACCEPTED;
}

/*---------------------------------------------------------------------------------------------------- */
/* Public API */


/**
 * @ingroup mqtt
 * MQTT publish function.
 * @param client MQTT client
 * @param topic Publish topic string
 * @param payload Data to publish (NULL is allowed)
 * @param payload_length Length of payload (0 is allowed)
 * @param qos Quality of service, 0 1 or 2
 * @param retain MQTT retain flag
 * @param cb Callback to call when publish is complete or has timed out
 * @param arg User supplied argument to publish callback
 * @return MQTT_ERR_OK if successful
 *         MQTT_ERR_CONN if client is disconnected
 *         MQTT_ERR_MEM if short on memory
 */
mqtt_err_t
mqtt_publish(mqtt_client_t *client, const char *topic, const void *payload, uint16_t payload_length, uint8_t qos, uint8_t retain,
             mqtt_request_cb_t cb, void *arg)
{
  struct mqtt_request_t *r;
  uint16_t pkt_id;
  size_t topic_strlen;
  size_t total_len;
  uint16_t topic_len;
  uint16_t remaining_length;

  MQTT_ASSERT_CORE_LOCKED();
  MQTT_ASSERT("mqtt_publish: client != NULL", client);
  MQTT_ASSERT("mqtt_publish: topic != NULL", topic);
  MQTT_ERROR("mqtt_publish: TCP disconnected", (client->conn_state != TCP_DISCONNECTED), return MQTT_ERR_CONN);

  topic_strlen = strlen(topic);
  MQTT_ERROR("mqtt_publish: topic length overflow", (topic_strlen <= (0xFFFF - 2)), return MQTT_ERR_ARG);
  topic_len = (uint16_t)topic_strlen;
  total_len = 2 + topic_len + payload_length;

  if (qos > 0) {
    total_len += 2;
    /* Generate pkt_id id for QoS1 and 2 */
    pkt_id = msg_generate_packet_id(client);
  } else {
    /* Use reserved value pkt_id 0 for QoS 0 in request handle */
    pkt_id = 0;
  }
  MQTT_ERROR("mqtt_publish: total length overflow", (total_len <= 0xFFFF), return MQTT_ERR_ARG);
  remaining_length = (uint16_t)total_len;

  MQTT_DEBUGF(("mqtt_publish: Publish with payload length %d to topic \"%s\"\n", payload_length, topic));

  r = mqtt_create_request(client->req_list, MQTT_ARRAYSIZE(client->req_list), pkt_id, cb, arg);
  if (r == NULL) {
    return MQTT_ERR_MEM;
  }

  if (mqtt_output_check_space(&client->output, remaining_length) == 0) {
    mqtt_delete_request(r);
    return MQTT_ERR_MEM;
  }
  /* Append fixed header */
  mqtt_output_append_fixed_header(&client->output, MQTT_MSG_TYPE_PUBLISH, 0, qos, retain, remaining_length);

  /* Append Topic */
  mqtt_output_append_string(&client->output, topic, topic_len);

  /* Append packet if for QoS 1 and 2*/
  if (qos > 0) {
    mqtt_output_append_uint16(&client->output, pkt_id);
  }

  /* Append optional publish payload */
  if ((payload != NULL) && (payload_length > 0)) {
    mqtt_output_append_buf(&client->output, payload, payload_length);
  }

  mqtt_append_request(&client->pend_req_queue, r);
  mqtt_output_send(&client->output, client->conn);
  return MQTT_ERR_OK;
}


/**
 * @ingroup mqtt
 * MQTT subscribe/unsubscribe function.
 * @param client MQTT client
 * @param topic topic to subscribe to
 * @param qos Quality of service, 0 1 or 2 (only used for subscribe)
 * @param cb Callback to call when subscribe/unsubscribe reponse is received
 * @param arg User supplied argument to publish callback
 * @param sub 1 for subscribe, 0 for unsubscribe
 * @return MQTT_ERR_OK if successful, @see mqtt_err_t enum for other results
 */
mqtt_err_t
mqtt_sub_unsub(mqtt_client_t *client, const char *topic, uint8_t qos, mqtt_request_cb_t cb, void *arg, uint8_t sub)
{
  size_t topic_strlen;
  size_t total_len;
  uint16_t topic_len;
  uint16_t remaining_length;
  uint16_t pkt_id;
  struct mqtt_request_t *r;

  MQTT_ASSERT_CORE_LOCKED();
  MQTT_ASSERT("mqtt_sub_unsub: client != NULL", client);
  MQTT_ASSERT("mqtt_sub_unsub: topic != NULL", topic);

  topic_strlen = strlen(topic);
  MQTT_ERROR("mqtt_sub_unsub: topic length overflow", (topic_strlen <= (0xFFFF - 2)), return MQTT_ERR_ARG);
  topic_len = (uint16_t)topic_strlen;
  /* Topic string, pkt_id, qos for subscribe */
  total_len =  topic_len + 2 + 2 + (sub != 0);
  MQTT_ERROR("mqtt_sub_unsub: total length overflow", (total_len <= 0xFFFF), return MQTT_ERR_ARG);
  remaining_length = (uint16_t)total_len;

  MQTT_ASSERT("mqtt_sub_unsub: qos < 3", qos < 3);
  if (client->conn_state == TCP_DISCONNECTED) {
    MQTT_DEBUGF( ("mqtt_sub_unsub: Can not (un)subscribe in disconnected state\n"));
    return MQTT_ERR_CONN;
  }

  pkt_id = msg_generate_packet_id(client);
  r = mqtt_create_request(client->req_list, MQTT_ARRAYSIZE(client->req_list), pkt_id, cb, arg);
  if (r == NULL) {
    return MQTT_ERR_MEM;
  }

  if (mqtt_output_check_space(&client->output, remaining_length) == 0) {
    mqtt_delete_request(r);
    return MQTT_ERR_MEM;
  }

  MQTT_DEBUGF(("mqtt_sub_unsub: Client (un)subscribe to topic \"%s\", id: %d\n", topic, pkt_id));

  mqtt_output_append_fixed_header(&client->output, sub ? MQTT_MSG_TYPE_SUBSCRIBE : MQTT_MSG_TYPE_UNSUBSCRIBE, 0, 1, 0, remaining_length);
  /* Packet id */
  mqtt_output_append_uint16(&client->output, pkt_id);
  /* Topic */
  mqtt_output_append_string(&client->output, topic, topic_len);
  /* QoS */
  if (sub != 0) {
    mqtt_output_append_uint8(&client->output, MQTT_MIN(qos, 2));
  }

  mqtt_append_request(&client->pend_req_queue, r);
  mqtt_output_send(&client->output, client->conn);
  return MQTT_ERR_OK;
}


/**
 * @ingroup mqtt
 * Set callback to handle incoming publish requests from server
 * @param client MQTT client
 * @param pub_cb Callback invoked when publish starts, contain topic and total length of payload
 * @param data_cb Callback for each fragment of payload that arrives
 * @param arg User supplied argument to both callbacks
 */
void
mqtt_set_inpub_callback(mqtt_client_t *client, mqtt_incoming_publish_cb_t pub_cb,
                        mqtt_incoming_data_cb_t data_cb, void *arg)
{
  MQTT_ASSERT_CORE_LOCKED();
  MQTT_ASSERT("mqtt_set_inpub_callback: client != NULL", client != NULL);
  client->data_cb = data_cb;
  client->pub_cb = pub_cb;
  client->inpub_arg = arg;
}

/**
 * @ingroup mqtt
 * Create a new MQTT client instance
 * @return Pointer to instance on success, NULL otherwise
 */
mqtt_client_t *
mqtt_client_new(void)
{
  MQTT_ASSERT_CORE_LOCKED();
  return (mqtt_client_t *)mem_calloc(1, sizeof(mqtt_client_t));
}

/**
 * @ingroup mqtt
 * Free MQTT client instance
 * @param client Pointer to instance to be freed
 */
void
mqtt_client_free(mqtt_client_t *client)
{
  mem_free(client);
}

/**
 * @ingroup mqtt
 * Connect to MQTT server
 * @param client MQTT client
 * @param ip_addr Server IP
 * @param port Server port
 * @param cb Connection state change callback
 * @param arg User supplied argument to connection callback
 * @param client_info Client identification and connection options
 * @return MQTT_ERR_OK if successful, @see mqtt_err_t enum for other results
 */
mqtt_err_t
mqtt_client_connect(mqtt_client_t *client, void *conn, mqtt_connection_cb_t cb, void *arg,
                    const struct mqtt_connect_client_info_t *client_info)
{
  size_t len;
  uint16_t client_id_length;
  /* Length is the sum of 2+"MQTT", protocol level, flags and keep alive */
  uint16_t remaining_length = 2 + 4 + 1 + 1 + 2;
  uint8_t flags = 0, will_topic_len = 0, will_msg_len = 0;
  uint16_t client_user_len = 0, client_pass_len = 0;

  MQTT_ASSERT_CORE_LOCKED();
  MQTT_ASSERT("mqtt_client_connect: client != NULL", client != NULL);
  MQTT_ASSERT("mqtt_client_connect: conn != NULL", conn != NULL);
  MQTT_ASSERT("mqtt_client_connect: client_info != NULL", client_info != NULL);
  MQTT_ASSERT("mqtt_client_connect: client_info->client_id != NULL", client_info->client_id != NULL);

  if (client->conn_state != TCP_DISCONNECTED) {
    MQTT_DEBUGF(("mqtt_client_connect: Already connected\n"));
    return MQTT_ERR_ISCONN;
  }

  /* Wipe clean */
  memset(client, 0, sizeof(mqtt_client_t));
  client->connect_arg = arg;
  client->connect_cb = cb;
  client->keep_alive = client_info->keep_alive;
  mqtt_init_requests(client->req_list, MQTT_ARRAYSIZE(client->req_list));

  /* Build connect message */
  if (client_info->will_topic != NULL && client_info->will_msg != NULL) {
    flags |= MQTT_CONNECT_FLAG_WILL;
    flags |= (client_info->will_qos & 3) << 3;
    if (client_info->will_retain) {
      flags |= MQTT_CONNECT_FLAG_WILL_RETAIN;
    }
    len = strlen(client_info->will_topic);
    MQTT_ERROR("mqtt_client_connect: client_info->will_topic length overflow", len <= 0xFF, return MQTT_ERR_VAL);
    MQTT_ERROR("mqtt_client_connect: client_info->will_topic length must be > 0", len > 0, return MQTT_ERR_VAL);
    will_topic_len = (uint8_t)len;
    len = strlen(client_info->will_msg);
    MQTT_ERROR("mqtt_client_connect: client_info->will_msg length overflow", len <= 0xFF, return MQTT_ERR_VAL);
    will_msg_len = (uint8_t)len;
    len = remaining_length + 2 + will_topic_len + 2 + will_msg_len;
    MQTT_ERROR("mqtt_client_connect: remaining_length overflow", len <= 0xFFFF, return MQTT_ERR_VAL);
    remaining_length = (uint16_t)len;
  }
  if (client_info->client_user != NULL) {
    flags |= MQTT_CONNECT_FLAG_USERNAME;
    len = strlen(client_info->client_user);
    MQTT_ERROR("mqtt_client_connect: client_info->client_user length overflow", len <= 0xFFFF, return MQTT_ERR_VAL);
    MQTT_ERROR("mqtt_client_connect: client_info->client_user length must be > 0", len > 0, return MQTT_ERR_VAL);
    client_user_len = (uint16_t)len;
    len = remaining_length + 2 + client_user_len;
    MQTT_ERROR("mqtt_client_connect: remaining_length overflow", len <= 0xFFFF, return MQTT_ERR_VAL);
    remaining_length = (uint16_t)len;
  }
  if (client_info->client_pass != NULL) {
    flags |= MQTT_CONNECT_FLAG_PASSWORD;
    len = strlen(client_info->client_pass);
    MQTT_ERROR("mqtt_client_connect: client_info->client_pass length overflow", len <= 0xFFFF, return MQTT_ERR_VAL);
    MQTT_ERROR("mqtt_client_connect: client_info->client_pass length must be > 0", len > 0, return MQTT_ERR_VAL);
    client_pass_len = (uint16_t)len;
    len = remaining_length + 2 + client_pass_len;
    MQTT_ERROR("mqtt_client_connect: remaining_length overflow", len <= 0xFFFF, return MQTT_ERR_VAL);
    remaining_length = (uint16_t)len;
  }

  /* Don't complicate things, always connect using clean session */
  flags |= MQTT_CONNECT_FLAG_CLEAN_SESSION;

  len = strlen(client_info->client_id);
  MQTT_ERROR("mqtt_client_connect: client_info->client_id length overflow", len <= 0xFFFF, return MQTT_ERR_VAL);
  client_id_length = (uint16_t)len;
  len = remaining_length + 2 + client_id_length;
  MQTT_ERROR("mqtt_client_connect: remaining_length overflow", len <= 0xFFFF, return MQTT_ERR_VAL);
  remaining_length = (uint16_t)len;

  if (mqtt_output_check_space(&client->output, remaining_length) == 0) {
    return MQTT_ERR_MEM;
  }

  /* Append fixed header */
  mqtt_output_append_fixed_header(&client->output, MQTT_MSG_TYPE_CONNECT, 0, 0, 0, remaining_length);
  /* Append Protocol string */
  mqtt_output_append_string(&client->output, "MQTT", 4);
  /* Append Protocol level */
  mqtt_output_append_uint8(&client->output, 4);
  /* Append connect flags */
  mqtt_output_append_uint8(&client->output, flags);
  /* Append keep-alive */
  mqtt_output_append_uint16(&client->output, client_info->keep_alive);
  /* Append client id */
  mqtt_output_append_string(&client->output, client_info->client_id, client_id_length);
  /* Append will message if used */
  if ((flags & MQTT_CONNECT_FLAG_WILL) != 0) {
    mqtt_output_append_string(&client->output, client_info->will_topic, will_topic_len);
    mqtt_output_append_string(&client->output, client_info->will_msg, will_msg_len);
  }
  /* Append user name if given */
  if ((flags & MQTT_CONNECT_FLAG_USERNAME) != 0) {
    mqtt_output_append_string(&client->output, client_info->client_user, client_user_len);
  }
  /* Append password if given */
  if ((flags & MQTT_CONNECT_FLAG_PASSWORD) != 0) {
    mqtt_output_append_string(&client->output, client_info->client_pass, client_pass_len);
  }
  client->conn = conn;

  /* Initiate receiver state */
  client->msg_idx = 0;
  /* Enter MQTT connect state */
  client->conn_state = MQTT_CONNECTING;

  /* Start cyclic timer */
  client->conn->timer_start(MQTT_CYCLIC_TIMER_INTERVAL*1000, mqtt_cyclic_timer, client);
  client->cyclic_tick = 0;

  /* Start transmission from output queue, connect message is the first one out*/
  mqtt_output_send(&client->output, client->conn);

  return MQTT_ERR_OK;
}


/**
 * @ingroup mqtt
 * Disconnect from MQTT server
 * @param client MQTT client
 */
void
mqtt_disconnect(mqtt_client_t *client)
{
  MQTT_ASSERT_CORE_LOCKED();
  MQTT_ASSERT("mqtt_disconnect: client != NULL", client);
  /* If connection in not already closed */
  if (client->conn_state != TCP_DISCONNECTED) {
    /* Set conn_state before calling mqtt_close to prevent callback from being called */
    client->conn_state = TCP_DISCONNECTED;
    mqtt_close(client, (mqtt_connection_status_t)0);
  }
}

/**
 * @ingroup mqtt
 * Check connection with server
 * @param client MQTT client
 * @return 1 if connected to server, 0 otherwise
 */
uint8_t
mqtt_client_is_connected(mqtt_client_t *client)
{
  MQTT_ASSERT_CORE_LOCKED();
  MQTT_ASSERT("mqtt_client_is_connected: client != NULL", client);
  return client->conn_state == MQTT_CONNECTED;
}
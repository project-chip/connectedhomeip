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

#include "MQTT_transport.h"
#include <string.h>
#include "silabs_utils.h"
#include "lwip/timeouts.h"
#include "lwip/dns.h"
#include "altcp.h"
#include "altcp_tcp.h"

struct MQTT_Transport_t
{
  u8_t conn_state;
  struct altcp_tls_config *tls_config;
  struct altcp_pcb *conn;

  dic_connect_cb dic_conn_cb;
  ip_addr_t *ipaddr;
  /** received buffers from tcp */
  struct pbuf *pcbRxStart;
  struct pbuf *pcbRxEnd;

  SemaphoreHandle_t sync_sem;
  /* Temp */
  EventGroupHandle_t events;
};

enum{
  TRANSPORT_NOT_CONNECTED,
  TRANSPORT_CONNECTED,
};

static void setup_transport_callbacks(MQTT_Transport_t *client, mqtt_transport_intf_t *trans);
static err_t connection_new(MQTT_Transport_t* , const ip_addr_t *ipaddr, u16_t port);
uint8_t convert_write_flags(uint8_t apiflag);

inline uint8_t convert_write_flags(uint8_t apiflag){
  uint8_t transport_flag=0;
  if(apiflag & MQTT_WRITE_FLAG_COPY){
    transport_flag |= TCP_WRITE_FLAG_COPY;
  }
  if(apiflag & MQTT_WRITE_FLAG_MORE){
    transport_flag |= TCP_WRITE_FLAG_MORE;
  }
  return transport_flag;
}

void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg){
  SILABS_LOG("DNS CALL BACK");
  MQTT_Transport_t* transP = (MQTT_Transport_t* )callback_arg;
  memcpy(transP->ipaddr, ipaddr, sizeof(ip_addr_t));
  xSemaphoreGive(transP->sync_sem);
  return;
}

MQTT_Transport_t * MQTT_Transport_Init(mqtt_transport_intf_t *trans,mqtt_client_t* mqtt_client,EventGroupHandle_t dicEvents)
{ 
  if(trans == NULL && mqtt_client == NULL && dicEvents == NULL){
    SILABS_LOG("MQTT transport init failed");
    return NULL;
  }
  MQTT_Transport_t *client = (MQTT_Transport_t *)pvPortMalloc(sizeof(MQTT_Transport_t));
  if (client != NULL) {
    SILABS_LOG("dns_servers[0] = %s\n", ipaddr_ntoa(dns_getserver(0)));
    memset(client, 0, sizeof(MQTT_Transport_t));
    setup_transport_callbacks(client, trans);
    client->events = dicEvents;
  }
  return client;
}

err_t MQTT_Transport_SSLConfigure(MQTT_Transport_t *transP, const u8_t *ca, size_t ca_len, const u8_t *privkey, size_t privkey_len,
                                        const u8_t *privkey_pass, size_t privkey_pass_len,
                                        const u8_t *cert, size_t cert_len)
{
  struct altcp_tls_config *altcp_tls_config;
  altcp_tls_config = altcp_tls_create_config_client_2wayauth(ca,ca_len,privkey,privkey_len,NULL,0,cert,cert_len);
  if (NULL == altcp_tls_config)
  {
    return ERR_FAIL;
  }
  transP->tls_config = altcp_tls_config;
  return ERR_OK;
}

err_t MQTT_Transport_Connect(MQTT_Transport_t* transP,const char *host, u16_t port, dic_connect_cb dic_conn_cb)
{
  err_t ret;
  err_t dns_ret;
  ip_addr_t ipaddr;
  if(transP == NULL  && host == NULL){
    SILABS_LOG("MQTT transport connect failed");
    return ERR_ARG;
  }
  transP->sync_sem = xSemaphoreCreateCounting(1, 0);
  transP->ipaddr = &ipaddr;
  if((dns_ret = dns_gethostbyname(host,&ipaddr,dns_callback,transP)) != ERR_OK) {
    if(dns_ret == ERR_INPROGRESS){
      SILABS_LOG("in progress");
      xSemaphoreTake(transP->sync_sem, portMAX_DELAY);
    }
    else {
      SILABS_LOG("dns resolving failed %d",dns_ret);
      return dns_ret;
    }
  }
  transP->dic_conn_cb = dic_conn_cb;
  ret = connection_new(transP,&ipaddr,port);
  return ret;
}

static uint16_t transport_get_sendlen_cb(void *conn)
{
  MQTT_Transport_t *client = (MQTT_Transport_t*)conn;
  if (!client) return 0;
  return altcp_sndbuf(client->conn);
}

static int8_t transport_write_cb(void *conn, void *buff, uint16_t len, uint8_t flags, uint8_t isFinal)
{
  MQTT_Transport_t *client = (MQTT_Transport_t*)conn;
  if (!client) return ERR_FAIL;
  if (ERR_OK != altcp_write(client->conn, buff, len, convert_write_flags(flags)))
    return ERR_FAIL;
  if (isFinal)
    altcp_output(client->conn);
  return ERR_OK;
}

static uint16_t transport_get_recvlen_cb(void *conn)
{
  MQTT_Transport_t *client = (MQTT_Transport_t*)conn;
  if (!client) return 0;
  if (!client->pcbRxStart) return 0;
  return client->pcbRxStart->tot_len;
}

static int8_t transport_recv_from_nw(void *conn, void *buf, uint16_t len, uint16_t offset)
{
  MQTT_Transport_t *client = (MQTT_Transport_t*)conn;
  struct pbuf *curr = client->pcbRxStart, *next;
  uint16_t slen;
  if (!client) return 0;
  if (!client->pcbRxStart) return 0;
  if (client->pcbRxStart->tot_len < (offset + len)) return 0;
  pbuf_copy_partial(curr, buf, len, offset);
  if (offset + len >= curr->tot_len)
  {
    slen=curr->len;
    while (slen < client->pcbRxStart->tot_len)
    {
      curr=curr->next;
      slen+= curr->len;
    }
    next = curr->next;
    curr->next = NULL;
    pbuf_free(client->pcbRxStart);
    client->pcbRxStart=next;
  }
  return len;
}

static void
transport_close_cb(void *arg){
  MQTT_Transport_t *client = (MQTT_Transport_t *)arg;
  struct altcp_pcb *conn = client->conn;
	/* Bring down TCP connection if not already done */
  if (conn != NULL && client->conn_state == TRANSPORT_CONNECTED) {
    err_t res;
    altcp_recv(conn, NULL);
    altcp_err(conn,  NULL);
    altcp_sent(conn, NULL);
    res = altcp_close(conn);
    if (res != ERR_OK) {
      altcp_abort(conn);
      TRANSPORT_DEBUGF(("transport_close_cb: Close err=%s\n", lwip_strerr(res)));
    }
    client->conn = NULL;
  }
}

static void transport_timerStart(u32_t msecs, timer_callback handler, void *arg)
{
  sys_timeout(msecs, (sys_timeout_handler)handler, arg);
}

static void transport_timerStop(timer_callback handler, void *arg)
{
  sys_untimeout((sys_timeout_handler)handler, arg);
}

/**
 * TCP error callback function. @see tcp_err_fn
 * @param arg MQTT client
 * @param err Error encountered
 */

static void
transport_err_cb(void *arg, err_t err)
{
  MQTT_Transport_t *client = (MQTT_Transport_t *)arg;
  TRANSPORT_UNUSED_ARG(err); /* only used for debug output */
  TRANSPORT_DEBUGF(("transport_err_cb: TCP error callback: error %d, arg: %p\n", err, arg));
  TRANSPORT_ASSERT("transport_err_cb: client != NULL", client != NULL);
  /* Set conn to null before calling close as pcb is already deallocated*/
  if (client->conn_state)
    xEventGroupSetBits(client->events, SIGNAL_TRANSINTF_CONN_CLOSE);
  client->conn_state = TRANSPORT_NOT_CONNECTED;
  client->dic_conn_cb(err);
}

static err_t
transport_recv_cb(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err)
{
  MQTT_Transport_t *client = (MQTT_Transport_t *)arg;
  TRANSPORT_ASSERT("transport_recv_cb: client != NULL", client != NULL);
  TRANSPORT_ASSERT("transport_recv_cb: client->conn == pcb", client->conn == pcb);

  if (p == NULL) {
    TRANSPORT_DEBUGF(("transport_recv_cb: Recv pbuf=NULL, remote has closed connection\n"));
    xEventGroupSetBits(client->events, SIGNAL_TRANSINTF_CONN_CLOSE);
  } else {
    if (err != ERR_OK) {
      TRANSPORT_DEBUGF(("transport_recv_cb: Recv err=%d\n", err));
      pbuf_free(p);
      return err;
    }

#if 0
    /* Tell remote that data has been received */
    altcp_recved(pcb, p->tot_len);
    res = mqtt_parse_incoming(client, p);
    pbuf_free(p);

    if (res != MQTT_CONNECT_ACCEPTED) {
      mqtt_close(client, res);
    }
    //#endif
    /* If keep alive functionality is used */
    if (client->keep_alive != 0) {
      /* Reset server alive watchdog */
      client->server_watchdog = 0;
    }
#else
  if (client->pcbRxStart == NULL) {
    client->pcbRxStart = p;
    while (p->next) p = p->next;
      client->pcbRxEnd = p;
    }
  else
  {
    client->pcbRxEnd->next = p;
    client->pcbRxEnd = p;
  }
  xEventGroupSetBits(client->events, SIGNAL_TRANSINTF_RX);
#endif
}
  return ERR_OK;
}

/**
 * TCP data sent callback function. @see tcp_sent_fn
 * @param arg MQTT client
 * @param tpcb TCP connection handle
 * @param len Number of bytes sent
 * @return ERR_OK
 */

static err_t
transport_sent_cb(void *arg, struct altcp_pcb *tpcb, u16_t len)
{
  MQTT_Transport_t *client = (MQTT_Transport_t *)arg;
  TRANSPORT_ASSERT("transport_recv_cb: client != NULL", client != NULL);
  TRANSPORT_UNUSED_ARG(tpcb);
  TRANSPORT_UNUSED_ARG(len);
  if (client->conn_state == TRANSPORT_CONNECTED) {
    xEventGroupSetBits(client->events, SIGNAL_TRANSINTF_TX_ACK);
  }
  return ERR_OK;
}

#if 0 //future use
/**
 * TCP poll callback function. @see tcp_poll_fn
 * @param arg MQTT client
 * @param tpcb TCP connection handle
 * @return err ERR_OK
 */
static err_t
transport_poll_cb(void *arg, struct tcp_pcb *tpcb)
{
  // MQTT_Transport_t *tclient = (MQTT_Transport_t *)arg;
  // mqtt_client_t *client = (mqtt_client_t *)tclient->appHdl;
  // if (client->conn_state == TRANSPORT_CONNECTED) {
  MQTT_Transport_t *client = (MQTT_Transport_t *)arg;
  if (client->conn_state == TRANSPORT_CONNECTED) {
    /* Try send any remaining buffers from output queue */
    mqtt_output_send(&client->output, tpcb);
  }
  return ERR_OK;
}
#endif

/**
 * TCP connect callback function. @see tcp_connected_fn
 * @param arg MQTT client
 * @param err Always ERR_OK, transport_err_cb is called in case of error
 * @return ERR_OK
 */

static err_t
transport_connect_cb(void *arg, struct altcp_pcb *tpcb, err_t err)
{
  MQTT_Transport_t* client = (MQTT_Transport_t *)arg;
  if (err != ERR_OK) {
    TRANSPORT_DEBUGF(("transport_connect_cb: TCP connect error %d\n", err));
	  SILABS_LOG("TCP connect error");
    client->conn_state = TRANSPORT_NOT_CONNECTED;
    client->dic_conn_cb(err);
    return err;
  }
  /* Setup TCP callbacks */
  altcp_recv(tpcb, transport_recv_cb);
  altcp_sent(tpcb, transport_sent_cb);
  //tcp_poll(tpcb, tcp_poll_cb, 2);
  TRANSPORT_DEBUGF(("transport_connect_cb: TCP connection established to server\n"));
  SILABS_LOG("tcp_cb: TCP connection established to server\n");
  client->conn_state = TRANSPORT_CONNECTED;
  if(client->dic_conn_cb != NULL){
    client->dic_conn_cb(ERR_OK);
  }
  return ERR_OK;
}

void mbedtls_signal_app(void *arg)
{
  MQTT_Transport_t *client = (MQTT_Transport_t *)arg;
  xEventGroupSetBits(client->events, SIGNAL_TRANSINTF_MBEDTLS_RX);
}
extern err_t
altcp_mbedtls_lower_recv_process(struct altcp_pcb *conn);

void transport_process_mbedtls_rx(MQTT_Transport_t *client)
{
  altcp_mbedtls_lower_recv_process(client->conn);
}

static err_t connection_new(MQTT_Transport_t* client , const ip_addr_t *ipaddr, u16_t port)
{
	err_t err = ERR_ABRT;
	if(client == NULL || ipaddr == NULL) {
		SILABS_LOG("NULL argments");
		return ERR_VAL;
	}
	if (client->conn != NULL) {
		SILABS_LOG("connection already established");
		return ERR_ISCONN;
	}

#if TRANSPORT_ALTCP && TRANSPORT_ALTCP_TLS
  if (client->tls_config) {
    SILABS_LOG("executing tls new");
    client->conn = altcp_tls_new(client->tls_config, IP_GET_TYPE(ipaddr));
  } else
#endif
  {
    SILABS_LOG("not executing");
    client->conn = altcp_tcp_new_ip_type(IP_GET_TYPE(ipaddr));
  }
	if (client->conn == NULL) {
    return ERR_MEM;
  }
	/* Set arg pointer for callbacks */
  altcp_arg(client->conn, client);
  /* Any local address, pick random local port number */
  err = altcp_bind(client->conn, IP_ADDR_ANY, 0);
  if (err != ERR_OK) {
    TRANSPORT_DEBUGF(("client_connect: Error binding to local ip/port, %d\n", err));
    goto transport_fail;
  }

  /* Connect to server */
  err = altcp_connect(client->conn, ipaddr, port, transport_connect_cb);
  if (err != ERR_OK) {
    TRANSPORT_DEBUGF(("client_connect: Error connecting to remote ip/port, %d\n", err));
    goto transport_fail;
  }
   /* Set error callback */
  altcp_err(client->conn, transport_err_cb);
  client->conn_state = TRANSPORT_NOT_CONNECTED;

  return ERR_OK;

transport_fail:
  altcp_abort(client->conn);
  client->conn = NULL;
  return err;
}

static void setup_transport_callbacks(MQTT_Transport_t *client, mqtt_transport_intf_t *trans)
{
  trans->connCtxt = client;
  trans->get_send_len = transport_get_sendlen_cb;
  trans->send_to_network = transport_write_cb;
  trans->get_recv_len = transport_get_recvlen_cb;
  trans->recv_from_network = transport_recv_from_nw;
  trans->close_connection = transport_close_cb;
  trans->timer_start = transport_timerStart;
  trans->timer_stop = transport_timerStop;
}

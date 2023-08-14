/**
 * @file
 * Application layered TCP connection API (to be used from TCPIP thread)\n
 *
 * This file contains the generic API.
 * For more details see @ref altcp_api.
 */

/*
 * Copyright (c) 2017 Simon Goldschmidt
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
 * Author: Simon Goldschmidt <goldsimon@gmx.de>
 *
 */
#ifndef TRANSPORT_HDR_ALTCP_H
#define TRANSPORT_HDR_ALTCP_H

#include "altcp_opt.h"
#if TRANSPORT_ALTCP /* don't build if not configured for use in lwipopts.h */

#include "lwip/tcpbase.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"



struct altcp_pcb;
struct altcp_functions;

typedef err_t (*altcp_accept_fn)(void *arg, struct altcp_pcb *new_conn, err_t err);
typedef err_t (*altcp_connected_fn)(void *arg, struct altcp_pcb *conn, err_t err);
typedef err_t (*altcp_recv_fn)(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err);
typedef err_t (*altcp_sent_fn)(void *arg, struct altcp_pcb *conn, u16_t len);
typedef err_t (*altcp_poll_fn)(void *arg, struct altcp_pcb *conn);
typedef void  (*altcp_err_fn)(void *arg, err_t err);

typedef struct altcp_pcb* (*altcp_new_fn)(void *arg, u8_t ip_type);

struct altcp_pcb {
  const struct altcp_functions *fns;
  struct altcp_pcb *inner_conn;
  void *arg;
  void *state;
  /* application callbacks */
  altcp_accept_fn     accept;
  altcp_connected_fn  connected;
  altcp_recv_fn       recv;
  altcp_sent_fn       sent;
  altcp_poll_fn       poll;
  altcp_err_fn        err;
  u8_t pollinterval;
};

/** @ingroup altcp */
typedef struct altcp_allocator_s {
  /** Allocator function */
  altcp_new_fn  alloc;
  /** Argument to allocator function */
  void         *arg;
} altcp_allocator_t;

//#define LWIP_DBG_ON 0x80U
#define ALTCP_DEBUG
//#define ALTCP_MBEDTLS_DEBUG (LWIP_DBG_ON)

#ifndef TRANSPORT_UNUSED_ARG
#define TRANSPORT_UNUSED_ARG(x) (void)x
#endif /* TRANSPORT_UNUSED_ARG */
#define TRANSPORT_MIN(x , y)  (((x) < (y)) ? (x) : (y))
#define TRANSPORT_PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); fflush(NULL); abort();} while(0)
#define TRANSPORT_ASSERT(message, assertion) do { if (!(assertion)) { \
  TRANSPORT_PLATFORM_ASSERT(message); }} while(0)
#ifdef TRANSPORT_DEBUG
#include "silabs_utils.h"
void silabsLog(const char * aFormat, ...);
#define TRANSPORT_DEBUGF(x) silabsLog x;
#else
#define TRANSPORT_DEBUGF(x)
#endif
#define TRANSPORT_ERROR(message, expression, handler) do { if (!(expression)) { \
  printf("Assertion \"%s\" failed at line %d in %s\n", message, __LINE__, __FILE__); \
  fflush(NULL);handler;} } while(0)

struct altcp_pcb *altcp_new(altcp_allocator_t *allocator);
struct altcp_pcb *altcp_new_ip6(altcp_allocator_t *allocator);
struct altcp_pcb *altcp_new_ip_type(altcp_allocator_t *allocator, u8_t ip_type);

void altcp_arg(struct altcp_pcb *conn, void *arg);
void altcp_accept(struct altcp_pcb *conn, altcp_accept_fn accept);
void altcp_recv(struct altcp_pcb *conn, altcp_recv_fn recv);
void altcp_sent(struct altcp_pcb *conn, altcp_sent_fn sent);
void altcp_poll(struct altcp_pcb *conn, altcp_poll_fn poll, u8_t interval);
void altcp_err(struct altcp_pcb *conn, altcp_err_fn err);

void  altcp_recved(struct altcp_pcb *conn, u16_t len);
err_t altcp_bind(struct altcp_pcb *conn, const ip_addr_t *ipaddr, u16_t port);
err_t altcp_connect(struct altcp_pcb *conn, const ip_addr_t *ipaddr, u16_t port, altcp_connected_fn connected);

/* return conn for source code compatibility to tcp callback API only */
struct altcp_pcb *altcp_listen_with_backlog_and_err(struct altcp_pcb *conn, u8_t backlog, err_t *err);
#define altcp_listen_with_backlog(conn, backlog) altcp_listen_with_backlog_and_err(conn, backlog, NULL)
/** @ingroup altcp */
#define altcp_listen(conn) altcp_listen_with_backlog_and_err(conn, TCP_DEFAULT_LISTEN_BACKLOG, NULL)

void altcp_abort(struct altcp_pcb *conn);
err_t altcp_close(struct altcp_pcb *conn);
err_t altcp_shutdown(struct altcp_pcb *conn, int shut_rx, int shut_tx);

err_t altcp_write(struct altcp_pcb *conn, const void *dataptr, u16_t len, u8_t apiflags);
err_t altcp_output(struct altcp_pcb *conn);

u16_t altcp_mss(struct altcp_pcb *conn);
u16_t altcp_sndbuf(struct altcp_pcb *conn);
u16_t altcp_sndqueuelen(struct altcp_pcb *conn);
void  altcp_nagle_disable(struct altcp_pcb *conn);
void  altcp_nagle_enable(struct altcp_pcb *conn);
int   altcp_nagle_disabled(struct altcp_pcb *conn);

void  altcp_setprio(struct altcp_pcb *conn, u8_t prio);

err_t altcp_get_tcp_addrinfo(struct altcp_pcb *conn, int local, ip_addr_t *addr, u16_t *port);
ip_addr_t *altcp_get_ip(struct altcp_pcb *conn, int local);
u16_t altcp_get_port(struct altcp_pcb *conn, int local);

#ifdef ALTCP_DEBUG
enum tcp_state altcp_dbg_get_tcp_state(struct altcp_pcb *conn);
#endif


#else /* TRANSPORT_ALTCP */

/* ALTCP disabled, define everything to link against tcp callback API (e.g. to get a small non-ssl httpd) */

#include "lwip/tcp.h"

#define altcp_accept_fn tcp_accept_fn
#define altcp_connected_fn tcp_connected_fn
#define altcp_recv_fn tcp_recv_fn
#define altcp_sent_fn tcp_sent_fn
#define altcp_poll_fn tcp_poll_fn
#define altcp_err_fn tcp_err_fn

#define altcp_pcb tcp_pcb
#define altcp_tcp_new_ip_type tcp_new_ip_type
#define altcp_tcp_new tcp_new
#define altcp_tcp_new_ip6 tcp_new_ip6

#define altcp_new(allocator) tcp_new()
#define altcp_new_ip6(allocator) tcp_new_ip6()
#define altcp_new_ip_type(allocator, ip_type) tcp_new_ip_type(ip_type)

#define altcp_arg tcp_arg
#define altcp_accept tcp_accept
#define altcp_recv tcp_recv
#define altcp_sent tcp_sent
#define altcp_poll tcp_poll
#define altcp_err tcp_err

#define altcp_recved tcp_recved
#define altcp_bind tcp_bind
#define altcp_connect tcp_connect

#define altcp_listen_with_backlog_and_err tcp_listen_with_backlog_and_err
#define altcp_listen_with_backlog tcp_listen_with_backlog
#define altcp_listen tcp_listen

#define altcp_abort tcp_abort
#define altcp_close tcp_close
#define altcp_shutdown tcp_shutdown

#define altcp_write tcp_write
#define altcp_output tcp_output

#define altcp_mss tcp_mss
#define altcp_sndbuf tcp_sndbuf
#define altcp_sndqueuelen tcp_sndqueuelen
#define altcp_nagle_disable tcp_nagle_disable
#define altcp_nagle_enable tcp_nagle_enable
#define altcp_nagle_disabled tcp_nagle_disabled
#define altcp_setprio tcp_setprio

#define altcp_get_tcp_addrinfo tcp_get_tcp_addrinfo
#define altcp_get_ip(pcb, local) ((local) ? (&(pcb)->local_ip) : (&(pcb)->remote_ip))

#ifdef ALTCP_DEBUG
#define altcp_dbg_get_tcp_state tcp_dbg_get_tcp_state
#endif

#endif /* TRANSPORT_ALTCP */

#endif /* TRANSPORT_HDR_ALTCP_H */
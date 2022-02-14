/**
 * @file
 * lwIP iPerf client/server implementation
 */

/**
 * @defgroup iperf Iperf client/server
 * @ingroup apps
 *
 * This is a simple performance measuring client/server to check your bandwidth using
 * iPerf2 on a PC as client/server.
 *
 * @todo:
 * - protect combined sessions handling (via 'related_master_state') against reallocation
 *   (this is a pointer address, currently, so if the same memory is allocated again,
 *    session pairs (tx/rx) can be confused on reallocation)
 *
 */

/*
 * Copyright (c) 2014 Simon Goldschmidt
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
 * Author: Simon Goldschmidt
 */

#include "lwip/apps/lwiperf.h"

#include "lwip/igmp.h"
#include "lwip/inet.h"
#include "lwip/mld6.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"

#include <string.h>

/* Only use callback API for TCP & UDP */
#if LWIP_CALLBACK_API && LWIP_TCP && LWIP_UDP

/** Specify the idle timeout (in seconds) after that the test fails */
#ifndef LWIPERF_MAX_IDLE_SEC
#define LWIPERF_MAX_IDLE_SEC 10U
#endif
#if LWIPERF_MAX_IDLE_SEC > 255
#error LWIPERF_MAX_IDLE_SEC must fit into an u8_t
#endif

/** Change this if you don't want to lwiperf to listen to any IP version */
#ifndef LWIPERF_SERVER_IP_TYPE
#define LWIPERF_SERVER_IP_TYPE IPADDR_TYPE_ANY
#endif

/* File internal memory allocation (struct lwiperf_*): this defaults to
   the heap */
#ifndef LWIPERF_ALLOC
#define LWIPERF_ALLOC(type) mem_malloc(sizeof(type))
#define LWIPERF_FREE(type, item) mem_free(item)
#endif

/** If this is 1, check that received data has the correct format */
#ifndef LWIPERF_CHECK_RX_DATA
#define LWIPERF_CHECK_RX_DATA 0
#endif

/** The IDs of clocks used by clock_gettime() */
#define CLOCK_MONOTONIC 1

/** The resolution of the clock in microseconds */
#ifdef FSL_RTOS_FREE_RTOS
#include "portmacro.h"
#define CLOCK_RESOLUTION_US (portTICK_PERIOD_MS * 1000u)
#else
#define CLOCK_RESOLUTION_US 1000u
#endif

// Mw320 Matter
#ifndef LWIP_TIMEVAL_PRIVATE
#define LWIP_TIMEVAL_PRIVATE 1
#endif

#if LWIP_TIMEVAL_PRIVATE
/** Added for compatibility */
struct timespec
{
    long tv_sec;
    long tv_nsec;
};
#endif

/** This is the Iperf settings struct sent from the client */
typedef struct _lwiperf_settings
{
#define LWIPERF_FLAGS_ANSWER_TEST 0x80000000
#define LWIPERF_FLAGS_EXTEND 0x40000000
#define LWIPERF_FLAGS_ANSWER_NOW 0x00000001
    u32_t flags;
    u32_t num_threads; /* unused for now */
    u32_t remote_port;
    u32_t buffer_len; /* unused for now */
    u32_t win_band;   /* TCP window / UDP rate */
    u32_t amount;     /* pos. value: bytes?; neg. values: time (unit is 10ms: 1/100 second) */
} lwiperf_settings_t;

typedef struct _lwiperf_settings_ext
{
#define LWIPERF_EFLAGS_UNITS_PPS 0x00000001
    lwiperf_settings_t base;
    s32_t type;
    s32_t len; /* length from flags to real_time */
    s32_t eflags;
    s32_t version_u;
    s32_t version_l;
    s32_t reserved;
    s32_t rate;
    s32_t UDP_rate_units;
    s32_t real_time;
} lwiperf_settings_ext_t;

/** This is the header structure for all UDP datagram */
struct UDP_datagram
{
    int32_t id;
    uint32_t tv_sec;
    uint32_t tv_usec;
};

/** This is the Iperf reporting struct sent to the client */
typedef struct _lwiperf_udp_report
{
    int32_t flags; /* LWIPERF_FLAGS_* */
    int32_t total_len1;
    int32_t total_len2;
    int32_t stop_sec;
    int32_t stop_usec;
    int32_t error_cnt;
    int32_t outorder_cnt;
    int32_t datagrams;
    int32_t jitter1;
    int32_t jitter2;
} lwiperf_udp_report_t;

/** Basic connection handle */
struct _lwiperf_state_base;
typedef struct _lwiperf_state_base lwiperf_state_base_t;
struct _lwiperf_state_base
{
    /* linked list */
    lwiperf_state_base_t * next;
    /* 1=tcp, 0=udp */
    u8_t tcp;
    /* 1=server, 0=client */
    u8_t server;
    /* master state used to abort sessions (e.g. listener, main client) */
    lwiperf_state_base_t * related_master_state;
};

/** Connection handle for a UDP iperf session */
typedef struct _lwiperf_state_udp
{
    lwiperf_state_base_t base;
    struct udp_pcb * pcb;
    struct pbuf * reported;
    ip_addr_t remote_addr;
    u16_t remote_port;
    u8_t report_count;
    u8_t have_settings_buf;
    lwiperf_settings_ext_t settings;
    u32_t delay_target;
    u32_t frames_per_delay;
    u32_t time_started;
    u64_t bytes_transferred;
    lwiperf_report_fn report_fn;
    void * report_arg;
    struct timespec udp_lastpkt;
    u32_t udp_seq;
    u32_t udp_rx_lost;
    u32_t udp_rx_outorder;
    u32_t udp_rx_total_pkt;
    u32_t udp_rx_total_size;
    u32_t udp_last_transit;
    long jitter;
} lwiperf_state_udp_t;

/** Connection handle for a TCP iperf session */
typedef struct _lwiperf_state_tcp
{
    lwiperf_state_base_t base;
    struct tcp_pcb * server_pcb;
    struct tcp_pcb * conn_pcb;
    u32_t time_started;
    lwiperf_report_fn report_fn;
    void * report_arg;
    u8_t poll_count;
    u8_t next_num;
    /* 1=start server when client is closed */
    u8_t client_tradeoff_mode;
    u64_t bytes_transferred;
    lwiperf_settings_t settings;
    u8_t have_settings_buf;
    u8_t specific_remote;
    ip_addr_t remote_addr;
} lwiperf_state_tcp_t;

/** List of active iperf sessions */
static lwiperf_state_base_t * lwiperf_all_connections;

/** A const buffer to send from: we want to measure sending, not copying! */
static const u8_t lwiperf_txbuf_const[1600] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};

static err_t lwiperf_tcp_poll(void * arg, struct tcp_pcb * tpcb);
static void lwiperf_tcp_err(void * arg, err_t err);
static err_t lwiperf_start_tcp_server_impl(const ip_addr_t * local_addr, u16_t local_port, lwiperf_report_fn report_fn,
                                           void * report_arg, lwiperf_state_base_t * related_master_state,
                                           lwiperf_state_tcp_t ** state);
static void lwiperf_udp_recv(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port);
static err_t lwiperf_udp_tx_start(lwiperf_state_udp_t * conn);

/** Get time roughly derived from lwIP's sys_now(), ms resolution clock */
static int clock_gettime(int clk_id, struct timespec * tp)
{
    u32_t now = sys_now();

    tp->tv_sec  = now / 1000;
    tp->tv_nsec = (now % 1000) * 1000000;

    return 0;
}

static inline void diff_ts(const struct timespec * start, const struct timespec * stop, struct timespec * result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0)
    {
        result->tv_sec  = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    }
    else
    {
        result->tv_sec  = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}

/** Add an iperf session to the 'active' list */
static void lwiperf_list_add(lwiperf_state_base_t * item)
{
    item->next              = lwiperf_all_connections;
    lwiperf_all_connections = item;
}

/** Remove an iperf session from the 'active' list */
static void lwiperf_list_remove(lwiperf_state_base_t * item)
{
    lwiperf_state_base_t * prev = NULL;
    lwiperf_state_base_t * iter;
    for (iter = lwiperf_all_connections; iter != NULL; prev = iter, iter = iter->next)
    {
        if (iter == item)
        {
            if (prev == NULL)
            {
                lwiperf_all_connections = iter->next;
            }
            else
            {
                prev->next = iter->next;
            }
            /* @debug: ensure this item is listed only once */
            for (iter = iter->next; iter != NULL; iter = iter->next)
            {
                LWIP_ASSERT("duplicate entry", iter != item);
            }
            break;
        }
    }
}

static lwiperf_state_base_t * lwiperf_list_find(lwiperf_state_base_t * item)
{
    lwiperf_state_base_t * iter;
    for (iter = lwiperf_all_connections; iter != NULL; iter = iter->next)
    {
        if (iter == item)
        {
            return item;
        }
    }
    return NULL;
}

/** Call the report function of an iperf tcp session */
static void lwip_tcp_conn_report(lwiperf_state_tcp_t * conn, enum lwiperf_report_type report_type)
{
    if ((conn != NULL) && (conn->report_fn != NULL))
    {
        u32_t now, duration_ms, bandwidth_kbitpsec;
        now         = sys_now();
        duration_ms = now - conn->time_started;
        if (duration_ms == 0)
        {
            bandwidth_kbitpsec = 0;
        }
        else
        {
            bandwidth_kbitpsec = (u32_t)((conn->bytes_transferred * 8U) / duration_ms);
        }
        if (conn->conn_pcb)
            conn->report_fn(conn->report_arg, report_type, &conn->conn_pcb->local_ip, conn->conn_pcb->local_port,
                            &conn->conn_pcb->remote_ip, conn->conn_pcb->remote_port, conn->bytes_transferred, duration_ms,
                            bandwidth_kbitpsec);
        else if (conn->server_pcb)
            conn->report_fn(conn->report_arg, report_type, &conn->server_pcb->local_ip, conn->server_pcb->local_port,
                            &conn->server_pcb->remote_ip, conn->server_pcb->remote_port, conn->bytes_transferred, duration_ms,
                            bandwidth_kbitpsec);
        else
            conn->report_fn(conn->report_arg, report_type, NULL, 0, NULL, 0, 0, 0, 0);
    }
}

/** Close an iperf tcp session */
static void lwiperf_tcp_close(lwiperf_state_tcp_t * conn, enum lwiperf_report_type report_type)
{
    err_t err;

    lwiperf_list_remove(&conn->base);
    lwip_tcp_conn_report(conn, report_type);
    if (conn->conn_pcb != NULL)
    {
        tcp_arg(conn->conn_pcb, NULL);
        tcp_poll(conn->conn_pcb, NULL, 0);
        tcp_sent(conn->conn_pcb, NULL);
        tcp_recv(conn->conn_pcb, NULL);
        tcp_err(conn->conn_pcb, NULL);
        err = tcp_close(conn->conn_pcb);
        if (err != ERR_OK)
        {
            /* don't want to wait for free memory here... */
            tcp_abort(conn->conn_pcb);
        }
    }
    else if (conn->server_pcb)
    {
        /* no conn pcb, this is the listener pcb */
        err = tcp_close(conn->server_pcb);
        LWIP_ASSERT("error", err == ERR_OK);
    }
    LWIPERF_FREE(lwiperf_state_tcp_t, conn);
}

/** Try to send more data on an iperf tcp session */
static err_t lwiperf_tcp_client_send_more(lwiperf_state_tcp_t * conn)
{
    int send_more;
    err_t err;
    u16_t txlen;
    u16_t txlen_max;
    void * txptr;
    u8_t apiflags;

    LWIP_ASSERT("conn invalid", (conn != NULL) && conn->base.tcp && (conn->base.server == 0));

    do
    {
        send_more = 0;
        if (conn->settings.amount & PP_HTONL(0x80000000))
        {
            /* this session is time-limited */
            u32_t now     = sys_now();
            u32_t diff_ms = now - conn->time_started;
            u32_t time    = (u32_t) - (s32_t) lwip_ntohl(conn->settings.amount);
            u32_t time_ms = time * 10;
            if (diff_ms >= time_ms)
            {
                /* time specified by the client is over -> close the connection */
                lwiperf_tcp_close(conn, LWIPERF_TCP_DONE_CLIENT);
                return ERR_OK;
            }
        }
        else
        {
            /* this session is byte-limited */
            u32_t amount_bytes = lwip_ntohl(conn->settings.amount);
            /* @todo: this can send up to 1*MSS more than requested... */
            if (amount_bytes >= conn->bytes_transferred)
            {
                /* all requested bytes transferred -> close the connection */
                lwiperf_tcp_close(conn, LWIPERF_TCP_DONE_CLIENT);
                return ERR_OK;
            }
        }

        if (conn->bytes_transferred < 24)
        {
            /* transmit the settings a first time */
            txptr     = &((u8_t *) &conn->settings)[conn->bytes_transferred];
            txlen_max = (u16_t)(24 - conn->bytes_transferred);
            apiflags  = TCP_WRITE_FLAG_COPY;
        }
        else if (conn->bytes_transferred < 48)
        {
            /* transmit the settings a second time */
            txptr     = &((u8_t *) &conn->settings)[conn->bytes_transferred - 24];
            txlen_max = (u16_t)(48 - conn->bytes_transferred);
            apiflags  = TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE;
            send_more = 1;
        }
        else
        {
            /* transmit data */
            /* @todo: every x bytes, transmit the settings again */
            txptr     = LWIP_CONST_CAST(void *, &lwiperf_txbuf_const[conn->bytes_transferred % 10]);
            txlen_max = TCP_MSS;
            if (conn->bytes_transferred == 48)
            { /* @todo: fix this for intermediate settings, too */
                txlen_max = TCP_MSS - 24;
            }
            apiflags  = 0; /* no copying needed */
            send_more = 1;
        }
        txlen = txlen_max;
        do
        {
            err = tcp_write(conn->conn_pcb, txptr, txlen, apiflags);
            if (err == ERR_MEM)
            {
                txlen /= 2;
            }
        } while ((err == ERR_MEM) && (txlen >= (TCP_MSS / 2)));

        if (err == ERR_OK)
        {
            conn->bytes_transferred += txlen;
        }
        else
        {
            send_more = 0;
        }
    } while (send_more);

    tcp_output(conn->conn_pcb);
    return ERR_OK;
}

/** TCP sent callback, try to send more data */
static err_t lwiperf_tcp_client_sent(void * arg, struct tcp_pcb * tpcb, u16_t len)
{
    lwiperf_state_tcp_t * conn = (lwiperf_state_tcp_t *) arg;
    /* @todo: check 'len' (e.g. to time ACK of all data)? for now, we just send more... */
    LWIP_ASSERT("invalid conn", conn->conn_pcb == tpcb);
    LWIP_UNUSED_ARG(tpcb);
    LWIP_UNUSED_ARG(len);

    conn->poll_count = 0;

    return lwiperf_tcp_client_send_more(conn);
}

/** TCP connected callback (active connection), send data now */
static err_t lwiperf_tcp_client_connected(void * arg, struct tcp_pcb * tpcb, err_t err)
{
    lwiperf_state_tcp_t * conn = (lwiperf_state_tcp_t *) arg;
    LWIP_ASSERT("invalid conn", conn->conn_pcb == tpcb);
    LWIP_UNUSED_ARG(tpcb);
    if (err != ERR_OK)
    {
        lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_REMOTE);
        return ERR_OK;
    }
    conn->poll_count   = 0;
    conn->time_started = sys_now();
    return lwiperf_tcp_client_send_more(conn);
}

/** Start TCP connection back to the client (either parallel or after the
 * receive test has finished.
 */
static err_t lwiperf_tx_start_impl(const ip_addr_t * remote_ip, u16_t remote_port, lwiperf_settings_t * settings,
                                   lwiperf_report_fn report_fn, void * report_arg, lwiperf_state_base_t * related_master_state,
                                   lwiperf_state_tcp_t ** new_conn)
{
    err_t err;
    lwiperf_state_tcp_t * client_conn;
    struct tcp_pcb * newpcb;
    ip_addr_t remote_addr;

    LWIP_ASSERT("remote_ip != NULL", remote_ip != NULL);
    LWIP_ASSERT("remote_ip != NULL", settings != NULL);
    LWIP_ASSERT("new_conn != NULL", new_conn != NULL);
    *new_conn = NULL;

    client_conn = (lwiperf_state_tcp_t *) LWIPERF_ALLOC(lwiperf_state_tcp_t);
    if (client_conn == NULL)
    {
        return ERR_MEM;
    }
    newpcb = tcp_new_ip_type(IP_GET_TYPE(remote_ip));
    if (newpcb == NULL)
    {
        LWIPERF_FREE(lwiperf_state_tcp_t, client_conn);
        return ERR_MEM;
    }
    memset(client_conn, 0, sizeof(lwiperf_state_tcp_t));
    client_conn->base.tcp                  = 1;
    client_conn->base.related_master_state = related_master_state;
    client_conn->conn_pcb                  = newpcb;
    client_conn->time_started              = sys_now(); /* @todo: set this again on 'connected' */
    client_conn->report_fn                 = report_fn;
    client_conn->report_arg                = report_arg;
    client_conn->next_num                  = 4; /* initial nr is '4' since the header has 24 byte */
    client_conn->bytes_transferred         = 0;
    memcpy(&client_conn->settings, settings, sizeof(*settings));
    client_conn->have_settings_buf = 1;

    tcp_arg(newpcb, client_conn);
    tcp_sent(newpcb, lwiperf_tcp_client_sent);
    tcp_poll(newpcb, lwiperf_tcp_poll, 2U);
    tcp_err(newpcb, lwiperf_tcp_err);

    ip_addr_copy(remote_addr, *remote_ip);

    err = tcp_connect(newpcb, &remote_addr, remote_port, lwiperf_tcp_client_connected);
    if (err != ERR_OK)
    {
        lwiperf_tcp_close(client_conn, LWIPERF_TCP_ABORTED_LOCAL);
        return err;
    }
    lwiperf_list_add(&client_conn->base);
    *new_conn = client_conn;
    return ERR_OK;
}

static err_t lwiperf_tx_start_passive(lwiperf_state_tcp_t * conn)
{
    err_t ret;
    lwiperf_state_tcp_t * new_conn = NULL;
    u16_t remote_port              = (u16_t) lwip_htonl(conn->settings.remote_port);

    ret = lwiperf_tx_start_impl(&conn->conn_pcb->remote_ip, remote_port, &conn->settings, conn->report_fn, conn->report_arg,
                                conn->base.related_master_state, &new_conn);
    if (ret == ERR_OK)
    {
        LWIP_ASSERT("new_conn != NULL", new_conn != NULL);
        new_conn->settings.flags = 0; /* prevent the remote side starting back as client again */
    }
    return ret;
}

/** Receive data on an iperf tcp session */
static err_t lwiperf_tcp_recv(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err)
{
    u8_t tmp;
    u16_t tot_len;
    u32_t packet_idx;
    struct pbuf * q;
    lwiperf_state_tcp_t * conn = (lwiperf_state_tcp_t *) arg;

    LWIP_ASSERT("pcb mismatch", conn->conn_pcb == tpcb);
    LWIP_UNUSED_ARG(tpcb);

    if (err != ERR_OK)
    {
        lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_REMOTE);
        return ERR_OK;
    }
    if (p == NULL)
    {
        /* connection closed -> test done */
        if (conn->settings.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST))
        {
            if ((conn->settings.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_NOW)) == 0)
            {
                /* client requested transmission after end of test */
                LWIP_PLATFORM_DIAG(("client requested transmission after end of test\n"));
                lwiperf_tx_start_passive(conn);
            }
        }
        lwiperf_tcp_close(conn, LWIPERF_TCP_DONE_SERVER);
        return ERR_OK;
    }
    tot_len = p->tot_len;

    conn->poll_count = 0;

    if ((!conn->have_settings_buf) || ((conn->bytes_transferred - 24) % (1024 * 128) == 0))
    {
        /* wait for 24-byte header */
        if (p->tot_len < sizeof(lwiperf_settings_t))
        {
            lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_DATAERROR);
            pbuf_free(p);
            return ERR_OK;
        }
        if (!conn->have_settings_buf)
        {
            if (pbuf_copy_partial(p, &conn->settings, sizeof(lwiperf_settings_t), 0) != sizeof(lwiperf_settings_t))
            {
                lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL);
                pbuf_free(p);
                return ERR_OK;
            }
            conn->have_settings_buf = 1;
            LWIP_PLATFORM_DIAG(("New TCP client (settings flags 0x%x)\n", lwip_ntohl(conn->settings.flags)));
            if (conn->settings.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST))
            {
                if (conn->settings.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_NOW))
                {
                    /* client requested parallel transmission test */
                    LWIP_PLATFORM_DIAG(("client requested parallel transmission test\n"));
                    err_t err2 = lwiperf_tx_start_passive(conn);
                    if (err2 != ERR_OK)
                    {
                        lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_TXERROR);
                        pbuf_free(p);
                        return ERR_OK;
                    }
                }
            }
        }
        else
        {
            if (conn->settings.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST))
            {
                if (pbuf_memcmp(p, 0, &conn->settings, sizeof(lwiperf_settings_t)) != 0)
                {
                    lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_DATAERROR);
                    pbuf_free(p);
                    return ERR_OK;
                }
            }
        }
        conn->bytes_transferred += sizeof(lwiperf_settings_t);
        if (conn->bytes_transferred <= 24)
        {
            conn->time_started = sys_now();
            tcp_recved(tpcb, p->tot_len);
            pbuf_free(p);
            return ERR_OK;
        }
        conn->next_num = 4; /* 24 bytes received... */
        tmp            = pbuf_remove_header(p, 24);
        LWIP_ASSERT("pbuf_remove_header failed", tmp == 0);
        LWIP_UNUSED_ARG(tmp); /* for LWIP_NOASSERT */
    }

    packet_idx = 0;
    for (q = p; q != NULL; q = q->next)
    {
#if LWIPERF_CHECK_RX_DATA
        const u8_t * payload = (const u8_t *) q->payload;
        u16_t i;
        for (i = 0; i < q->len; i++)
        {
            u8_t val = payload[i];
            u8_t num = val - '0';
            if (num == conn->next_num)
            {
                conn->next_num++;
                if (conn->next_num == 10)
                {
                    conn->next_num = 0;
                }
            }
            else
            {
                lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL_DATAERROR);
                pbuf_free(p);
                return ERR_OK;
            }
        }
#endif
        packet_idx += q->len;
    }
    LWIP_ASSERT("count mismatch", packet_idx == p->tot_len);
    conn->bytes_transferred += packet_idx;
    tcp_recved(tpcb, tot_len);
    pbuf_free(p);
    return ERR_OK;
}

/** Error callback, iperf tcp session aborted */
static void lwiperf_tcp_err(void * arg, err_t err)
{
    lwiperf_state_tcp_t * conn = (lwiperf_state_tcp_t *) arg;
    LWIP_UNUSED_ARG(err);
    /* lwiperf_tcp_close MUST not close itself conn_pcb */
    conn->conn_pcb = NULL;
    lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_REMOTE);
}

/** TCP poll callback, try to send more data */
static err_t lwiperf_tcp_poll(void * arg, struct tcp_pcb * tpcb)
{
    lwiperf_state_tcp_t * conn = (lwiperf_state_tcp_t *) arg;
    LWIP_ASSERT("pcb mismatch", conn->conn_pcb == tpcb);
    LWIP_UNUSED_ARG(tpcb);
    if (++conn->poll_count >= LWIPERF_MAX_IDLE_SEC)
    {
        lwiperf_tcp_close(conn, LWIPERF_TCP_ABORTED_LOCAL);
        return ERR_OK; /* lwiperf_tcp_close frees conn */
    }

    if (!conn->base.server)
    {
        lwiperf_tcp_client_send_more(conn);
    }

    return ERR_OK;
}

/** This is called when a new client connects for an iperf tcp session */
static err_t lwiperf_tcp_accept(void * arg, struct tcp_pcb * newpcb, err_t err)
{
    lwiperf_state_tcp_t *s, *conn;
    if ((err != ERR_OK) || (newpcb == NULL) || (arg == NULL))
    {
        return ERR_VAL;
    }

    s = (lwiperf_state_tcp_t *) arg;
    LWIP_ASSERT("invalid session", s->base.server);
    LWIP_ASSERT("invalid listen pcb", s->server_pcb != NULL);
    LWIP_ASSERT("invalid conn pcb", s->conn_pcb == NULL);
    if (s->specific_remote)
    {
        LWIP_ASSERT("s->base.related_master_state != NULL", s->base.related_master_state != NULL);
        if (!ip_addr_eq(&newpcb->remote_ip, &s->remote_addr))
        {
            /* this listener belongs to a client session, and this is not the correct remote */
            return ERR_VAL;
        }
    }
    else
    {
        LWIP_ASSERT("s->base.related_master_state == NULL", s->base.related_master_state == NULL);
    }

    conn = (lwiperf_state_tcp_t *) LWIPERF_ALLOC(lwiperf_state_tcp_t);
    if (conn == NULL)
    {
        return ERR_MEM;
    }
    memset(conn, 0, sizeof(lwiperf_state_tcp_t));
    conn->base.tcp                  = 1;
    conn->base.server               = 1;
    conn->base.related_master_state = &s->base;
    conn->conn_pcb                  = newpcb;
    conn->time_started              = sys_now();
    conn->report_fn                 = s->report_fn;
    conn->report_arg                = s->report_arg;

    /* setup the tcp rx connection */
    tcp_arg(newpcb, conn);
    tcp_recv(newpcb, lwiperf_tcp_recv);
    tcp_poll(newpcb, lwiperf_tcp_poll, 2U);
    tcp_err(newpcb, lwiperf_tcp_err);

    if (s->specific_remote)
    {
        /* this listener belongs to a client, so make the client the master of the newly created connection */
        conn->base.related_master_state = s->base.related_master_state;
        /* if dual mode or (tradeoff mode AND client is done): close the listener */
        if (!s->client_tradeoff_mode || !lwiperf_list_find(s->base.related_master_state))
        {
            /* prevent report when closing: this is expected */
            s->report_fn = NULL;
            lwiperf_tcp_close(s, LWIPERF_TCP_ABORTED_LOCAL);
        }
    }
    lwiperf_list_add(&conn->base);
    return ERR_OK;
}

/**
 * @ingroup iperf
 * Start a TCP iperf server on the default TCP port (5001) and listen for
 * incoming connections from iperf clients.
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void * lwiperf_start_tcp_server_default(lwiperf_report_fn report_fn, void * report_arg)
{
    return lwiperf_start_tcp_server(IP_ADDR_ANY, LWIPERF_TCP_PORT_DEFAULT, report_fn, report_arg);
}

/**
 * @ingroup iperf
 * Start a TCP iperf server on a specific IP address and port and listen for
 * incoming connections from iperf clients.
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void * lwiperf_start_tcp_server(const ip_addr_t * local_addr, u16_t local_port, lwiperf_report_fn report_fn, void * report_arg)
{
    err_t err;
    lwiperf_state_tcp_t * state = NULL;

    err = lwiperf_start_tcp_server_impl(local_addr, local_port, report_fn, report_arg, NULL, &state);
    if (err == ERR_OK)
    {
        return state;
    }
    return NULL;
}

static err_t lwiperf_start_tcp_server_impl(const ip_addr_t * local_addr, u16_t local_port, lwiperf_report_fn report_fn,
                                           void * report_arg, lwiperf_state_base_t * related_master_state,
                                           lwiperf_state_tcp_t ** state)
{
    err_t err;
    struct tcp_pcb * pcb;
    lwiperf_state_tcp_t * s;

    LWIP_ASSERT_CORE_LOCKED();

    LWIP_ASSERT("state != NULL", state != NULL);

    if (local_addr == NULL)
    {
        return ERR_ARG;
    }

    s = (lwiperf_state_tcp_t *) LWIPERF_ALLOC(lwiperf_state_tcp_t);
    if (s == NULL)
    {
        return ERR_MEM;
    }
    memset(s, 0, sizeof(lwiperf_state_tcp_t));
    s->base.tcp                  = 1;
    s->base.server               = 1;
    s->base.related_master_state = related_master_state;
    s->report_fn                 = report_fn;
    s->report_arg                = report_arg;

    pcb = tcp_new_ip_type(LWIPERF_SERVER_IP_TYPE);
    if (pcb == NULL)
    {
        return ERR_MEM;
    }
    err = tcp_bind(pcb, local_addr, local_port);
    if (err != ERR_OK)
    {
        return err;
    }
    s->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (s->server_pcb == NULL)
    {
        if (pcb != NULL)
        {
            tcp_close(pcb);
        }
        LWIPERF_FREE(lwiperf_state_tcp_t, s);
        return ERR_MEM;
    }
    pcb = NULL;

    tcp_arg(s->server_pcb, s);
    tcp_accept(s->server_pcb, lwiperf_tcp_accept);

    lwiperf_list_add(&s->base);
    *state = s;
    return ERR_OK;
}

/**
 * @ingroup iperf
 * Start a TCP iperf client to the default TCP port (5001).
 *
 * @returns a connection handle that can be used to abort the client
 *          by calling @ref lwiperf_abort()
 */
void * lwiperf_start_tcp_client_default(const ip_addr_t * remote_addr, lwiperf_report_fn report_fn, void * report_arg)
{
    return lwiperf_start_tcp_client(remote_addr, LWIPERF_TCP_PORT_DEFAULT, LWIPERF_CLIENT, -1000, report_fn, report_arg);
}

/**
 * @ingroup iperf
 * Start a TCP iperf client to a specific IP address and port.
 *
 * @returns a connection handle that can be used to abort the client
 *          by calling @ref lwiperf_abort()
 */
void * lwiperf_start_tcp_client(const ip_addr_t * remote_addr, u16_t remote_port, enum lwiperf_client_type type, int amount,
                                lwiperf_report_fn report_fn, void * report_arg)
{
    err_t ret;
    lwiperf_settings_t settings;
    lwiperf_state_tcp_t * state = NULL;

    memset(&settings, 0, sizeof(settings));
    switch (type)
    {
    case LWIPERF_CLIENT:
        /* Unidirectional tx only test */
        settings.flags = 0;
        break;
    case LWIPERF_DUAL:
        /* Do a bidirectional test simultaneously */
        settings.flags = htonl(LWIPERF_FLAGS_ANSWER_TEST | LWIPERF_FLAGS_ANSWER_NOW);
        break;
    case LWIPERF_TRADEOFF:
        /* Do a bidirectional test individually */
        settings.flags = htonl(LWIPERF_FLAGS_ANSWER_TEST);
        break;
    default:
        /* invalid argument */
        return NULL;
    }
    settings.num_threads = htonl(1);
    settings.remote_port = htonl(LWIPERF_TCP_PORT_DEFAULT);
    settings.amount      = htonl((u32_t) amount);

    ret = lwiperf_tx_start_impl(remote_addr, remote_port, &settings, report_fn, report_arg, NULL, &state);
    if (ret == ERR_OK)
    {
        LWIP_ASSERT("state != NULL", state != NULL);
        if (type != LWIPERF_CLIENT)
        {
            /* start corresponding server now */
            lwiperf_state_tcp_t * server = NULL;
            ret = lwiperf_start_tcp_server_impl(&state->conn_pcb->local_ip, LWIPERF_TCP_PORT_DEFAULT, report_fn, report_arg,
                                                (lwiperf_state_base_t *) state, &server);
            if (ret != ERR_OK)
            {
                /* starting server failed, abort client */
                lwiperf_abort(state);
                return NULL;
            }
            /* make this server accept one connection only */
            server->specific_remote = 1;
            server->remote_addr     = state->conn_pcb->remote_ip;
            if (type == LWIPERF_TRADEOFF)
            {
                /* tradeoff means that the remote host connects only after the client is done,
                   so keep the listen pcb open until the client is done */
                server->client_tradeoff_mode = 1;
            }
        }
        return state;
    }
    return NULL;
}

/** This is called when a new client connects for an iperf udp session */
static lwiperf_state_udp_t * lwiperf_udp_new_client(lwiperf_state_udp_t * s)
{
    lwiperf_state_udp_t * conn;
    if (s == NULL)
        return NULL;
    conn = (lwiperf_state_udp_t *) LWIPERF_ALLOC(lwiperf_state_udp_t);
    if (conn == NULL)
        return NULL;
    memset(conn, 0, sizeof(lwiperf_state_udp_t));
    conn->base.tcp                  = 0;
    conn->base.server               = 1;
    conn->base.related_master_state = &s->base;
    conn->pcb                       = NULL;
    conn->time_started              = sys_now();
    conn->report_fn                 = s->report_fn;
    conn->report_arg                = s->report_arg;
    lwiperf_list_add(&conn->base);
    return conn;
}

/** This is called when a new client connects for an iperf udp session */
static lwiperf_state_udp_t * lwiperf_udp_search_client(lwiperf_state_udp_t * s, const ip_addr_t * addr, u16_t port)
{
    lwiperf_state_udp_t * iter;
    for (iter = (lwiperf_state_udp_t *) lwiperf_all_connections; iter != NULL; iter = (lwiperf_state_udp_t *) iter->base.next)
    {
        if (iter->base.tcp || !iter->base.related_master_state)
            continue;
        if (iter->base.related_master_state != (lwiperf_state_base_t *) s)
            continue;
        if (ip_addr_eq(addr, &iter->remote_addr) && (port == iter->remote_port))
            break;
    }
    return iter;
}

/** Call the report function of an iperf tcp session */
static void lwip_udp_conn_report(lwiperf_state_udp_t * conn, enum lwiperf_report_type report_type)
{
    if ((conn != NULL) && (conn->report_fn != NULL))
    {
        u32_t now, duration_ms, bandwidth_kbitpsec;
        now         = sys_now();
        duration_ms = now - conn->time_started;
        if (duration_ms == 0)
        {
            bandwidth_kbitpsec = 0;
        }
        else
        {
            bandwidth_kbitpsec = (u32_t)((8ull * conn->bytes_transferred) / duration_ms);
        }

        ip_addr_t * local_ip = NULL;
        u16_t local_port     = 0u;

        if (conn->pcb != NULL)
        {
            local_ip   = &(conn->pcb->local_ip);
            local_port = conn->pcb->local_port;
        }
        else if (conn->base.related_master_state != NULL)
        {
            /* conn->pcb is NULL for incoming UDP clients, try to take local IP
             * and port number from server pcb stored in related master state
             */
            lwiperf_state_udp_t * s = (lwiperf_state_udp_t *) conn->base.related_master_state;
            if (s->pcb != NULL)
            {
                local_ip   = &(s->pcb->local_ip);
                local_port = s->pcb->local_port;
            }
        }

        conn->report_fn(conn->report_arg, report_type, local_ip, local_port, &conn->remote_addr, conn->remote_port,
                        conn->bytes_transferred, duration_ms, bandwidth_kbitpsec);
    }
}

/** Close an iperf udp session */
static void lwiperf_udp_close(lwiperf_state_udp_t * conn, enum lwiperf_report_type report_type)
{
    lwip_udp_conn_report(conn, report_type);
    lwiperf_list_remove(&conn->base);
    if (conn->pcb != NULL)
    {
        ip_addr_t * local_addr = &conn->pcb->local_ip;
        if (ip_addr_ismulticast(local_addr))
        {
            if (IP_IS_V6(local_addr))
            {
#if LWIP_IPV6_MLD
                mld6_leavegroup(IP6_ADDR_ANY6, ip_2_ip6(local_addr));
#endif
            }
            else
            {
#if LWIP_IGMP
                igmp_leavegroup(IP4_ADDR_ANY4, ip_2_ip4(local_addr));
#endif
            }
        }
        udp_remove(conn->pcb);
        conn->pcb = NULL;
    }
    LWIPERF_FREE(lwiperf_state_udp_t, conn);
}

/** Receive data on an iperf udp session */
static void lwiperf_udp_send_report(lwiperf_state_udp_t * conn)
{
    lwiperf_state_udp_t * s = (lwiperf_state_udp_t *) conn->base.related_master_state;
    struct pbuf * q         = conn->reported;
    LWIP_ASSERT("no report buffer!", q != NULL);
    udp_sendto(s->pcb, q, &conn->remote_addr, conn->remote_port);
    conn->report_count++;
    if (conn->report_count < 2)
    {
        /* Send twice after a little delay! */
        sys_timeout(10, (sys_timeout_handler) lwiperf_udp_send_report, conn);
    }
    else
    {
        pbuf_free(q);
        conn->reported = NULL;
        /* reported twice -> test done */
        if (conn->settings.base.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST))
        {
            if ((conn->settings.base.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_NOW)) == 0)
            {
                /* client requested transmission after end of test */
                LWIP_PLATFORM_DIAG(("client requested transmission after end of test\n"));
                lwiperf_udp_tx_start(conn);
            }
        }
        lwiperf_udp_close(conn, LWIPERF_UDP_DONE_SERVER);
        if (s->base.server & 0x80)
        {
            /* this is a temporary server for tradeoff or dualtest, ensure no report for this temporary server */
            s->report_fn = NULL;
            lwiperf_udp_close(s, LWIPERF_UDP_DONE_SERVER);
        }
    }
}

static void lwiperf_udp_set_client_rate(lwiperf_state_udp_t * c, s32_t rate, u32_t buf_len)
{
    /* compute delay for bandwidth restriction, constrained to [0,1]s in microseconds */
    c->delay_target = (uint32_t)((buf_len * 8 * 1000000ull) / rate);
    LWIP_PLATFORM_DIAG(("Ideal frame delay: %lu us\n", c->delay_target));
    /* truncate the delay according to clock resolution, may result in a higher bitrate */
    c->delay_target = (c->delay_target / CLOCK_RESOLUTION_US) * CLOCK_RESOLUTION_US;
    if (c->delay_target == 0u)
    {
        /* bitrate is high - have to send more than 1 frame per CLOCK_RESOLUTION_US
         * period, may result in a lower bitrate and/or a higher jitter */
        c->delay_target     = CLOCK_RESOLUTION_US;
        c->frames_per_delay = CLOCK_RESOLUTION_US / (uint32_t)((buf_len * 8 * 1000000ull) / rate);
    }
    else
    {
        c->frames_per_delay = 1u;
    }
    LWIP_PLATFORM_DIAG(("Send %u frame(s) once per %lu us\n", c->frames_per_delay, c->delay_target));
}

/** Try to send more data on an iperf udp session */
/* Must be called in main loop */
static void lwiperf_udp_client_send_more(lwiperf_state_udp_t * conn)
{
    struct pbuf * p;
    struct timespec ts, dt;
    err_t err;
    int ending = 0;
    int i;

    LWIP_ASSERT("conn invalid", (conn != NULL) && !conn->base.tcp && (conn->base.server == 0));

    if (conn->settings.base.amount & PP_HTONL(0x80000000))
    {
        /* this session is time-limited */
        u32_t now     = sys_now();
        u32_t diff_ms = now - conn->time_started;
        u32_t time    = (u32_t) - (s32_t) lwip_ntohl(conn->settings.base.amount);
        u32_t time_ms = time * 10;
        if (diff_ms >= time_ms)
        {
            ending = 1;
            if (diff_ms > (time_ms + 500))
                ending++;
        }
    }
    else
    {
        /* this session is byte-limited */
        u32_t amount_bytes = lwip_ntohl(conn->settings.base.amount);
        if (conn->bytes_transferred >= amount_bytes)
        {
            ending = 1;
            if (conn->bytes_transferred >= (amount_bytes + 4096))
                ending++;
        }
    }
    if (ending && (ending > 1 || conn->report_count > 0))
    {
        lwiperf_udp_close(conn, LWIPERF_UDP_DONE_CLIENT);
        return;
    }
    /* check time/bw */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    diff_ts(&conn->udp_lastpkt, &ts, &dt);
    if ((uint32_t)((dt.tv_sec * 1000000) + (dt.tv_nsec / 1000)) < conn->delay_target)
        return;

    for (i = 0; i < conn->frames_per_delay; i++)
    {
        /* make pbuf to transmit */
        p = pbuf_alloc(PBUF_TRANSPORT, lwip_ntohl(conn->settings.base.buffer_len), PBUF_POOL);
        if (p)
        {
            struct UDP_datagram * pkt = (struct UDP_datagram *) p->payload;
            int hsz =
                (conn->settings.base.flags & PP_HTONL(LWIPERF_FLAGS_EXTEND) ? sizeof(conn->settings) : sizeof(conn->settings.base));
            int offset = sizeof(struct UDP_datagram);
            /* set UDP datagram header */
            if (ending)
                pkt->id = htonl(-conn->udp_seq);
            else
                pkt->id = htonl(conn->udp_seq);
            pkt->tv_sec  = htonl(ts.tv_sec);
            pkt->tv_usec = htonl(ts.tv_nsec / 1000);
            /* save last packet time */
            conn->udp_lastpkt = ts;
            /* add settings in all buffer sent */
            pbuf_take_at(p, &conn->settings, hsz, offset);
            offset += hsz;
            /* add data */
            pbuf_take_at(p, &lwiperf_txbuf_const[0], p->tot_len - offset, offset);
            /* send it */
            err = udp_send(conn->pcb, p);
            if (err == ERR_OK)
            {
                conn->udp_seq++;
                conn->bytes_transferred += p->tot_len;
            }
            else
            {
                /* Do not close connection when udp_send() fails - tx may be overloaded
                 * momentarily, the datagram will be just lost: */
                /* lwiperf_udp_close(conn, LWIPERF_UDP_ABORTED_LOCAL_TXERROR); */
                /* release pbuf */
                pbuf_free(p);
                return;
            }
            /* release pbuf */
            pbuf_free(p);
            /* adjust delay for ending retries */
            if (ending)
            {
                conn->delay_target     = 250000; /* ending retry delay : 250ms */
                conn->frames_per_delay = 1U;
            }
        }
        else
        {
            /* Do not close connection when pbuf_alloc() fails - it may recover later */
            /* lwiperf_udp_close(conn, LWIPERF_UDP_ABORTED_LOCAL); */
            return;
        }
    }
}

/** Create a new UDP connection back to the client.
 */
static lwiperf_state_udp_t * lwiperf_udp_tx_new(lwiperf_state_udp_t * conn)
{
    lwiperf_state_udp_t * client_conn;
    struct udp_pcb * newpcb;

    client_conn = (lwiperf_state_udp_t *) LWIPERF_ALLOC(lwiperf_state_udp_t);
    if (client_conn == NULL)
    {
        return NULL;
    }
    memset(client_conn, 0, sizeof(lwiperf_state_udp_t));
    newpcb = udp_new();
    if (newpcb == NULL)
    {
        LWIPERF_FREE(lwiperf_state_udp_t, client_conn);
        return NULL;
    }
    if (conn)
    {
        MEMCPY(client_conn, conn, sizeof(lwiperf_state_udp_t));
    }
    client_conn->base.tcp            = 0;
    client_conn->base.server         = 0;
    client_conn->pcb                 = newpcb;
    client_conn->time_started        = sys_now();
    client_conn->bytes_transferred   = 0;
    client_conn->settings.base.flags = 0; /* prevent the remote side starting back as client again */

    udp_recv(newpcb, lwiperf_udp_recv, client_conn);
    return client_conn;
}

/** Start UDP connection back to the client (either parallel or after the
 * receive test has finished).
 */
static err_t lwiperf_udp_tx_start(lwiperf_state_udp_t * conn)
{
    lwiperf_state_udp_t * cc;
    u32_t buf_len;
    err_t err;

    cc = lwiperf_udp_tx_new(conn);
    if (cc == NULL)
    {
        return ERR_MEM;
    }
    /* take remote port to use from received settings */
    cc->remote_port = (u16_t) lwip_htonl(cc->settings.base.remote_port);
    /* buffer_len depend on address type */
    buf_len                      = (u32_t)(IP_IS_V6(&cc->remote_addr) ? 1450 : 1470);
    cc->settings.base.buffer_len = lwip_htonl(buf_len);
    /* compute delay for bandwidth restriction, constrained to [0,1000] milliseconds */
    /* 10Mbit/s by default if not extended settings */
    if (cc->settings.base.flags & PP_HTONL(LWIPERF_FLAGS_EXTEND))
    {
        lwiperf_udp_set_client_rate(cc, lwip_ntohl(cc->settings.rate), buf_len);
    }
    else if (cc->settings.base.win_band != 0)
    {
        lwiperf_udp_set_client_rate(cc, lwip_ntohl(cc->settings.base.win_band), buf_len);
    }
    else
    {
        lwiperf_udp_set_client_rate(cc, (1024 * 1024), buf_len);
    }

    err = udp_connect(cc->pcb, &cc->remote_addr, cc->remote_port);
    if (err != ERR_OK)
    {
        lwiperf_udp_close(cc, LWIPERF_TCP_ABORTED_LOCAL);
        return err;
    }
    lwiperf_list_add(&cc->base);
    /* start sending immediately */
    lwiperf_udp_client_send_more(cc);
    return ERR_OK;
}

/** Receive data on an iperf udp session
 * If client session, will receive final FIN from server,
 * else, will receive all server traffic.
 */
static void lwiperf_udp_recv(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port)
{
    lwiperf_state_udp_t * server = (lwiperf_state_udp_t *) arg;
    lwiperf_state_udp_t * conn   = NULL;
    struct UDP_datagram * pkt;
    int32_t datagramID;
    u16_t tot_len = p->tot_len;

    LWIP_ASSERT("pcb mismatch", server->pcb == pcb);
    LWIP_UNUSED_ARG(pcb);

    /* lookup client using remote addr */
    if (server->base.server)
        conn = lwiperf_udp_search_client(server, addr, port);
    else
        conn = server;

    /* Read packet iperf data. */
    pkt        = (struct UDP_datagram *) p->payload;
    datagramID = ntohl(pkt->id);

    if (conn && !conn->base.server)
    {
        /* received server reports for client instances -> close it. */
        if (!conn->report_count)
        {
            lwiperf_udp_report_t * hdr;
            pkt     = (struct UDP_datagram *) p->payload;
            pkt->id = htonl(datagramID);
            hdr     = (lwiperf_udp_report_t *) (pkt + 1);
            if (hdr->flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST))
            {
                /* Adjust bytes transferred with the one from server report */
                LWIP_PLATFORM_DIAG(("Received report from server (0x%x).\n", lwip_ntohl(hdr->flags)));
                /*LWIP_PLATFORM_DIAG(("Stop %ld.%03ld sec, ", ntohl(hdr->stop_sec), ntohl(hdr->stop_usec)/1000));
                  LWIP_PLATFORM_DIAG(("Total %ldKB, ", ntohl(hdr->total_len2)/1024));*/
                LWIP_PLATFORM_DIAG(("Jitter %ld.%03ld, ", ntohl(hdr->jitter1), ntohl(hdr->jitter2)));
                LWIP_PLATFORM_DIAG(
                    ("Lost %ld/%ld datagrams, OoO %ld\n", ntohl(hdr->error_cnt), ntohl(hdr->datagrams), ntohl(hdr->outorder_cnt)));
                conn->bytes_transferred = (((u64_t) ntohl(hdr->total_len1)) << 32) + ntohl(hdr->total_len2);
            }
            if (hdr->flags & PP_HTONL(LWIPERF_FLAGS_EXTEND))
            {
                LWIP_PLATFORM_DIAG(("Extended report unsupported yet.\n"));
            }
        }
        conn->report_count++;
    }
    else if (datagramID >= 0)
    {
        struct timespec ts, dt;
        uint32_t transit;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        if (!conn || !conn->have_settings_buf)
        {
            /* allocate struct for a new client */
            if (!conn)
            {
                conn = lwiperf_udp_new_client(server);
                if (!conn)
                {
                    pbuf_free(p);
                    return;
                }
            }
            /* wait for 24-byte header */
            if (p->tot_len < (sizeof(struct UDP_datagram) + sizeof(lwiperf_settings_t)))
            {
                lwiperf_udp_close(conn, LWIPERF_UDP_ABORTED_LOCAL_DATAERROR);
                pbuf_free(p);
                return;
            }
            /* copy settings */
            if (pbuf_copy_partial(p, &conn->settings, sizeof(lwiperf_settings_t), sizeof(struct UDP_datagram)) !=
                sizeof(lwiperf_settings_t))
            {
                lwiperf_udp_close(conn, LWIPERF_UDP_ABORTED_LOCAL);
                pbuf_free(p);
                return;
            }
            conn->have_settings_buf = 1;
            LWIP_PLATFORM_DIAG(("New UDP client (settings flags 0x%x)\n", lwip_ntohl(conn->settings.base.flags)));
            /* Save client data. */
            ip_addr_copy(conn->remote_addr, *addr);
            conn->remote_port  = port;
            conn->time_started = sys_now();
            /* Reset tradeoff & dualtest setting if temporary server */
            if (server->base.server & 0x80)
            {
                conn->settings.base.flags &= ~PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST | LWIPERF_FLAGS_ANSWER_NOW);
            }
            /* check if dualtest requested */
            if (conn->settings.base.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST))
            {
                if (conn->settings.base.flags & PP_HTONL(LWIPERF_FLAGS_ANSWER_NOW))
                {
                    /* client requested parallel transmission test */
                    LWIP_PLATFORM_DIAG(("client requested parallel transmission test\n"));
                    err_t err2 = lwiperf_udp_tx_start(conn);
                    if (err2 != ERR_OK)
                    {
                        lwiperf_udp_close(conn, LWIPERF_UDP_ABORTED_LOCAL_TXERROR);
                        pbuf_free(p);
                        return;
                    }
                }
            }
        }
        /* Update stats. */
        conn->udp_lastpkt.tv_sec  = ntohl(pkt->tv_sec);
        conn->udp_lastpkt.tv_nsec = ntohl(pkt->tv_usec) * 1000;
        if (conn->udp_seq != (uint32_t) datagramID)
        {
            conn->udp_rx_lost += (uint32_t) datagramID - conn->udp_seq;
            conn->udp_seq = datagramID + 1;
            conn->udp_rx_outorder += 1;
        }
        else
        {
            conn->bytes_transferred += tot_len;
            conn->udp_rx_total_pkt += 1;
            conn->udp_seq += 1;
        }
        /* Jitter calculation
         * from RFC 1889, Real Time Protocol (RTP)
         *   J = J + ( | D(i-1,i) | - J ) / Compute jitter
         */
        diff_ts(&conn->udp_lastpkt, &ts, &dt);
        transit = (uint32_t)((dt.tv_sec * 1000000) + (dt.tv_nsec / 1000));
        if (conn->udp_last_transit)
        {
            long deltaTransit = (long) (transit - conn->udp_last_transit);
            if (deltaTransit < 0.0)
            {
                deltaTransit = -deltaTransit;
            }
            conn->jitter += (deltaTransit - conn->jitter) >> 4;
        }
        else
        {
            conn->udp_last_transit = transit;
        }
    }
    else
    {
        if (conn && conn->have_settings_buf && !conn->report_count)
        {
            lwiperf_udp_report_t * hdr;
            u32_t now, duration_ms;
            now         = sys_now();
            duration_ms = now - conn->time_started;
            /* Copy packet and send report back. */
            struct pbuf * q = pbuf_clone(PBUF_TRANSPORT, PBUF_POOL, p);
            LWIP_ASSERT("can't clone buffer", q != NULL);
            pkt               = (struct UDP_datagram *) q->payload;
            pkt->id           = htonl(datagramID);
            hdr               = (lwiperf_udp_report_t *) (pkt + 1);
            hdr->flags        = PP_HTONL(LWIPERF_FLAGS_ANSWER_TEST);
            hdr->total_len1   = htonl((u32_t)(conn->bytes_transferred >> 32));
            hdr->total_len2   = htonl((u32_t)(conn->bytes_transferred & 0xFFFFFFFF));
            hdr->stop_sec     = htonl(duration_ms / 1000);
            hdr->stop_usec    = htonl((duration_ms % 1000) * 1000);
            hdr->error_cnt    = htonl(conn->udp_rx_lost);
            hdr->outorder_cnt = htonl(conn->udp_rx_outorder);
            hdr->datagrams    = htonl(conn->udp_rx_total_pkt);
            hdr->jitter1      = htonl(conn->jitter / 1000000);
            hdr->jitter2      = htonl((conn->jitter % 1000000) / 1000);
            /* Adjust bytes transferred with the one from server report */
            LWIP_PLATFORM_DIAG(("Sending report back to client (0x%x).\n", hdr->flags));
            /*LWIP_PLATFORM_DIAG(("Stop %ld.%03ld sec, ", ntohl(hdr->stop_sec), ntohl(hdr->stop_usec)/1000));
              LWIP_PLATFORM_DIAG(("Total %ldKB, ", ntohl(hdr->total_len2)/1024));*/
            LWIP_PLATFORM_DIAG(("Jitter %ld.%03ld, ", ntohl(hdr->jitter1), ntohl(hdr->jitter2)));
            LWIP_PLATFORM_DIAG(
                ("Lost %ld/%ld datagrams, OoO %ld\n", ntohl(hdr->error_cnt), ntohl(hdr->datagrams), ntohl(hdr->outorder_cnt)));
            /* Store report buffer in conn structure */
            conn->reported = q;
            /* Send report to client. */
            lwiperf_udp_send_report(conn);
        }
    }
    pbuf_free(p);
}

/**
 * @ingroup iperf
 * Start an UDP iperf server on a specific IP address and port and listen for
 * incoming datagrams from iperf clients.
 *
 * @returns a connection handle that can be used to abort the server
 *          by calling @ref lwiperf_abort()
 */
void * lwiperf_start_udp_server(const ip_addr_t * local_addr, u16_t local_port, lwiperf_report_fn report_fn, void * report_arg)
{
    lwiperf_state_udp_t * s;
    err_t err;

    LWIP_ASSERT_CORE_LOCKED();

    if (local_addr == NULL)
        return NULL;
    s = (lwiperf_state_udp_t *) LWIPERF_ALLOC(lwiperf_state_udp_t);
    if (s == NULL)
        return NULL;
    do
    {
        memset(s, 0, sizeof(lwiperf_state_udp_t));
        s->base.tcp    = 0;
        s->base.server = 1;
        s->report_fn   = report_fn;
        s->report_arg  = report_arg;
        /* allocate udp pcb */
        s->pcb = udp_new();
        if (s->pcb == NULL)
            break;
        /* bind to locat port/address */
        err = udp_bind(s->pcb, local_addr, local_port);
        if (err != ERR_OK)
            break;
        /* join group if multicast address */
        if (ip_addr_ismulticast(local_addr))
        {
            if (IP_IS_V6(local_addr))
            {
#if LWIP_IPV6_MLD
                err = mld6_joingroup(IP6_ADDR_ANY6, ip_2_ip6(local_addr));
#endif
            }
            else
            {
#if LWIP_IGMP
                err = igmp_joingroup(IP4_ADDR_ANY4, ip_2_ip4(local_addr));
#endif
            }
            if (err != ERR_OK)
                break;
        }
        /* start receiving datagrams */
        udp_recv(s->pcb, lwiperf_udp_recv, s);
        /* finally, add to list */
        lwiperf_list_add(&s->base);
        return s;
    } while (0);
    /* error occurs, cleanup */
    if (s->pcb)
        udp_remove(s->pcb);
    LWIPERF_FREE(lwiperf_state_udp_t, s);
    return NULL;
}

/**
 * @ingroup iperf
 * Start a UDP iperf client connected to a specific IP address and port.
 *
 * @returns a connection handle that can be used to abort the client
 *          by calling @ref lwiperf_abort()
 */
void * lwiperf_start_udp_client(const ip_addr_t * local_addr, u16_t local_port, const ip_addr_t * remote_addr, u16_t remote_port,
                                enum lwiperf_client_type type, int amount, s32_t rate, u8_t tos, lwiperf_report_fn report_fn,
                                void * report_arg)
{
    err_t err;
    lwiperf_state_udp_t * c;
    lwiperf_state_udp_t * s = NULL;
    u32_t buf_len, flags;
    u16_t sport = 0;

    switch (type)
    {
    case LWIPERF_CLIENT:
        /* Unidirectional tx only test */
        flags = 0;
        break;
    case LWIPERF_DUAL:
        /* Do a bidirectional test simultaneously */
        flags = htonl(LWIPERF_FLAGS_ANSWER_TEST | LWIPERF_FLAGS_ANSWER_NOW);
        break;
    case LWIPERF_TRADEOFF:
        /* Do a bidirectional test individually */
        flags = htonl(LWIPERF_FLAGS_ANSWER_TEST);
        break;
    default:
        /* invalid argument */
        return NULL;
    }
    if (type != LWIPERF_CLIENT)
    {
        /* tradeoff or dualtest requested. need to start a new server on another port */
        s = lwiperf_start_udp_server(local_addr ? local_addr : IP4_ADDR_ANY, local_port, report_fn, report_arg);
        if (s)
        {
            s->base.server |= 0x80;     /* put a temporary server mark */
            sport = s->pcb->local_port; /* retrieve this server port */
            LWIP_PLATFORM_DIAG(("Dualtest port: %u\n", sport));
        }
        else
        {
            LWIP_PLATFORM_DIAG(("Dualtest disabled!\n"));
        }
    }
    /* Create client */
    c = lwiperf_udp_tx_new(NULL);
    if (c == NULL)
        return NULL;
    c->report_fn  = report_fn;
    c->report_arg = report_arg;
    /* save remote */
    ip_addr_copy(c->remote_addr, *remote_addr);
    c->remote_port = remote_port;
    /* save client settings, to be copied in each packets */
    c->have_settings_buf        = 1;
    c->settings.base.amount     = lwip_htonl((uint32_t) amount);
    buf_len                     = (u32_t)(IP_IS_V6(remote_addr) ? 1450 : 1470);
    c->settings.base.buffer_len = lwip_htonl(buf_len);
    if (rate != (1024 * 1024))
    { /* 1Mb/s is the default if not specified. */
        c->settings.rate = lwip_htonl(rate);
        c->settings.base.flags |= PP_HTONL(LWIPERF_FLAGS_EXTEND);
        c->settings.base.win_band = lwip_htonl(rate);
    }
    if (sport)
    {
        /* tradeoff or dualtest requested need to put server port in settings */
        c->settings.base.flags |= flags;
        c->settings.base.remote_port = lwip_htonl(sport);
    }
    lwiperf_udp_set_client_rate(c, rate, buf_len);
    /* set tos if specified */
    if (tos)
        c->pcb->tos = tos;
    do
    {
        if (local_addr)
        {
            /* bind to local address if specified */
            err = udp_bind(c->pcb, local_addr, 0);
            if (err != ERR_OK)
                break;
            /* join multicast group? */
            if (ip_addr_ismulticast(local_addr))
            {
                if (IP_IS_V6(local_addr))
                {
#if LWIP_IPV6_MLD
                    err = mld6_joingroup(IP6_ADDR_ANY6, ip_2_ip6(local_addr));
#endif
                }
                else
                {
#if LWIP_IGMP
                    err = igmp_joingroup(IP4_ADDR_ANY4, ip_2_ip4(local_addr));
#endif
                }
                if (err != ERR_OK)
                    break;
            }
        }
        /* set pseudo-connect parameters */
        err = udp_connect(c->pcb, remote_addr, remote_port);
        if (err != ERR_OK)
            break;
        /* and add to instance list */
        lwiperf_list_add(&c->base);
        if (s != NULL)
            s->base.related_master_state = &c->base;
        /* start sending immediately */
        lwiperf_udp_client_send_more(c);
        return c;
    } while (0);
    /* error occurs, cleanup */
    if (c->pcb)
        udp_remove(c->pcb);
    LWIPERF_FREE(lwiperf_state_udp_t, c);
    return NULL;
}

/**
 * @ingroup iperf
 * Poll all running UDP client to send more according to specified BW.
 */
void lwiperf_poll_udp_client(void)
{
    lwiperf_state_base_t * c;
    for (c = lwiperf_all_connections; c != NULL; c = c->next)
    {
        if (!c->server && !c->tcp)
            lwiperf_udp_client_send_more((lwiperf_state_udp_t *) c);
    }
}

/**
 * @ingroup iperf
 * Abort an iperf session (handle returned by `lwiperf_start_(tcp|udp)_server`)
 */
void lwiperf_abort(void * lwiperf_session)
{
    lwiperf_state_base_t *i, *dealloc, *last = NULL;

    LWIP_ASSERT_CORE_LOCKED();

    for (i = lwiperf_all_connections; i != NULL;)
    {
        if ((i == lwiperf_session) || (i->related_master_state == lwiperf_session))
        {
            dealloc = i;
            i       = i->next;
            if (last != NULL)
            {
                last->next = i;
            }
            if (dealloc->tcp)
            {
                lwiperf_tcp_close((lwiperf_state_tcp_t *) dealloc, LWIPERF_TCP_ABORTED_LOCAL);
            }
            else
            {
                lwiperf_udp_close((lwiperf_state_udp_t *) dealloc, LWIPERF_UDP_ABORTED_LOCAL);
            }
        }
        else
        {
            last = i;
            i    = i->next;
        }
    }
}

#endif /* LWIP_CALLBACK_API && LWIP_TCP && LWIP_UDP */

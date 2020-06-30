/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
 *    Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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

/**
 *    @file
 *      This file implements a TUN/TAP interface shim for LwIP, used
 *      when running LwIP on non-LWIP-native platforms such as BSD
 *      sockets, to interface with host OS network interfaces on such
 *      platforms and the Internet, accessed via those host OS network
 *      interfaces.
 *
 *      The APIs in this module used by the setup and intialization
 *      code of applications are:
 *
 *        1. TapInterface_Init
 *        2. TapInterface_SetupNetif
 *        3. TapInterface_Select
 *
 *      and are generally used in the order listed.
 *
 *                               .------------------------------------------.
 *                               |                                          |
 *               .--.------------|               Application                |
 *      .--------|--|--------.   |      .           .             .         |
 *      |        V  V        |   '------+-----------+-------------+---------'
 *      |                    |          |           |             |
 *      |        LwIP        |   .------|-----------|-------------+---------.
 *      |                    |   |      V           V             V         |
 *      | .----------------. |   |  .-------. .------------. .---------.    |
 *      | |                | |   |  | Init  | | SetupNetif | | Select  |--. |
 *      | |                | |   |  '-------' '------------' '---------'  | |
 *      | |     netif      | |   |                                        | |
 *      | |                | |   |    .-------------.  .-------------.    | |
 *      | | .------------. | |   |    |  Low Level  |  |  Low Level  |    | |
 *      | | | linkoutput +-|-+---+----+>   Output   |  |    Input   <+----| |
 *      | | '------------' | |   |    |      .      |  |      ^      |    | |
 *      | |                | |   |    '------+------'  '------|------'    | |
 *      | | .------------. | |   |           |                |           | |
 *      | | |   input   <+-+-+---+-----------|----------------|-----------' |
 *      | | '------------' | |   |           |                |             |
 *      | '----------------' |   |   .-------|----------------+---------.   |
 *      '--------------------'   |   |       V                '         |   |
 *                               |   |         File Descriptor          |   |
 *                               |   |                ^                 |   |
 *                               |   '----------------|-----------------'   |
 *                               '--------------------|---------------------'
 *                                                    |
 *                                 .------------------|-------------------.
 *                                 |                  |                   |
 *                                 |   .--------------|---------------.   |
 *                                 |   | .------------|-------------. |   |
 *                                 |   | |            V             | |   |
 *                                 |   | |  TUN/TAP Shim Interface  | |   |
 *                                 |   | |                          | |   |
 *                                 |   | '--------------------------' |   |
 *                                 |   |                              |   |
 *                                 |   |        TUN/TAP Driver        |   |
 *                                 |   |                              |   |
 *                                 |   '------------------------------'   |
 *                                 |                                      |
 *                                 |               Host OS                |
 *                                 |                                      |
 *                                 '--------------------------------------'
 *
 *      This file was originally adapted from 'contrib/ports/unix/port/
 *      netif/tapif.c' in the LwIP distribution.
 *
 */

/*-----------------------------------------------------------------------------------*/
/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "TapInterface.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#if defined(linux)
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#define DEVTAP "/dev/net/tun"

#elif defined(openbsd)
#define DEVTAP "/dev/tun0"

#else /* freebsd, cygwin? */
#include <net/if.h>
#define DEVTAP "/dev/tap0"
#endif

#include <lwip/ethip6.h>
#include <lwip/mem.h>
#include <lwip/snmp.h>
#include <lwip/stats.h>
#include <netif/etharp.h>

/* Global Variables */

static const size_t kMacLength = sizeof(((TapInterface *) (0))->macAddr);

/**
 *  This is the LwIP TUN/TAP shim interface low level output method.
 *
 *  When the native LwIP stack has output for its native interface, \c
 *  netif, associated with the LwIP TUN/TAP shim interface, LwIP
 *  invokes this method to drive the specified buffer to the shim
 *  interface.
 *
 *  @param[in]  netif      A pointer to the LwIP native interface
 *                         associated with the TUN/TAP shim interface
 *                         onto which the output should be driven.
 *
 *  @param[in]  buf        A pointer to the packet buffer containing the
 *                         output to drive onto the shim interface.
 *
 *  @retval  #ERR_OK       on successfully driving the buffer onto the
 *                         shim interface.
 *
 *  @retval  #ERR_MEM      if an encapsulation buffer cannot be allocated.
 *
 *  @retval  #ERR_BUF      if the allocated encapsulation buffer is not big
 *                         enough to encapsulate the output data.
 *
 *  @retval  #ERR_IF       if the output data cannot be driven onto the shim
 *                         interface.
 *
 *  @sa #TapInterface_SetupNetif
 *
 */
static err_t TapInterface_low_level_output(struct netif * netif, struct pbuf * buf)
{
    const TapInterface * tapif = (const TapInterface *) (netif->state);
    err_t retval               = ERR_OK;
    struct pbuf * outBuf;
    int written;

    if (buf->tot_len > buf->len)
    {
        // Allocate a buffer from the buffer pool. Fail if none available.
        outBuf = pbuf_alloc(PBUF_RAW, buf->tot_len + PBUF_LINK_ENCAPSULATION_HLEN, PBUF_POOL);
        if (outBuf == NULL)
        {
            fprintf(stderr, "TapInterface: Failed to allocate buffer\n");
            retval = ERR_MEM;
            goto done;
        }

        // Fail if the buffer is not big enough to hold the output data.
        if (outBuf->tot_len != outBuf->len)
        {
            fprintf(stderr, "TapInterface: Output data bigger than single PBUF\n");
            retval = ERR_BUF;
            goto done;
        }

        // Reserve the space needed by WICED for its buffer management.
        pbuf_header(outBuf, -PBUF_LINK_ENCAPSULATION_HLEN);

        // Copy output data to the new buffer.
        retval = pbuf_copy(outBuf, buf);
        if (retval != ERR_OK)
            goto done;
    }

    // Otherwise send using the supplied buffer.
    else
        outBuf = buf;

    written = write(tapif->fd, outBuf->payload, outBuf->tot_len);
    if (written == -1)
    {
        snmp_inc_ifoutdiscards(netif);
        perror("TapInterface: write failed");
        retval = ERR_IF;
    }
    else
    {
        snmp_add_ifoutoctets(netif, written);
    }

done:
    if (outBuf != NULL && outBuf != buf)
        pbuf_free(outBuf);

    return retval;
}

/**
 *  This is the LwIP TUN/TAP shim interface low level input method.
 *
 *  When input has been identified as pending on the LwIP TUN/TAP shim
 *  interface, this API allocates a buffer and reads the packet from
 *  the underlying host OS into the buffer and returns it.
 *
 *  @param[in]  tapif      A pointer to the LwIP TUN/TAP shim interface
 *                         from which to read pending input.
 *
 *  @param[in]  netif      A pointer to the LwIP native interface
 *                         associated with the TUN/TAP shim interface.
 *
 *  @returns  A pointer to the buffer containing the read input on
 *  success; otherwise, NULL on error.
 *
 *  @sa #TapInterface_Select
 *
 */
static struct pbuf * TapInterface_low_level_input(TapInterface * tapif, struct netif * netif)
{
    struct pbuf *p, *q;
    u16_t len;
    char buf[2048];
    char * bufptr;

    /* Obtain the size of the packet and put it into the "len"
     variable. */
    len = read(tapif->fd, buf, sizeof(buf));
    snmp_add_ifinoctets(netif, len);

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_LINK, len, PBUF_POOL);

    if (p != NULL)
    {
        /* We iterate over the pbuf chain until we have read the entire
       packet into the pbuf. */
        bufptr = &buf[0];
        for (q = p; q != NULL; q = q->next)
        {
            /* Read enough bytes to fill this pbuf in the chain. The
         available data in the pbuf is given by the q->len
         variable. */
            /* read data into(q->payload, q->len); */
            memcpy(q->payload, bufptr, q->len);
            bufptr += q->len;
        }
        /* acknowledge that packet has been read(); */
    }
    else
    {
        /* drop packet(); */
        snmp_inc_ifindiscards(netif);
        printf("Could not allocate pbufs\n");
    }

    return p;
}

/**
 *  LwIP netif_add setup callback function for LwIP TUN/TAP shim
 *  interfaces.
 *
 *  The interface mimics / effects an Ethernet-like interface and,
 *  consequently, reuses and leverages existing LwIP low-level APIs
 *  for such interfaces.
 *
 *  This interface should / will be called by LwIP's netif_add
 *  function for a LwIP TUN/TAP shim interface. \c #TapInterface_Init
 *  should have been called prior to invoking netif_add with this
 *  callback.
 *
 *  @param[in,out] netif  A pointer to the LwIP netif associated with the
 *                        TUN/TAP shim interface.
 *
 *  @retval  #ERR_OK  on successfully setting up the TUN/TAP interface.
 *
 *  @sa #TapInterface_Init
 *  @sa #TapInterface_Select
 *
 */
err_t TapInterface_SetupNetif(struct netif * netif)
{
    const TapInterface * tapif = (const TapInterface *) (netif->state);

    /* As far as LwIP is concerned, the network interface is an
     * Ethernet-like interface, so set it up accordingly, reusing
     * existing LwIP APIs for such interfaces where possible.
     */

    netif->name[0] = tapif->interfaceName[0];
    netif->name[1] = tapif->interfaceName[1];
    netif->output  = etharp_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = TapInterface_low_level_output;
    netif->mtu        = 1500;

    netif->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP);
#if LWIP_IPV4
    netif->flags |= (NETIF_FLAG_IGMP);
#endif
#if LWIP_IPV6
    netif->flags |= (NETIF_FLAG_MLD6);
#endif

    netif->hwaddr_len = kMacLength;

    memcpy(netif->hwaddr, tapif->macAddr, kMacLength);

    return ERR_OK;
}

/**
 *  Establish an LwIP TUN/TAP interface on the underlying host OS with
 *  the specified name.
 *
 *  This interface should be invoked before either \c #TapInterface_SetupNetif
 *  or \c #TapInterface_Select are invoked.
 *
 *  @param[out]  tapif          A pointer to storage for the established
 *                              TUN/TAP interface details.
 *
 *  @param[in]   interfaceName  A pointer to a NULL-terminated C string
 *                              containing the name of the TUN/TAP interface.
 *
 *  @param[in]   macAddr        An optional pointer to the MAC address to be
 *                              used for the TUN/TAP interface.
 *
 *  @retval  #ERR_OK            on successfully establishing the TUN/TAP
 *                              interface.
 *
 *  @retval  #ERR_ARG           if \c tapif is NULL or if \c interfaceName is
 *                              too long.
 *
 *  @retval  #ERR_IF            if the OS-specific TUN/TAP driver cannot be
 *                              opened or if the requested TUN/TAP device
 *                              cannot be created.
 *
 *  @sa #TapInterface_SetupNetif
 *  @sa #TapInterface_Select
 *
 */
err_t TapInterface_Init(TapInterface * tapif, const char * interfaceName, u8_t * macAddr)
{
#if defined(linux)
    struct ifreq ifr;
#endif

    /* The TUN/TAP interface storage pointer is required; error out if
     * it hasn't been provided.
     */

    if (tapif == NULL)
        return ERR_ARG;

    /* Set the TUN/TAP interface storage to initial defaults.
     */

    memset(tapif, 0, sizeof(*tapif));
    tapif->fd = -1;

    /* Intialize the TUN/TAP interface name and MAC address. If the
     * optional MAC address was not provided, use the current process
     * identifier as a MAC address.
     */

    tapif->interfaceName = interfaceName;

    if (macAddr != NULL)
    {
        memcpy(tapif->macAddr, macAddr, kMacLength);
    }
    else
    {
        const u32_t pid = htonl((u32_t) getpid());
        memset(tapif->macAddr, 0, kMacLength);
        memcpy(tapif->macAddr + 2, &pid, sizeof(pid));
    }

    /* Attempt to open the OS-specific TUN/TAP driver control interface.
     */

    tapif->fd = open(DEVTAP, O_RDWR);
    if (tapif->fd == -1)
    {
        perror("TapInterface: unable to open " DEVTAP);
        return ERR_IF;
    }

#if defined(linux)
    /* On Linux, prepare and issue the TUNSETIFF ioctl to establish a
     * TAP interface (IFF_TAP) with the previously-specified name and
     * no packet information (IFF_NO_PI).
     */

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = (IFF_TAP | IFF_NO_PI);

    if (strlen(tapif->interfaceName) >= sizeof(ifr.ifr_name))
    {
        perror("TapInterface: invalid device name");
        return ERR_ARG;
    }

    strcpy(ifr.ifr_name, tapif->interfaceName);

    if (ioctl(tapif->fd, TUNSETIFF, (void *) &ifr) < 0)
    {
        perror("TapInterface: ioctl(TUNSETIFF) failed");
        return ERR_IF;
    }

#else
#warning "The LwIP TAP/TUN interface may not be fully-supported on your platform."
#endif /* defined(linux) */

    return ERR_OK;
}

/**
 *  Check the LwIP TUN/TAP shim interface to see if any input / read
 *  activity is pending and, if there is, process it.
 *
 *  @param[in]  tapif      An array of  LwIP TUN/TAP shim interfaces
 *                         to check and, if necessary, to process the
 *                         input for.
 *
 *  @param[in]  netif      An array of the LwIP native interfaces
 *                         associated with the TUN/TAP shim interface.
 *
 *  @param[in]  sleepTime  The interval that the call should block for
 *                         waiting for input.
 *
 *  @param[in]  numIntfs   The number of elements in the tapif and netif arrays.
 *
 *  @retval  >= 0 on a successful check and/or processing of pending input.
 *
 *  @retval  -EINVAL if either \c tapif or \c netif are NULL.
 *
 *  @sa #TapInterface_Init
 *  @sa #TapInterface_SetupNetif
 *
 */
int TapInterface_Select(TapInterface * tapif, struct netif * netif, struct timeval sleepTime, size_t numIntfs)
{
    fd_set readfds;
    int ret;
    size_t j;

    if ((tapif == NULL) || (netif == NULL) || (numIntfs == 0))
    {
        return -EINVAL;
    }

    FD_ZERO(&readfds);

    for (j = 0; j < numIntfs; j++)
    {
        FD_SET(tapif[j].fd, &readfds);
    }

    ret = select(tapif[numIntfs - 1].fd + 1, &readfds, NULL, NULL, &sleepTime);
    if (ret > 0)
    {
        for (j = 0; j < numIntfs; j++)
        {
            if (!FD_ISSET(tapif[j].fd, &readfds))
                continue;

            struct pbuf * p = TapInterface_low_level_input(&(tapif[j]), &(netif[j]));
            if (p != NULL)
            {
#if LINK_STATS
                lwip_stats.link.recv++;
#endif /* LINK_STATS */

                netif[j].input(p, &(netif[j]));
            }
        }
    }

    return ret;
}

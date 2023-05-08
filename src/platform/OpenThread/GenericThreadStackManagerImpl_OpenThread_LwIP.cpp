/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Contains non-inline method definitions for the
 *          GenericThreadStackManagerImpl_OpenThread_LwIP<> template.
 */

#include <lib/core/CHIPEncoding.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <openthread/error.h>
#include <openthread/icmp6.h>
#include <openthread/ip6.h>
#include <openthread/netdata.h>
#include <openthread/thread.h>

#include <credentials/GroupDataProvider.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>

#include <transport/raw/PeerAddress.h>

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
#error "When using OpenThread Endpoints, one should also use GenericThreadStackManagerImpl_OpenThread"
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread_LwIP<ImplClass>::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Pass the event to the base class first.
    GenericThreadStackManagerImpl_OpenThread<ImplClass>::_OnPlatformEvent(event);

    if (event->Type == DeviceEventType::kThreadStateChange)
    {
        // If the Thread device role has changed, or if an IPv6 address has been added or
        // removed in the Thread stack, update the state and configuration of the LwIP Thread interface.
        if (event->ThreadStateChange.RoleChanged || event->ThreadStateChange.AddressChanged)
        {
            UpdateThreadInterface(event->ThreadStateChange.AddressChanged);
        }
    }
}

template <class ImplClass>
CHIP_ERROR GenericThreadStackManagerImpl_OpenThread_LwIP<ImplClass>::DoInit(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    static struct netif sThreadNetIf;

    // Initialize member data.
    memset(mAddrAssigned, 0, sizeof(mAddrAssigned));

    sThreadNetIf.name[0] = 'o';
    sThreadNetIf.name[1] = 't';

    // Initialize the base class.
    err = GenericThreadStackManagerImpl_OpenThread<ImplClass>::DoInit(otInst);
    SuccessOrExit(err);

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Initialize a LwIP netif structure for the OpenThread interface and
    // add it to the list of interfaces known to LwIP.
    mNetIf = netif_add(&sThreadNetIf,
#if LWIP_IPV4
                       NULL, NULL, NULL,
#endif // LWIP_IPV4
                       NULL, DoInitThreadNetIf, tcpip_input);

    // Start with the interface in the down state.
    netif_set_link_down(mNetIf);

    // Unkock LwIP stack
    UNLOCK_TCPIP_CORE();

    VerifyOrExit(mNetIf != NULL, err = INET_ERROR_INTERFACE_INIT_FAILURE);

    // Lock OpenThread
    Impl()->LockThreadStack();

    // Arrange for OpenThread to call our ReceivePacket() method whenever an
    // IPv6 packet is received.
    otIp6SetReceiveCallback(Impl()->OTInstance(), ReceivePacket, NULL);

    // Disable automatic echo mode in OpenThread.
    otIcmp6SetEchoMode(Impl()->OTInstance(), OT_ICMP6_ECHO_HANDLER_DISABLED);

    // Enable the receive filter for Thread control traffic.
    otIp6SetReceiveFilterEnabled(Impl()->OTInstance(), true);

    // Unlock OpenThread
    Impl()->UnlockThreadStack();

exit:
    return err;
}

template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread_LwIP<ImplClass>::UpdateThreadInterface(bool addrChange)
{
    err_t lwipErr = ERR_OK;
    bool isInterfaceUp;
    bool addrAssigned[LWIP_IPV6_NUM_ADDRESSES];

    memset(addrAssigned, 0, sizeof(addrAssigned));

    // Lock LwIP stack first, then OpenThread.
    LOCK_TCPIP_CORE();
    Impl()->LockThreadStack();

    // Determine whether the device Thread interface is up..
    isInterfaceUp = GenericThreadStackManagerImpl_OpenThread<ImplClass>::IsThreadInterfaceUpNoLock();

    // If needed, adjust the link state of the LwIP netif to reflect the state of the OpenThread stack.
    // Set ifConnectivity to indicate the change in the link state.
    if (isInterfaceUp != (bool) netif_is_link_up(mNetIf))
    {
        ChipLogDetail(DeviceLayer, "LwIP Thread interface %s", isInterfaceUp ? "UP" : "DOWN");

        if (isInterfaceUp)
        {
            netif_set_link_up(mNetIf);
        }
        else
        {
            netif_set_link_down(mNetIf);
        }

        // Presume the interface addresses are also changing.
        addrChange = true;
    }

    // If needed, adjust the set of addresses associated with the LwIP netif to reflect those
    // known to the Thread stack.
    if (addrChange)
    {
        // If attached to a Thread network, add addresses to the LwIP netif to match those
        // configured in the Thread stack...
        if (isInterfaceUp)
        {
            // Enumerate the list of unicast IPv6 addresses known to OpenThread...
            const otNetifAddress * otAddrs = otIp6GetUnicastAddresses(Impl()->OTInstance());
            for (const otNetifAddress * otAddr = otAddrs; otAddr != NULL; otAddr = otAddr->mNext)
            {
                Inet::IPAddress addr = ToIPAddress(otAddr->mAddress);

                // Assign the following OpenThread addresses to LwIP's address table:
                //   - link-local addresses.
                //   - mesh-local addresses that are NOT RLOC addresses.
                //   - global unicast addresses.
                if (otAddr->mValid && !otAddr->mRloc)
                {
                    ip_addr_t lwipAddr = addr.ToLwIPAddr();
                    s8_t addrIdx;

                    // Add the address to the LwIP netif.  If the address is a link-local, and the primary
                    // link-local address* for the LwIP netif has not been set already, then use netif_ip6_addr_set()
                    // to set the primary address.  Otherwise use netif_add_ip6_address(). This special case is
                    // required because LwIP's netif_add_ip6_address() will never set the primary link-local address.
                    //
                    // * -- The primary link-local address always appears in the first slot in the netif address table.
                    //
                    if (addr.IsIPv6LinkLocal() && !addrAssigned[0])
                    {
                        netif_ip6_addr_set(mNetIf, 0, ip_2_ip6(&lwipAddr));
                        addrIdx = 0;
                    }
                    else
                    {
                        // Add the address to the LwIP netif.  If the address table fills (ERR_VAL), simply stop
                        // adding addresses.  If something else fails, log it and soldier on.
                        lwipErr = netif_add_ip6_address(mNetIf, ip_2_ip6(&lwipAddr), &addrIdx);
                        if (lwipErr == ERR_VAL)
                        {
                            break;
                        }
                        else if (lwipErr != ERR_OK)
                        {
                            ChipLogProgress(DeviceLayer, "netif_add_ip6_address) failed: %s",
                                            ErrorStr(chip::System::MapErrorLwIP(lwipErr)));
                        }
                    }

                    // Set non-mesh-local address state to PREFERRED or ACTIVE depending on the state in OpenThread.
                    netif_ip6_addr_set_state(mNetIf, addrIdx,
                                             (otAddr->mPreferred && !IsOpenThreadMeshLocalAddress(Impl()->OTInstance(), addr))
                                                 ? IP6_ADDR_PREFERRED
                                                 : IP6_ADDR_VALID);

                    // Record that the netif address slot was assigned during this loop.
                    addrAssigned[addrIdx] = true;
                }
            }
        }

        ChipLogDetail(DeviceLayer, "LwIP Thread interface addresses %s", isInterfaceUp ? "updated" : "cleared");

        // For each address associated with the netif that was *not* assigned above, remove the address
        // from the netif if the address is one that was previously assigned by this method.
        // In the case where the device is no longer attached to a Thread network, remove all addresses
        // from the netif.
        for (u8_t addrIdx = 0; addrIdx < LWIP_IPV6_NUM_ADDRESSES; addrIdx++)
        {
            if (!isInterfaceUp || (mAddrAssigned[addrIdx] && !addrAssigned[addrIdx]))
            {
                // Remove the address from the netif by setting its state to INVALID
                netif_ip6_addr_set_state(mNetIf, addrIdx, IP6_ADDR_INVALID);
            }

#if CHIP_DETAIL_LOGGING
            else
            {
                uint8_t state = netif_ip6_addr_state(mNetIf, addrIdx);
                if (state != IP6_ADDR_INVALID)
                {
                    Inet::IPAddress addr = Inet::IPAddress(*netif_ip6_addr(mNetIf, addrIdx));
                    char addrStr[50];
                    addr.ToString(addrStr, sizeof(addrStr));
                    const char * typeStr;
                    if (IsOpenThreadMeshLocalAddress(Impl()->OTInstance(), addr))
                        typeStr = "Thread mesh-local address";
                    else
                        typeStr = CharacterizeIPv6Address(addr);
                    ChipLogDetail(DeviceLayer, "   %s %s%s)", addrStr, typeStr, (state == IP6_ADDR_PREFERRED) ? ", preferred" : "");
                }
            }
#endif // CHIP_DETAIL_LOGGING
        }

        // Remember the set of assigned addresses.
        memcpy(mAddrAssigned, addrAssigned, sizeof(mAddrAssigned));
    }

    Impl()->UnlockThreadStack();
    UNLOCK_TCPIP_CORE();
}

template <class ImplClass>
err_t GenericThreadStackManagerImpl_OpenThread_LwIP<ImplClass>::DoInitThreadNetIf(struct netif * netif)
{
    netif->name[0]    = CHIP_DEVICE_CONFIG_LWIP_THREAD_IF_NAME[0];
    netif->name[1]    = CHIP_DEVICE_CONFIG_LWIP_THREAD_IF_NAME[1];
    netif->output_ip6 = SendPacket;
#if LWIP_IPV4
    netif->output = NULL;
#endif // LWIP_IPV4
    netif->linkoutput = NULL;
    netif->flags      = NETIF_FLAG_UP | NETIF_FLAG_LINK_UP | NETIF_FLAG_BROADCAST;
    netif->mtu        = CHIP_DEVICE_CONFIG_THREAD_IF_MTU;
    return ERR_OK;
}

/**
 * Send an outbound packet via the LwIP Thread interface.
 *
 * This method is called by LwIP, via a pointer in the netif structure, whenever
 * an IPv6 packet is to be sent out the Thread interface.
 *
 * NB: This method is called in the LwIP TCPIP thread with the LwIP core lock held.
 */
template <class ImplClass>
err_t GenericThreadStackManagerImpl_OpenThread_LwIP<ImplClass>::SendPacket(struct netif * netif, struct pbuf * pktPBuf,
                                                                           const struct ip6_addr * ipaddr)
{
    err_t lwipErr = ERR_OK;
    otError otErr;
    otMessage * pktMsg                  = NULL;
    const otMessageSettings msgSettings = { true, OT_MESSAGE_PRIORITY_NORMAL };
    uint16_t remainingLen;

    // Lock the OpenThread stack.
    // Note that at this point the LwIP core lock is also held.
    ThreadStackMgrImpl().LockThreadStack();

    // Allocate an OpenThread message
    pktMsg = otIp6NewMessage(ThreadStackMgrImpl().OTInstance(), &msgSettings);
    VerifyOrExit(pktMsg != NULL, lwipErr = ERR_MEM);

    // Copy data from LwIP's packet buffer chain into the OpenThread message.
    remainingLen = pktPBuf->tot_len;
    for (struct pbuf * partialPkt = pktPBuf; partialPkt != NULL; partialPkt = partialPkt->next)
    {
        VerifyOrExit(partialPkt->len <= remainingLen, lwipErr = ERR_VAL);

        otErr = otMessageAppend(pktMsg, partialPkt->payload, partialPkt->len);
        VerifyOrExit(otErr == OT_ERROR_NONE, lwipErr = ERR_MEM);

        remainingLen = static_cast<uint16_t>(remainingLen - partialPkt->len);
    }
    VerifyOrExit(remainingLen == 0, lwipErr = ERR_VAL);

#if CHIP_DETAIL_LOGGING
    LogOpenThreadPacket("Thread packet SENT", pktMsg);
#endif // CHIP_DETAIL_LOGGING

    // Pass the packet to OpenThread to be sent.  Note that OpenThread takes care of releasing
    // the otMessage object regardless of whether otIp6Send() succeeds or fails.
    // Propagate the error back up the stack UNLESS it is a transient error.
    otErr  = otIp6Send(ThreadStackMgrImpl().OTInstance(), pktMsg);
    pktMsg = NULL;
#if CHIP_DETAIL_LOGGING
    if (otErr != OT_ERROR_NONE)
    {
        ChipLogDetail(DeviceLayer, "ThreadStackManagerImpl: otIp6Send() error: %s", otThreadErrorToString(otErr));
    }
#endif // CHIP_DETAIL_LOGGING
    VerifyOrExit(otErr == OT_ERROR_NONE || otErr == OT_ERROR_DROP || otErr == OT_ERROR_NO_BUFS || otErr == OT_ERROR_NO_ROUTE,
                 lwipErr = ERR_IF);

exit:

    if (pktMsg != NULL)
    {
        otMessageFree(pktMsg);
    }

    if (lwipErr == ERR_MEM)
    {
        ThreadStackMgrImpl().OverrunErrorTally();
    }

    // Unlock the OpenThread stack.
    ThreadStackMgrImpl().UnlockThreadStack();

    return lwipErr;
}

/**
 * Receive an inbound packet from the LwIP Thread interface.
 *
 * This method is called by OpenThread whenever an IPv6 packet has been received destined
 * to the local node has been received from the Thread interface.
 *
 * NB: This method is called with the OpenThread stack lock held.  To ensure proper
 * lock ordering, it must *not* attempt to acquire the LwIP TCPIP core lock, or the
 * CHIP stack lock.
 */
template <class ImplClass>
void GenericThreadStackManagerImpl_OpenThread_LwIP<ImplClass>::ReceivePacket(otMessage * pkt, void *)
{
    struct pbuf * pbuf         = NULL;
    err_t lwipErr              = ERR_OK;
    uint16_t pktLen            = otMessageGetLength(pkt);
    struct netif * threadNetIf = ThreadStackMgrImpl().ThreadNetIf();

    // Allocate an LwIP pbuf to hold the inbound packet.
    pbuf = pbuf_alloc(PBUF_LINK, pktLen, PBUF_POOL);
    VerifyOrExit(pbuf != NULL, lwipErr = ERR_MEM);

    // Copy the packet data from the OpenThread message object to the pbuf.
    if (otMessageRead(pkt, 0, pbuf->payload, pktLen) != pktLen)
    {
        ExitNow(lwipErr = ERR_IF);
    }

#if CHIP_DETAIL_LOGGING
    LogOpenThreadPacket("Thread packet RCVD", pkt);
#endif // CHIP_DETAIL_LOGGING

    // Deliver the packet to the input function associated with the LwIP netif.
    // NOTE: The input function posts the inbound packet to LwIP's TCPIP thread.
    // Thus there's no need to acquire the LwIP TCPIP core lock here.
    lwipErr = threadNetIf->input(pbuf, threadNetIf);

exit:

    // Always free the OpenThread message.
    otMessageFree(pkt);

    if (lwipErr != ERR_OK)
    {
        // If an error occurred, make sure the pbuf gets freed.
        if (pbuf != NULL)
        {
            pbuf_free(pbuf);
        }

        // TODO: log packet reception error
        // TODO: deliver CHIP platform event signaling loss of inbound packet.
    }
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

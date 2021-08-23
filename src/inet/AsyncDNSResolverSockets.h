/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      This file defines the AsyncDNSResolver, the object that performs
 *      Asynchronous Domain Name System (DNS) resolution in InetLayer.
 *
 */
#pragma once

#include <inet/IPAddress.h>
#include <inet/InetError.h>

#if INET_CONFIG_ENABLE_DNS_RESOLVER
#include <inet/DNSResolver.h>
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

namespace chip {
namespace Inet {

/**
 *  @class AsyncDNSResolverSockets
 *
 *  @brief
 *    This is an internal class to InetLayer that provides helper APIs for
 *    Asynchronous Domain Name System (DNS) resolution in InetLayer.
 *    There is no public interface available for the application layer.
 *
 */
class AsyncDNSResolverSockets
{
    friend class InetLayer;
    friend class DNSResolver;

public:
    CHIP_ERROR EnqueueRequest(DNSResolver & resolver);

    CHIP_ERROR Init(InetLayer * inet);

    CHIP_ERROR Cancel(DNSResolver & resolver);

    CHIP_ERROR Shutdown();

    CHIP_ERROR PrepareDNSResolver(DNSResolver & resolver, const char * hostName, uint16_t hostNameLen, uint8_t options,
                                  uint8_t maxAddrs, IPAddress * addrArray, DNSResolver::OnResolveCompleteFunct onComplete,
                                  void * appState);

private:
    pthread_t mAsyncDNSThreadHandle[INET_CONFIG_DNS_ASYNC_MAX_THREAD_COUNT];
    pthread_mutex_t mAsyncDNSMutex;            /* Mutex for accessing the DNSResolver queue. */
    pthread_cond_t mAsyncDNSCondVar;           /* Condition Variable for thread synchronization. */
    volatile DNSResolver * mAsyncDNSQueueHead; /* The head of the asynchronous DNSResolver object queue. */
    volatile DNSResolver * mAsyncDNSQueueTail; /* The tail of the asynchronous DNSResolver object queue. */
    InetLayer * mInet;                         /* The pointer to the InetLayer. */
    static void DNSResultEventHandler(chip::System::Layer * aLayer,
                                      void * aAppState); /* Timer event handler function for asynchronous DNS notification */

    CHIP_ERROR DequeueRequest(DNSResolver ** outResolver);

    bool ShouldThreadShutdown();

    void Resolve(DNSResolver & resolver);

    void UpdateDNSResult(DNSResolver & resolver, struct addrinfo * lookupRes);

    static void * AsyncDNSThreadRun(void * args);

    static void NotifyChipThread(DNSResolver * resolver);

    void AsyncMutexLock();

    void AsyncMutexUnlock();
};

} // namespace Inet
} // namespace chip
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

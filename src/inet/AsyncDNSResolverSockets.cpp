/*
 *
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      This file implements AsyncDNSResolverSockets, the object that implements
 *      Asynchronous Domain Name System (DNS) resolution in InetLayer.
 *
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <Weave/Support/CodeUtils.h>
#include <Weave/Support/logging/WeaveLogging.h>
#include <InetLayer/InetLayer.h>

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

#include <InetLayer/AsyncDNSResolverSockets.h>

namespace nl {
namespace Inet {

/**
 *  The explicit initializer for the AsynchronousDNSResolverSockets class.
 *  This initializes the mutex and semaphore variables and creates the
 *  threads for handling the asynchronous DNS resolution.
 *
 *  @param[in]  aInet  A pointer to the InetLayer object.
 *
 *  @retval #INET_NO_ERROR                   if initialization is
 *                                           successful.
 *  @retval other appropriate POSIX network or OS error.
 */
INET_ERROR AsyncDNSResolverSockets::Init(InetLayer *aInet)
{
    INET_ERROR err = INET_NO_ERROR;
    int pthreadErr;

    mInet = aInet;

    mAsyncDNSQueueHead = NULL;
    mAsyncDNSQueueTail = NULL;

    pthreadErr = pthread_cond_init(&mAsyncDNSCondVar, NULL);
    VerifyOrDie(pthreadErr == 0);

    pthreadErr = pthread_mutex_init(&mAsyncDNSMutex, NULL);
    VerifyOrDie(pthreadErr == 0);

    // Create the thread pool for asynchronous DNS resolution.
    for (int i = 0; i < INET_CONFIG_DNS_ASYNC_MAX_THREAD_COUNT; i++)
    {
        pthreadErr = pthread_create(&mAsyncDNSThreadHandle[i], NULL, &AsyncDNSThreadRun, this);
        VerifyOrDie(pthreadErr == 0);
    }

    return err;
}

/**
 *  This is the explicit deinitializer of the AsyncDNSResolverSockets class
 *  and it takes care of shutting the threads down and destroying the mutex
 *  and semaphore variables.
 *
 *  @retval #INET_NO_ERROR                   if shutdown is successful.
 *  @retval other appropriate POSIX network or OS error.
 */
INET_ERROR AsyncDNSResolverSockets::Shutdown(void)
{
    INET_ERROR err = INET_NO_ERROR;
    int pthreadErr;

    AsyncMutexLock();

    mInet->State = InetLayer::kState_ShutdownInProgress;

    pthreadErr = pthread_cond_broadcast(&mAsyncDNSCondVar);
    VerifyOrDie(pthreadErr == 0);

    AsyncMutexUnlock();

    // Have the Weave thread join the thread pool for asynchronous DNS resolution.
    for (int i = 0; i < INET_CONFIG_DNS_ASYNC_MAX_THREAD_COUNT; i++)
    {
        pthreadErr = pthread_join(mAsyncDNSThreadHandle[i], NULL);
        VerifyOrDie(pthreadErr == 0);
    }

    pthreadErr = pthread_mutex_destroy(&mAsyncDNSMutex);
    VerifyOrDie(pthreadErr == 0);

    pthreadErr = pthread_cond_destroy(&mAsyncDNSCondVar);
    VerifyOrDie(pthreadErr == 0);

    return err;
}

/**
 *  This method prepares a DNSResolver object prior to asynchronous resolution.
 *
 *  @param[in]  resolver    A reference to an allocated DNSResolver object.
 *
 *  @param[in]  hostName    A pointer to a C string representing the host name
 *                          to be queried.
 *  @param[in]  hostNameLen The string length of host name.
 *  @param[in]  options     An integer value controlling how host name address
 *                          resolution is performed.  Values are from the #DNSOptions
 *                          enumeration.
 *  @param[in]  maxAddrs    The maximum number of addresses to store in the DNS
 *                          table.
 *  @param[in]  addrArray   A pointer to the DNS table.
 *  @param[in]  onComplete  A pointer to the callback function when a DNS
 *                          request is complete.
 *  @param[in]  appState    A pointer to the application state to be passed to
 *                          onComplete when a DNS request is complete.
 *
 *  @retval INET_NO_ERROR                   if a DNS request is handled
 *                                          successfully.
 *
 */
INET_ERROR AsyncDNSResolverSockets::PrepareDNSResolver(DNSResolver &resolver, const char *hostName, uint16_t hostNameLen,
                                                       uint8_t options, uint8_t maxAddrs, IPAddress *addrArray,
                                                       DNSResolver::OnResolveCompleteFunct onComplete, void *appState)
{
    INET_ERROR err = INET_NO_ERROR;

    memcpy(resolver.asyncHostNameBuf, hostName, hostNameLen);
    resolver.asyncHostNameBuf[hostNameLen] = 0;
    resolver.MaxAddrs = maxAddrs;
    resolver.NumAddrs = 0;
    resolver.DNSOptions = options;
    resolver.AddrArray = addrArray;
    resolver.AppState = appState;
    resolver.OnComplete = onComplete;
    resolver.asyncDNSResolveResult = INET_NO_ERROR;
    resolver.mState = DNSResolver::kState_Active;
    resolver.pNextAsyncDNSResolver = NULL;

    return err;
}

/**
 *  Enqueue a DNSResolver object for asynchronous IP address resolution of a specified hostname.
 *
 *  @param[in]  resolver    A reference to the DNSResolver object.
 *
 *  @retval #INET_NO_ERROR                   if a DNS request is queued
 *                                           successfully.
 *  @retval #INET_ERROR_NO_MEMORY            if the Inet layer resolver pool
 *                                           is full.
 *  @retval other appropriate POSIX network or OS error.
 *
 */
INET_ERROR AsyncDNSResolverSockets::EnqueueRequest(DNSResolver &resolver)
{
    INET_ERROR err = INET_NO_ERROR;
    int pthreadErr;

    AsyncMutexLock();

    // Add the DNSResolver object to the queue.
    if (mAsyncDNSQueueHead == NULL)
    {
        mAsyncDNSQueueHead = &resolver;
    }

    if (mAsyncDNSQueueTail != NULL)
    {
        mAsyncDNSQueueTail->pNextAsyncDNSResolver = &resolver;
    }

    mAsyncDNSQueueTail = &resolver;

    pthreadErr = pthread_cond_signal(&mAsyncDNSCondVar);
    VerifyOrDie(pthreadErr == 0);

    AsyncMutexUnlock();

    return err;
}

/**
 * Dequeue a DNSResolver object from the queue if there is one.
 * Make the worker thread block if there is no item in the queue.
 *
 */
INET_ERROR AsyncDNSResolverSockets::DequeueRequest(DNSResolver **outResolver)
{
    INET_ERROR err = INET_NO_ERROR;
    int pthreadErr;

    AsyncMutexLock();

    // block until there is work to do or we detect a shutdown
    while ( (mAsyncDNSQueueHead == NULL) &&
            (mInet->State == InetLayer::kState_Initialized) )
    {
        pthreadErr = pthread_cond_wait(&mAsyncDNSCondVar, &mAsyncDNSMutex);
        VerifyOrDie(pthreadErr == 0);
    }

    WeaveLogDetail(Inet, "Async DNS worker thread woke up.");

    // on shutdown, return NULL. Otherwise, pop the head of the DNS request queue
    if (mInet->State != InetLayer::kState_Initialized)
    {
        *outResolver = NULL;
    }
    else
    {
        *outResolver = const_cast<DNSResolver *>(mAsyncDNSQueueHead);

        mAsyncDNSQueueHead = mAsyncDNSQueueHead->pNextAsyncDNSResolver;

        if (mAsyncDNSQueueHead == NULL)
        {
            // Queue is empty
            mAsyncDNSQueueTail = NULL;
        }
    }

    AsyncMutexUnlock();

    return err;
}

/**
 *  Cancel an outstanding DNS query that may still be active.
 *
 *  @param[in]    resolver   A reference to the DNSResolver object.
 */
INET_ERROR AsyncDNSResolverSockets::Cancel(DNSResolver &resolver)
{
    INET_ERROR err = INET_NO_ERROR;

    AsyncMutexLock();

    resolver.mState = DNSResolver::kState_Canceled;

    AsyncMutexUnlock();

    return err;
}

void AsyncDNSResolverSockets::UpdateDNSResult(DNSResolver &resolver, struct addrinfo *inLookupRes)
{
    resolver.NumAddrs = 0;

    for (struct addrinfo *addr = inLookupRes; addr != NULL && resolver.NumAddrs < resolver.MaxAddrs; addr = addr->ai_next, resolver.NumAddrs++)
    {
        resolver.AddrArray[resolver.NumAddrs] = IPAddress::FromSockAddr(*addr->ai_addr);
    }
}

void AsyncDNSResolverSockets::Resolve(DNSResolver &resolver)
{
    struct addrinfo gaiHints;
    struct addrinfo * gaiResults = NULL;
    int gaiReturnCode;

    // Configure the hints argument for getaddrinfo()
    resolver.InitAddrInfoHints(gaiHints);

    // Call getaddrinfo() to perform the name resolution.
    gaiReturnCode = getaddrinfo(resolver.asyncHostNameBuf, NULL, &gaiHints, &gaiResults);

    // Mutex protects the read and write operation on resolver->mState
    AsyncMutexLock();

    // Process the return code and results list returned by getaddrinfo(). If the call
    // was successful this will copy the resultant addresses into the caller's array.
    resolver.asyncDNSResolveResult = resolver.ProcessGetAddrInfoResult(gaiReturnCode, gaiResults);

    // Set the DNS resolver state.
    resolver.mState = DNSResolver::kState_Complete;

    // Release lock.
    AsyncMutexUnlock();

    return;
}

/* Event handler function for asynchronous DNS notification */
void  AsyncDNSResolverSockets::DNSResultEventHandler (Weave::System::Layer* aLayer, void* aAppState, Weave::System::Error aError)
{
    DNSResolver *resolver = static_cast<DNSResolver *>(aAppState);

    if (resolver)
    {
        resolver->HandleAsyncResolveComplete();
    }
}

void AsyncDNSResolverSockets::NotifyWeaveThread(DNSResolver *resolver)
{
    // Post work item via Timer Event for the Weave thread
    Weave::System::Layer& lSystemLayer = resolver->SystemLayer();

    WeaveLogDetail(Inet, "Posting DNS completion event to Weave thread.");
    lSystemLayer.ScheduleWork(AsyncDNSResolverSockets::DNSResultEventHandler, resolver);
}

void *AsyncDNSResolverSockets::AsyncDNSThreadRun(void *args)
{

    INET_ERROR err = INET_NO_ERROR;
    AsyncDNSResolverSockets *asyncResolver = static_cast<AsyncDNSResolverSockets*>(args);

    while (true)
    {
        DNSResolver *request = NULL;

        // Dequeue a DNSResolver for resolution. This function would block until there
        // is an item in the queue or shutdown has been called.
        err = asyncResolver->DequeueRequest(&request);

        // If shutdown has been called, DeQueue would return with an empty request.
        // In that case, break out of the loop and exit thread.
        VerifyOrExit(err == INET_NO_ERROR && request != NULL, );

        if (request->mState != DNSResolver::kState_Canceled)
        {
            asyncResolver->Resolve(*request);
        }

        asyncResolver->NotifyWeaveThread(request);
    }

exit:
    WeaveLogDetail(Inet, "Async DNS worker thread exiting.");
    return NULL;
}

void AsyncDNSResolverSockets::AsyncMutexLock(void)
{
    int pthreadErr;

    pthreadErr = pthread_mutex_lock(&mAsyncDNSMutex);
    VerifyOrDie(pthreadErr == 0);
}

void AsyncDNSResolverSockets::AsyncMutexUnlock(void)
{
    int pthreadErr;

    pthreadErr = pthread_mutex_unlock(&mAsyncDNSMutex);
    VerifyOrDie(pthreadErr == 0);
}

} // namespace Inet
} // namespace nl
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER && INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

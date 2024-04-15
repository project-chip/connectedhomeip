/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRDeviceConnectivityMonitor.h"
#import "MTRLogging_Internal.h"
#import "MTRUnfairLock.h"

#import <Network/Network.h>
#import <dns_sd.h>
#import <os/lock.h>

@interface MTRDeviceConnectivityMonitor ()
- (void)handleResolvedHostname:(const char *)hostName port:(uint16_t)port error:(DNSServiceErrorType)error;
@end

@implementation MTRDeviceConnectivityMonitor {
    NSString * _instanceName;
    DNSServiceRef _resolver;
    NSMutableDictionary<NSString *, nw_connection_t> * _connections;

    MTRDeviceConnectivityMonitorHandler _monitorHandler;
    dispatch_queue_t _handlerQueue;
}

namespace {
constexpr char kLocalDot[] = "local.";
constexpr char kOperationalType[] = "_matter._tcp";
}

static dispatch_once_t sConnecitivityMonitorOnceToken;
static os_unfair_lock sConnectivityMonitorLock;
static NSUInteger sConnectivityMonitorCount;
static DNSServiceRef sSharedResolverConnection;
static dispatch_queue_t sSharedResolverQueue;

- (instancetype)initWithInstanceName:(NSString *)instanceName
{
    if (self = [super init]) {
        dispatch_once(&sConnecitivityMonitorOnceToken, ^{
            sConnectivityMonitorLock = OS_UNFAIR_LOCK_INIT;
        });
        _instanceName = [instanceName copy];
        _connections = [NSMutableDictionary dictionary];
    }
    return self;
}

- (void)dealloc
{
    if (_resolver) {
        DNSServiceRefDeallocate(_resolver);
    }
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDeviceConnectivityMonitor: %@>", _instanceName];
}

+ (DNSServiceRef)_sharedResolverConnection
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);

    if (!sSharedResolverConnection) {
        DNSServiceErrorType dnsError = DNSServiceCreateConnection(&sSharedResolverConnection);
        if (dnsError) {
            MTR_LOG_ERROR("MTRDeviceConnectivityMonitor: DNSServiceCreateConnection failed %d", dnsError);
            return NULL;
        }
        sSharedResolverQueue = dispatch_queue_create("MTRDeviceConnectivityMonitor", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        dnsError = DNSServiceSetDispatchQueue(sSharedResolverConnection, sSharedResolverQueue);
        if (dnsError != kDNSServiceErr_NoError) {
            MTR_LOG_ERROR("%@ cannot set dispatch queue on resolve", self);
            DNSServiceRefDeallocate(sSharedResolverConnection);
            sSharedResolverConnection = NULL;
            sSharedResolverQueue = nil;
            return NULL;
        }
    }

    return sSharedResolverConnection;
}

- (void)_callHandler
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);
    MTRDeviceConnectivityMonitorHandler handlerToCall = self->_monitorHandler;
    if (handlerToCall) {
        dispatch_async(self->_handlerQueue, ^{ handlerToCall(); });
    }
}

- (void)handleResolvedHostname:(const char *)hostName port:(uint16_t)port error:(DNSServiceErrorType)error
{
    std::lock_guard lock(sConnectivityMonitorLock);

    // dns_sd.h: must check and call deallocate if error is kDNSServiceErr_ServiceNotRunning
    if (error == kDNSServiceErr_ServiceNotRunning) {
        MTR_LOG_ERROR("%@ disconnected from dns-sd subsystem", self);
        [self _stopMonitoring];
        return;
    }

    // Create a nw_connection to monitor connectivity if the host name is not being monitored yet
    NSString * hostNameString = [NSString stringWithUTF8String:hostName];
    if (!_connections[hostNameString]) {
        char portString[6];
        snprintf(portString, sizeof(portString), "%d", ntohs(port));
        nw_endpoint_t endpoint = nw_endpoint_create_host(hostName, portString);
        if (!endpoint) {
            MTR_LOG_ERROR("%@ failed to create endpoint for %s:%s", self, hostName, portString);
            return;
        }
        nw_parameters_t params = nw_parameters_create_secure_udp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION);
        if (!params) {
            MTR_LOG_ERROR("%@ failed to create udp parameters", self);
            return;
        }
        nw_connection_t connection = nw_connection_create(endpoint, params);
        if (!connection) {
            MTR_LOG_ERROR("%@ failed to create connection for %s:%s", self, hostName, portString);
            return;
        }
        nw_connection_set_queue(connection, sSharedResolverQueue);
        nw_connection_set_path_changed_handler(connection, ^(nw_path_t _Nonnull path) {
            nw_path_status_t status = nw_path_get_status(path);
            if (status == nw_path_status_satisfied) {
                MTR_LOG_INFO("%@ path is satisfied", self);
                std::lock_guard lock(sConnectivityMonitorLock);
                [self _callHandler];
            }
        });
        nw_connection_set_viability_changed_handler(connection, ^(bool viable) {
            if (viable) {
                std::lock_guard lock(sConnectivityMonitorLock);
                MTR_LOG_INFO("%@ connectivity now viable", self);
                [self _callHandler];
            }
        });
        nw_connection_start(connection);
    }
}

static void _resolveReplyCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char * fullname,
    const char * hosttarget,
    uint16_t port, /* In network byte order */
    uint16_t txtLen,
    const unsigned char * txtRecord,
    void * context)
{
    auto * connectivityMonitor = (__bridge MTRDeviceConnectivityMonitor *) context;
    [connectivityMonitor handleResolvedHostname:hosttarget port:port error:errorCode];
}

- (void)startMonitoringWithHandler:(MTRDeviceConnectivityMonitorHandler)handler queue:(dispatch_queue_t)queue
{
    std::lock_guard lock(sConnectivityMonitorLock);

    MTRDeviceConnectivityMonitorHandler handlerCopy = [handler copy];
    _monitorHandler = handlerCopy;
    _handlerQueue = queue;

    // If there's already a resolver running, just return
    if (_resolver) {
        MTR_LOG_INFO("%@ connectivity monitor updated handler", self);
        return;
    }

    MTR_LOG_INFO("%@ start connectivity monitoring for %@ (%lu monitoring objects)", self, _instanceName, static_cast<unsigned long>(sConnectivityMonitorCount));

    _resolver = [MTRDeviceConnectivityMonitor _sharedResolverConnection];
    if (!_resolver) {
        MTR_LOG_ERROR("%@ failed to get shared resolver connection", self);
        return;
    }
    DNSServiceErrorType dnsError = DNSServiceResolve(&_resolver,
        kDNSServiceFlagsShareConnection,
        kDNSServiceInterfaceIndexAny,
        _instanceName.UTF8String,
        kOperationalType,
        kLocalDot,
        _resolveReplyCallback,
        (__bridge void *) self);
    if (dnsError != kDNSServiceErr_NoError) {
        MTR_LOG_ERROR("%@ failed to create resolver", self);
        return;
    }

    sConnectivityMonitorCount++;
}

#define MTRDEVICECONNECTIVITYMONITOR_SHARED_CONNECTION_LINGER_INTERVAL (10)

- (void)_stopMonitoring
{
    os_unfair_lock_assert_owner(&sConnectivityMonitorLock);
    for (NSString * hostName in _connections) {
        nw_connection_cancel(_connections[hostName]);
    }
    [_connections removeAllObjects];

    if (_resolver) {
        DNSServiceRefDeallocate(_resolver);
        _resolver = NULL;

        // If no monitor objects exist, schedule to deallocate shared connection and queue
        sConnectivityMonitorCount--;
        if (!sConnectivityMonitorCount) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (MTRDEVICECONNECTIVITYMONITOR_SHARED_CONNECTION_LINGER_INTERVAL * NSEC_PER_SEC)), sSharedResolverQueue, ^{
                std::lock_guard lock(sConnectivityMonitorLock);

                if (!sConnectivityMonitorCount) {
                    MTR_LOG_INFO("%@ Closing shared resolver connection", self);
                    DNSServiceRefDeallocate(sSharedResolverConnection);
                    sSharedResolverConnection = NULL;
                    sSharedResolverQueue = nil;
                }
            });
        }
    }
}

- (void)stopMonitoring
{
    MTR_LOG_INFO("%@ stop connectivity monitoring for %@", self, _instanceName);
    std::lock_guard lock(sConnectivityMonitorLock);
    [self _stopMonitoring];
}
@end

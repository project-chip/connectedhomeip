/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <atomic>
#include <openthread/dns_client.h>
#include <openthread/ip6.h>
#include <openthread/sntp.h>

class ThreadTimeSync
{
public:
    static ThreadTimeSync & getInstance();

    bool has_been_sync() { return static_cast<bool>(m_already_sync.load()); }
    void sync_ip(const otIp6Address * server_ipv6, bool force = false);
    void sync_dns(const char * server_name, bool force = false);

    ThreadTimeSync(ThreadTimeSync const &) = delete;
    void operator=(ThreadTimeSync const &) = delete;

private:
    static void on_ntp_ip_response_wrap(otError error, const otDnsAddressResponse * resp, void * ctx);
    static void on_ntp_time_response_wrap(void * ctx, uint64_t time, otError error);

    ThreadTimeSync() : m_in_progress(false), m_already_sync(false) {}
    void sync_ip_internal(const otIp6Address * server_ipv6);

    constexpr static const char * m_unit_name = "ThreadTimeSync";

    std::atomic<int> m_in_progress;
    std::atomic<int> m_already_sync;
    otMessageInfo m_sntp_msg_info;
    const otSntpQuery m_sntp_query = { .mMessageInfo = &m_sntp_msg_info };
};

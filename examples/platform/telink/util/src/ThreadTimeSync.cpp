/*
 *
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

#include "ThreadTimeSync.h"
#include <ctime>
#include <system/SystemClock.h>
#include <zephyr/net/openthread.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ThreadTimeSync, CONFIG_CHIP_APP_LOG_LEVEL);

ThreadTimeSync & ThreadTimeSync::getInstance()
{
    static ThreadTimeSync instance;

    return instance;
}

void ThreadTimeSync::sync_ip(const otIp6Address * server_ipv6, bool force)
{
    if (m_already_sync.load() && !force)
    {
        return;
    }

    int expected_in_process = false;

    if (!m_in_progress.compare_exchange_strong(expected_in_process, true))
    {
        LOG_DBG("[%s] operation in process", m_unit_name);
        return;
    }

    sync_ip_internal(server_ipv6);
}

void ThreadTimeSync::sync_dns(const char * server_name, bool force)
{
    if (m_already_sync.load() && !force)
    {
        return;
    }

    int expected_in_process = false;

    if (!m_in_progress.compare_exchange_strong(expected_in_process, true))
    {
        LOG_DBG("[%s] operation in process", m_unit_name);
        return;
    }

    LOG_DBG("[%s] sync_dns %s", m_unit_name, server_name);
    otError error =
        otDnsClientResolveIp4Address(openthread_get_default_instance(), server_name, on_ntp_ip_response_wrap, this, nullptr);

    if (error != OT_ERROR_NONE)
    {
        m_in_progress.store(false);
        LOG_WRN("[%s] otDnsClientResolveIp4Address request failed %u", m_unit_name, error);
    }
}

void ThreadTimeSync::sync_ip_internal(const otIp6Address * server_ipv6)
{
    char server_ipv6_str[OT_IP6_ADDRESS_STRING_SIZE];

    otIp6AddressToString(server_ipv6, server_ipv6_str, sizeof(server_ipv6_str));
    LOG_DBG("[%s] sync_ip %s", m_unit_name, server_ipv6_str);
    memset(&m_sntp_msg_info, 0, sizeof(m_sntp_msg_info));
    m_sntp_msg_info.mPeerAddr = *server_ipv6;
    m_sntp_msg_info.mPeerPort = 123;

    otError error = otSntpClientQuery(openthread_get_default_instance(), &m_sntp_query, on_ntp_time_response_wrap, this);

    if (error != OT_ERROR_NONE)
    {
        m_in_progress.store(false);
        LOG_WRN("[%s] otSntpClientQuery request failed %u", m_unit_name, error);
    }
}

void ThreadTimeSync::on_ntp_ip_response_wrap(otError error, const otDnsAddressResponse * resp, void * ctx)
{
    ThreadTimeSync * thread_time_sync = static_cast<ThreadTimeSync *>(ctx);

    if (error == OT_ERROR_NONE)
    {
        otIp6Address addr;
        if (otDnsAddressResponseGetAddress(resp, 0, &addr, nullptr) == OT_ERROR_NONE)
        {
            thread_time_sync->sync_ip_internal(&addr);
        }
        else
        {
            thread_time_sync->m_in_progress.store(false);
            LOG_WRN("[%s] otDnsClientResolveIp4Address response no address", m_unit_name);
        }
    }
    else
    {
        thread_time_sync->m_in_progress.store(false);
        LOG_WRN("[%s] otDnsClientResolveIp4Address response failed %u", m_unit_name, error);
    }
}

void ThreadTimeSync::on_ntp_time_response_wrap(void * ctx, uint64_t time, otError error)
{
    ThreadTimeSync * thread_time_sync = static_cast<ThreadTimeSync *>(ctx);

    if (error == OT_ERROR_NONE)
    {
        chip::System::Clock::Microseconds64 chip_time = chip::System::Clock::Seconds64(time);

        if (chip::System::SystemClock().SetClock_RealTime(chip_time) != CHIP_NO_ERROR)
        {
            LOG_ERR("[%s] SetClock_RealTime failed", m_unit_name);
        }
        thread_time_sync->m_already_sync.store(true);
        thread_time_sync->m_in_progress.store(false);
        LOG_INF("[%s] SNTP time sync timestamp %llu", m_unit_name, time);

        time_t ts = time;
        struct tm info;
        char tm_str[40];

        localtime_r(&ts, &info);
        strftime(tm_str, sizeof(tm_str), "%A %d %B %Y %H:%M:%S", &info);
        LOG_INF("[%s] SNTP sync at %s", m_unit_name, tm_str);
    }
    else
    {
        thread_time_sync->m_in_progress.store(false);
        LOG_WRN("[%s] otSntpClientQuery response failed %u", m_unit_name, error);
    }
}

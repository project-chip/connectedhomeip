/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {
namespace Dnssd {

struct DnssdService;

/**
 * A delegate that can be notified of service additions/removals as a mdns browse proceeds.
 */
class DLL_EXPORT DnssdBrowseDelegate
{
public:
    virtual ~DnssdBrowseDelegate() {}

    /**
     * @brief
     *   Called when a service is added.
     *
     * @param[in] service The service.
     */
    virtual void OnBrowseAdd(DnssdService service) = 0;

    /**
     * @brief
     *   Called when a service is removed.
     *
     * @param[in] service The service.
     */
    virtual void OnBrowseRemove(DnssdService service) = 0;

    /**
     * @brief
     *   Called when the browse stops.
     *
     * @param error Error cause, if any
     */
    virtual void OnBrowseStop(CHIP_ERROR error) = 0;
};

} // namespace Dnssd
} // namespace chip

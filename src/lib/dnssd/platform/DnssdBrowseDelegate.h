/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>

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

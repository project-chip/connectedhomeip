/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "pw_status/status.h"
#include <app/AttributeReportBuilder.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>

namespace chip {
namespace rpc {

/**
 * Callback class that clusters can implement in order to interpose custom
 * interception logic.
 */
class PigweedDebugAccessInterceptor
{
public:
    PigweedDebugAccessInterceptor()          = default;
    virtual ~PigweedDebugAccessInterceptor() = default;

    /**
     * Callback for writing attributes.
     *
     * The implementation can do one of three things:
     *
     * Returns:
     *   - `std::nullopt` if the `path` was not handled by this Interceptor.
     *     Interceptor MUST NOT have attepted to decode `decoder`.
     *   - A `::pw::Status` value that is considered the FINAL result of the
     *     write (i.e. write handled) either with success or failure.
     */
    virtual std::optional<::pw::Status> Write(const chip::app::ConcreteDataAttributePath & path,
                                              chip::app::AttributeValueDecoder & decoder) = 0;
};

} // namespace rpc
} // namespace chip

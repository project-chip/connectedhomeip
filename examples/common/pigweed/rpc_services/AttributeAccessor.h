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

/**
 * Callback class that clusters can implement in order to interpose custom
 * attribute-handling logic.  An AttributeAccessor instance is associated
 * with some specific cluster.  A single instance may be used for a specific
 * endpoint or for all endpoints.
 *
 * Instances of AttributeAccessor that are registered via
 * AttributeAccessorRegistry::Instance().Register will be consulted before taking the
 * normal attribute access codepath and can use that codepath as a fallback if desired.
 */
namespace chip {
namespace rpc {

class AttributeAccessor
{
public:
    AttributeAccessor() = default;
    virtual ~AttributeAccessor() {}

    /**
     * Callback for reading attributes.
     *
     * @param [in] aPath indicates which exact data is being read.
     * @param [in] aEncoder the AttributeValueEncoder to use for encoding the
     *             data.
     *
     * The implementation can do one of three things:
     *
     * 1) Return a failure.  This is treated as a failed read and the error is
     *    returned to the client. Caller will not look for alternatives
     *    paths to read when any of the accessors returns a failure.
     * 2) Return not found. This implies the accessor does not handle read to
     *    the specified attribute. Caller can look for another accessor or
     *    use the fallback path in when a "not found" is returned.
     * 3) Return success and attempt to encode data using aEncoder.  The data is
     *    returned to the client. Fallback path will not be taken.
     */
    virtual ::pw::Status Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder)
    {
        return ::pw::Status::NotFound();
    }

    /**
     * Callback for writing attributes.
     *
     * @param [in] aPath indicates which exact data is being written.
     * @param [in] aDecoder the AttributeValueDecoder to use for decoding the
     *             data.
     *
     * The implementation can do one of three things:
     *
     * 1) Return a failure. This is treated as a failed write and the error is
     *    sent to the client. Caller will not look for alternatives
     *    paths to write when any of the accessors returns a failure.
     * 2) Return not found. This implies the accessor does not handle write to
     *    the specified attribute. Caller can look for another accessor or
     *    use the fallback path in when a "not found" is returned.
     * 3) Return success and attempt to decode from aDecoder.  This is
     *    treated as a successful write. Caller will treat this as a successful
     *    write and report to client. Fallback path will not be taken.
     */
    virtual ::pw::Status Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder)
    {
        return ::pw::Status::NotFound();
    }
};

} // namespace rpc
} // namespace chip

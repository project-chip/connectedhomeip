/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Contains the API definition for a message buffer reader for the data
 *          model.  This reader does the necessary bounds-checks before reading
 *          and updates its own state as the buffer is read.
 */

#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <stdint.h>

namespace chip {

class DataModelReader
{
public:
    /**
     * Create a data model reader from a given buffer and length.
     *
     * @param buffer The octet buffer to read from.  The caller must ensure
     *               (most simply by allocating the reader on the stack) that
     *               the buffer outlives the reader.  The buffer is allowed to
     *               be null if buf_len is 0.
     * @param buf_len The number of octets in the buffer.
     */
    DataModelReader(const uint8_t * buffer, uint16_t buf_len) : mReader(buffer, buf_len) {}

    /**
     * Number of octets we have read so far.  This might be able to go away once
     * we do less switching back and forth between DataModelReader and raw
     * buffers.
     */
    size_t OctetsRead() const { return mReader.OctetsRead(); }

    /**
     * The reader status.
     */
    CHIP_ERROR StatusCode() const { return mReader.StatusCode(); }

    /**
     * Read a cluster id.
     *
     * @param [out] cluster_id Where the cluster id goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     */
    CHECK_RETURN_VALUE DataModelReader & ReadClusterId(ClusterId * cluster_id)
    {
        mReader.RawRead(cluster_id);
        return *this;
    }

    /**
     * Read an endpoint id.
     *
     * @param [out] endpoint_id Where the endpoint id goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     */
    CHECK_RETURN_VALUE DataModelReader & ReadEndpointId(EndpointId * endpoint_id)
    {
        mReader.RawRead(endpoint_id);
        return *this;
    }

    /**
     * Read a group id.
     *
     * @param [out] group_id Where the group id goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     */
    CHECK_RETURN_VALUE DataModelReader & ReadGroupId(GroupId * group_id)
    {
        mReader.RawRead(group_id);
        return *this;
    }

    /**
     * Read a single octet.
     *
     * @param [out] octet Where the octet goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     *
     * @note Use of APIs that read some semantically-meaningful type is preferred.
     */
    CHECK_RETURN_VALUE DataModelReader & ReadOctet(uint8_t * octet)
    {
        mReader.RawRead(octet);
        return *this;
    }

    /**
     * Read a single 16-bit unsigned integer.
     *
     * @param [out] dest Where the 16-bit integer goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     *
     * @note Use of APIs that read some semantically-meaningful type is preferred.
     */
    CHECK_RETURN_VALUE DataModelReader & Read16(uint16_t * dest)
    {
        mReader.RawRead(dest);
        return *this;
    }

private:
    Encoding::LittleEndian::Reader mReader;
};

} // namespace chip

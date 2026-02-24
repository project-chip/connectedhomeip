/*
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

#include "UboxUtils.h"

#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace ubus {

namespace detail {
int BlobMsgType<ByteSpan>::get(blob_attr * attr, ByteSpan & value)
{
    char * str     = blobmsg_get_string(attr);
    size_t len     = strlen(str);
    uint8_t * bin  = reinterpret_cast<uint8_t *>(str); // decode in-place
    size_t decoded = Encoding::HexToBytes(str, len, bin, len);
    VerifyOrReturnValue(len == 0 || decoded > 0, false);
    value = ByteSpan(bin, decoded);
    return 0;
}

int BlobMsgType<ByteSpan>::add(blob_buf * buf, const char * name, ByteSpan const & value)
{
    unsigned len = HEX_ENCODED_LENGTH(value.size()) + 1 /* for null termination */;
    char * str   = static_cast<char *>(blobmsg_alloc_string_buffer(buf, name, len));
    VerifyOrReturnValue(str != nullptr, -1);
    SuccessOrDie(Encoding::BytesToLowercaseHexString(value.data(), value.size(), str, len));
    blobmsg_add_string_buffer(buf);
    return 0;
}
} // namespace detail

bool BlobMsgBuf::AddFormat(const char * name, const char * format, ...)
{
    VerifyOrReturnValue(!HasError(), false);
    va_list args;
    va_start(args, format);
    int status = blobmsg_vprintf(this, name, format, args);
    va_end(args);
    return Check(status);
}

} // namespace ubus
} // namespace chip

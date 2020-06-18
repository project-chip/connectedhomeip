/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements the CHIP Security Header object.
 *
 */

#include <core/CHIPEncoding.h>
#include <support/CodeUtils.h>
#include <transport/SecurityHeader.h>

#include <string.h>

namespace chip {

CHIP_ERROR SecurityHeader::Serialize(void * out, size_t & len)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    security_header_t * header = (security_header_t *) out;
    VerifyOrExit(len >= sizeof(security_header_t), err = CHIP_ERROR_INVALID_ARGUMENT);
    chip::Encoding::LittleEndian::Put16((uint8_t *) &header->mVersion, mHeader.mVersion);
    chip::Encoding::LittleEndian::Put16((uint8_t *) &header->mType, mHeader.mType);
    chip::Encoding::LittleEndian::Put32((uint8_t *) &header->mSessionID, mHeader.mSessionID);
    memcpy(&header->mIV, &mHeader.mIV, sizeof(header->mIV));
    memcpy(&header->mTag, &mHeader.mTag, sizeof(header->mTag));
    len = sizeof(security_header_t);
exit:
    return err;
}

CHIP_ERROR SecurityHeader::Deserialize(const void * in, size_t len)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    const security_header_t * header = (const security_header_t *) in;
    VerifyOrExit(len >= sizeof(security_header_t), err = CHIP_ERROR_INVALID_ARGUMENT);

    mHeader.mVersion = chip::Encoding::LittleEndian::Get16((uint8_t *) &header->mVersion);
    // In future, we need to support older versions for backward compatibility
    VerifyOrExit(mHeader.mVersion == kHeaderVersion, err = CHIP_ERROR_VERSION_MISMATCH);

    mHeader.mType      = chip::Encoding::LittleEndian::Get16((uint8_t *) &header->mType);
    mHeader.mSessionID = chip::Encoding::LittleEndian::Get32((uint8_t *) &header->mSessionID);
    memcpy(&mHeader.mIV, &header->mIV, sizeof(mHeader.mIV));
    memcpy(&mHeader.mTag, &header->mTag, sizeof(mHeader.mTag));
exit:
    return err;
}

} // namespace chip

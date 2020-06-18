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
 *      This file defines the CHIP Security Header object.
 *
 */

#ifndef __SECURITYHEADER_H__
#define __SECURITYHEADER_H__

#include <core/CHIPCore.h>

namespace chip {

class DLL_EXPORT SecurityHeader
{
public:
    SecurityHeader() { mHeader.mVersion = kHeaderVersion; }

    /** Set the header type for this header. */
    SecurityHeader & SetType(uint16_t type)
    {
        mHeader.mType = type;
        return *this;
    }

    /** Set the security session ID for this header. */
    SecurityHeader & SetSessionID(uint32_t id)
    {
        mHeader.mSessionID = id;
        return *this;
    }

    /** Set the initialization vector for this header. */
    SecurityHeader & SetIV(uint64_t IV)
    {
        mHeader.mIV = IV;
        return *this;
    }

    /** Set the message auth tag for this header. */
    SecurityHeader & SetTag(uint64_t tag)
    {
        mHeader.mTag = tag;
        return *this;
    }

    /** Get the header type from this header. */
    uint16_t Type(void) { return mHeader.mType; }

    /** Get the Session ID from this header. */
    uint32_t SessionID(void) { return mHeader.mSessionID; }

    /** Get the initialization vector from this header. */
    uint64_t IV(void) { return mHeader.mIV; }

    /** Get the message auth tag from this header. */
    uint64_t Tag(void) { return mHeader.mTag; }

    /** Get the raw header bytes. */
    const unsigned char * RawHeader(void) { return (const unsigned char *) &mHeader; }

    /** Get the length of raw header */
    static size_t RawHeaderLen(void) { return sizeof(security_header_t); }

    CHIP_ERROR Serialize(void * out, size_t & len);

    CHIP_ERROR Deserialize(const void * in, size_t len);

private:
    /// Represents the current security header version
    static constexpr uint16_t kHeaderVersion = 2;

    /**
     * @param mVersion   Security header version
     * @param mType      Packet type (application data, security control packets, e.g. pairing, configuration, rekey etc)
     * @param mSessionID Security session identifier
     * @param mIV        Initialization vector used for encryption of the packet
     * @param mTag       Message authentication tag generated at encryption
     */
    typedef struct
    {
        uint16_t mVersion;
        uint16_t mType;
        uint32_t mSessionID;
        uint64_t mIV;
        uint64_t mTag;
    } security_header_t;

    security_header_t mHeader;
};

} // namespace chip

#endif // __SECURITYHEADER_H__

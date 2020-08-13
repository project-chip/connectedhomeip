/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines a class for managing and manipulating a Host
 *      Port List, a compact, binary-encoded collection of host and port
 *      identifier tuples associated with the CHIP Service Directory.
 *
 */

#ifndef HOSTPORTLIST_H_
#define HOSTPORTLIST_H_

#include <stdint.h>

#include <core/CHIPError.h>

namespace chip {

/**
 *  @class HostPortList
 *
 *  @brief
 *    For managing and manipulating a Host Port List, a compact,
 *    binary-encoded collection of host and port identifier tuples
 *    associated with the CHIP Service Directory.
 *
 */
class HostPortList
{
public:
    HostPortList(void);
    inline HostPortList(const uint8_t * hostPortList, uint8_t hostPortCount, const uint8_t * suffixList, const uint8_t suffixCount)
    {
        mElements    = hostPortList;
        mCount       = hostPortCount;
        mSuffixTable = suffixList;
        mSuffixCount = suffixCount;
    }

    CHIP_ERROR Get(uint8_t index, char * hostBuf, uint32_t hostBufSize, uint16_t & port) const;
    CHIP_ERROR Pop(char * hostBuf, uint32_t hostBufSize, uint16_t & port);

    void Clear(void);

    inline bool IsEmpty(void) const { return (mCount == 0); }

private:
    enum
    {
        kHostPortControl_TypeMask       = 0x03, ///< Mask for the type of entry flags.
        kHostPortControl_HasSuffixIndex = 0x04, ///< Flag indicating whether the host port tuple has an associated suffix index.
        kHostPortControl_HasPort        = 0x08, ///< Flag indicating whether the host port tuple has a port number.

        kHostPortType_FullyQualified = 0x00, ///< Flag indicating whether the host name is fully-qualified.
        kHostPortType_Indexed        = 0x01, ///< Flag indicating whether the host name is index (i.e. has a suffix).
    };

    CHIP_ERROR Get(const uint8_t *& elem, char * hostBuf, uint32_t hostBufSize, uint16_t & port) const;
    CHIP_ERROR Skip(const uint8_t *& elem) const;
    CHIP_ERROR GetSuffix(uint8_t index, char * buf, uint32_t bufSize, uint8_t & suffixLen) const;

private:
    const uint8_t * mElements;    ///< A read-only pointer to the host and port tuples list.
    const uint8_t * mSuffixTable; ///< A read-only pointer to the host suffix list.
    uint8_t mCount;               ///< The number of host and port tuples in the list.
    uint8_t mSuffixCount;         ///< The number of host suffixes in the list.
};

} /* namespace chip */

#endif /* HOSTPORTLIST_H_ */

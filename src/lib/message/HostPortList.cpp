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
 *      This file implements a class for managing and manipulating a Host
 *      Port List, a compact, binary-encoded collection of host and port
 *      identifier tuples associated with the CHIP Service Directory.
 *
 */

#include "HostPortList.h"

#include <core/CHIPEncoding.h>
#include <support/CodeUtils.h>

namespace chip {

using namespace chip::Encoding;

/**
 *  Class default (void) constructor.
 *
 */
HostPortList::HostPortList(void)
{
    Clear();
}

/**
 *  Reset the list to empty.
 *
 */
void HostPortList::Clear(void)
{
    mElements = mSuffixTable = NULL;
    mCount = mSuffixCount = 0;
}

/**
 *  Get the host name and port at the specified index from the list.
 *
 *  @param[in]     index        The position in the list to return the host
 *                              name and port.
 *
 *  @param[inout]  hostBuf      A pointer to storage into which to copy the
 *                              host name.
 *
 *  @param[in]     hostBufSize  The amount of storage available in hostBuf.
 *
 *  @param[out]    port         The port number.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT               The requested index
 *                                                       exceeds the size of
 *                                                       the list.
 *  @retval  #CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT  The Host Port element
 *                                                       requested was an
 *                                                       unsupported type.
 *  @retval  #CHIP_ERROR_BUFFER_TOO_SMALL               The Host Port host
 *                                                       name length
 *                                                       is longer
 *                                                       than the
 *                                                       provided
 *                                                       buffer.
 *  @retval  #CHIP_NO_ERROR                             On success.
 */
CHIP_ERROR HostPortList::Get(uint8_t index, char * hostBuf, uint32_t hostBufSize, uint16_t & port) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    const uint8_t * elem;

    VerifyOrExit(index < mCount, err = CHIP_ERROR_INVALID_ARGUMENT);

    elem = mElements;
    for (; index > 0; index--)
    {
        err = Skip(elem);
        SuccessOrExit(err);
    }

    err = Get(elem, hostBuf, hostBufSize, port);

exit:
    return err;
}

/**
 *  Get and remove the first host name and port from the list.
 *
 *  @param[inout]  hostBuf      A pointer to storage into which to copy the
 *                              host name.
 *
 *  @param[in]     hostBufSize  The amount of storage available in hostBuf.
 *
 *  @param[out]    port         The port number.
 *
 *  @retval  #CHIP_ERROR_HOST_PORT_LIST_EMPTY           There are no entries
 *                                                       in the list.
 *  @retval  #CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT  The Host Port element
 *                                                       encountered was an
 *                                                       unsupported type.
 *  @retval  #CHIP_ERROR_BUFFER_TOO_SMALL               The Host Port host
 *                                                       name length
 *                                                       is longer
 *                                                       than the
 *                                                       provided
 *                                                       buffer.
 *  @retval  #CHIP_NO_ERROR                             On success.
 */
CHIP_ERROR HostPortList::Pop(char * hostBuf, uint32_t hostBufSize, uint16_t & port)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mCount > 0, err = CHIP_ERROR_HOST_PORT_LIST_EMPTY);

    err = Get(mElements, hostBuf, hostBufSize, port);
    if (err == CHIP_NO_ERROR)
        mCount--;

exit:
    return err;
}

/**
 *  Get the host name and port at the specified element.
 *
 *  @param[inout]  elem         The host name and port element to get. The
 *                              pointer will be advanced to the next
 *                              element on success and may be left in
 *                              an indeterminate state on failure.
 *
 *  @param[inout]  hostBuf      A pointer to storage into which to copy the
 *                              host name.
 *
 *  @param[in]     hostBufSize  The amount of storage available in hostBuf.
 *
 *  @param[out]    port         The port number.
 *
 *  @retval  #CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT  The Host Port element
 *                                                       encountered was an
 *                                                       unsupported type.
 *  @retval  #CHIP_ERROR_BUFFER_TOO_SMALL               The Host Port host
 *                                                       name length
 *                                                       is longer
 *                                                       than the
 *                                                       provided
 *                                                       buffer.
 *  @retval  #CHIP_NO_ERROR                             On success.
 */
CHIP_ERROR HostPortList::Get(const uint8_t *& elem, char * hostBuf, uint32_t hostBufSize, uint16_t & port) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t control, type, hostLen;

    control = *elem++;
    type    = control & kHostPortControl_TypeMask;
    VerifyOrExit(type == kHostPortType_FullyQualified || type == kHostPortType_Indexed,
                 err = CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT);

    hostLen = *elem++;
    VerifyOrExit(hostLen < hostBufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(hostBuf, elem, hostLen);
    elem += hostLen;

    if ((control & kHostPortControl_HasSuffixIndex) != 0)
    {
        if (type == kHostPortType_Indexed)
        {
            uint8_t suffixLen;
            err = GetSuffix(*elem, hostBuf + hostLen, hostBufSize - hostLen, suffixLen);
            SuccessOrExit(err);

            hostLen += suffixLen;
        }

        elem++;
    }

    hostBuf[hostLen] = 0;

    if ((control & kHostPortControl_HasPort) != 0)
        port = LittleEndian::Read16(elem);
    else
        port = CHIP_PORT;

exit:
    return err;
}

/**
 *  Skip the host name and port at the specified element.
 *
 *  @param[inout]  elem  The host name and port element to skip. The pointer
 *                       will be advanced to the next element on success and
 *                       may be left in an indeterminate state on failure.
 *
 *  @retval  #CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT  The Host Port element
 *                                                       encountered was an
 *                                                       unsupported type.
 *  @retval  #CHIP_NO_ERROR                             On success.
 */
CHIP_ERROR HostPortList::Skip(const uint8_t *& elem) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t control, type, hostLen;

    control = *elem++;
    type    = control & kHostPortControl_TypeMask;
    VerifyOrExit(type == kHostPortType_FullyQualified || type == kHostPortType_Indexed,
                 err = CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT);

    hostLen = *elem++;
    elem += hostLen;

    if ((control & kHostPortControl_HasSuffixIndex) != 0)
        elem++;

    if ((control & kHostPortControl_HasPort) != 0)
        elem += 2;

exit:
    return err;
}

/**
 *  Get the Host Port element host name suffix (e.g. ".acme.com") at the
 *  specified position in the list.
 *
 *  @param[in]     index      The position in the list to return the host
 *                            name suffix.
 *
 *  @param[inout]  buf        A pointer to storage into which to copy the
 *                            host name suffix.
 *
 *  @param[in]     bufSize    The amount of storage available in buf.
 *
 *  @param[out]    suffixLen  The length, in characters, of the host name
 *                            suffix.
 *
 *  @retval  #CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX  The requested index
 *                                                   exceeds the size of the
 *                                                   list.
 *  @retval  #CHIP_ERROR_BUFFER_TOO_SMALL           The Host Port host
 *                                                   suffix name length is
 *                                                   longer than the provided
 *                                                   buffer.
 *  @retval  #CHIP_NO_ERROR                         On success.
 *
 */
CHIP_ERROR HostPortList::GetSuffix(uint8_t index, char * buf, uint32_t bufSize, uint8_t & suffixLen) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    const uint8_t * elem;

    VerifyOrExit(index < mSuffixCount, err = CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX);

    elem = mSuffixTable;
    for (; index > 0; index--)
        elem += 1 + elem[0];

    suffixLen = *elem++;
    VerifyOrExit(suffixLen < bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(buf, elem, suffixLen);
    buf[suffixLen] = 0;

exit:
    return err;
}

} /* namespace chip */

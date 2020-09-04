/*
 *
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
 *      This file implements a base class that serves as a convenience
 *      object for automatically reference counting a System::PacketBuffer.
 *
 */

#include "CHIPMessage.h"

namespace chip {
namespace Protocols {

/**
 *  This is the class default (void) constructor.
 *
 */
RetainedPacketBuffer::RetainedPacketBuffer(void)
{
    mBuffer = NULL;
}

/**
 *  This is a class copy constructor. It increases the reference
 *  count, creating a strong reference to the buffer associated with
 *  the copied object.
 *
 *  @param[in]  aRetainedPacketBuffer  A constant reference to the object to be
 *                                     copied.
 *
 */
RetainedPacketBuffer::RetainedPacketBuffer(const RetainedPacketBuffer &aRetainedPacketBuffer)
{
    mBuffer = NULL;

    Retain(aRetainedPacketBuffer.mBuffer);
}

/**
 *  This is the class destructor. It removes the strong reference to
 *  the associated buffer.
 *
 */
RetainedPacketBuffer::~RetainedPacketBuffer(void)
{
    Release();
}

/**
 *  This is a class assignment operator. As long as the assigned
 *  object is not the current object, this creates a strong reference
 *  to the buffer associated with the copied object while, if
 *  necessary, removing the strong reference to the buffer associated
 *  with this object.
 *
 *  @param[inout]  aRetainedPacketBuffer  A read-only reference to the object
 *                                        to assign to this one. If
 *                                        the reference is not this
 *                                        object, the source object's
 *                                        associated buffer is
 *                                        retained, displacing the
 *                                        currently associated buffer.
 *
 *  @return a read-only reference to the current object.
 *
 */
RetainedPacketBuffer &RetainedPacketBuffer::operator =(const RetainedPacketBuffer &aRetainedPacketBuffer)
{
    if (this != &aRetainedPacketBuffer)
        Retain(aRetainedPacketBuffer.mBuffer);

    return *this;
}

/**
 *  Verify whether or not this object is retaining a buffer.
 *
 *  @return @p true if the object is retaining a buffer; otherwise, @p
 *  false.
 *
 */
bool RetainedPacketBuffer::IsRetaining(void) const
{
    return (mBuffer != NULL);
}

/**
 *  Create a strong reference to the specified packet buffer and, if
 *  necessary, displace and remove the strong reference to another
 *  buffer associated with this object.
 *
 */
void RetainedPacketBuffer::Retain(System::PacketBuffer *aBuffer)
{
    if (aBuffer != NULL)
    {
        aBuffer->AddRef();
    }

    if (mBuffer != NULL)
    {
        System::PacketBuffer::Free(mBuffer);
    }

    mBuffer = aBuffer;
}

/**
 *  Remove the strong reference to the buffer associated with the
 *  object, making this object available to retain another buffer.
 *
 */
void RetainedPacketBuffer::Release(void)
{
    Retain(NULL);
}

} // namespace Protocols
} // namespace chip

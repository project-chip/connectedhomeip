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
 *      This file implements objects commonly used for the
 *      processing of Weave messages.
 *
 */

#include "CHIPMessage.h"
#include <core/CHIPCore.h>

using namespace chip;
using namespace chip::TLV;
using namespace chip::Protocols;

//-------------------- definitions for the message iterator --------------------

/**
 * The constructor method.
 *
 * @param aBuffer  A message buffer to iterate over.
 */

MessageIterator::MessageIterator(PacketBuffer * aBuffer)
{
    Retain(aBuffer);

    thePoint = aBuffer->Start();
}

/**
 * @param aDestination  A place to put a byte read off the buffer.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::readByte(uint8_t * aDestination)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasData(1))
    {
        err = CHIP_NO_ERROR;

        *aDestination = READBYTE(thePoint);
    }

    return err;
}

/**
 * @param aDestination  A place to put a short read off the buffer.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::read16(uint16_t * aDestination)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasData(2))
    {
        err = CHIP_NO_ERROR;

        READ16(thePoint, *aDestination);
    }

    return err;
}

/**
 * @param aDestination  A place to put a 32-bit value read off the buffer.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::read32(uint32_t * aDestination)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasData(4))
    {
        err = CHIP_NO_ERROR;

        READ32(thePoint, *aDestination);
    }

    return err;
}

/**
 * @param aDestination  A place to put a 64-bit value read off the buffer.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::read64(uint64_t * aDestination)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;
    uint8_t * p;

    if (hasData(8))
    {
        err = CHIP_NO_ERROR;
        p   = (uint8_t *) aDestination;

        for (int i = 0; i < 8; i++)
            readByte(p++);
    }

    return err;
}

/**
 * @param aLength  The length of the string to be read.
 * @param aString  A place to put the string.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::readString(uint16_t aLength, char * aString)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasData(aLength))
    {
        err = CHIP_NO_ERROR;

        for (uint16_t i = 0; i < aLength; i++)
        {
            *aString = READBYTE(thePoint);
            aString++;
        }
    }

    return err;
}

/**
 * @param aLength      The length of the byte string to be read.
 * @param aByteString  A place to put the bytes.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::readBytes(uint16_t aLength, uint8_t * aByteString)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasData(aLength))
    {
        err = CHIP_NO_ERROR;

        for (uint16_t i = 0; i < aLength; i++)
        {
            *aByteString = READBYTE(thePoint);
            aByteString++;
        }
    }

    return err;
}

/**
 * @param aValue  A byte value to write out.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::writeByte(uint8_t aValue)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasRoom(1))
    {
        err = CHIP_NO_ERROR;

        WRITEBYTE(thePoint, aValue);
        finishWriting();
    }

    return err;
}

/**
 * @param aValue  A short value to write out.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::write16(uint16_t aValue)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasRoom(2))
    {
        err = CHIP_NO_ERROR;

        WRITE16(thePoint, aValue);
        finishWriting();
    }

    return err;
}

/**
 * @param aValue  A 32-bit value to write out.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::write32(uint32_t aValue)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasRoom(4))
    {
        err = CHIP_NO_ERROR;

        WRITE32(thePoint, aValue);
        finishWriting();
    }

    return err;
}

/**
 * @param aValue  A 64-bit value to write out.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::write64(uint64_t aValue)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;
    uint8_t * p    = (uint8_t *) &aValue;

    if (hasRoom(8))
    {
        err = CHIP_NO_ERROR;

        for (int i = 0; i < 8; i++)
            writeByte(*p++);
    }

    return err;
}

/**
 * @param aLength  The length of the string to write.
 * @param aString  The string itself.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::writeString(uint16_t aLength, char * aString)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasRoom(aLength))
    {
        err = CHIP_NO_ERROR;

        for (uint16_t i = 0; i < aLength; i++)
        {
            WRITEBYTE(thePoint, *aString);
            aString++;
        }
        finishWriting();
    }

    return err;
}

/**
 * @param aLength  The length of the byte string to write.
 * @param aString  The byte string itself.
 *
 * @retval CHIP_NO_ERROR               If it's all OK.
 * @retval CHIP_ERROR_BUFFER_TOO_SMALL If we're running past the end of the buffer.
 */

CHIP_ERROR MessageIterator::writeBytes(uint16_t aLength, uint8_t * aByteString)
{
    CHIP_ERROR err = CHIP_ERROR_BUFFER_TOO_SMALL;

    if (hasRoom(aLength))
    {
        err = CHIP_NO_ERROR;

        for (uint16_t i = 0; i < aLength; i++)
        {
            WRITEBYTE(thePoint, *aByteString);
            aByteString++;
        }
        finishWriting();
    }

    return err;
}

/**
 * Increment a message iterator by 1 if there's room.
 */

MessageIterator & MessageIterator::operator++(void)
{
    if (hasRoom(1))
        ++thePoint;

    return *this;
}

/**
 * @param inc  An increment to apply to the message iterator.
 *
 * @return         The iterator incremented by the given value if there's
 *                 room, or else slammed right up against the end if there's not.
 */

MessageIterator & MessageIterator::operator+(uint16_t inc)
{
    if (hasRoom(inc))
        thePoint += inc;

    else
        thePoint += mBuffer->AvailableDataLength();

    return *this;
}

/**
 * @param dec  A decrement to apply to the message iterator.
 *
 * @return         The iterator decremented by the given value if there's
 *                 room, or else slammed right up against the beginning if there's not.
 */

MessageIterator & MessageIterator::operator-(uint16_t dec)
{
    if (mBuffer->DataLength() > dec)
        thePoint -= dec;
    else
        thePoint = mBuffer->Start();
    return *this;
}

/**
 * @param aMessageIterator  Another message iterator to compare with.
 */

bool MessageIterator::operator==(const MessageIterator & aMessageIterator)
{
    return (thePoint == aMessageIterator.thePoint && mBuffer == aMessageIterator.mBuffer);
}

/**
 * @param aMessageIterator  Another message iterator to compare with.
 */

bool MessageIterator::operator!=(const MessageIterator & aMessageIterator)
{
    return !(thePoint == aMessageIterator.thePoint && mBuffer == aMessageIterator.mBuffer);
}

/**
 * @return  What we're looking at in the buffer.
 */

uint8_t & MessageIterator::operator*(void)
{
    return *thePoint;
}

/**
 * Set the point to after any data currently in the buffer.
 */

void MessageIterator::append(void)
{
    thePoint = mBuffer->Start() + mBuffer->DataLength();
}

/**
 * @param inc  An integer amount that may be read from the
 *             buffer.
 *
 * @retval true  The buffer's current data length is greater than or equal
 *               to the given increment.
 * @retval false Otherwise.
 */

bool MessageIterator::hasData(uint16_t inc)
{
    return (thePoint + inc) <= (mBuffer->Start() + mBuffer->DataLength());
}

/**
 * @param inc  An integer amount that may be written to the
 *             buffer.
 *
 * @retval true  The difference between the buffer's current data
 *               length and its maximum allowable data length, (its available data
 *               length), is less than or equal to the given increment.
 * @retval false Otherwise.
 */

bool MessageIterator::hasRoom(uint16_t inc)
{
    return inc <= (mBuffer->AvailableDataLength());
}

/**
 * Adjust the buffer after writing.
 */

void MessageIterator::finishWriting(void)
{
    mBuffer->SetDataLength((uint16_t)(thePoint - mBuffer->Start()));
}

// -------------------- definitions for referenced strings --------------------

/**
 * The no-arg constructor for referenced strings.
 */

ReferencedString::ReferencedString(void) : RetainedPacketBuffer()
{
    theLength = 0;
    theString = NULL;
    isShort   = false;
}

/***
 * @fn CHIP_ERROR ReferencedString::init(uint16_t aLength, char *aString, System::PacketBuffer *aBuffer)
 * @brief  Initialize a ReferencedString
 *
 * Initialize the ReferencedString with a string and a PacketBuffer backing a that string.
 *
 * @param[in] aLength A length for the referenced string
 * @param[in] aString A pointer to the string data (in the buffer)
 * @param[in] aBuffer A messsage buffer in which the string resides
 *
 * @retval #CHIP_NO_ERROR On success
 * @retval #CHIP_ERROR_INVALID_STRING_LENGTH The supplied string is too long
 */

CHIP_ERROR ReferencedString::init(uint16_t aLength, char * aString, System::PacketBuffer * aBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aLength > (aBuffer->AvailableDataLength() - aBuffer->DataLength()))
        err = CHIP_ERROR_INVALID_STRING_LENGTH;

    else
    {
        Retain(aBuffer);

        theLength = aLength;
        theString = aString;
        isShort   = false;
    }

    return err;
}

/***
 * @fn CHIP_ERROR ReferencedString::init(uint16_t aLength, char *aString)
 * @brief Initialize a ReferencedString
 *
 * This initializer should be used if there's no message buffer because we're
 * creating this object with an intention of immediately sending it.
 *
 * @note If the string passed in here is stack-allocated, any outgoing message
 * created in this way must be sent before the stack context in which it was
 * created is exited.
 *
 * @param[in] aLength A length for the referenced string
 * @param[in] aString A pointer to the string data
 *
 * @retval #CHIP_NO_ERROR Unconditionally
 */

CHIP_ERROR ReferencedString::init(uint16_t aLength, char * aString)
{
    theLength = aLength;
    theString = aString;

    Release();

    isShort = false;

    return CHIP_NO_ERROR;
}

/***
 * @fn CHIP_ERROR ReferencedString::init(uint8_t aLength, char *aString, System::PacketBuffer *aBuffer)
 * @overload
 */
CHIP_ERROR ReferencedString::init(uint8_t aLength, char * aString, System::PacketBuffer * aBuffer)
{
    if (aLength > (aBuffer->AvailableDataLength() - aBuffer->DataLength()))
        return CHIP_ERROR_INVALID_STRING_LENGTH;

    Retain(aBuffer);

    theLength = (uint16_t) aLength;
    theString = aString;
    isShort   = true;

    return CHIP_NO_ERROR;
}

/***
 * @fn CHIP_ERROR ReferencedString::init(uint8_t aLength, char *aString)
 * @overload
 */

CHIP_ERROR ReferencedString::init(uint8_t aLength, char * aString)
{
    theLength = (uint16_t) aLength;
    theString = aString;

    Release();

    isShort = true;

    return CHIP_NO_ERROR;
}

/**
 * @param  &i  An iterator over the message being packed.
 * @return     CHIP_ERROR
 */

CHIP_ERROR ReferencedString::pack(MessageIterator & i)
{
    CHIP_ERROR e;

    if (isShort)
        e = i.writeByte((uint8_t) theLength);

    else
        e = i.write16(theLength);

    if (e == CHIP_NO_ERROR)
        e = i.writeString(theLength, theString);

    return e;
}

/**
 * @param &i        An iterator over the message being parsed.
 * @param &aString  A place to put the result of parsing.
 *
 * @retval CHIP_NO_ERROR                    String parsed successfully.
 * @retval CHIP_ERROR_INVALID_STRING_LENGTH The string is too long for the buffer
 *                                           (this should never happen).
 */

CHIP_ERROR ReferencedString::parse(MessageIterator & i, ReferencedString & aString)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t len   = 0;

    if (aString.isShort)
        i.readByte((uint8_t *) &len);

    else
        i.read16(&len);

    if (i.hasRoom(len))
    {
        aString.theLength = len;
        aString.theString = (char *) i.thePoint;
        aString.Retain(i.GetBuffer());

        // we need to skip over the string

        (i.thePoint) += len;
    }

    else
        err = CHIP_ERROR_INVALID_STRING_LENGTH;

    return err;
}

/**
 * @param &aReferencedString  A string to check against.
 *
 * @retval true  The strings are equal.
 * @retval false The strings are not equal.
 */

bool ReferencedString::operator==(const ReferencedString & aReferencedString) const
{
    bool result = false;

    if (theLength == aReferencedString.theLength)
    {
        for (int i = 0; i < theLength; i++)
        {
            if (theString[i] != aReferencedString.theString[i])
                goto exit;
        }

        result = true;
    }

exit:

    return result;
}

/**
 * @return  A printable string
 */

char * ReferencedString::printString(void)
{
    theString[theLength] = 0;

    return theString;
}

/**
 *-------------------- definitions for TLV data --------------------
 */

/**
 * The no-arg constructor for TLV data. Delivers a free/uninitialized
 * object which must be subjected to one of the init() methods defined
 * here in order to be useful.
 */

ReferencedTLVData::ReferencedTLVData(void) : RetainedPacketBuffer()
{
    theLength        = 0;
    theMaxLength     = 0;
    theData          = NULL;
    theWriteCallback = NULL;
    theAppState      = NULL;
}

/**
 * @fn CHIP_ERROR ReferencedTLVData::init(System::PacketBuffer *aBuffer)
 *
 * @brief Initialize the ReferencedTLVData object given a PacketBuffer
 *
 * Initialize a ReferencedTLVData object given a buffer full of
 * TLV. This assumes that the buffer ONLY contains TLV.
 *
 * @param [in] aBuffer     A message buffer in which the TLV resides.
 *
 * @retval #CHIP_NO_ERROR Unconditionally
 */

CHIP_ERROR ReferencedTLVData::init(System::PacketBuffer * aBuffer)
{
    Retain(aBuffer);

    theData      = mBuffer->Start();
    theLength    = mBuffer->DataLength();
    theMaxLength = mBuffer->MaxDataLength();

    theWriteCallback = NULL;
    theAppState      = NULL;

    return CHIP_NO_ERROR;
}

/**
 * @fn CHIP_ERROR ReferencedTLVData::init(MessageIterator &i)
 *
 * @brief Initialize a ReferencedTLVData object given a MessageIterator.
 *
 * Initialize a ReferencedTLVData object given a MessageIterator. In
 * this case, the TLV is that last portion of the buffer and we pass in
 * a message iterator that's pointing to it.
 *
 * @param [in] i           A message iterator pointing to TLV to be extracted.
 *
 * @retval #CHIP_NO_ERROR Unconditionally
 */

CHIP_ERROR ReferencedTLVData::init(MessageIterator & i)
{
    System::PacketBuffer * theBuffer = i.GetBuffer();

    Retain(theBuffer);

    theData      = i.thePoint;
    theLength    = theBuffer->DataLength() - (i.thePoint - mBuffer->Start());
    theMaxLength = theBuffer->MaxDataLength();

    theWriteCallback = NULL;
    theAppState      = NULL;

    return CHIP_NO_ERROR;
}

/**
 * @fn CHIP_ERROR ReferencedTLVData::init(uint16_t aLength, uint16_t aMaxLength, uint8_t *aByteString)
 *
 * @brief Initialize a ReferencedTLVObject given a byte string.
 *
 * Initialize ReferencedTLVData object with a byte string containing
 * TLV. This initializer is the one we use if there's no PacketrBuffer
 * because we're creating one of these to pack and send.
 *
 * @note if the string passed in here is stack-allocated, any
 * outgoing message created in this way must be sent before the stack
 * context in which it was created is exited.
 *
 * @param[in] aLength      A length for the TLV data
 * @param[in] aMaxLength   The total length of the buffer
 * @param[in] aByteString  A pointer to the string data
 *
 * @retval #CHIP_NO_ERROR Unconditionally
 */

CHIP_ERROR ReferencedTLVData::init(uint16_t aLength, uint16_t aMaxLength, uint8_t * aByteString)
{
    theLength    = aLength;
    theMaxLength = aMaxLength;
    theData      = aByteString;

    Release();

    theWriteCallback = NULL;
    theAppState      = NULL;

    return CHIP_NO_ERROR;
}

/**
 * @fn CHIP_ERROR ReferencedTLVData::init(TLVWriteCallback aWriteCallback, void *anAppState)
 *
 * @brief Initialize a RefererencedTLVData object given a callback function.
 *
 * Initialize a ReferencedTLVData object. Instead of explicitly
 * supplying the data, this version provides function, the write
 * callback, and a reference object, which will be passed to it, along
 * with a TLVWriter object, when the referenced data is supposed to be
 * packed and sent. The signature of that callback is:
 *
 * @code
 *   typedef void (*TLVWriteCallback)(TLV::TLVWriter &aWriter, void *aAppState);
 * @endcode
 * @param [in] aWriteCallback the function to be
 *             called when it's time to write some TLV.
 * @param [in] anAppState an application state object to be
 *             passed to the callback along with the writer.
 *
 * @retval #CHIP_NO_ERROR               On success.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT If the write callback is not supplied.
 */

CHIP_ERROR ReferencedTLVData::init(TLVWriteCallback aWriteCallback, void * anAppState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aWriteCallback != NULL)
    {
        theWriteCallback = aWriteCallback;
        theAppState      = anAppState;
    }

    else
        err = CHIP_ERROR_INVALID_ARGUMENT;

    theLength    = 0;
    theMaxLength = 0;
    theData      = NULL;
    mBuffer      = NULL;

    return err;
}

/**
 * Free a ReferencedTLVData object, which is to say, undefine it.
 *
 */
void ReferencedTLVData::free(void)
{
    RetainedPacketBuffer::Release();

    /**
     * in this case you have to clear out the write callback and app
     * state as well since that may be how the data is getting generated.
     */

    theWriteCallback = NULL;
    theAppState      = NULL;

    // and the rest of it for good measure

    theLength    = 0;
    theMaxLength = 0;
    theData      = NULL;
}

/**
 * Check if a ReferencedTLVData object is "free", i.e. undefined.
 *
 * @return true if the object is undefined, false otherwise.
 */

bool ReferencedTLVData::isFree(void)
{
    return (mBuffer == NULL && theWriteCallback == NULL && theAppState == NULL);
}

/**
 * @fn CHIP_ERROR ReferencedTLVData::pack(MessageIterator &i)
 *
 * @brief Pack a ReferencedTLVData object using a TLVWriter.
 *
 * @param [in] i An iterator over the message being packed.
 * @param [in] maxLen The maximum number of bytes that should be written to the output buffer.
 *
 * @return a CHIP_ERROR - CHIP_NO_ERROR if all goes well, otherwise
 * an error reflecting an inability of the writer to write the
 * relevant bytes. Note that the write callback is not allowed to
 * return an error and so fails silently.
 */

CHIP_ERROR ReferencedTLVData::pack(MessageIterator & i, uint32_t maxLen)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    System::PacketBuffer * theBuffer = i.GetBuffer();
    uint16_t oldDataLength           = theBuffer->DataLength();
    TLVWriter writer;

    if (theWriteCallback != NULL)
    {
        theData = i.thePoint;
        writer.Init(theBuffer, maxLen);
        theWriteCallback(writer, theAppState);
        theLength = theBuffer->DataLength() - oldDataLength;
        i.thePoint += theLength;
    }
    else
        err = i.writeBytes(theLength, theData);

    return err;
}

/**
 * @fn CHIP_ERROR ReferencedTLVData::parse(MessageIterator &i, ReferencedTLVData &aTarget)
 *
 * @brief Parse a ReferencedTLVData object from a supplied MessageIterator
 *
 * Parse a ReferenceTLVData object from a MessageIterator object
 * assumed to be pointing at the TLV portion of a message.
 *
 * Note that no actual "parsing" is done here since the TLV is left in
 * the buffer and not manipulated at all. This method mainly just sets
 * up the ReferencedTLVData structure for later use.
 *
 * @param [in] i           An iterator over the message being
 *                         parsed.
 * @param [out] aTarget    A place to put the result
 *                         of parsing.
 *
 * @retval #CHIP_NO_ERROR Unconditionally
 */

CHIP_ERROR ReferencedTLVData::parse(MessageIterator & i, ReferencedTLVData & aTarget)
{
    PacketBuffer * buff = i.GetBuffer();

    aTarget.Retain(buff);

    aTarget.theLength = buff->DataLength() - (i.thePoint - buff->Start());

    if (aTarget.theLength != 0)
        aTarget.theData = i.thePoint;

    else
        aTarget.theData = NULL;

    // we need to skip over the data

    (i.thePoint) += aTarget.theLength;

    return CHIP_NO_ERROR;
}

/**
 * Check a ReferencedTLVData object against another for equality.
 *
 * Note that this only really makes sense in the case of two objects
 * that have actual data in them backed by a buffer or string.
 *
 * @param [in] Another an object to check against
 *
 * @retval true  The objects are equal.
 * @retval false The objects strings are not equal.
 */

bool ReferencedTLVData::operator==(const ReferencedTLVData & another) const
{
    bool result = false;

    if (theLength == another.theLength)
    {
        for (int i = 0; i < theLength; i++)
        {
            if (theData[i] != another.theData[i])
                goto exit;
        }

        result = true;
    }

exit:

    return result;
}

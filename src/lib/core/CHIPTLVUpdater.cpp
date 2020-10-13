/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file implements an updating encoder for the CHIP TLV
 *      (Tag-Length-Value) encoding format.
 *
 */

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>

namespace chip {
namespace TLV {

using namespace chip::Encoding;

/**
 * Initialize a TLVUpdater object to edit a single input buffer.
 *
 * On calling this method, the TLV data in the buffer is moved to the end of the
 * buffer and a private TLVReader object is initialized on this relocated
 * buffer. A private TLVWriter object is also initialized on the free space that
 * is now available at the beginning. Applications can use the TLVUpdater object
 * to parse the TLV data and modify/delete existing elements or add new elements
 * to the encoding.
 *
 * @param[in]   buf     A pointer to a buffer containing the TLV data to be edited.
 * @param[in]   dataLen The length of the TLV data in the buffer.
 * @param[in]   maxLen  The total length of the buffer.
 *
 * @retval #CHIP_NO_ERROR                  If the method succeeded.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT    If the buffer address is invalid.
 * @retval #CHIP_ERROR_BUFFER_TOO_SMALL    If the buffer is too small.
 *
 */
CHIP_ERROR TLVUpdater::Init(uint8_t * buf, uint32_t dataLen, uint32_t maxLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t freeLen;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(maxLen >= dataLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // memmove the buffer data to end of the buffer
    freeLen = maxLen - dataLen;
    memmove(buf + freeLen, buf, dataLen);

    // Init reader
    mUpdaterReader.Init(buf + freeLen, dataLen);

    // Init writer
    mUpdaterWriter.Init(buf, freeLen);
    mUpdaterWriter.SetCloseContainerReserved(false);
    mElementStartAddr = buf + freeLen;

exit:
    return err;
}

/**
 * Initialize a TLVUpdater object using a TLVReader.
 *
 * On calling this method, TLV data in the buffer pointed to by the TLVReader
 * is moved from the current read point to the end of the buffer. A new
 * private TLVReader object is initialized to read from this new location, while
 * a new private TLVWriter object is initialized to write to the freed up buffer
 * space.
 *
 * Note that if the TLVReader is already positioned "on" an element, it is first
 * backed-off to the start of that element. Also note that this backing off
 * works well with container elements, i.e., if the TLVReader was already  used
 * to call EnterContainer(), then there is nothing to back-off. But if the
 * TLVReader was positioned on the container element and EnterContainer() was
 * not yet called, then the TLVReader object is backed-off to the start of the
 * container head.
 *
 * The input TLVReader object will be destroyed before returning and the
 * application must not make use of the same on return.
 *
 * @param[in,out]   aReader     Reference to a TLVReader object that will be
 *                              destroyed before returning.
 * @param[in]       freeLen     The length of free space (in bytes) available
 *                              in the pre-encoded data buffer.
 *
 * @retval #CHIP_NO_ERROR                  If the method succeeded.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT    If the buffer address is invalid.
 * @retval #CHIP_ERROR_NOT_IMPLEMENTED     If reader was initialized on a chain
 *                                          of buffers.
 */
CHIP_ERROR TLVUpdater::Init(TLVReader & aReader, uint32_t freeLen)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    uint8_t * buf             = const_cast<uint8_t *>(aReader.GetReadPoint());
    uint32_t remainingDataLen = aReader.GetRemainingLength();
    uint32_t readDataLen      = aReader.GetLengthRead();

    // TLVUpdater does not support chain of buffers yet
    VerifyOrExit(aReader.mBufHandle == 0, err = CHIP_ERROR_NOT_IMPLEMENTED);

    // TLVReader should point to a non-NULL buffer
    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    // If reader is already on an element, reset it to start of element
    if (aReader.ElementType() != TLVElementType::NotSpecified)
    {
        uint8_t elemHeadLen;

        err = aReader.GetElementHeadLength(elemHeadLen);
        SuccessOrExit(err);

        buf -= elemHeadLen;
        remainingDataLen += elemHeadLen;
        readDataLen -= elemHeadLen;
    }

    // memmove the buffer data to end of the buffer
    memmove(buf + freeLen, buf, remainingDataLen);

    // Initialize the internal reader object
    mUpdaterReader.mBufHandle     = 0;
    mUpdaterReader.mReadPoint     = buf + freeLen;
    mUpdaterReader.mBufEnd        = buf + freeLen + remainingDataLen;
    mUpdaterReader.mLenRead       = readDataLen;
    mUpdaterReader.mMaxLen        = aReader.mMaxLen;
    mUpdaterReader.mControlByte   = kTLVControlByte_NotSpecified;
    mUpdaterReader.mElemTag       = AnonymousTag;
    mUpdaterReader.mElemLenOrVal  = 0;
    mUpdaterReader.mContainerType = aReader.mContainerType;
    mUpdaterReader.SetContainerOpen(false);

    mUpdaterReader.ImplicitProfileId = aReader.ImplicitProfileId;
    mUpdaterReader.AppData           = aReader.AppData;
    mUpdaterReader.GetNextBuffer     = nullptr;

    // Initialize the internal writer object
    mUpdaterWriter.mBufHandle     = 0;
    mUpdaterWriter.mBufStart      = buf - readDataLen;
    mUpdaterWriter.mWritePoint    = buf;
    mUpdaterWriter.mRemainingLen  = freeLen;
    mUpdaterWriter.mLenWritten    = readDataLen;
    mUpdaterWriter.mMaxLen        = readDataLen + freeLen;
    mUpdaterWriter.mContainerType = aReader.mContainerType;
    mUpdaterWriter.SetContainerOpen(false);
    mUpdaterWriter.SetCloseContainerReserved(false);

    mUpdaterWriter.ImplicitProfileId = aReader.ImplicitProfileId;
    mUpdaterWriter.GetNewBuffer      = nullptr;
    mUpdaterWriter.FinalizeBuffer    = nullptr;

    // Cache element start address for internal use
    mElementStartAddr = buf + freeLen;

    // Clear the input reader object before returning. The user can no longer
    // use the original TLVReader object anymore.
    aReader.Init(static_cast<const uint8_t *>(nullptr), 0);

exit:
    return err;
}

/**
 * Set the Implicit Profile ID for the TLVUpdater object.
 *
 * This method sets the implicit profile ID for the TLVUpdater object. When the
 * updater is asked to encode a new element, if the profile ID of the tag
 * associated with the new element matches the value of the @p profileId, the
 * updater will encode the tag in implicit form, thereby omitting the profile ID
 * in the process.
 *
 * @param[in]   profileId   The profile id of tags that should be encoded in
 *                          implicit form.
 */
void TLVUpdater::SetImplicitProfileId(uint32_t profileId)
{
    mUpdaterReader.ImplicitProfileId = profileId;
    mUpdaterWriter.ImplicitProfileId = profileId;
}

/**
 * Skip the current element and advance the TLVUpdater object to the next
 * element in the input TLV.
 *
 * The Next() method skips the current element in the input TLV and advances the
 * TLVUpdater's reader to the next element that resides in the same containment
 * context. In particular, if the reader is positioned at the outer most level
 * of a TLV encoding, calling Next() will advance it to the next, top most
 * element. If the reader is positioned within a TLV container element (a
 * structure, array or path), calling Next() will advance it to the next member
 * element of the container.
 *
 * Since Next() constrains reader motion to the current containment context,
 * calling Next() when the reader is positioned on a container element will
 * advance @em over the container, skipping its member elements (and the members
 * of any nested containers) until it reaches the first element after the
 * container.
 *
 * When there are no further elements within a particular containment context
 * the Next() method will return a #CHIP_END_OF_TLV error and the position of
 * the reader will remain unchanged.
 *
 * @note The Next() method implicitly skips the current element. Hence, the
 * TLVUpdater's private writer state variables will be adjusted to account for
 * the new freed space (made available by skipping). This means that the
 * application is expected to call Next() on the TLVUpdater object after a Get()
 * whose value the application does @em not write back (which from the
 * TLVUpdater's view is equivalent to skipping that element).
 *
 * @note Applications are also expected to call Next() when they are at the end
 * of a container, and want to add new elements there. This is particularly
 * important in situations where there is a fixed schema. Applications that have
 * fixed schemas and know where the container end is cannot just add new
 * elements at the end, because the TLVUpdater writer's state will not reflect
 * the correct free space available for the Put() operation. Hence, applications
 * must call Next() (and possibly also test for CHIP_END_OF_TLV) before adding
 * elements at the end of a container.
 *
 * @retval #CHIP_NO_ERROR              If the TLVUpdater reader was
 *                                      successfully positioned on a new
 *                                      element.
 * @retval other                        Returns the CHIP or platform error
 *                                      codes returned by the TLVReader::Skip()
 *                                      and TLVReader::Next() method.
 *
 */
CHIP_ERROR TLVUpdater::Next()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Skip current element if the reader is already positioned on an element
    err = mUpdaterReader.Skip();
    SuccessOrExit(err);

    AdjustInternalWriterFreeSpace();

    // Move the reader to next element
    err = mUpdaterReader.Next();
    SuccessOrExit(err);

exit:
    return err;
}

/**
 * Copies the current element from input TLV to output TLV.
 *
 * The Move() method copies the current element on which the TLVUpdater's reader
 * is positioned on, to the TLVUpdater's writer. The application should call
 * Next() and position the TLVUpdater's reader on an element before calling this
 * method. Just like the TLVReader::Next() method, if the reader is positioned
 * on a container element at the time of the call, all the members of the
 * container will be copied. If the reader is not positioned on any element,
 * nothing changes on calling this method.
 *
 * @retval #CHIP_NO_ERROR              If the TLVUpdater reader was
 *                                      successfully positioned on a new
 *                                      element.
 * @retval #CHIP_END_OF_TLV            If the TLVUpdater's reader is pointing
 *                                      to end of container.
 * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
 *                                      If the TLVIpdater's reader is not
 *                                      positioned on a valid TLV element.
 * @retval other                        Returns other error codes returned by
 *                                      TLVReader::Skip() method.
 *
 */
CHIP_ERROR TLVUpdater::Move()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    const uint8_t * elementEnd;
    uint32_t copyLen;

    VerifyOrExit(static_cast<TLVElementType>((mUpdaterReader.mControlByte & kTLVTypeMask)) != TLVElementType::EndOfContainer,
                 err = CHIP_END_OF_TLV);

    VerifyOrExit(mUpdaterReader.GetType() != kTLVType_NotSpecified, err = CHIP_ERROR_INVALID_TLV_ELEMENT);

    // Skip to the end of the element
    err = mUpdaterReader.Skip();
    SuccessOrExit(err);

    elementEnd = mUpdaterReader.mReadPoint;

    copyLen = elementEnd - mElementStartAddr;

    // Move the element to output TLV
    memmove(mUpdaterWriter.mWritePoint, mElementStartAddr, copyLen);

    // Adjust the updater state
    mElementStartAddr += copyLen;
    mUpdaterWriter.mWritePoint += copyLen;
    mUpdaterWriter.mLenWritten += copyLen;
    mUpdaterWriter.mMaxLen += copyLen;

exit:
    return err;
}

/**
 * Move everything from the TLVUpdater's current read point till end of input
 * TLV buffer over to output.
 *
 * This method supports moving everything from the TLVUpdater's current read
 * point till the end of the reader buffer over to the TLVUpdater's writer.
 *
 * @note This method can be called with the TLVUpdater's reader positioned
 * anywhere within the input TLV. The reader can also be positioned under
 * multiple levels of nested containers and this method will still work.
 *
 * @note This method also changes the state of the TLVUpdater object to a state
 * it would be in if the application had painstakingly parsed each element from
 * the current read point till the end of the input encoding and copied them to
 * the output TLV.
 */
void TLVUpdater::MoveUntilEnd()
{
    const uint8_t * buffEnd = mUpdaterReader.GetReadPoint() + mUpdaterReader.GetRemainingLength();

    uint32_t copyLen = buffEnd - mElementStartAddr;

    // Move all elements till end to output TLV
    memmove(mUpdaterWriter.mWritePoint, mElementStartAddr, copyLen);

    // Adjust the updater state
    mElementStartAddr += copyLen;
    mUpdaterWriter.mWritePoint += copyLen;
    mUpdaterWriter.mLenWritten += copyLen;
    mUpdaterWriter.mMaxLen += copyLen;
    mUpdaterWriter.mContainerType = kTLVType_NotSpecified;
    mUpdaterWriter.SetContainerOpen(false);
    mUpdaterWriter.SetCloseContainerReserved(false);
    mUpdaterReader.mReadPoint += copyLen;
    mUpdaterReader.mLenRead += copyLen;
    mUpdaterReader.mControlByte   = kTLVControlByte_NotSpecified;
    mUpdaterReader.mElemTag       = AnonymousTag;
    mUpdaterReader.mElemLenOrVal  = 0;
    mUpdaterReader.mContainerType = kTLVType_NotSpecified;
    mUpdaterReader.SetContainerOpen(false);
}

/**
 * Prepares a TLVUpdater object for reading elements of a container. It also
 * encodes a start of container object in the output TLV.
 *
 * The EnterContainer() method prepares the current TLVUpdater object to begin
 * reading the member elements of a TLV container (a structure, array or path).
 * For every call to EnterContainer() applications must make a corresponding
 * call to ExitContainer().
 *
 * When EnterContainer() is called the TLVUpdater's reader must be positioned on
 * the container element. The method takes as an argument a reference to a
 * TLVType value which will be used to save the context of the updater while it
 * is reading the container.
 *
 * When the EnterContainer() method returns, the updater is positioned
 * immediately @em before the first member of the container. Repeatedly calling
 * Next() will advance the updater through the members of the collection until
 * the end is reached, at which point the updater will return CHIP_END_OF_TLV.
 *
 * Once the application has finished reading a container it can continue reading
 * the elements after the container by calling the ExitContainer() method.
 *
 * @note This method implicitly encodes a start of container element in the
 * output TLV buffer.
 *
 * @param[out] outerContainerType       A reference to a TLVType value that will
 *                                      receive the context of the updater.
 *
 * @retval #CHIP_NO_ERROR              If the method succeeded.
 * @retval #CHIP_ERROR_INCORRECT_STATE If the TLVUpdater reader is not
 *                                      positioned on a container element.
 * @retval other                        Any other CHIP or platform error code
 *                                      returned by TLVWriter::StartContainer()
 *                                      or TLVReader::EnterContainer().
 *
 */
CHIP_ERROR TLVUpdater::EnterContainer(TLVType & outerContainerType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVType containerType;

    VerifyOrExit(TLVTypeIsContainer(static_cast<TLVType>(mUpdaterReader.mControlByte & kTLVTypeMask)),
                 err = CHIP_ERROR_INCORRECT_STATE);

    // Change the updater state
    AdjustInternalWriterFreeSpace();

    err = mUpdaterWriter.StartContainer(mUpdaterReader.GetTag(), mUpdaterReader.GetType(), containerType);
    SuccessOrExit(err);

    err = mUpdaterReader.EnterContainer(containerType);
    SuccessOrExit(err);

    outerContainerType = containerType;

exit:
    return err;
}

/**
 * Completes the reading of a TLV container element and encodes an end of TLV
 * element in the output TLV.
 *
 * The ExitContainer() method restores the state of a TLVUpdater object after a
 * call to EnterContainer(). For every call to EnterContainer() applications
 * must make a corresponding call to ExitContainer(), passing the context value
 * returned by the EnterContainer() method.
 *
 * When ExitContainer() returns, the TLVUpdater reader is positioned immediately
 * before the first element that follows the container in the input TLV. From
 * this point applications can call Next() to advance through any remaining
 * elements.
 *
 * Once EnterContainer() has been called, applications can call ExitContainer()
 * on the updater at any point in time, regardless of whether all elements in
 * the underlying container have been read. Also, note that calling
 * ExitContainer() before reading all the elements in the container, will result
 * in the updated container getting truncated in the output TLV.
 *
 * @note Any changes made to the configuration of the updater between the calls
 * to EnterContainer() and ExitContainer() are NOT undone by the call to
 * ExitContainer(). For example, a change to the implicit profile id
 * (@p ImplicitProfileId) will not be reversed when a container is exited. Thus
 * it is the application's responsibility to adjust the configuration
 * accordingly at the appropriate times.
 *
 * @param[in] outerContainerType        The TLVType value that was returned by
 *                                      the EnterContainer() method.
 *
 * @retval #CHIP_NO_ERROR              If the method succeeded.
 * @retval #CHIP_ERROR_TLV_UNDERRUN    If the underlying TLV encoding ended
 *                                      prematurely.
 * @retval #CHIP_ERROR_INVALID_TLV_ELEMENT
 *                                      If the updater encountered an invalid or
 *                                      unsupported TLV element type.
 * @retval #CHIP_ERROR_INVALID_TLV_TAG If the updater encountered a TLV tag in
 *                                      an invalid context.
 * @retval other                        Any other CHIP or platform error code
 *                                      returned by TLVWriter::EndContainer() or
 *                                      TLVReader::ExitContainer().
 *
 */
CHIP_ERROR TLVUpdater::ExitContainer(TLVType outerContainerType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mUpdaterReader.ExitContainer(outerContainerType);
    SuccessOrExit(err);

    // Change the updater's state
    AdjustInternalWriterFreeSpace();

    err = mUpdaterWriter.EndContainer(outerContainerType);
    SuccessOrExit(err);

exit:
    return err;
}

/**
 * This is a private method that adjusts the TLVUpdater's free space count by
 * accounting for the freespace from mElementStartAddr to current read point.
 */
void TLVUpdater::AdjustInternalWriterFreeSpace()
{
    const uint8_t * nextElementStart = mUpdaterReader.mReadPoint;

    if (nextElementStart != mElementStartAddr)
    {
        // Increase the internal writer's free space state variables
        mUpdaterWriter.mRemainingLen += nextElementStart - mElementStartAddr;
        mUpdaterWriter.mMaxLen += nextElementStart - mElementStartAddr;

        // Cache the start address of the next element
        mElementStartAddr = nextElementStart;
    }
}

} // namespace TLV
} // namespace chip

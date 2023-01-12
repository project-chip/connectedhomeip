/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file contains definitions for working with data encoded in CHIP TLV format.
 *
 *      CHIP TLV (Tag-Length-Value) is a generalized encoding method for simple structured data. It
 *      shares many properties with the commonly used JSON serialization format while being considerably
 *      more compact over the wire.
 */

#pragma once

#include "TLVCommon.h"

#include "TLVReader.h"
#include "TLVWriter.h"

/**
 * @namespace chip::TLV
 *
 * Definitions for working with data encoded in CHIP TLV format.
 *
 * CHIP TLV is a generalized encoding method for simple structured data. It shares many properties
 * with the commonly used JSON serialization format while being considerably more compact over the wire.
 */

namespace chip {
namespace TLV {

/**
 * Provides a unified Reader/Writer interface for editing/adding/deleting elements in TLV encoding.
 *
 * The TLVUpdater is a union of the TLVReader and TLVWriter objects and provides interface methods
 * for editing/deleting data in an encoding as well as adding new elements to the TLV encoding. The
 * TLVUpdater object essentially acts like two cursors, one for reading existing encoding and
 * another for writing (either for copying over existing data or writing new data).
 *
 * Semantically, the TLVUpdater object functions like a union of the TLVReader and TLVWriter. The
 * TLVUpdater methods have more or less similar meanings as similarly named counterparts in
 * TLVReader/TLVWriter. Where there are differences in the semantics, the differences are clearly
 * documented in the function's comment section in TLVUpdater.cpp.
 *
 * One particularly important note about the TLVUpdater's PutBytes() and PutString() methods is that
 * it can leave the encoding in a corrupt state with only the element header written when an
 * overflow occurs. Applications can call GetRemainingFreeLength() to make sure there is
 * @em approximately enough free space to write the encoding. Note that GetRemainingFreeLength()
 * only tells you the available free bytes and there is @em no way for the application to know the
 * length of encoded data that gets written. In the event of an overflow, both PutBytes() and
 * PutString() will return CHIP_ERROR_BUFFER_TOO_SMALL to the caller.
 *
 * Also, note that Next() method is overloaded to both skip the current element and also advance the
 * internal reader to the next element. Because skipping already encoded elements requires changing
 * the internal writer's free space state variables to account for the new freed space (made
 * available by skipping), the application is expected to call Next() on the updater after a Get()
 * method whose value it doesn't wish to write back (which is equivalent to skipping the current
 * element).
 *
 * @note The application is expected to use the TLVUpdater object atomically from the time it calls
 * Init() till it calls Finalize(). The same buffer should NOT be used with other TLVWriter objects.
 *
 * @note The TLVUpdater currently only supports single static buffers. TLVBackingStore is NOT supported.
 */
class DLL_EXPORT TLVUpdater
{
public:
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
    CHIP_ERROR Init(uint8_t * buf, uint32_t dataLen, uint32_t maxLen);

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
    CHIP_ERROR Init(TLVReader & aReader, uint32_t freeLen);

    CHIP_ERROR Finalize() { return mUpdaterWriter.Finalize(); }

    // Common methods

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
    void SetImplicitProfileId(uint32_t profileId);
    uint32_t GetImplicitProfileId() const { return mUpdaterReader.ImplicitProfileId; }

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
    CHIP_ERROR Move();

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
    void MoveUntilEnd();

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
    CHIP_ERROR EnterContainer(TLVType & outerContainerType);

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
    CHIP_ERROR ExitContainer(TLVType outerContainerType);

    void GetReader(TLVReader & containerReader) { containerReader = mUpdaterReader; }

    // Reader methods

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
    CHIP_ERROR Next();

    CHIP_ERROR Get(bool & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int8_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int16_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int32_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(int64_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint8_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint16_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint32_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(uint64_t & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(float & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(double & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(ByteSpan & v) { return mUpdaterReader.Get(v); }
    CHIP_ERROR Get(CharSpan & v) { return mUpdaterReader.Get(v); }

    CHIP_ERROR GetBytes(uint8_t * buf, uint32_t bufSize) { return mUpdaterReader.GetBytes(buf, bufSize); }
    CHIP_ERROR DupBytes(uint8_t *& buf, uint32_t & dataLen) { return mUpdaterReader.DupBytes(buf, dataLen); }
    CHIP_ERROR GetString(char * buf, uint32_t bufSize) { return mUpdaterReader.GetString(buf, bufSize); }
    CHIP_ERROR DupString(char *& buf) { return mUpdaterReader.DupString(buf); }

    TLVType GetType() const { return mUpdaterReader.GetType(); }
    Tag GetTag() const { return mUpdaterReader.GetTag(); }
    uint32_t GetLength() const { return mUpdaterReader.GetLength(); }
    CHIP_ERROR GetDataPtr(const uint8_t *& data) { return mUpdaterReader.GetDataPtr(data); }
    CHIP_ERROR VerifyEndOfContainer() { return mUpdaterReader.VerifyEndOfContainer(); }
    TLVType GetContainerType() const { return mUpdaterReader.GetContainerType(); }
    uint32_t GetLengthRead() const { return mUpdaterReader.GetLengthRead(); }
    uint32_t GetRemainingLength() const { return mUpdaterReader.GetRemainingLength(); }

    // Writer methods
    CHIP_ERROR Put(Tag tag, int8_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, int16_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, int32_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, int64_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, uint8_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, uint16_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, uint32_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, uint64_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, int8_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, int16_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, int32_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, int64_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, uint8_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, uint16_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, uint32_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, uint64_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(Tag tag, float v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(Tag tag, double v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR PutBoolean(Tag tag, bool v) { return mUpdaterWriter.PutBoolean(tag, v); }
    CHIP_ERROR PutNull(Tag tag) { return mUpdaterWriter.PutNull(tag); }
    CHIP_ERROR PutBytes(Tag tag, const uint8_t * buf, uint32_t len) { return mUpdaterWriter.PutBytes(tag, buf, len); }
    CHIP_ERROR PutString(Tag tag, const char * buf) { return mUpdaterWriter.PutString(tag, buf); }
    CHIP_ERROR PutString(Tag tag, const char * buf, uint32_t len) { return mUpdaterWriter.PutString(tag, buf, len); }
    CHIP_ERROR CopyElement(TLVReader & reader) { return mUpdaterWriter.CopyElement(reader); }
    CHIP_ERROR CopyElement(Tag tag, TLVReader & reader) { return mUpdaterWriter.CopyElement(tag, reader); }
    CHIP_ERROR StartContainer(Tag tag, TLVType containerType, TLVType & outerContainerType)
    {
        return mUpdaterWriter.StartContainer(tag, containerType, outerContainerType);
    }
    CHIP_ERROR EndContainer(TLVType outerContainerType) { return mUpdaterWriter.EndContainer(outerContainerType); }
    uint32_t GetLengthWritten() { return mUpdaterWriter.GetLengthWritten(); }
    uint32_t GetRemainingFreeLength() const { return mUpdaterWriter.mRemainingLen; }

private:
    void AdjustInternalWriterFreeSpace();

    TLVWriter mUpdaterWriter;
    TLVReader mUpdaterReader;
    const uint8_t * mElementStartAddr;
};

} // namespace TLV
} // namespace chip

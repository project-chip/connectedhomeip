/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <core/CHIPError.h>
#include <core/CHIPTLVTags.h>
#include <core/CHIPTLVTypes.h>

#include <support/DLLUtil.h>

#include <stdarg.h>
#include <stdlib.h>

// forward declaration of the PacketBuffer class used within the header.
namespace chip {
namespace System {

class PacketBuffer;

} // namespace System
} // namespace chip

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

inline uint8_t operator|(TLVElementType lhs, TLVTagControl rhs)
{
    return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
}

inline uint8_t operator|(TLVTagControl lhs, TLVElementType rhs)
{
    return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
}

using chip::System::PacketBuffer;

enum
{
    kTLVControlByte_NotSpecified = 0xFFFF
};

/**
 * Provides a memory efficient parser for data encoded in CHIP TLV format.
 *
 * TLVReader implements a forward-only, “pull-style” parser for CHIP TLV data.  The TLVReader
 * object operates as a cursor that can be used to iterate over a sequence of TLV elements
 * and interpret their contents.  When positioned on an element, applications can make calls
 * to the reader's Get() methods to query the current element’s type and tag, and to extract
 * any associated value.  The reader’s Next() method is used to advance from element to element.
 *
 * A TLVReader object is always positioned either before, on or after a TLV element.  When first
 * initialized, a TLVReader is positioned immediately before the first element of the encoding.
 * To begin reading, an application must make an initial call to the Next() method to position
 * the reader on the first element.  When a container element is encountered--either a structure,
 * an array or a path--the OpenContainer() or EnterContainer() methods can be used to iterate
 * through the contents of the container.
 *
 * When the reader reaches the end of a TLV encoding, or the last element within a container,
 * it signals the application by returning a CHIP_END_OF_TLV error from the Next() method.
 * The reader will continue to return CHIP_END_OF_TLV until it is reinitialized, or the current
 * container is exited (via CloseContainer() / ExitContainer()).
 *
 * A TLVReader object can parse data directly from a fixed input buffer, or from a chain of one
 * or more PacketBuffers.  Additionally, applications can supply a @p GetNextBuffer function to
 * feed data to the reader from an arbitrary source, e.g. a socket or a serial port.
 *
 */
class DLL_EXPORT TLVReader
{
    friend class TLVWriter;
    friend class TLVUpdater;

public:
    // *** See CHIPTLVReader.cpp file for API documentation ***

    void Init(const TLVReader & aReader);
    void Init(const uint8_t * data, uint32_t dataLen);
    void Init(PacketBuffer * buf, uint32_t maxLen = 0xFFFFFFFFUL);
    void Init(PacketBuffer * buf, uint32_t maxLen, bool allowDiscontiguousBuffers);

    CHIP_ERROR Next();
    CHIP_ERROR Next(TLVType expectedType, uint64_t expectedTag);

    TLVType GetType() const;
    uint64_t GetTag() const;
    uint32_t GetLength() const;
    uint16_t GetControlByte() const;

    CHIP_ERROR Get(bool & v);
    CHIP_ERROR Get(int8_t & v);
    CHIP_ERROR Get(int16_t & v);
    CHIP_ERROR Get(int32_t & v);
    CHIP_ERROR Get(int64_t & v);
    CHIP_ERROR Get(uint8_t & v);
    CHIP_ERROR Get(uint16_t & v);
    CHIP_ERROR Get(uint32_t & v);
    CHIP_ERROR Get(uint64_t & v);
    CHIP_ERROR Get(float & v);
    CHIP_ERROR Get(double & v);
    CHIP_ERROR GetBytes(uint8_t * buf, uint32_t bufSize);
    CHIP_ERROR DupBytes(uint8_t *& buf, uint32_t & dataLen);
    CHIP_ERROR GetString(char * buf, uint32_t bufSize);
    CHIP_ERROR DupString(char *& buf);
    CHIP_ERROR GetDataPtr(const uint8_t *& data);

    CHIP_ERROR EnterContainer(TLVType & outerContainerType);
    CHIP_ERROR ExitContainer(TLVType outerContainerType);
    CHIP_ERROR OpenContainer(TLVReader & containerReader);
    CHIP_ERROR CloseContainer(TLVReader & containerReader);
    TLVType GetContainerType() const;
    CHIP_ERROR VerifyEndOfContainer();

    uint32_t GetLengthRead() const { return mLenRead; }
    uint32_t GetRemainingLength() const { return mMaxLen - mLenRead; }

    const uint8_t * GetReadPoint() const { return mReadPoint; }
    uintptr_t GetBufHandle() const { return mBufHandle; }

    CHIP_ERROR Skip();

    CHIP_ERROR FindElementWithTag(const uint64_t tagInApiForm, TLVReader & destReader) const;

    uint32_t ImplicitProfileId;
    void * AppData;

    typedef CHIP_ERROR (*GetNextBufferFunct)(TLVReader & reader, uintptr_t & bufHandle, const uint8_t *& bufStart,
                                             uint32_t & bufLen);
    GetNextBufferFunct GetNextBuffer;

protected:
    uint64_t mElemTag;
    uint64_t mElemLenOrVal;
    uintptr_t mBufHandle;
    const uint8_t * mReadPoint;
    const uint8_t * mBufEnd;
    uint32_t mLenRead;
    uint32_t mMaxLen;
    TLVType mContainerType;
    uint16_t mControlByte;

private:
    bool mContainerOpen;

protected:
    bool IsContainerOpen() const { return mContainerOpen; }
    void SetContainerOpen(bool aContainerOpen) { mContainerOpen = aContainerOpen; }

    CHIP_ERROR ReadElement();
    void ClearElementState();
    CHIP_ERROR SkipData();
    CHIP_ERROR SkipToEndOfContainer();
    CHIP_ERROR VerifyElement();
    uint64_t ReadTag(TLVTagControl tagControl, const uint8_t *& p);
    CHIP_ERROR EnsureData(CHIP_ERROR noDataErr);
    CHIP_ERROR ReadData(uint8_t * buf, uint32_t len);
    CHIP_ERROR GetElementHeadLength(uint8_t & elemHeadBytes) const;
    TLVElementType ElementType() const;

    static CHIP_ERROR GetNextPacketBuffer(TLVReader & reader, uintptr_t & bufHandle, const uint8_t *& bufStart, uint32_t & bufLen);
    static CHIP_ERROR FailGetNextBuffer(TLVReader & reader, uintptr_t & bufHandle, const uint8_t *& bufStart, uint32_t & bufLen);
};

/**
 * Provides a memory efficient encoder for writing data in CHIP TLV format.
 *
 * TLVWriter implements a forward-only, stream-style encoder for CHIP TLV data.  Applications
 * write data to an encoding by calling one of the writer's Put() methods, passing associated
 * tag and value information as necessary.  Similarly applications can encode TLV container types
 * (structures, arrays or paths) by calling the writer's OpenContainer() or EnterContainer()
 * methods.
 *
 * A TLVWriter object can write data directly to a fixed output buffer, or to a chain of one or
 * more PacketBuffer objects.  Additionally, applications can supply their own @p GetNewBuffer and
 * @p FinalizeBuffer functions to direct output to an arbitrary destination, e.g. a socket or an
 * event queue.
 *
 */
class DLL_EXPORT TLVWriter
{
    friend class TLVUpdater;

public:
    // *** See CHIPTLVWriter.cpp file for API documentation ***

    void Init(uint8_t * buf, uint32_t maxLen);
    void Init(PacketBuffer * buf, uint32_t maxLen = 0xFFFFFFFFUL);
    void Init(PacketBuffer * buf, uint32_t maxLen, bool allowDiscontiguousBuffers);

    CHIP_ERROR Finalize();

    CHIP_ERROR Put(uint64_t tag, int8_t v);
    CHIP_ERROR Put(uint64_t tag, int8_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, int16_t v);
    CHIP_ERROR Put(uint64_t tag, int16_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, int32_t v);
    CHIP_ERROR Put(uint64_t tag, int32_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, int64_t v);
    CHIP_ERROR Put(uint64_t tag, int64_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, uint8_t v);
    CHIP_ERROR Put(uint64_t tag, uint8_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, uint16_t v);
    CHIP_ERROR Put(uint64_t tag, uint16_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, uint32_t v);
    CHIP_ERROR Put(uint64_t tag, uint32_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, uint64_t v);
    CHIP_ERROR Put(uint64_t tag, uint64_t v, bool preserveSize);
    CHIP_ERROR Put(uint64_t tag, float v);
    CHIP_ERROR Put(uint64_t tag, double v);
    CHIP_ERROR PutBoolean(uint64_t tag, bool v);
    CHIP_ERROR PutBytes(uint64_t tag, const uint8_t * buf, uint32_t len);
    CHIP_ERROR PutString(uint64_t tag, const char * buf);
    CHIP_ERROR PutString(uint64_t tag, const char * buf, uint32_t len);
    CHIP_ERROR PutStringF(uint64_t tag, const char * fmt, ...);
    CHIP_ERROR VPutStringF(uint64_t tag, const char * fmt, va_list ap);
    CHIP_ERROR PutNull(uint64_t tag);
    CHIP_ERROR CopyElement(TLVReader & reader);
    CHIP_ERROR CopyElement(uint64_t tag, TLVReader & reader);

    CHIP_ERROR StartContainer(uint64_t tag, TLVType containerType, TLVType & outerContainerType);
    CHIP_ERROR EndContainer(TLVType outerContainerType);
    CHIP_ERROR OpenContainer(uint64_t tag, TLVType containerType, TLVWriter & containerWriter);
    CHIP_ERROR CloseContainer(TLVWriter & containerWriter);
    CHIP_ERROR PutPreEncodedContainer(uint64_t tag, TLVType containerType, const uint8_t * data, uint32_t dataLen);
    CHIP_ERROR CopyContainer(TLVReader & container);
    CHIP_ERROR CopyContainer(uint64_t tag, TLVReader & container);
    CHIP_ERROR CopyContainer(uint64_t tag, const uint8_t * encodedContainer, uint16_t encodedContainerLen);
    TLVType GetContainerType() const;

    uint32_t GetLengthWritten();

    uint32_t ImplicitProfileId;
    void * AppData;

    typedef CHIP_ERROR (*GetNewBufferFunct)(TLVWriter & writer, uintptr_t & bufHandle, uint8_t *& bufStart, uint32_t & bufLen);
    GetNewBufferFunct GetNewBuffer;

    typedef CHIP_ERROR (*FinalizeBufferFunct)(TLVWriter & writer, uintptr_t bufHandle, uint8_t * bufStart, uint32_t bufLen);
    FinalizeBufferFunct FinalizeBuffer;

    // Implementations of GetNewBufferFunct/FinalizeBufferFunct that support writing into one or more
    // PacketBuffers.
    static CHIP_ERROR GetNewPacketBuffer(TLVWriter & writer, uintptr_t & bufHandle, uint8_t *& bufStart, uint32_t & bufLen);
    static CHIP_ERROR FinalizePacketBuffer(TLVWriter & writer, uintptr_t bufHandle, uint8_t * bufStart, uint32_t dataLen);

protected:
    uintptr_t mBufHandle;
    uint8_t * mBufStart;
    uint8_t * mWritePoint;
    uint32_t mRemainingLen;
    uint32_t mLenWritten;
    uint32_t mMaxLen;
    TLVType mContainerType;

private:
    bool mContainerOpen;
    bool mCloseContainerReserved;

protected:
    bool IsContainerOpen() const { return mContainerOpen; }
    void SetContainerOpen(bool aContainerOpen) { mContainerOpen = aContainerOpen; }

    enum
    {
        kEndOfContainerMarkerSize = 1, /**< Size of the EndOfContainer marker, used in reserving space. */
    };

    /**
     * @brief
     *   Determine whether the container should reserve space for the
     *   CloseContainer symbol at the point of starting / opening the
     *   container.
     */
    bool IsCloseContainerReserved() const { return mCloseContainerReserved; }

    /**
     * @brief
     * Set whether the container should reserve the space for the
     * CloseContainer symbol at the point of starting / opening the
     * container.
     */
    void SetCloseContainerReserved(bool aCloseContainerReserved) { mCloseContainerReserved = aCloseContainerReserved; }

#if CONFIG_HAVE_VCBPRINTF
    static void CHIPTLVWriterPutcharCB(uint8_t c, void * appState);
#endif
    CHIP_ERROR WriteElementHead(TLVElementType elemType, uint64_t tag, uint64_t lenOrVal);
    CHIP_ERROR WriteElementWithData(TLVType type, uint64_t tag, const uint8_t * data, uint32_t dataLen);
    CHIP_ERROR WriteData(const uint8_t * p, uint32_t len);
};

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
 * documented in the function's comment section in CHIPTLVUpdater.cpp.
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
 * @note The TLVUpdater currently only supports single static buffers. Support for chain of buffers
 * (PacketBuffer) is NOT supported.
 */
class DLL_EXPORT TLVUpdater
{
public:
    CHIP_ERROR Init(uint8_t * buf, uint32_t dataLen, uint32_t maxLen);
    CHIP_ERROR Init(TLVReader & aReader, uint32_t freeLen);
    CHIP_ERROR Finalize() { return mUpdaterWriter.Finalize(); }

    // Common methods
    void SetImplicitProfileId(uint32_t profileId);
    uint32_t GetImplicitProfileId() { return mUpdaterReader.ImplicitProfileId; }
    CHIP_ERROR Move();
    void MoveUntilEnd();
    CHIP_ERROR EnterContainer(TLVType & outerContainerType);
    CHIP_ERROR ExitContainer(TLVType outerContainerType);
    void GetReader(TLVReader & containerReader) { containerReader = mUpdaterReader; }

    // Reader methods
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
    CHIP_ERROR GetBytes(uint8_t * buf, uint32_t bufSize) { return mUpdaterReader.GetBytes(buf, bufSize); }
    CHIP_ERROR DupBytes(uint8_t *& buf, uint32_t & dataLen) { return mUpdaterReader.DupBytes(buf, dataLen); }
    CHIP_ERROR GetString(char * buf, uint32_t bufSize) { return mUpdaterReader.GetString(buf, bufSize); }
    CHIP_ERROR DupString(char *& buf) { return mUpdaterReader.DupString(buf); }

    TLVType GetType() const { return mUpdaterReader.GetType(); }
    uint64_t GetTag() const { return mUpdaterReader.GetTag(); }
    uint32_t GetLength() const { return mUpdaterReader.GetLength(); }
    CHIP_ERROR GetDataPtr(const uint8_t *& data) { return mUpdaterReader.GetDataPtr(data); }
    CHIP_ERROR VerifyEndOfContainer() { return mUpdaterReader.VerifyEndOfContainer(); }
    TLVType GetContainerType() const { return mUpdaterReader.GetContainerType(); }
    uint32_t GetLengthRead() const { return mUpdaterReader.GetLengthRead(); }
    uint32_t GetRemainingLength() const { return mUpdaterReader.GetRemainingLength(); }

    // Writer methods
    CHIP_ERROR Put(uint64_t tag, int8_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int16_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int32_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int64_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint8_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint16_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint32_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, uint64_t v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, int8_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, int16_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, int32_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, int64_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint8_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint16_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint32_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, uint64_t v, bool preserveSize) { return mUpdaterWriter.Put(tag, v, preserveSize); }
    CHIP_ERROR Put(uint64_t tag, float v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR Put(uint64_t tag, double v) { return mUpdaterWriter.Put(tag, v); }
    CHIP_ERROR PutBoolean(uint64_t tag, bool v) { return mUpdaterWriter.PutBoolean(tag, v); }
    CHIP_ERROR PutNull(uint64_t tag) { return mUpdaterWriter.PutNull(tag); }
    CHIP_ERROR PutBytes(uint64_t tag, const uint8_t * buf, uint32_t len) { return mUpdaterWriter.PutBytes(tag, buf, len); }
    CHIP_ERROR PutString(uint64_t tag, const char * buf) { return mUpdaterWriter.PutString(tag, buf); }
    CHIP_ERROR PutString(uint64_t tag, const char * buf, uint32_t len) { return mUpdaterWriter.PutString(tag, buf, len); }
    CHIP_ERROR CopyElement(TLVReader & reader) { return mUpdaterWriter.CopyElement(reader); }
    CHIP_ERROR CopyElement(uint64_t tag, TLVReader & reader) { return mUpdaterWriter.CopyElement(tag, reader); }
    CHIP_ERROR StartContainer(uint64_t tag, TLVType containerType, TLVType & outerContainerType)
    {
        return mUpdaterWriter.StartContainer(tag, containerType, outerContainerType);
    }
    CHIP_ERROR EndContainer(TLVType outerContainerType) { return mUpdaterWriter.EndContainer(outerContainerType); }
    uint32_t GetLengthWritten() { return mUpdaterWriter.GetLengthWritten(); }
    uint32_t GetRemainingFreeLength() { return mUpdaterWriter.mRemainingLen; }

private:
    void AdjustInternalWriterFreeSpace();

    TLVWriter mUpdaterWriter;
    TLVReader mUpdaterReader;
    const uint8_t * mElementStartAddr;
};

} // namespace TLV
} // namespace chip

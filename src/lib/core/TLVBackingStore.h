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
 * Provides an interface for TLVReader or TLVWriter to use memory other than a simple contiguous buffer.
 */
class DLL_EXPORT TLVBackingStore
{
public:
    virtual ~TLVBackingStore() {}
    /**
     * A function to provide a backing store's initial start position and data length to a reader.
     *
     * @param[in]       reader          A reference to the TLVReader object that is requesting input data.
     * @param[out]      bufStart        A reference to a data pointer. On exit, bufStart is expected to point
     *                                  to the first byte of TLV data to be parsed.
     * @param[out]      bufLen          A reference to an unsigned integer that the function must set to
     *                                  the number of TLV data bytes being returned.  If the end of the
     *                                  input TLV data has been reached, the function should set this value
     *                                  to 0.
     *
     * @retval #CHIP_NO_ERROR           If the function successfully produced TLV data.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing the requested data.
     */
    virtual CHIP_ERROR OnInit(TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function that can be used to retrieve additional TLV data to be parsed.
     *
     * When called, the function is expected to produce additional data for the reader to parse or signal
     * the reader that no more data is available.
     *
     * @param[in]       reader          A reference to the TLVReader object that is requesting input data.
     * @param[in,out]   bufStart        A reference to a data pointer. On entry to the function, @p bufStart
     *                                  points to one byte beyond the last TLV data byte consumed by the
     *                                  reader.  On exit, bufStart is expected to point to the first byte
     *                                  of new TLV data to be parsed.  The new pointer value can be within
     *                                  the same buffer as the previously consumed data, or it can point
     *                                  to an entirely new buffer.
     * @param[out]      bufLen          A reference to an unsigned integer that the function must set to
     *                                  the number of TLV data bytes being returned.  If the end of the
     *                                  input TLV data has been reached, the function should set this value
     *                                  to 0.
     *
     * @retval #CHIP_NO_ERROR           If the function successfully produced more TLV data, or the end of
     *                                  the input data was reached (@p bufLen should be set to 0 in this case).
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing the requested data.
     */
    virtual CHIP_ERROR GetNextBuffer(TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function to provide a backing store's initial start position and data length to a writer.
     *
     * @param[in]       writer          A reference to the TLVWriter object that is requesting new buffer
     *                                  space.
     * @param[out]      bufStart        A reference to a data pointer. On exit, @p bufStart is expected to
     *                                  point to the beginning of the new output buffer.
     * @param[out]      bufLen          A reference to an unsigned integer. On exit, @p bufLen is expected
     *                                  to contain the maximum number of bytes that can be written to the
     *                                  new output buffer.
     *
     * @retval #CHIP_NO_ERROR           If the function was able to supply buffer space for the writer.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing buffer space.
     */
    virtual CHIP_ERROR OnInit(TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function that supplies new output buffer space to a TLVWriter.
     *
     * The function is expected to return a pointer to a memory location where new data should be written,
     * along with an associated maximum length. The function can supply write space either by allocating
     * a new buffer to hold the data or by clearing out previously written data from an existing buffer.
     *
     * @param[in]       writer          A reference to the TLVWriter object that is requesting new buffer
     *                                  space.
     * @param[in,out]   bufStart        A reference to a data pointer. On entry to the function, @p bufStart
     *                                  points the beginning of the current output buffer.  On exit, @p bufStart
     *                                  is expected to point to the beginning of the new output buffer.
     *                                  The new pointer value can be the same as the previous value (e.g.
     *                                  if the function copied the existing data elsewhere), or it can point
     *                                  to an entirely new location.
     * @param[in,out]   bufLen          A reference to an unsigned integer. On entry to the function,
     *                                  @p bufLen contains the number of byte of @em unused space in the
     *                                  current buffer.  On exit, @p bufLen is expected to contain the maximum
     *                                  number of bytes that can be written to the new output buffer.
     *
     * @retval #CHIP_NO_ERROR          If the function was able to supply more buffer space for the writer.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred preventing the function from producing additional buffer
     *                                  space.
     */
    virtual CHIP_ERROR GetNewBuffer(TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) = 0;

    /**
     * A function used to perform finalization of the output from a TLVWriter object.
     *
     * Functions of this type are called when a TLVWriter's Finalize() method is called. The function is
     * expected to perform any necessary clean-up or finalization related to consuming the output of the
     * writer object. Examples of this include such things as recording the final length of the encoding,
     * or closing a file descriptor.
     *
     * @param[in]       writer          A reference to the TLVWriter object that is being finalized.
     * @param[in,out]   bufStart        A pointer to the beginning of the current (and final) output buffer.
     * @param[in,out]   bufLen          The number of bytes contained in the buffer pointed to by @p bufStart.
     *
     * @retval #CHIP_NO_ERROR           If finalization was successful.
     * @retval other                    Other CHIP or platform-specific error codes indicating that an error
     *                                  occurred during finalization.
     *
     */
    virtual CHIP_ERROR FinalizeBuffer(TLVWriter & writer, uint8_t * bufStart, uint32_t bufLen) = 0;

    /**
     * Returns whether call to GetNewBuffer will always fail.
     *
     * There are some implementations of TLVBackingStore that provide some level of utility, such as access to pool
     * of particular kind of buffer and/or reserving space for headers. Some implementation allow the caller to
     * specify that they only intend to use a single buffer. It is useful for TLVWriter to know if this is the case.
     *
     */
    virtual bool GetNewBufferWillAlwaysFail() { return false; }
};

} // namespace TLV
} // namespace chip

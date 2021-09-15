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
 *      This file implements utility interfaces for managing and
 *      working with CHIP TLV.
 *
 */

#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CodeUtils.h>

namespace chip {

namespace TLV {

namespace Utilities {

struct FindContext
{
    const uint64_t & mTag;
    TLVReader & mReader;
};

/**
 *  Iterate through the TLV data referenced by @a aReader and invoke @a aHandler
 *  for each visited TLV element in the context of @a aContext.
 *  The iteration is aborted if @a aHandler returns anything other than #CHIP_NO_ERROR
 *
 *  @param[in]     aReader      A reference to the TLV reader containing the TLV
 *                              data to iterate.
 *  @param[in]     aDepth       The current depth into the TLV data.
 *  @param[in]     aHandler     A callback to invoke for the current TLV element
 *                              being visited.
 *  @param[in,out] aContext     An optional pointer to caller-provided context data.
 *  @param[in]     aRecurse     A Boolean indicating whether (true) or not (false)
 *                              any encountered arrays or structures should be
 *                              descended into.
 *
 *  @retval  #CHIP_END_OF_TLV   On a successful iteration to the end of a TLV encoding,
 *                              or to the end of a TLV container.
 *
 *  @retval  The last value returned by @a aHandler, if different than #CHIP_NO_ERROR
 */
static CHIP_ERROR Iterate(TLVReader & aReader, size_t aDepth, IterateHandler aHandler, void * aContext, bool aRecurse)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (aReader.GetType() == kTLVType_NotSpecified)
    {
        ReturnErrorOnFailure(aReader.Next());
    }

    do
    {
        const TLVType theType = aReader.GetType();

        ReturnErrorOnFailure((aHandler)(aReader, aDepth, aContext));

        if (aRecurse && TLVTypeIsContainer(theType))
        {
            TLVType containerType;

            ReturnErrorOnFailure(aReader.EnterContainer(containerType));

            retval = Iterate(aReader, aDepth + 1, aHandler, aContext, aRecurse);
            if ((retval != CHIP_END_OF_TLV) && (retval != CHIP_NO_ERROR))
            {
                return retval;
            }

            ReturnErrorOnFailure(aReader.ExitContainer(containerType));
        }
    } while ((retval = aReader.Next()) == CHIP_NO_ERROR);

    return retval;
}

/**
 *  Iterate through the TLV data referenced by @a aReader and invoke @a aHandler
 *  for each visited TLV element in the context of @a aContext.
 *  The iteration is aborted if @a aHandler returns anything other than #CHIP_NO_ERROR
 *
 *  @param[in]     aReader      A reference to the TLV reader containing the TLV
 *                              data to iterate.
 *  @param[in]     aHandler     A callback to invoke for the current TLV element
 *                              being visited.
 *  @param[in,out] aContext     An optional pointer to caller-provided context data.
 *
 *  @retval  #CHIP_END_OF_TLV  On a successful iteration to the end of a TLV encoding,
 *                              or to the end of a TLV container.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT  If @a aHandler is NULL.
 *
 *  @retval  The last value returned by @a aHandler, if different than #CHIP_NO_ERROR
 *
 */
CHIP_ERROR Iterate(const TLVReader & aReader, IterateHandler aHandler, void * aContext)
{
    const bool recurse = true;
    CHIP_ERROR retval;

    retval = Iterate(aReader, aHandler, aContext, recurse);

    return retval;
}

/**
 *  Iterate through the TLV data referenced by @a aReader and invoke @a aHandler
 *  for each visited TLV element in the context of @a aContext.
 *  The iteration is aborted if @a aHandler returns anything other than #CHIP_NO_ERROR
 *
 *  @param[in]     aReader      A reference to the TLV reader containing the TLV
 *                              data to iterate.
 *  @param[in]     aHandler     A callback to invoke for the current TLV element
 *                              being visited.
 *  @param[in,out] aContext     An optional pointer to caller-provided context data.
 *  @param[in]     aRecurse     A Boolean indicating whether (true) or not (false)
 *                              any encountered arrays or structures should be
 *                              descended into.
 *
 *  @retval  #CHIP_END_OF_TLV  On a successful iteration to the end of a TLV encoding,
 *                              or to the end of a TLV container.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT  If @a aHandler is NULL.
 *
 *  @retval  The last value returned by @a aHandler, if different than #CHIP_NO_ERROR
 *
 */
CHIP_ERROR Iterate(const TLVReader & aReader, IterateHandler aHandler, void * aContext, const bool aRecurse)
{
    VerifyOrReturnError(aHandler != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    TLVReader temp;
    temp.Init(aReader);

    constexpr size_t depth = 0;
    return Iterate(temp, depth, aHandler, aContext, aRecurse);
}

/**
 *  Increment the counter when iterating through the TLV data.
 *
 *  @param[in]     aReader      A reference to the TLV reader containing the TLV
 *                              data to count the number of TLV elements.
 *  @param[in]     aDepth       The current depth into the TLV data.
 *  @param[in,out] aContext     A pointer to the handler-specific context which
 *                              is a pointer to storage for the count value.
 *
 *  @retval  #CHIP_NO_ERROR                On success.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT  If @a aContext is NULL.
 *
 */
static CHIP_ERROR CountHandler(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    *static_cast<size_t *>(aContext) += 1;

    return CHIP_NO_ERROR;
}

/**
 *  Count the number of TLV elements within the specified TLV reader,
 *  descending into arrays or structures.
 *
 *  @param[in]     aReader      A read-only reference to the TLV reader for
 *                              which to count the number of TLV elements.
 *  @param[in,out] aCount       A reference to storage for the returned count.
 *                              This is initialized to zero (0) prior to counting
 *                              and is set to the number of elements counted on
 *                              success.
 *
 *  @retval  #CHIP_NO_ERROR    On success.
 *
 */
CHIP_ERROR Count(const TLVReader & aReader, size_t & aCount)
{
    constexpr bool recurse = true;
    return Count(aReader, aCount, recurse);
}

/**
 *  Count the number of TLV elements within the specified TLV reader,
 *  optionally descending into arrays or structures.
 *
 *  @param[in]     aReader      A read-only reference to the TLV reader for
 *                              which to count the number of TLV elements.
 *  @param[in,out] aCount       A reference to storage for the returned count.
 *                              This is initialized to zero (0) prior to counting
 *                              and is set to the number of elements counted on
 *                              success.
 *  @param[in]     aRecurse     A Boolean indicating whether (true) or not (false)
 *                              any encountered arrays or structures should be
 *                              descended into.
 *
 *  @retval  #CHIP_NO_ERROR    On success.
 *
 */
CHIP_ERROR Count(const TLVReader & aReader, size_t & aCount, const bool aRecurse)
{
    aCount = 0;

    CHIP_ERROR retval = Iterate(aReader, CountHandler, &aCount, aRecurse);

    if (retval == CHIP_END_OF_TLV)
        retval = CHIP_NO_ERROR;

    return retval;
}

/**
 *  Search for the specified tag within the provided TLV reader.
 *
 *  @param[in]     aReader      A read-only reference to the TLV reader in
 *                              which to find the specified tag.
 *  @param[in]     aDepth       The current depth into the TLV data.
 *  @param[in,out] aContext     A pointer to the handler-specific context.
 *
 *  @retval  #CHIP_NO_ERROR                On success.
 *
 *  @retval  #CHIP_ERROR_INVALID_ARGUMENT  If @a aContext is NULL.
 *
 *  @retval  #CHIP_ERROR_SENTINEL          If the specified tag is found.
 *
 */
static CHIP_ERROR FindHandler(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    const FindContext * const theContext = static_cast<const FindContext *>(aContext);

    if (theContext->mTag == aReader.GetTag())
    {
        theContext->mReader.Init(aReader);
        // terminate the iteration when the specified tag is found
        return CHIP_ERROR_SENTINEL;
    }

    return CHIP_NO_ERROR;
}

/**
 *  Search for the specified tag within the provided TLV reader.
 *
 *  @param[in]   aReader        A read-only reference to the TLV reader in
 *                              which to find the specified tag.
 *  @param[in]   aTag           A read-only reference to the TLV tag to find.
 *  @param[out]  aResult        A reference to storage to a TLV reader which
 *                              will be positioned at the specified tag
 *                              on success.
 *
 *  @retval  #CHIP_NO_ERROR                    On success.
 *
 *  @retval  #CHIP_ERROR_TLV_TAG_NOT_FOUND     If the specified tag @a aTag was not found.
 *
 */
CHIP_ERROR Find(const TLVReader & aReader, const uint64_t & aTag, TLVReader & aResult)
{
    constexpr bool recurse = true;
    return Find(aReader, aTag, aResult, recurse);
}

/**
 *  Search for the specified tag within the provided TLV reader,
 *  optionally descending into arrays or structures.
 *
 *  @param[in]   aReader        A read-only reference to the TLV reader in
 *                              which to find the specified tag.
 *  @param[in]   aTag           A read-only reference to the TLV tag to find.
 *  @param[out]  aResult        A reference to storage to a TLV reader which
 *                              will be positioned at the specified tag
 *                              on success.
 *  @param[in]   aRecurse       A Boolean indicating whether (true) or not (false)
 *                              any encountered arrays or structures should be
 *                              descended into.
 *
 *  @retval  #CHIP_NO_ERROR                    On success.
 *
 *  @retval  #CHIP_ERROR_TLV_TAG_NOT_FOUND     If the specified tag @a aTag was not found.
 *
 */
CHIP_ERROR Find(const TLVReader & aReader, const uint64_t & aTag, TLVReader & aResult, const bool aRecurse)
{
    FindContext theContext = { aTag, aResult };
    CHIP_ERROR retval      = Iterate(aReader, FindHandler, &theContext, aRecurse);

    if (retval == CHIP_ERROR_SENTINEL)
        retval = CHIP_NO_ERROR;
    else
        retval = CHIP_ERROR_TLV_TAG_NOT_FOUND;

    return retval;
}

struct FindPredicateContext
{
    TLVReader & mResult;
    IterateHandler mHandler;
    void * mContext;
    FindPredicateContext(TLVReader & inReader, IterateHandler inHandler, void * inContext);
};

FindPredicateContext::FindPredicateContext(TLVReader & inReader, IterateHandler inHandler, void * inContext) :
    mResult(inReader), mHandler(inHandler), mContext(inContext)
{}

static CHIP_ERROR FindPredicateHandler(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    FindPredicateContext * theContext = static_cast<FindPredicateContext *>(aContext);
    CHIP_ERROR err;

    err = theContext->mHandler(aReader, aDepth, theContext->mContext);

    if (err == CHIP_ERROR_SENTINEL)
        theContext->mResult.Init(aReader);

    return err;
}

/**
 *  Search for the first element matching the predicate within the TLV reader
 *  descending into arrays or structures. The @a aPredicate is applied
 *  to each visited TLV element; the @a aPredicate shall return #CHIP_ERROR_SENTINEL
 *  for the matching elements, #CHIP_NO_ERROR for non-matching elements, and any
 *  other value to terminate the search.
 *
 *  @param[in] aReader     A read-only reference to the TLV reader in which to find the
 *                         element matching the predicate.
 *  @param[in] aPredicate  A predicate to be applied to each TLV element.  To
 *                         support the code reuse, aPredicate has the
 *                         IterateHandler type.  The return value of aPredicate
 *                         controls the search: a #CHIP_ERROR_SENTINEL signals that
 *                         desired element has been found, #CHIP_NO_ERROR
 *                         signals that the desired element has not been found,
 *                         and all other values signal that the search should be
 *                         terminated.
 *  @param[in] aContext    An optional pointer to caller-provided context data.
 *
 *  @param[out] aResult    A reference to storage to a TLV reader which
 *                         will be positioned at the specified tag
 *                         on success.
 *  @retval  #CHIP_NO_ERROR                    On success.
 *
 *  @retval  #CHIP_ERROR_TLV_TAG_NOT_FOUND     If the specified @a aPredicate did not locate the specified element
 *
 */
CHIP_ERROR Find(const TLVReader & aReader, IterateHandler aPredicate, void * aContext, TLVReader & aResult)
{
    const bool recurse = true;
    return Find(aReader, aPredicate, aContext, aResult, recurse);
}

/**
 *  Search for the first element matching the predicate within the TLV reader
 *  optionally descending into arrays or structures. The @a aPredicate is applied
 *  to each visited TLV element; the @a aPredicate shall return #CHIP_ERROR_SENTINEL
 *  for the matching elements, #CHIP_NO_ERROR for non-matching elements, and any
 *  other value to terminate the search.
 *
 *  @param[in] aReader     A read-only reference to the TLV reader in which to find the
 *                         element matching the predicate.
 *  @param[in] aPredicate  A predicate to be applied to each TLV element.  To
 *                         support the code reuse, aPredicate has the
 *                         @a IterateHandler type.  The return value of aPredicate
 *                         controls the search: a #CHIP_ERROR_SENTINEL signals that
 *                         desired element has been found, #CHIP_NO_ERROR
 *                         signals that the desired element has not been found,
 *                         and all other values signal that the saerch should be
 *                         terminated.
 *  @param[in] aContext    An optional pointer to caller-provided context data.
 *  @param[out] aResult    A reference to storage to a TLV reader which
 *                         will be positioned at the specified tag
 *                         on success.
 *  @param[in] aRecurse    A boolean indicating whether (true) or not (false) any
 *                         encountered arrays or structures should be descended
 *                         into.
 *
 *  @retval  #CHIP_NO_ERROR                    On success.
 *
 *  @retval  #CHIP_ERROR_TLV_TAG_NOT_FOUND     If the specified @a aPredicate did not locate the specified element
 *
 */
CHIP_ERROR Find(const TLVReader & aReader, IterateHandler aPredicate, void * aContext, TLVReader & aResult, const bool aRecurse)
{
    CHIP_ERROR retval;
    FindPredicateContext theContext(aResult, aPredicate, aContext);

    retval = Iterate(aReader, FindPredicateHandler, &theContext, aRecurse);

    if (retval == CHIP_ERROR_SENTINEL)
        retval = CHIP_NO_ERROR;
    else
        retval = CHIP_ERROR_TLV_TAG_NOT_FOUND;

    return retval;
}

} // namespace Utilities

} // namespace TLV

} // namespace chip

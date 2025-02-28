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
#include <lib/core/ErrorStr.h>

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>

#include <inttypes.h>
#include <stdio.h>

namespace chip {

/**
 * Static buffer to store the formatted error string.
 */
static ErrorStrStorage sErrorStr;

/**
 * Linked-list of error formatter functions.
 */
static ErrorFormatter * sErrorFormatterList = nullptr;

/**
 * This routine returns a human-readable NULL-terminated C string
 * describing the provided error. This uses the global static storage.
 *
 * @param[in] err                      The error for format and describe.
 * @param[in] withSourceLocation       Whether or not to include the source
 * location in the output string. Only used if CHIP_CONFIG_ERROR_SOURCE &&
 * !CHIP_CONFIG_SHORT_ERROR_STR. Defaults to true.
 *
 * @return A pointer to a NULL-terminated C string describing the
 *         provided error.
 */
DLL_EXPORT const char * ErrorStr(CHIP_ERROR err, bool withSourceLocation)
{
    return ErrorStr(err, withSourceLocation, sErrorStr);
}

/**
 * This routine writess a human-readable NULL-terminated C string into the buf
 * which describes the provided error.
 *
 * @param[in] err                      The error for format and describe.
 * @param[in] withSourceLocation       Whether or not to include the source
 * @param[in] storage                  ErrorStrStorage to write into
 * location in the output string. Only used if CHIP_CONFIG_ERROR_SOURCE &&
 * !CHIP_CONFIG_SHORT_ERROR_STR. Defaults to true.
 *
 * @return A pointer to a NULL-terminated C string describing the
 *         provided error.
 */
DLL_EXPORT const char * ErrorStr(CHIP_ERROR err, bool withSourceLocation, ErrorStrStorage & storage)
{
    char * formattedError   = storage.buff;
    uint16_t formattedSpace = storage.kBufferSize;

#if CHIP_CONFIG_ERROR_SOURCE && !CHIP_CONFIG_SHORT_ERROR_STR

    if (const char * const file = err.GetFile(); withSourceLocation && file != nullptr)
    {
        int n = snprintf(formattedError, formattedSpace, "%s:%u: ", file, err.GetLine());
        if (n > formattedSpace)
        {
            n = formattedSpace;
        }
        formattedError += n;
        formattedSpace = static_cast<uint16_t>(formattedSpace - n);
    }
#endif // CHIP_CONFIG_ERROR_SOURCE && !CHIP_CONFIG_SHORT_ERROR_STR

    if (err == CHIP_NO_ERROR)
    {
        (void) snprintf(formattedError, formattedSpace, CHIP_NO_ERROR_STRING);
        return storage.buff;
    }

    // Search the registered error formatter for one that will format the given
    // error code.
    for (const ErrorFormatter * errFormatter = sErrorFormatterList; errFormatter != nullptr; errFormatter = errFormatter->Next)
    {
        if (errFormatter->FormatError(formattedError, formattedSpace, err))
        {
            return storage.buff;
        }
    }

    // Use a default formatting if no formatter found.
    FormatError(formattedError, formattedSpace, nullptr, err, nullptr);
    return storage.buff;
}

/**
 * Add a new error formatter function to the global list of error formatters.
 *
 * @param[in] errFormatter             An ErrorFormatter structure containing a
 *                                     pointer to the new error function.  Note
 *                                     that a pointer to the supplied ErrorFormatter
 *                                     structure will be retained by the function.
 *                                     Thus the memory for the structure must
 *                                     remain reserved.
 */
DLL_EXPORT void RegisterErrorFormatter(ErrorFormatter * errFormatter)
{
    // Do nothing if a formatter with the same format function is already in the list.
    for (ErrorFormatter * existingFormatter = sErrorFormatterList; existingFormatter != nullptr;
         existingFormatter                  = existingFormatter->Next)
    {
        if (existingFormatter->FormatError == errFormatter->FormatError)
        {
            return;
        }
    }

    // Add the formatter to the global list.
    errFormatter->Next  = sErrorFormatterList;
    sErrorFormatterList = errFormatter;
}

/**
 * Remove an error formatter function from the global list of error formatters.
 *
 * @param[in] errFormatter             An ErrorFormatter structure containing a
 *                                     pointer to the new error function.
 */
DLL_EXPORT void DeregisterErrorFormatter(ErrorFormatter * errFormatter)
{
    // Remove the formatter if present
    for (ErrorFormatter ** lfp = &sErrorFormatterList; *lfp != nullptr;)
    {
        // Remove the formatter from the global list, if found.
        if (*lfp == errFormatter)
        {
            *lfp = errFormatter->Next;
        }
        else
        {
            lfp = &(*lfp)->Next;
        }
    }
}

#if !CHIP_CONFIG_CUSTOM_ERROR_FORMATTER

/**
 * Generates a human-readable NULL-terminated C string describing the provided error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] subsys                A short string describing the subsystem that originated
 *                                  the error, or NULL if the origin of the error is
 *                                  unknown/unavailable.  This string should be 10
 *                                  characters or less.
 * @param[in] err                   The error to be formatted.
 * @param[in] desc                  A string describing the cause or meaning of the error,
 *                                  or NULL if no such information is available.
 */
DLL_EXPORT void FormatError(char * buf, uint16_t bufSize, const char * subsys, CHIP_ERROR err, const char * desc)
{
#if CHIP_CONFIG_SHORT_ERROR_STR

    if (subsys == nullptr)
    {
        (void) snprintf(buf, bufSize, "Error " CHIP_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT, err.AsInteger());
    }
    else
    {
        (void) snprintf(buf, bufSize, "Error %s:" CHIP_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT, subsys, err.AsInteger());
    }

#else // CHIP_CONFIG_SHORT_ERROR_STR

    const char * subsysSep = " ";
    const char * descSep   = ": ";

    if (subsys == nullptr)
    {
        subsys    = "";
        subsysSep = "";
    }
    if (desc == nullptr)
    {
        desc    = "";
        descSep = "";
    }

    (void) snprintf(buf, bufSize, "%s%sError 0x%08" PRIX32 "%s%s", subsys, subsysSep, err.AsInteger(), descSep, desc);

#endif // CHIP_CONFIG_SHORT_ERROR_STR
}

#endif // CHIP_CONFIG_CUSTOM_ERROR_FORMATTER

} // namespace chip

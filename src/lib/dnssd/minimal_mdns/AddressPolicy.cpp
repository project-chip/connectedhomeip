/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "AddressPolicy.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cctype>
#include <cstdlib>
#include <cstring>

namespace mdns {
namespace Minimal {

namespace {
AddressPolicy * gAddressPolicy        = nullptr;
InterfaceFilter * gInterfaceFilter    = nullptr;
InterfaceNameListFilter gDefaultFilter;
} // namespace

AddressPolicy * GetAddressPolicy()
{
    VerifyOrDie(gAddressPolicy != nullptr);
    return gAddressPolicy;
}

void SetAddressPolicy(AddressPolicy * policy)
{
    VerifyOrDie(policy != nullptr);
    gAddressPolicy = policy;
}

InterfaceFilter & GetInterfaceFilter()
{
    if (gInterfaceFilter == nullptr)
    {
        gInterfaceFilter = &gDefaultFilter;
    }
    return *gInterfaceFilter;
}

void SetInterfaceFilter(InterfaceFilter * filter)
{
    gInterfaceFilter = (filter != nullptr) ? filter : &gDefaultFilter;
}

bool InterfaceNameListFilter::IsInterfaceAllowed(const char * name) const
{
    if ((mCount == 0) || (name == nullptr))
    {
        return true;
    }

    for (size_t i = 0; i < mCount; ++i)
    {
        if (strcmp(mAcceptedNames[i], name) == 0)
        {
            return true;
        }
    }

    return false;
}

void InterfaceNameListFilter::Reset()
{
    mNamesStorage.Free();
    mAcceptedNames.Free();
    mCount = 0;
}

CHIP_ERROR InterfaceNameListFilter::SetAllowedInterfaces(chip::Span<const chip::CharSpan> allowed)
{
    Reset();

    if (allowed.empty())
    {
        return CHIP_NO_ERROR;
    }

    size_t totalChars = 0;
    for (const auto & span : allowed)
    {
        VerifyOrReturnError(span.data() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        totalChars += span.size() + 1;
    }

    if (!mNamesStorage.Calloc(totalChars) || !mAcceptedNames.Calloc(allowed.size()))
    {
        Reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    char * writePtr = mNamesStorage.Get();
    for (size_t i = 0; i < allowed.size(); ++i)
    {
        const auto & span = allowed[i];
        memcpy(writePtr, span.data(), span.size());
        writePtr[span.size()] = '\0';
        mAcceptedNames.Get()[i] = writePtr;
        writePtr += span.size() + 1;
    }

    mCount = allowed.size();
    return CHIP_NO_ERROR;
}

CHIP_ERROR InterfaceNameListFilter::SetAllowedInterfacesFromCommaSeparatedList(const char * commaSeparatedNames)
{
    Reset();

    if ((commaSeparatedNames == nullptr) || (commaSeparatedNames[0] == '\0'))
    {
        ChipLogProgress(Discovery, "InterfaceNameListFilter: no filter configured, allowing all interfaces");
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(Discovery, "InterfaceNameListFilter: configured with '%s'", commaSeparatedNames);

    auto countTrimmedSegments = [](const char * str, size_t & outTotalChars) -> size_t {
        size_t count        = 0;
        outTotalChars       = 0;
        const char * start  = str;
        const char * cursor = str;

        auto trim = [](const char * tokenStart, size_t length, const char *& outStart, size_t & outLen) -> bool {
            while ((length > 0) && std::isspace(static_cast<unsigned char>(*tokenStart)))
            {
                ++tokenStart;
                --length;
            }
            while ((length > 0) && std::isspace(static_cast<unsigned char>(tokenStart[length - 1])))
            {
                --length;
            }
            if (length == 0)
            {
                return false;
            }
            outStart = tokenStart;
            outLen   = length;
            return true;
        };

        auto processSegment = [&](const char * segmentStart, size_t len) {
            const char * trimmedStart = nullptr;
            size_t trimmedLen         = 0;
            if (trim(segmentStart, len, trimmedStart, trimmedLen))
            {
                ++count;
                outTotalChars += trimmedLen + 1;
            }
        };

        while (true)
        {
            if ((*cursor == ',') || (*cursor == '\0'))
            {
                processSegment(start, static_cast<size_t>(cursor - start));
                start = cursor + 1;
            }

            if (*cursor == '\0')
            {
                break;
            }

            ++cursor;
        }

        return count;
    };

    size_t totalChars   = 0;
    size_t segmentCount = countTrimmedSegments(commaSeparatedNames, totalChars);

    if (segmentCount == 0)
    {
        ChipLogError(Discovery, "InterfaceNameListFilter: no valid interface names parsed; rejecting all interfaces");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!mNamesStorage.Calloc(totalChars) || !mAcceptedNames.Calloc(segmentCount))
    {
        Reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    char * writePtr     = mNamesStorage.Get();
    size_t segmentIndex = 0;
    const char * start  = commaSeparatedNames;
    const char * cursor = commaSeparatedNames;

    auto copySegment = [&](const char * segmentStart, size_t len) {
        const char * trimmedStart = nullptr;
        size_t trimmedLen         = 0;
        if (!len)
        {
            return;
        }
        const char * tokenStart = segmentStart;
        size_t tokenLen         = len;
        while ((tokenLen > 0) && std::isspace(static_cast<unsigned char>(*tokenStart)))
        {
            ++tokenStart;
            --tokenLen;
        }
        while ((tokenLen > 0) && std::isspace(static_cast<unsigned char>(tokenStart[tokenLen - 1])))
        {
            --tokenLen;
        }
        if (tokenLen == 0)
        {
            return;
        }
        trimmedStart = tokenStart;
        trimmedLen   = tokenLen;

        memcpy(writePtr, trimmedStart, trimmedLen);
        writePtr[trimmedLen] = '\0';
        mAcceptedNames.Get()[segmentIndex++] = writePtr;
        writePtr += trimmedLen + 1;
    };

    while (true)
    {
        if ((*cursor == ',') || (*cursor == '\0'))
        {
            copySegment(start, static_cast<size_t>(cursor - start));
            start = cursor + 1;
        }

        if (*cursor == '\0')
        {
            break;
        }

        ++cursor;
    }

    mCount = segmentIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigureInterfaceFilter(chip::Span<const chip::CharSpan> allowedInterfaces)
{
    ReturnErrorOnFailure(gDefaultFilter.SetAllowedInterfaces(allowedInterfaces));
    SetInterfaceFilter(&gDefaultFilter);

    if (gDefaultFilter.Count() == 0)
    {
        ChipLogProgress(Discovery, "InterfaceFilter: no interfaces specified, allowing all");
    }
    else
    {
        ChipLogProgress(Discovery, "InterfaceFilter: configured for %zu interface(s)", gDefaultFilter.Count());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigureInterfaceFilterFromCommaSeparatedList(const char * commaSeparatedNames)
{
    ReturnErrorOnFailure(gDefaultFilter.SetAllowedInterfacesFromCommaSeparatedList(commaSeparatedNames));
    SetInterfaceFilter(&gDefaultFilter);

    if (gDefaultFilter.Count() == 0)
    {
        ChipLogProgress(Discovery, "InterfaceFilter: configuration empty, allowing all interfaces");
    }
    else
    {
        ChipLogProgress(Discovery, "InterfaceFilter: configured for %zu interface(s)", gDefaultFilter.Count());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigureInterfaceFilterFromEnvironmentVariable(const char * envVarName)
{
    const char * name = (envVarName != nullptr) ? envVarName : kMinimalMdnsInterfaceEnvVar;
    const char * env  = std::getenv(name);

    if ((env == nullptr) || (env[0] == '\0'))
    {
        ChipLogProgress(Discovery, "InterfaceFilter: %s not set, allowing all interfaces", name);
        gDefaultFilter.Reset();
        SetInterfaceFilter(&gDefaultFilter);
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(Discovery, "InterfaceFilter: %s='%s'", name, env);
    return ConfigureInterfaceFilterFromCommaSeparatedList(env);
}

} // namespace Minimal
} // namespace mdns

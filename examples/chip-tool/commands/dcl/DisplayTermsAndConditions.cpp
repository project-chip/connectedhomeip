/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "DisplayTermsAndConditions.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

#include <algorithm>
#include <iostream>
#include <regex>
#include <unordered_map>

namespace chip {
namespace tool {
namespace dcl {
namespace {
constexpr const char * kAcceptTerms       = "Do you accept these terms? [<b>Y</b>/n]: ";
constexpr const char * kRequiredTerms     = "Required";
constexpr const char * kOptionalTerms     = "Optional";
constexpr const char * kTitleAllowedTags  = R"(<(/?)(b|em|i|small|strong|u)>)";
constexpr const char * kTextAllowedTags   = R"(<(/?)(b|br|em|h1|h2|h3|h4|h5|h6|hr|i|li|ol|p|small|strong|u|ul)>)";
constexpr const char * kAnsiCodeReset     = "\033[0m";
constexpr const char * kAnsiCodeBold      = "\033[1m";
constexpr const char * kAnsiCodeFaint     = "\033[2m";
constexpr const char * kAnsiCodeItalics   = "\033[3m";
constexpr const char * kAnsiCodeUnderline = "\033[4m";
constexpr const char * kLineBreak         = "\n";
constexpr const char * kListItem          = "  - ";
constexpr const char * kHorizontalLine    = "\n==========================================\n";
constexpr const char * kErrorInvalidInput = "Invalid input. Please enter 'Y' (yes) or 'N' (no). Default is 'Y'.";

// Fields names for the ESF JSON schema
constexpr const char * kFieldCountryEntries  = "countryEntries";
constexpr const char * kFieldDefaultCountry  = "defaultCountry";
constexpr const char * kFieldLanguageEntries = "languageEntries";
constexpr const char * kFieldDefaultLanguage = "defaultLanguage";
constexpr const char * kFieldOrdinal         = "ordinal";
constexpr const char * kFieldRequired        = "required";
constexpr const char * kFieldSchemaVersion   = "schemaVersion";
constexpr const char * kFieldText            = "text";
constexpr const char * kFieldTitle           = "title";

const std::unordered_map<std::string, std::string> kHtmlToAnsiCodes = {
    { "b", kAnsiCodeBold },      //
    { "br", kLineBreak },        //
    { "em", kAnsiCodeItalics },  //
    { "h1", kAnsiCodeBold },     //
    { "h2", kAnsiCodeBold },     //
    { "h3", kAnsiCodeBold },     //
    { "h4", kAnsiCodeBold },     //
    { "h5", kAnsiCodeBold },     //
    { "h6", kAnsiCodeBold },     //
    { "hr", kHorizontalLine },   //
    { "i", kAnsiCodeItalics },   //
    { "li", kListItem },         //
    { "ol", kLineBreak },        //
    { "p", kLineBreak },         //
    { "small", kAnsiCodeFaint }, //
    { "strong", kAnsiCodeBold }, //
    { "u", kAnsiCodeUnderline }, //
    { "ul", kLineBreak },        //
};

std::string ToUpperCase(const std::string & input)
{
    std::string output = input;
    std::transform(output.begin(), output.end(), output.begin(), [](unsigned char c) { return std::toupper(c); });
    return output;
}

std::string ToLowerCase(const std::string & input)
{
    std::string output = input;
    std::transform(output.begin(), output.end(), output.begin(), [](unsigned char c) { return std::tolower(c); });
    return output;
}

std::string Center(const std::string & text)
{
    size_t lineWidth = strlen(kHorizontalLine) - 1;
    if (text.length() >= lineWidth)
    {
        return text; // No padding if the text is longer than the width
    }

    size_t totalPadding = lineWidth - text.length();
    size_t paddingLeft  = totalPadding / 2;
    size_t paddingRight = totalPadding - paddingLeft;

    return std::string(paddingLeft, ' ') + text + std::string(paddingRight, ' ');
}

std::string HTMLTagToAnsiCode(const std::smatch & match)
{
    if (match[1] == "/")
    {
        return kAnsiCodeReset;
    }

    std::string tag = match[2];
    auto ansiCode   = kHtmlToAnsiCodes.find(ToLowerCase(tag));
    if (ansiCode == kHtmlToAnsiCodes.end())
    {
        return "<" + tag + ">";
    }

    return ansiCode->second;
}

std::string renderHTMLInTerminal(const std::string & html, const std::string & allowedTags = kTextAllowedTags)
{
    std::string formattedText;
    std::string::const_iterator current = html.cbegin();

    std::regex regex(allowedTags, std::regex_constants::icase);
    for (std::sregex_iterator it(html.cbegin(), html.cend(), regex), end; it != end; ++it)
    {
        const auto & match = *it;

        formattedText += std::string(current, html.cbegin() + match.position());
        formattedText += HTMLTagToAnsiCode(match);

        current = html.cbegin() + match.position() + match.length();
    }

    formattedText += std::string(current, html.cend());
    formattedText += kAnsiCodeReset;
    return formattedText;
}

const char * ResolveValueOrDefault(const Json::Value & entries, const Optional<const char *> & userValue, const char * defaultValue,
                                   const char * valueType)
{
    const char * resolvedValue = userValue.ValueOr(defaultValue);

    if (userValue.HasValue() && !entries.isMember(resolvedValue))
    {
        ChipLogProgress(chipTool, "User-chosen %s ('%s') not found. Defaulting to '%s'", valueType, userValue.Value(),
                        defaultValue);
        resolvedValue = defaultValue;
    }

    return resolvedValue;
}

const Json::Value & GetTexts(const Json::Value & tc, Optional<const char *> optionalCountryCode,
                             Optional<const char *> optionalLanguageCode)
{
    const char * defaultCountry = tc[kFieldDefaultCountry].asCString();
    const char * chosenCountry  = ResolveValueOrDefault(tc[kFieldCountryEntries], optionalCountryCode, defaultCountry, "country");
    auto & countryEntry         = tc[kFieldCountryEntries][chosenCountry];

    const char * defaultLanguage = countryEntry[kFieldDefaultLanguage].asCString();
    const char * chosenLanguage =
        ResolveValueOrDefault(countryEntry[kFieldLanguageEntries], optionalLanguageCode, defaultLanguage, "language");
    auto & languageEntry = countryEntry[kFieldLanguageEntries][chosenLanguage];

    return languageEntry;
}

void PrintText(const Json::Value & json)
{
    auto title        = renderHTMLInTerminal(Center(ToUpperCase(json[kFieldTitle].asCString())), kTitleAllowedTags);
    auto text         = renderHTMLInTerminal(json[kFieldText].asCString());
    auto userQuestion = renderHTMLInTerminal(kAcceptTerms);
    auto required     = json[kFieldRequired].asBool() ? kRequiredTerms : kOptionalTerms;

    printf("%s", kHorizontalLine);
    printf("%s", title.c_str());
    printf("%s", kHorizontalLine);
    printf("%s", text.c_str());
    printf("%s", kHorizontalLine);
    printf("[%s] %s", required, userQuestion.c_str());
}

bool AcknowledgeText()
{
    while (true)
    {
        std::string userInput;
        std::getline(std::cin, userInput);

        VerifyOrReturnValue(!userInput.empty() && userInput != "Y" && userInput != "y", true);
        VerifyOrReturnValue(userInput != "N" && userInput != "n", false);

        ChipLogError(chipTool, "%s", kErrorInvalidInput);
    }
}

} // namespace

CHIP_ERROR DisplayTermsAndConditions(const Json::Value & tc, uint16_t & outVersion, uint16_t & outUserResponse,
                                     Optional<const char *> countryCode, Optional<const char *> languageCode)
{
    VerifyOrReturnError(CanCastTo<uint16_t>(tc[kFieldSchemaVersion].asUInt()), CHIP_ERROR_INVALID_ARGUMENT);
    outVersion = static_cast<uint16_t>(tc[kFieldSchemaVersion].asUInt());

    auto texts = GetTexts(tc, countryCode, languageCode);
    for (const auto & text : texts)
    {
        PrintText(text);

        if (AcknowledgeText())
        {
            auto ordinal = text[kFieldOrdinal].asUInt();
            VerifyOrReturnError(ordinal < 16, CHIP_ERROR_INVALID_ARGUMENT); // Only 16 bits are available for user response
            uint16_t shiftedValue = static_cast<uint16_t>((1U << (ordinal & 0x0F)) & 0xFFFF);
            outUserResponse |= shiftedValue;
        }
    }
    return CHIP_NO_ERROR;
}
} // namespace dcl
} // namespace tool
} // namespace chip

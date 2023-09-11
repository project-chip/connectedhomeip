/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ContentLauncher.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR LaunchURLCommand::Invoke(const char * contentUrl, const char * contentDisplayStr,
                                    Optional<ContentLauncher::Structs::BrandingInformationStruct::Type> brandingInformation,
                                    std::function<void(CHIP_ERROR)> responseCallback)
{
    ContentLauncher::Commands::LaunchURL::Type request;
    request.contentURL          = CharSpan::fromCharString(contentUrl);
    request.displayString       = Optional<CharSpan>(CharSpan::fromCharString(contentDisplayStr));
    request.brandingInformation = brandingInformation;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR LaunchContentCommand::Invoke(ContentLauncher::Structs::ContentSearchStruct::Type search, bool autoPlay,
                                        Optional<chip::CharSpan> data, std::function<void(CHIP_ERROR)> responseCallback)
{
    ContentLauncher::Commands::LaunchContent::Type request;
    request.search   = search;
    request.autoPlay = autoPlay;
    request.data     = data;
    return MediaCommandBase::Invoke(request, responseCallback);
}

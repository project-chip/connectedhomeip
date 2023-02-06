/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

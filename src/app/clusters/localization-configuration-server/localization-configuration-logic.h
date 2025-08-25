/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <app/AttributeValueEncoder.h>
#include <app/persistence/String.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {

class LocalizationConfigurationServerLogic
{
public:
    LocalizationConfigurationServerLogic() = default;
    CHIP_ERROR SetActiveLocale(CharSpan activeLocale);
    CharSpan GetActiveLocale();
    CHIP_ERROR ReadSupportedLocales(AttributeValueEncoder & aEncoder);
    CHIP_ERROR IsSupportedLocale(CharSpan newLangtag, MutableCharSpan & validLocale);

private:
    Storage::String<35> mActiveLocale;
};
} // namespace Clusters
} // namespace app
} // namespace chip

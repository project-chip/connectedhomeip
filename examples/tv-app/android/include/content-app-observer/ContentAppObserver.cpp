/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ContentAppObserver.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>

using namespace chip;
using namespace chip::app::Clusters::ContentAppObserver;

ContentAppObserver::ContentAppObserver()
{
    // Create Test Data
}

void ContentAppObserver::HandleContentAppMessage(chip::app::CommandResponseHelper<ContentAppMessageResponse> & helper,
                                                 const chip::Optional<chip::CharSpan> & data, const chip::CharSpan & encodingHint)
{
    ChipLogProgress(Zcl, "ContentAppObserver::HandleContentAppMessage");
}

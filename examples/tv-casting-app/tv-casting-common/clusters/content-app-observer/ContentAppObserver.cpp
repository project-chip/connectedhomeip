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

using namespace std;
using namespace chip;
using namespace chip::app::Clusters::ContentAppObserver;

ContentAppObserverManager::ContentAppObserverManager()
{
    // Create Test Data
}

void ContentAppObserverManager::HandleContentAppMessage(chip::app::CommandResponseHelper<ContentAppMessageResponse> & helper,
                                                        const chip::Optional<chip::CharSpan> & data,
                                                        const chip::CharSpan & encodingHint)
{
    ChipLogProgress(Zcl, "ContentAppObserverManager::HandleContentAppMessage");

    string dataString(data.HasValue() ? data.Value().data() : "", data.HasValue() ? data.Value().size() : 0);
    string encodingHintString(encodingHint.data(), encodingHint.size());

    ChipLogProgress(Zcl, "ContentAppObserverManager::HandleContentAppMessage TEST CASE hint=%s data=%s ",
                    encodingHintString.c_str(), dataString.c_str());

    ContentAppMessageResponse response;
    // TODO: Insert code here
    response.data         = chip::MakeOptional(CharSpan::fromCharString("exampleData"));
    response.encodingHint = chip::MakeOptional(CharSpan::fromCharString(encodingHintString.c_str()));
    response.status       = StatusEnum::kSuccess;
    helper.Success(response);
}

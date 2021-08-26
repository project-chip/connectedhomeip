/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/enums.h>
#include <app/util/basic-types.h>

struct Identify
{
    using onIdentifyStart    = void (*)(Identify *);
    using onIdentifyStop     = onIdentifyStart;
    using onEffectIdentifier = onIdentifyStart;

    Identify(chip::EndpointId endpoint, onIdentifyStart onIdentifyStart, onIdentifyStop onIdentifyStop,
             EmberAfIdentifyIdentifyType identifyType, onEffectIdentifier onEffectIdentifier = nullptr,
             EmberAfIdentifyEffectIdentifier effectIdentifier = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK,
             EmberAfIdentifyEffectVariant effectVariant       = EMBER_ZCL_IDENTIFY_EFFECT_VARIANT_DEFAULT);
    ~Identify();

    chip::EndpointId mEndpoint;
    onIdentifyStart mOnIdentifyStart = nullptr;
    onIdentifyStop mOnIdentifyStop   = nullptr;
    onEffectIdentifier mOnEffectIdentifier;
    EmberAfIdentifyEffectIdentifier mCurrentEffectIdentifier;
    EmberAfIdentifyEffectIdentifier mTargetEffectIdentifier;
    uint8_t mEffectVariant;
    bool mActive = false;
};

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
    /**
     * Callbacks are not thread safe. To access the identify struct please
     * consider using the LockChipStack / UnlockChipStack functions of the PlatformMgr.
     */
    using onIdentifyStartCb    = void (*)(Identify *);
    using onIdentifyStopCb     = onIdentifyStartCb;
    using onEffectIdentifierCb = onIdentifyStartCb;

    /**
     * @brief Construct a new Identify object
     *
     * Please note: The app should create a instance of this object to handle
     * the identify.
     *
     * @param endpoint endpoint of the cluster
     * @param onIdentifyStart callback to indicate to the app to start identifiying
     * @param onIdentifyStop callback to indicate to the app to stop identifiying
     * @param identifyType initial identifying type
     * @param onEffectIdentifier if supported by the app, callback to the app to indicate change of effect identifier
     * @param effectIdentifier if supported by the app, initial identify effect
     * @param effectVariant if supported by the app, initial effect variant
     */
    Identify(chip::EndpointId endpoint, onIdentifyStartCb onIdentifyStart, onIdentifyStopCb onIdentifyStop,
             EmberAfIdentifyIdentifyType identifyType, onEffectIdentifierCb onEffectIdentifier = nullptr,
             EmberAfIdentifyEffectIdentifier effectIdentifier = EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK,
             EmberAfIdentifyEffectVariant effectVariant       = EMBER_ZCL_IDENTIFY_EFFECT_VARIANT_DEFAULT);
    ~Identify();

    chip::EndpointId mEndpoint;
    onIdentifyStartCb mOnIdentifyStart = nullptr;
    onIdentifyStopCb mOnIdentifyStop   = nullptr;
    EmberAfIdentifyIdentifyType mIdentifyType;
    onEffectIdentifierCb mOnEffectIdentifier;
    EmberAfIdentifyEffectIdentifier mCurrentEffectIdentifier;
    EmberAfIdentifyEffectIdentifier mTargetEffectIdentifier;
    uint8_t mEffectVariant;
    bool mActive            = false;
    Identify * nextIdentify = nullptr;

    bool hasNext() { return this->nextIdentify != nullptr; }

    Identify * next() { return this->nextIdentify; }

    void setNext(Identify * inst) { this->nextIdentify = inst; }
};

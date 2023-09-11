/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-enums.h>
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
             chip::app::Clusters::Identify::IdentifyTypeEnum identifyType, onEffectIdentifierCb onEffectIdentifier = nullptr,
             chip::app::Clusters::Identify::EffectIdentifierEnum effectIdentifier =
                 chip::app::Clusters::Identify::EffectIdentifierEnum::kBlink,
             chip::app::Clusters::Identify::EffectVariantEnum effectVariant =
                 chip::app::Clusters::Identify::EffectVariantEnum::kDefault);
    ~Identify();

    chip::EndpointId mEndpoint;
    onIdentifyStartCb mOnIdentifyStart = nullptr;
    onIdentifyStopCb mOnIdentifyStop   = nullptr;
    chip::app::Clusters::Identify::IdentifyTypeEnum mIdentifyType;
    onEffectIdentifierCb mOnEffectIdentifier;
    chip::app::Clusters::Identify::EffectIdentifierEnum mCurrentEffectIdentifier;
    chip::app::Clusters::Identify::EffectIdentifierEnum mTargetEffectIdentifier;
    chip::app::Clusters::Identify::EffectVariantEnum mEffectVariant;
    bool mActive            = false;
    Identify * nextIdentify = nullptr;

    bool hasNext() { return this->nextIdentify != nullptr; }

    Identify * next() { return this->nextIdentify; }

    void setNext(Identify * inst) { this->nextIdentify = inst; }
};

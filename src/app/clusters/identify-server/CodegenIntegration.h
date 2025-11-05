/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app/TimerDelegateDefault.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/util/basic-types.h>

struct Identify
{
    using onIdentifyStartCb    = void (*)(Identify *);
    using onIdentifyStopCb     = onIdentifyStartCb;
    using onEffectIdentifierCb = onIdentifyStartCb;

    /**
     * @brief Construct a new Identify object
     *
     * This provides backward compatibility for existing codegen-based implementations
     * by redirecting to the new code-driven implementation located in IdentifyCluster.h/.cpp.
     * New implementations should use IdentifyCluster.h/.cpp directly.
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
    Identify(
        chip::EndpointId endpoint, onIdentifyStartCb onIdentifyStart, onIdentifyStopCb onIdentifyStop,
        chip::app::Clusters::Identify::IdentifyTypeEnum identifyType, onEffectIdentifierCb onEffectIdentifier = nullptr,
        chip::app::Clusters::Identify::EffectIdentifierEnum effectIdentifier =
            chip::app::Clusters::Identify::EffectIdentifierEnum::kBlink,
        chip::app::Clusters::Identify::EffectVariantEnum effectVariant = chip::app::Clusters::Identify::EffectVariantEnum::kDefault,
        TimerDelegate * timerDelegate                       = nullptr);
    ~Identify();

    onIdentifyStartCb mOnIdentifyStart = nullptr;
    onIdentifyStopCb mOnIdentifyStop   = nullptr;
    bool mActive                       = false;
    chip::app::Clusters::Identify::IdentifyTypeEnum mIdentifyType;
    onEffectIdentifierCb mOnEffectIdentifier;
    chip::app::Clusters::Identify::EffectIdentifierEnum mCurrentEffectIdentifier;
    chip::app::Clusters::Identify::EffectVariantEnum mEffectVariant;

    // Intrusive list pointer to the next legacy identify struct instance
    Identify * nextIdentify = nullptr;

    // CodeDriven cluster instance
    chip::app::RegisteredServerCluster<chip::app::Clusters::IdentifyCluster> mCluster;
};

chip::app::Clusters::IdentifyCluster * FindIdentifyClusterOnEndpoint(chip::EndpointId endpoint);

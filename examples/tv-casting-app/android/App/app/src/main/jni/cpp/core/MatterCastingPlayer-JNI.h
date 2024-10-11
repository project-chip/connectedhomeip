/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#include "../support/MatterCallback-JNI.h"
#include "core/CastingPlayer.h" // from tv-casting-common

#include <jni.h>

namespace matter {
namespace casting {
namespace core {

/**
 * This class is used to manage the JNI callbacks and C++ to Java conversions for the CastingPlayer.
 */
class MatterCastingPlayerJNI
{
public:
    // Member initializer list
    MatterCastingPlayerJNI() :
        mConnectionSuccessHandler([](void *) { return nullptr; }),
        mConnectionFailureHandler(matter::casting::support::convertMatterErrorFromCppToJava),
        mCommissionerDeclarationHandler(matter::casting::support::convertCommissionerDeclarationFromCppToJava)
    {}
    support::MatterCallbackJNI<void *> mConnectionSuccessHandler;
    support::MatterCallbackJNI<CHIP_ERROR> mConnectionFailureHandler;
    support::MatterCallbackJNI<chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration> mCommissionerDeclarationHandler;

    static MatterCastingPlayerJNI & getInstance() { return sInstance; }
    auto getConnectCallback() const { return ConnectCallback; }
    auto getCommissionerDeclarationCallback() const { return CommissionerDeclarationCallback; }

private:
    friend MatterCastingPlayerJNI & MatterCastingPlayerJNIMgr();
    static MatterCastingPlayerJNI sInstance;

    // Handles the connection complete event and calls the ConnectionCallbacks onSuccess or onFailure callback provided by the Java
    // client. This callback is called by the cpp layer when the connection process has ended, regardless of whether it was
    // successful or not.
    static void ConnectCallback(CHIP_ERROR err, CastingPlayer * playerPtr);
    // Handles the Commissioner Declaration event and calls the ConnectionCallbacks onCommissionerDeclaration callback provided by
    // the Java client. This callback is called by the cpp layer when the Commissionee receives a CommissionerDeclaration message
    // from the CastingPlayer/Commissioner.
    static void CommissionerDeclarationCallback(const chip::Transport::PeerAddress & source,
                                                chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd);
};

inline class MatterCastingPlayerJNI & MatterCastingPlayerJNIMgr()
{
    return MatterCastingPlayerJNI::sInstance;
}
}; // namespace core
}; // namespace casting
}; // namespace matter

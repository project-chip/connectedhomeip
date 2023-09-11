/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>

#include <app/CommandResponseHelper.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {

/** @brief
 *    Defines methods for implementing application-specific logic for the Account Login Cluster.
 */
class Delegate
{
public:
    // helper method to allow the platform to facilitate providing the pin
    virtual void SetSetupPin(char * setupPin) = 0;

    virtual bool HandleLogin(const chip::CharSpan & tempAccountIdentifierString, const chip::CharSpan & setupPinString) = 0;
    virtual bool HandleLogout()                                                                                         = 0;
    virtual void HandleGetSetupPin(CommandResponseHelper<Commands::GetSetupPINResponse::Type> & helper,
                                   const chip::CharSpan & tempAccountIdentifierString)                                  = 0;
    virtual void GetSetupPin(char * setupPin, size_t setupPinSize, const chip::CharSpan & tempAccountIdentifierString)  = 0;

    virtual ~Delegate() = default;
};

} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip

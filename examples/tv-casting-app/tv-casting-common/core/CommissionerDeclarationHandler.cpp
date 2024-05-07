/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "CommissionerDeclarationHandler.h"

#include "Types.h"

namespace matter {
namespace casting {
namespace core {

CommissionerDeclarationHandler * CommissionerDeclarationHandler::sCommissionerDeclarationHandler_ = nullptr;

CommissionerDeclarationHandler * CommissionerDeclarationHandler::GetInstance()
{
    if (sCommissionerDeclarationHandler_ == nullptr)
    {
        sCommissionerDeclarationHandler_ = new CommissionerDeclarationHandler();
    }
    return sCommissionerDeclarationHandler_;
}

// TODO: In the following PRs. Implement setHandler() for CommissionerDeclaration messages and expose messages to higher layers for
// Linux, Android and iOS.
void CommissionerDeclarationHandler::OnCommissionerDeclarationMessage(const chip::Transport::PeerAddress & source,
                                        chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd)
{
    ChipLogProgress(AppServer, "CommissionerDeclarationHandler::OnCommissionerDeclarationMessage() called TODO: handle message");
    cd.DebugLog();
}

}; // namespace core
}; // namespace casting
}; // namespace matter

/*
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
#pragma once

#include <messaging/ExchangeContext.h>

namespace chip {
namespace app {
namespace InteractionModel {

// Context for a currently executing request
class RequestContext
{
public:
    virtual ~RequestContext() = default;

    /// Valid ONLY during synchronous handling of a Read/Write/Invoke
    ///
    /// Used sparingly, however some operations will require these. An example
    /// usage is "Operational Credentials aborting communications on removed fabrics"
    ///
    /// Callers MUST check for null here (e.g. unit tests mocks may set this to
    /// nullptr due to object complexity)
    virtual Messaging::ExchangeContext * CurrentExchange() = 0;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip

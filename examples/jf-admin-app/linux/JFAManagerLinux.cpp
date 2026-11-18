/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "JFAManagerLinux.h"

namespace chip {

JFAManagerLinux & JFAManagerLinux::GetDefaultInstance()
{
    static JFAManagerLinux sJFA;
    return sJFA;
}

// Provides the chip::JFAMgr() accessor declared in JFAManagerBase.h for the
// Linux example. Shared code (JFAManagerBase.cpp, JointFabric.h RPC server)
// never names the concrete derived type.
JFAManagerBase & JFAMgr()
{
    return JFAManagerLinux::GetDefaultInstance();
}

} // namespace chip

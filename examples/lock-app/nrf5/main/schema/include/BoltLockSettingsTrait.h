/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#ifndef _WEAVE_TRAIT_SECURITY__BOLT_LOCK_SETTINGS_TRAIT_H_
#define _WEAVE_TRAIT_SECURITY__BOLT_LOCK_SETTINGS_TRAIT_H_

#include <Weave/Profiles/data-management/DataManagement.h>
#include <Weave/Support/SerializationUtils.h>



namespace Schema {
namespace Weave {
namespace Trait {
namespace Security {
namespace BoltLockSettingsTrait {

extern const nl::Weave::Profiles::DataManagement::TraitSchemaEngine TraitSchema;

enum {
      kWeaveProfileId = (0x0U << 16) | 0xe08U
};

//
// Properties
//

enum {
    kPropertyHandle_Root = 1,

    //---------------------------------------------------------------------------------------------------------------------------//
    //  Name                                IDL Type                            TLV Type           Optional?       Nullable?     //
    //---------------------------------------------------------------------------------------------------------------------------//

    //
    //  auto_relock_on                      bool                                 bool              NO              NO
    //
    kPropertyHandle_AutoRelockOn = 2,

    //
    //  auto_relock_duration                google.protobuf.Duration             uint32 seconds    NO              NO
    //
    kPropertyHandle_AutoRelockDuration = 3,

    //
    // Enum for last handle
    //
    kLastSchemaHandle = 3,
};

} // namespace BoltLockSettingsTrait
} // namespace Security
} // namespace Trait
} // namespace Weave
} // namespace Schema
#endif // _WEAVE_TRAIT_SECURITY__BOLT_LOCK_SETTINGS_TRAIT_H_

/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
#ifndef _APPLICATION_KEYS_TRAIT_TAGS_H
#define _APPLICATION_KEYS_TRAIT_TAGS_H

#include <Profiles/data-management/TraitData.h>

/**
 *   @namespace Schema::CHIP::Trait::Auth::ApplicationKeysTrait
 *
 *   @brief
 *     This namespace includes all definitions and interfaces for the CHIP
 *     application keys trait.
 */

namespace Schema {
namespace chip {
namespace Trait {
namespace Auth {
namespace ApplicationKeysTrait {

/**
 * Epoch key data element tags.
 */
enum EpochKey
{
    //---------------------------------------------------------------------------------------------------------------------------//
    //  Name                                IDL Type                            TLV Type           Optional?       Nullable?     //
    //---------------------------------------------------------------------------------------------------------------------------//

    //
    //  key_id                              uint32                              unsigned int       NO              NO
    //
    kTag_EpochKey_KeyId = 1,

    //
    //  start_time                          google.protobuf.Timestamp           unsigned int       NO              NO
    //                                      (int64 milliseconds)
    kTag_EpochKey_StartTime = 2,

    //
    //  key                                 bytes                               array              NO              NO
    //
    kTag_EpochKey_Key = 3,
};

/**
 * Application master key data element tags.
 */
enum ApplicationMasterKey
{
    //---------------------------------------------------------------------------------------------------------------------------//
    //  Name                                IDL Type                            TLV Type           Optional?       Nullable?     //
    //---------------------------------------------------------------------------------------------------------------------------//

    //
    //  application_group_global_id         uint32                              unsigned int       NO              NO
    //
    kTag_ApplicationGroup_GlobalId = 1,

    //
    //  application_group_short_id          uint32                              unsigned int       NO              NO
    //
    kTag_ApplicationGroup_ShortId = 2,

    //
    //  key                                 bytes                               array              NO              NO
    //
    kTag_ApplicationGroup_Key = 3,
};

} // namespace ApplicationKeysTrait
} // namespace Auth
} // namespace Trait
} // namespace chip
} // namespace Schema

#endif // _APPLICATION_KEYS_TRAIT_TAGS_H

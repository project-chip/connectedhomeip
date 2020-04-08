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

#include <weave/trait/description/DeviceIdentityTrait.h>

namespace Schema {
namespace Weave {
namespace Trait {
namespace Description {
namespace DeviceIdentityTrait {

using namespace ::nl::Weave::Profiles::DataManagement;

//
// Property Table
//

const TraitSchemaEngine::PropertyInfo PropertyMap[] = {
    { kPropertyHandle_Root, 1 }, // vendor_id
    { kPropertyHandle_Root, 2 }, // vendor_id_description
    { kPropertyHandle_Root, 3 }, // vendor_product_id
    { kPropertyHandle_Root, 4 }, // product_id_description
    { kPropertyHandle_Root, 5 }, // product_revision
    { kPropertyHandle_Root, 6 }, // serial_number
    { kPropertyHandle_Root, 7 }, // software_version
    { kPropertyHandle_Root, 8 }, // manufacturing_date
    { kPropertyHandle_Root, 9 }, // device_id
    { kPropertyHandle_Root, 10 }, // fabric_id
};

//
// IsOptional Table
//

uint8_t IsOptionalHandleBitfield[] = {
        0x8a, 0x3
};

//
// IsNullable Table
//

uint8_t IsNullableHandleBitfield[] = {
        0x8a, 0x0
};

//
// Schema
//

const TraitSchemaEngine TraitSchema = {
    {
        kWeaveProfileId,
        PropertyMap,
        sizeof(PropertyMap) / sizeof(PropertyMap[0]),
        1,
#if (TDM_EXTENSION_SUPPORT) || (TDM_VERSIONING_SUPPORT)
        2,
#endif
        NULL,
        &IsOptionalHandleBitfield[0],
        NULL,
        &IsNullableHandleBitfield[0],
        NULL,
#if (TDM_EXTENSION_SUPPORT)
        NULL,
#endif
#if (TDM_VERSIONING_SUPPORT)
        NULL,
#endif
    }
};

} // namespace DeviceIdentityTrait
} // namespace Description
} // namespace Trait
} // namespace Weave
} // namespace Schema

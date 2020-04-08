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

#ifndef _WEAVE_TRAIT_DESCRIPTION__DEVICE_IDENTITY_TRAIT_H_
#define _WEAVE_TRAIT_DESCRIPTION__DEVICE_IDENTITY_TRAIT_H_

#include <Weave/Profiles/data-management/DataManagement.h>
#include <Weave/Support/SerializationUtils.h>



namespace Schema {
namespace Weave {
namespace Trait {
namespace Description {
namespace DeviceIdentityTrait {

extern const nl::Weave::Profiles::DataManagement::TraitSchemaEngine TraitSchema;

enum {
      kWeaveProfileId = (0x0U << 16) | 0x17U
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
    //  vendor_id                           uint32                               uint16            NO              NO
    //
    kPropertyHandle_VendorId = 2,

    //
    //  vendor_id_description               weave.common.StringRef               union             YES             YES
    //
    kPropertyHandle_VendorIdDescription = 3,

    //
    //  vendor_product_id                   uint32                               uint16            NO              NO
    //
    kPropertyHandle_VendorProductId = 4,

    //
    //  product_id_description              weave.common.StringRef               union             YES             YES
    //
    kPropertyHandle_ProductIdDescription = 5,

    //
    //  product_revision                    uint32                               uint16            NO              NO
    //
    kPropertyHandle_ProductRevision = 6,

    //
    //  serial_number                       string                               string            NO              NO
    //
    kPropertyHandle_SerialNumber = 7,

    //
    //  software_version                    string                               string            NO              NO
    //
    kPropertyHandle_SoftwareVersion = 8,

    //
    //  manufacturing_date                  string                               string            YES             YES
    //
    kPropertyHandle_ManufacturingDate = 9,

    //
    //  device_id                           uint64                               uint64            YES             NO
    //
    kPropertyHandle_DeviceId = 10,

    //
    //  fabric_id                           uint64                               uint64            YES             NO
    //
    kPropertyHandle_FabricId = 11,

    //
    // Enum for last handle
    //
    kLastSchemaHandle = 11,
};

} // namespace DeviceIdentityTrait
} // namespace Description
} // namespace Trait
} // namespace Weave
} // namespace Schema
#endif // _WEAVE_TRAIT_DESCRIPTION__DEVICE_IDENTITY_TRAIT_H_

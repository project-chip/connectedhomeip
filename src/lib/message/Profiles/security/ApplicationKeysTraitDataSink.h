/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file defines application keys trait data sink interfaces.
 *
 */

#ifndef APPLICATION_KEYS_TRAIT_DATA_SINK_H_
#define APPLICATION_KEYS_TRAIT_DATA_SINK_H_

#include <Profiles/data-management/TraitData.h>
#include <Profiles/security/ApplicationKeysStructSchema.h>
#include <Profiles/security/ApplicationKeysTrait.h>
#include <Profiles/security/CHIPApplicationKeys.h>

namespace Schema {
namespace chip {
namespace Trait {
namespace Auth {
namespace ApplicationKeysTrait {

/**
 *  @class ApplicationKeysTraitDataSink
 *
 *  @brief
 *    Contains interfaces for the CHIP application keys trait data sink.
 *
 */
class ApplicationKeysTraitDataSink : public chip::Profiles::DataManagement::TraitDataSink
{
public:
    ApplicationKeysTraitDataSink(void);

    void SetGroupKeyStore(chip::Profiles::Security::AppKeys::GroupKeyStoreBase * groupKeyStore);

    CHIP_ERROR OnEvent(uint16_t aType, void * aInEventParam) __OVERRIDE;

protected:
    chip::Profiles::Security::AppKeys::GroupKeyStoreBase * GroupKeyStore;

private:
    CHIP_ERROR SetLeafData(chip::Profiles::DataManagement::PropertyPathHandle aLeafHandle,
                           chip::TLV::TLVReader & aReader) __OVERRIDE;
};

} // namespace ApplicationKeysTrait
} // namespace Auth
} // namespace Trait
} // namespace chip
} // namespace Schema

#endif // APPLICATION_KEYS_TRAIT_DATA_SINK_H_

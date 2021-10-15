/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Access control data provider implementation.
 */

#pragma once

#include "DataProvider.h"

namespace chip {
namespace access {

/**
 * @class DataProviderImpl
 *
 * @brief Access control data provider implementation.
 */
class DataProviderImpl : public DataProvider
{
public:
    DataProviderImpl() = default;
    virtual ~DataProviderImpl() = default;

    CHIP_ERROR Init() override;
    void Finish() override;

    EntryIterator* Entries() const override;
    EntryIterator* Entries(FabricIndex fabricIndex) const override;
};

} // namespace access
} // namespace chip

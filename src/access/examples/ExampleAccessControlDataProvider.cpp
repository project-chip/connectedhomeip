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

#include "ExampleAccessControlDataProvider.h"

namespace {

using namespace chip::Access;

class ExampleDataProvider : public AccessControlDataProvider
{
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    void Finish() override {}

    EntryIterator * Entries() const override { return nullptr; }

    EntryIterator * Entries(FabricIndex fabricIndex) const override { return nullptr; }
};

} // namespace

namespace chip {
namespace Access {
namespace Examples {

AccessControlDataProvider * GetExampleAccessControlDataProvider()
{
    static ExampleDataProvider exampleProvider;
    return &exampleProvider;
}

} // namespace Examples
} // namespace Access
} // namespace chip

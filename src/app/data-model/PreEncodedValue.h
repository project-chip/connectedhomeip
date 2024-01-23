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

#include <lib/core/CHIPError.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVWriter.h>

namespace chip {
namespace app {
namespace DataModel {

/**
 * PreEncodedValue represents the value of an attribute or the value of a single
 * item in a list of non-fabric-scoped structs that has already been
 * encoded as TLV.
 */
class PreEncodedValue
{
public:
    /**
     * The data buffer backing the ByteSpan must outlive the PreEncodedValue
     * instance.
     */
    PreEncodedValue(const ByteSpan & aData);

    /**
     * Encodable object API implementation.
     */
    static constexpr bool kIsFabricScoped = false;
    CHIP_ERROR Encode(TLV::TLVWriter & aWriter, TLV::Tag aTag) const;

private:
    const ByteSpan mData;
};

} // namespace DataModel
} // namespace app
} // namespace chip

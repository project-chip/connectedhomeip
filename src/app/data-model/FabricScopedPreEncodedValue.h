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
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVWriter.h>

namespace chip {
namespace app {
namespace DataModel {

/**
 * FabridScopedPreEncodedValue represents the value of a single item in a list
 * of fabric-scoped structs that has already been encoded as TLV.  To enable
 * reading these values successfully, the struct must have the fabric index it
 * corresponds to encoded with the right tag.
 *
 * Note: Fabric-sensitive fields are currently not supported; there is no way to
 * specify which fields are fabric-sensitive.
 */
class FabricScopedPreEncodedValue
{
public:
    /**
     * The data buffer backing the ByteSpan must outlive the
     * FabricScopedPreEncodedValue instance.
     */
    FabricScopedPreEncodedValue(const ByteSpan & aData);

    /**
     * Encodable object API implementation.
     */
    static constexpr bool kIsFabricScoped = true;
    CHIP_ERROR EncodeForRead(TLV::TLVWriter & aWriter, TLV::Tag aTag, FabricIndex aFabricIndex) const;
    FabricIndex GetFabricIndex() const;

private:
    const ByteSpan mData;
};

} // namespace DataModel
} // namespace app
} // namespace chip

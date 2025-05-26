/*
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
#include "scan-response-encoder.h"

#include <app/data-model/EncodableToTLV.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

CHIP_ERROR ScanResponseEncoder::EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outerType));

    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kNetworkingStatus), mStatus));
    if (mDebugText.size() != 0)
    {
        ReturnErrorOnFailure(
            DataModel::Encode(writer, TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kDebugText), mDebugText));
    }

    {
        TLV::TLVType listContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(GetResultField()),
                                                   TLV::kTLVType_Array, listContainerType));

        ReturnErrorOnFailure(EncodeList(writer));
        ReturnErrorOnFailure(writer.EndContainer(listContainerType));
    }

    return writer.EndContainer(outerType);
}

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip

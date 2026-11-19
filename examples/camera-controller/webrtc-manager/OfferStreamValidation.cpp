/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "OfferStreamValidation.h"

#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

namespace {

// VideoStreams and AudioStreams each hold 1 to 16 stream IDs when present.
constexpr size_t kMaxStreamListLength = 16;

bool IsValidStreamList(const std::optional<std::vector<uint16_t>> & streams)
{
    return !streams.has_value() || (!streams->empty() && streams->size() <= kMaxStreamListLength);
}

} // namespace

CHIP_ERROR ValidateOfferStreams(const Optional<app::DataModel::Nullable<uint16_t>> & videoStreamId,
                                const Optional<app::DataModel::Nullable<uint16_t>> & audioStreamId,
                                const std::optional<std::vector<uint16_t>> & videoStreams,
                                const std::optional<std::vector<uint16_t>> & audioStreams, bool isNewSession)
{
    const bool hasStreamIds   = videoStreamId.HasValue() || audioStreamId.HasValue();
    const bool hasStreamLists = videoStreams.has_value() || audioStreams.has_value();

    if (!hasStreamIds && !hasStreamLists)
    {
        ChipLogError(Camera, "One of VideoStreamID, AudioStreamID, VideoStreams or AudioStreams must be present");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (isNewSession && hasStreamIds && hasStreamLists)
    {
        ChipLogError(Camera, "VideoStreamID and AudioStreamID cannot be combined with VideoStreams or AudioStreams");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!IsValidStreamList(videoStreams) || !IsValidStreamList(audioStreams))
    {
        ChipLogError(Camera, "VideoStreams and AudioStreams must each hold 1 to %u stream IDs",
                     static_cast<unsigned>(kMaxStreamListLength));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

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

#pragma once

#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>

#include <cstdint>
#include <optional>
#include <vector>

/**
 * Checks the stream fields of a SolicitOffer or ProvideOffer request before it is sent:
 *
 *   - At least one of VideoStreamID, AudioStreamID, VideoStreams and AudioStreams is present.
 *   - A new session does not combine VideoStreamID / AudioStreamID with VideoStreams / AudioStreams. The provider rejects
 *     that combination with INVALID_COMMAND.
 *   - A present VideoStreams or AudioStreams list holds 1 to 16 stream IDs.
 *
 * @param isNewSession False for a ProvideOffer re-offer (non-null WebRTCSessionID). The provider ignores the stream
 *                     fields of a re-offer, so they may be combined.
 * @return CHIP_ERROR_INVALID_ARGUMENT if a check fails, otherwise CHIP_NO_ERROR.
 */
CHIP_ERROR ValidateOfferStreams(const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & videoStreamId,
                                const chip::Optional<chip::app::DataModel::Nullable<uint16_t>> & audioStreamId,
                                const std::optional<std::vector<uint16_t>> & videoStreams,
                                const std::optional<std::vector<uint16_t>> & audioStreams, bool isNewSession);

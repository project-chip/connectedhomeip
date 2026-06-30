/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/clusters/audio-control-server/AudioControlCluster.h>
#include <app/clusters/audio-control-server/AudioControlDelegate.h>

namespace chip::app::Clusters::AudioControl {

/// Returns the cluster instance registered on the given endpoint, nullptr otherwise.
AudioControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

/// Convenience helper to set the volume from application code.
CHIP_ERROR SetVolume(EndpointId endpointId, uint16_t volume);

/// Convenience helper to update the soft-muted state (e.g., hardware-initiated mute).
CHIP_ERROR SetSoftMuted(EndpointId endpointId, bool softMuted);

/// Convenience helper to update the physically-muted state (e.g., from a hardware button).
CHIP_ERROR SetPhysicallyMuted(EndpointId endpointId, bool physicallyMuted);

/// Convenience helpers to report hardware-initiated EQ changes (BEQ feature, no delegate call).
CHIP_ERROR SetBass(EndpointId endpointId, int16_t bass);
CHIP_ERROR SetMid(EndpointId endpointId, int16_t mid);
CHIP_ERROR SetTreble(EndpointId endpointId, int16_t treble);

/// Register the application delegate for an endpoint.
/// Must be called before MatterAudioControlClusterInitCallback fires for that endpoint.
void SetDelegate(EndpointId endpointId, AudioControlDelegate * delegate);

} // namespace chip::app::Clusters::AudioControl

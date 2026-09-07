/*
 *
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

// Registers the code-driven Audio Control cluster for the tv-app Speaker endpoint
// (MA-speaker, endpoint 2) only.
//
// The cluster is configured with the BasicEqualizer (BEQ) feature: the FeatureMap is
// 0x02. Every optional attribute the cluster supports is enabled -- the three EQ bands
// (Bass, Mid, Treble), PhysicallyMuted, MaxUserVolume, MaxDeviceVolumeDB (allowed
// because the Decibel feature is off), StartUpMuted and StartUpVolume -- and
// MinCorrection/MaxCorrection are exposed because the feature is set. All six commands
// (Mute, Unmute, ToggleMuted, SetVolume, IncreaseVolume, DecreaseVolume) are mandatory
// and always accepted.
//
// This cluster is registered purely through CodegenDataModelProvider's cluster
// registry -- it is intentionally not present in tv-app.zap. Adding a ZAP entry would
// make the generated code call MatterAudioControlClusterInitCallback() and register a
// second instance for (endpoint 2, 0x0512) via the ember codegen path, colliding with
// the instance created here.

namespace chip::app::Clusters::Speaker {

// Must be called after Server::Init() has completed (the registry context is set up by
// then). Registers the Audio Control cluster on endpoint 2.
void InitAudioControl();

// Mirrors InitAudioControl(); safe to call even if Init did not run.
void ShutdownAudioControl();

} // namespace chip::app::Clusters::Speaker

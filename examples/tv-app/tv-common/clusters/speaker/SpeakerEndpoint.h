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

namespace chip::app::Clusters::Speaker {

// Builds and registers the tv-app MA-speaker endpoint (endpoint 2): a SpeakerAudioCoordinator
// owning the three code-driven clusters (On/Off, Level Control, Audio Control), plus a logging
// hardware delegate, seeded from tv-app.zap. Call once from ApplicationInit(), after
// Server::Init().
void InitSpeaker();

// Mirrors InitSpeaker(). Call from ApplicationShutdown().
void ShutdownSpeaker();

} // namespace chip::app::Clusters::Speaker

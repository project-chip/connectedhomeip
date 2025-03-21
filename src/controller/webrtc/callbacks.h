/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <functional>

using SdpOfferCallback         = void (*)(const char *, const int);
using SdpAnswerCallback        = void (*)(const char *, const int);
using IceCallback              = void (*)(const char **, const int);
using ErrorCallback            = void (*)(const char *, const int);
using PeerConnectedCallback    = void (*)(const int);
using PeerDisconnectedCallback = void (*)(const int);
using StatsCollectedCallback   = void (*)(const char *, const int);

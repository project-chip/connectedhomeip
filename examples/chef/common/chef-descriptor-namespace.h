/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceCommonLevel = 5;
// Common Number Namespace: 5, tag 0 (Low)
constexpr const uint8_t kTagCommonLow = 0;
// Common Number Namespace: 5, tag 1 (Medium)
constexpr const uint8_t kTagCommonMedium = 1;
// Common Number Namespace: 5, tag 2 (High)
constexpr const uint8_t kTagCommonHigh = 2;

constexpr const uint8_t kNamespaceCommonNumber = 7;
// Common Number Namespace: 7, tag 0 (Zero)
constexpr const uint8_t kTagCommonZero = 0;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagCommonOne = 1;
// Common Number Namespace: 7, tag 2 (Two)
constexpr const uint8_t kTagCommonTwo = 2;

constexpr const uint8_t kNamespacePosition = 8;
// Common Position Namespace: 8, tag: 0 (Left)
constexpr const uint8_t kTagPositionLeft = 0;
// Common Position Namespace: 8, tag: 1 (Right)
constexpr const uint8_t kTagPositionRight = 1;
// Common Position Namespace: 8, tag: 2 (Top)
constexpr const uint8_t kTagPositionTop = 2;
// Common Position Namespace: 8, tag: 3 (Bottom)
constexpr const uint8_t kTagPositionBottom = 3;
// Common Position Namespace: 8, tag: 4 (Middle)
constexpr const uint8_t kTagPositionMiddle = 4;
// Common Position Namespace: 8, tag: 5 (Row)
constexpr const uint8_t kTagPositionRow = 5;
// Common Position Namespace: 8, tag: 6 (Column)
constexpr const uint8_t kTagPositionColumn = 6;

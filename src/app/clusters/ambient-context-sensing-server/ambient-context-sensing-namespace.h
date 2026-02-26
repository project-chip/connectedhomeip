/*
 *
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

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces

namespace chip::app::Clusters {

// Identified Object
constexpr const uint8_t kNamespaceIdentifiedObject = 0x49;

enum class TagIdentifiedObject : uint8_t
{
    kUnknown = 0x00,
    kAdult   = 0x01,
    kChild   = 0x02,
    kPerson  = 0x03,
    kRVC     = 0x04,
    kPet     = 0x05,
    kDog     = 0x06,
    kCat     = 0x07,
    kAnimal  = 0x08,
    kCar     = 0x09,
    kVehicle = 0x0a,
    kPackage = 0x0b,
    kClothes = 0x0c,
    kMax
};

// Identified Sound
constexpr const uint8_t kNamespaceIdentifiedSound = 0x4A;

enum class TagIdentifiedSound : uint8_t
{
    kUnknown        = 0x00,
    kObjectFall     = 0x01,
    kSnoring        = 0x02,
    kCoughing       = 0x03,
    kBarking        = 0x04,
    kShattering     = 0x05,
    kBabyCrying     = 0x06,
    kUtilityAlarm   = 0x07,
    kUrgentShouting = 0x08,
    kDoorbell       = 0x09,
    kKnocking       = 0x0a,
    kUrgentSiren    = 0x0b,
    kFaucetRunning  = 0x0c,
    kKettleBoiling  = 0x0d,
    kFanDryer       = 0x0e,
    kClapping       = 0x0f,
    kFingerSnapping = 0x10,
    kMeowing        = 0x11,
    kLaughing       = 0x12,
    kGlassBreaking  = 0x13,
    kDoorKnocking   = 0x14,
    kPersonTalking  = 0x15,
    kMax
};

// Identified Human Activity
constexpr const uint8_t kNamespaceIdentifiedHumanActivity = 0x4B;

enum class TagIdentifiedHumanActivity
{
    kUnknown          = 0x00,
    kPresence         = 0x01,
    kFall             = 0x02,
    kSleeping         = 0x03,
    kWalking          = 0x04,
    kWorkout          = 0x05,
    kSitting          = 0x06,
    kStanding         = 0x07,
    kDancing          = 0x08,
    kPackageDelivery  = 0x09,
    kPackageRetrieval = 0x0a,
    kMax
};

} // namespace chip::app::Clusters

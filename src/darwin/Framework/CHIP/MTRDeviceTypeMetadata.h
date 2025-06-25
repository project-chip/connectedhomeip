/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#include <lib/core/DataModelTypes.h>

NS_ASSUME_NONNULL_BEGIN

enum class MTRDeviceTypeClass {
    Utility,
    Simple,
    Node, // Might not be a real class, but we have it for Root Node for now.
};

struct MTRDeviceTypeData {
    chip::DeviceTypeId id;
    MTRDeviceTypeClass deviceClass;
    NSString * name;
};

// Returns null for unknown device types.
const MTRDeviceTypeData * _Nullable MTRDeviceTypeDataForID(chip::DeviceTypeId aDeviceTypeId);

NS_ASSUME_NONNULL_END

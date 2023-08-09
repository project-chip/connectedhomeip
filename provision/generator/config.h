/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

/**
 *    @file
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the Silicon Labs SDK.
 */

#pragma once

#include "app.h"

#include <platform/silabs/SilabsConfig.h>

using Key = chip::DeviceLayer::Internal::SilabsConfig::Key;

namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace Config {

CHIP_ERROR Init(void);
void Finish(void);
CHIP_ERROR Read(Key key, bool & val);
CHIP_ERROR Read(Key key, uint16_t & val, uint16_t def_value = 0);
CHIP_ERROR Read(Key key, uint32_t & val, uint32_t def_value = 0);
CHIP_ERROR Read(Key key, uint64_t & val);
CHIP_ERROR ReadStr(Key key, char * buf, size_t bufSize, size_t & outLen);
CHIP_ERROR ReadBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
CHIP_ERROR ReadCounter(uint8_t counterIdx, uint32_t & val);
CHIP_ERROR Write(Key key, bool val);
CHIP_ERROR Write(Key key, uint16_t val);
CHIP_ERROR Write(Key key, uint32_t val);
CHIP_ERROR Write(Key key, uint64_t val);
CHIP_ERROR WriteStr(Key key, const char * str);
CHIP_ERROR WriteStr(Key key, const char * str, size_t strLen);
CHIP_ERROR WriteBin(Key key, const uint8_t * data, size_t dataLen);
CHIP_ERROR WriteCounter(uint8_t counterIdx, uint32_t val);
CHIP_ERROR Clear(Key key);
bool Exists(Key key);
bool Exists(Key key, size_t & dataLen);
bool IsValid(Key key);
void Repack(void);

} // namespace Generator
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

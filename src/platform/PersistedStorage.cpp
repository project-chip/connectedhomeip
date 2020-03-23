/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides implementations for the OpenWeave persistent storage
 *          APIs.  This implementation is common across all platforms.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ConfigurationManager.h>

namespace nl {
namespace Weave {
namespace Platform {
namespace PersistedStorage {

using namespace ::nl::Weave::DeviceLayer;

WEAVE_ERROR Read(Key key, uint32_t & value)
{
    return ConfigurationMgr().ReadPersistedStorageValue(key, value);
}

WEAVE_ERROR Write(Key key, uint32_t value)
{
    return ConfigurationMgr().WritePersistedStorageValue(key, value);
}

} // PersistedStorage
} // Platform
} // Weave
} // nl


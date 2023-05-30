/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <platform/nxp/k32w/common/FactoryDataProvider.h>

/* This flag should be defined when the factory data contains
 * the DAC private key in plain text. It usually occurs in
 * manufacturing.
 *
 * The init phase will use S200 to export an encrypted blob,
 * then overwrite the private key section from internal flash.
 *
 * Should be used one time only for securing the private key.
 * The manufacturer will then flash the real image, which shall
 * not define this flag.
 */
#ifndef CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
#define CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY 0
#endif

namespace chip {
namespace DeviceLayer {

/**
 * This class extends the default FactoryDataProvider functionality
 * by leveraging the secure subsystem for signing messages.
 */

class FactoryDataProviderImpl : public FactoryDataProvider
{
public:
    CHIP_ERROR Init() override;
    CHIP_ERROR SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

#if CHIP_DEVICE_CONFIG_SECURE_DAC_PRIVATE_KEY
private:
    void ConvertDacKey();
#endif
};

} // namespace DeviceLayer
} // namespace chip

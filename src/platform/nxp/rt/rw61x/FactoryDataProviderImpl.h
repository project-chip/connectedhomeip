/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright 2023-2024 NXP
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

#include <platform/nxp/common/factory_data/legacy/FactoryDataProvider.h>

#define FACTORY_DATA_MAX_SIZE 4096

namespace chip {
namespace DeviceLayer {

/**
 * @brief This class provides Commissionable data and Device Attestation Credentials.
 *
 * This implementation allows to use the ELS hardware module to load the Matter factory
 * dataset in RAM at the boot.
 *
 *
 */

class FactoryDataProviderImpl : public FactoryDataProvider
{
public:
    static FactoryDataProviderImpl sInstance;

    CHIP_ERROR SearchForId(uint8_t searchedType, uint8_t * pBuf, size_t bufLength, uint16_t & length,
                           uint32_t * contentAddr = NULL);

    ~FactoryDataProviderImpl(){};

    CHIP_ERROR Init(void);
    CHIP_ERROR SignWithDacKey(const ByteSpan & digestToSign, MutableByteSpan & outSignBuffer);
    CHIP_ERROR SetEncryptionMode(EncryptionMode mode);

private:
    uint8_t factoryDataRamBuffer[FACTORY_DATA_MAX_SIZE];

    CHIP_ERROR ReplaceWithBlob(uint8_t * data, uint8_t * blob, size_t blobLen, uint32_t offset);
    CHIP_ERROR ELS_ExportBlob(uint8_t * data, size_t * dataLen, uint32_t & offset);
    CHIP_ERROR ELS_ConvertDacKey();
    CHIP_ERROR DecryptAesEcb(uint8_t * dest, uint8_t * source);

    CHIP_ERROR ReadAndCheckFactoryDataInFlash(void);
};

FactoryDataProvider & FactoryDataPrvdImpl();

} // namespace DeviceLayer
} // namespace chip

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
 *
 */

#pragma once
#include "SubImageProcessor.h"
#include "crypto/CHIPCryptoPAL.h"
#include "esp_ota_ops.h"

namespace chip {

/**
 * @brief App image processor.
 * This processor is responsible for doing the OTA update of the app image.
 * It is registered automatically at startup for the platform-defined app image ID.
 * @note This processor is only responsible for the app image. It does not handle the other images.
 */
class AppImageProcessor : public SubImageProcessor
{
public:
    AppImageProcessor();
    ~AppImageProcessor();
    CHIP_ERROR Init(const SubImageHeader & entry) override;
    bool IsInitialized() override;
    CHIP_ERROR IsReadyForOTA(DeviceState & state) override;
    CHIP_ERROR Write(ByteSpan & block) override;
    void Abort(AbortContext & context) override;
    CHIP_ERROR Apply() override;

private:
    const esp_partition_t * mPartition = nullptr;
    esp_ota_handle_t mOtaHandle        = 0;
    uint64_t mTotalLength              = 0;
    uint64_t mBytesReceived            = 0;
    DeviceState mState                 = DeviceState::kUnknown;
    SubImageHeader mEntry;
    Crypto::Hash_SHA256_stream mHasher;
};

} // namespace chip

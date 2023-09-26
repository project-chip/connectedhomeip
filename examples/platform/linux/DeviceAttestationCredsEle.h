/*
 *    Copyright 2023 NXP
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <credentials/DeviceAttestationCredsProvider.h>
#include "hsm_api.h"

namespace chip {
namespace Credentials {
namespace ele {

class EleDACProvider : public DeviceAttestationCredentialsProvider
{
public:
    EleDACProvider();
    ~EleDACProvider();
    static EleDACProvider & GetEleDACProvider();

    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override;

private:
    hsm_hdl_t hsm_session_hdl = 0;
    hsm_hdl_t key_store_hdl = 0;
};

} // namespace ele
} // namespace Credentials
} // namespace chip

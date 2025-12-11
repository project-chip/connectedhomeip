/*
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

#pragma once

#include "hsm_api.h"
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <memory>

namespace chip {
namespace Credentials {
namespace ele {

class EleManagerImpl
{
public:
    EleManagerImpl()          = default;
    virtual ~EleManagerImpl() = default;

    hsm_err_t EleDeleteKey(uint32_t keyId);
    CHIP_ERROR EleGenerateCSR(uint32_t keyId, uint8_t * csr, size_t & csrLength);
    hsm_err_t EleSignMessage(uint32_t keyId, const uint8_t * msg, size_t msgSize, uint8_t * sig, size_t sigSize);

    hsm_hdl_t hsm_session_hdl = 0;
    hsm_hdl_t key_store_hdl   = 0;
    hsm_hdl_t key_mgmt_hdl    = 0;
    bool ele_service_ready    = false;
};

class EleManagerKeystore : public EleManagerImpl
{
private:
    EleManagerKeystore();
    virtual ~EleManagerKeystore();

    EleManagerKeystore(const EleManagerKeystore &)             = delete;
    EleManagerKeystore & operator=(const EleManagerKeystore &) = delete;

    /* ELE does not allow multiple channels with same keyID, so I have to use a smart pointer
     * to manage ELE project here to make sure the ELE channel with kKeyStoreId = 0xAAAA be
     * opened only once */
    static std::weak_ptr<EleManagerKeystore> mWeakInstance;

public:
    static std::shared_ptr<EleManagerKeystore> getInstance();

    /* Unique keyID to open the key store channel in ELE HSM */
    static constexpr uint32_t kKeyStoreId  = 0xAAAA;
    static constexpr uint32_t kAuthenNonce = 0x1111;
};

class EleManagerAttestation : public EleManagerImpl
{
private:
    EleManagerAttestation();
    virtual ~EleManagerAttestation();

    EleManagerAttestation(const EleManagerAttestation &)             = delete;
    EleManagerAttestation & operator=(const EleManagerAttestation &) = delete;

    /* ELE does not allow multiple channels with same keyID, so I have to use a smart pointer
     * to manage ELE project here to make sure the ELE channel with kKeyStoreId = 0xBBBB be
     * opened only once */
    static std::weak_ptr<EleManagerAttestation> mWeakInstance;

public:
    static std::shared_ptr<EleManagerAttestation> getInstance();

    /* Unique keyID to open the key store channel in ELE HSM */
    static constexpr uint32_t kKeyStoreId  = 0xBBBB;
    static constexpr uint32_t kAuthenNonce = 0x2222;
};

} // namespace ele
} // namespace Credentials
} // namespace chip

/**
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

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#import "CHIPError.h"

#include <controller/OperationalCredentialsDelegate.h>

NS_ASSUME_NONNULL_BEGIN

class CHIPOperationalCredentialsDelegate : public chip::Controller::OperationalCredentialsDelegate {
public:
    CHIPOperationalCredentialsDelegate() {}

    ~CHIPOperationalCredentialsDelegate() {}

    CHIP_ERROR init();

    CHIP_ERROR GenerateNodeOperationalCertificate(chip::NodeId nodeId, chip::FabricId fabricId, const uint8_t * csr,
        size_t csrLength, int64_t serialNumber, uint8_t * certBuf, uint32_t certBufSize, uint32_t & outCertLen) override;

    CHIP_ERROR GetRootCACertificate(
        chip::FabricId fabricId, uint8_t * certBuf, uint32_t certBufSize, uint32_t & outCertLen) override;

private:
    CHIP_ERROR GenerateKeys();
    CHIP_ERROR LoadKeysFromKeyChain();
    CHIP_ERROR DeleteKeys();

    CHIP_ERROR ConvertToP256Keypair(SecKeyRef privateKey);

    bool ToChipEpochTime(uint32_t offset, uint32_t & epoch);

    chip::Crypto::P256Keypair mIssuerKey;
    uint32_t mIssuerId;

    const uint32_t kCertificateValiditySecs = 365 * 24 * 60 * 60;
    const NSString * kCHIPCAKeyLabel = @"chip.nodeopcerts.CA:0";

    id mKeyType = (id) kSecAttrKeyTypeECSECPrimeRandom;
    id mKeySize = @256;
};

NS_ASSUME_NONNULL_END

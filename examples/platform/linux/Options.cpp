/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "Options.h"

#include <app/server/OnboardingCodesUtil.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>

#include <credentials/examples/DeviceAttestationCredsExample.h>

using namespace chip;
using namespace chip::ArgParser;

namespace {
LinuxDeviceOptions gDeviceOptions;

// Follow the code style of command line arguments in case we need to add more options in the future.
enum
{
    kDeviceOption_BleDevice                             = 0x1000,
    kDeviceOption_WiFi                                  = 0x1001,
    kDeviceOption_Thread                                = 0x1002,
    kDeviceOption_Version                               = 0x1003,
    kDeviceOption_VendorID                              = 0x1004,
    kDeviceOption_ProductID                             = 0x1005,
    kDeviceOption_CustomFlow                            = 0x1006,
    kDeviceOption_Capabilities                          = 0x1007,
    kDeviceOption_Discriminator                         = 0x1008,
    kDeviceOption_Passcode                              = 0x1009,
    kDeviceOption_SecuredDevicePort                     = 0x100a,
    kDeviceOption_SecuredCommissionerPort               = 0x100b,
    kDeviceOption_UnsecuredCommissionerPort             = 0x100c,
    kDeviceOption_Command                               = 0x100d,
    kDeviceOption_PICS                                  = 0x100e,
    kDeviceOption_KVS                                   = 0x100f,
    kDeviceOption_InterfaceId                           = 0x1010,
    kDeviceOption_Spake2pVerifierBase64                 = 0x1011,
    kDeviceOption_Spake2pSaltBase64                     = 0x1012,
    kDeviceOption_Spake2pIterations                     = 0x1013,
    kDeviceOption_TraceFile                             = 0x1014,
    kDeviceOption_TraceLog                              = 0x1015,
    kDeviceOption_TraceDecode                           = 0x1016,
    kOptionCSRResponseCSRIncorrectType                  = 0x1017,
    kOptionCSRResponseCSRNonceIncorrectType             = 0x1018,
    kOptionCSRResponseCSRNonceTooLong                   = 0x1019,
    kOptionCSRResponseCSRNonceInvalid                   = 0x101a,
    kOptionCSRResponseNOCSRElementsTooLong              = 0x101b,
    kOptionCSRResponseAttestationSignatureIncorrectType = 0x101c,
    kOptionCSRResponseAttestationSignatureInvalid       = 0x101d,
    kOptionCSRResponseCSRExistingKeyPair                = 0x101e,
    kDeviceOption_TestEventTriggerEnableKey             = 0x101f,
    kCommissionerOption_FabricID                        = 0x1020,
};

constexpr unsigned kAppUsageLength = 64;

OptionDef sDeviceOptionDefs[] = {
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    { "ble-device", kArgumentRequired, kDeviceOption_BleDevice },
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    { "wifi", kNoArgument, kDeviceOption_WiFi },
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#if CHIP_ENABLE_OPENTHREAD
    { "thread", kNoArgument, kDeviceOption_Thread },
#endif // CHIP_ENABLE_OPENTHREAD
    { "version", kArgumentRequired, kDeviceOption_Version },
    { "vendor-id", kArgumentRequired, kDeviceOption_VendorID },
    { "product-id", kArgumentRequired, kDeviceOption_ProductID },
    { "custom-flow", kArgumentRequired, kDeviceOption_CustomFlow },
    { "capabilities", kArgumentRequired, kDeviceOption_Capabilities },
    { "discriminator", kArgumentRequired, kDeviceOption_Discriminator },
    { "passcode", kArgumentRequired, kDeviceOption_Passcode },
    { "spake2p-verifier-base64", kArgumentRequired, kDeviceOption_Spake2pVerifierBase64 },
    { "spake2p-salt-base64", kArgumentRequired, kDeviceOption_Spake2pSaltBase64 },
    { "spake2p-iterations", kArgumentRequired, kDeviceOption_Spake2pIterations },
    { "secured-device-port", kArgumentRequired, kDeviceOption_SecuredDevicePort },
    { "secured-commissioner-port", kArgumentRequired, kDeviceOption_SecuredCommissionerPort },
    { "unsecured-commissioner-port", kArgumentRequired, kDeviceOption_UnsecuredCommissionerPort },
    { "command", kArgumentRequired, kDeviceOption_Command },
    { "PICS", kArgumentRequired, kDeviceOption_PICS },
    { "KVS", kArgumentRequired, kDeviceOption_KVS },
    { "interface-id", kArgumentRequired, kDeviceOption_InterfaceId },
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    { "trace_file", kArgumentRequired, kDeviceOption_TraceFile },
    { "trace_log", kArgumentRequired, kDeviceOption_TraceLog },
    { "trace_decode", kArgumentRequired, kDeviceOption_TraceDecode },
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    { "cert_error_csr_incorrect_type", kNoArgument, kOptionCSRResponseCSRIncorrectType },
    { "cert_error_csr_existing_keypair", kNoArgument, kOptionCSRResponseCSRExistingKeyPair },
    { "cert_error_csr_nonce_incorrect_type", kNoArgument, kOptionCSRResponseCSRNonceIncorrectType },
    { "cert_error_csr_nonce_too_long", kNoArgument, kOptionCSRResponseCSRNonceTooLong },
    { "cert_error_csr_nonce_invalid", kNoArgument, kOptionCSRResponseCSRNonceInvalid },
    { "cert_error_nocsrelements_too_long", kNoArgument, kOptionCSRResponseNOCSRElementsTooLong },
    { "cert_error_attestation_signature_incorrect_type", kNoArgument, kOptionCSRResponseAttestationSignatureIncorrectType },
    { "cert_error_attestation_signature_invalid", kNoArgument, kOptionCSRResponseAttestationSignatureInvalid },
    { "enable-key", kArgumentRequired, kDeviceOption_TestEventTriggerEnableKey },
    { "commissioner-fabric-id", kArgumentRequired, kCommissionerOption_FabricID },
    {}
};

const char * sDeviceOptionHelp =
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    "  --ble-device <number>\n"
    "       The device number for CHIPoBLE, without 'hci' prefix, can be found by hciconfig.\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    "\n"
    "  --wifi\n"
    "       Enable WiFi management via wpa_supplicant.\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#if CHIP_ENABLE_OPENTHREAD
    "\n"
    "  --thread\n"
    "       Enable Thread management via ot-agent.\n"
#endif // CHIP_ENABLE_OPENTHREAD
    "\n"
    "  --version <version>\n"
    "       The version indication provides versioning of the setup payload.\n"
    "\n"
    "  --vendor-id <id>\n"
    "       The Vendor ID is assigned by the Connectivity Standards Alliance.\n"
    "\n"
    "  --product-id <id>\n"
    "       The Product ID is specified by vendor.\n"
    "\n"
    "  --custom-flow <Standard = 0 | UserActionRequired = 1 | Custom = 2>\n"
    "       A 2-bit unsigned enumeration specifying manufacturer-specific custom flow options.\n"
    "\n"
    "  --capabilities <None = 0, SoftAP = 1 << 0, BLE = 1 << 1, OnNetwork = 1 << 2>\n"
    "       Discovery Capabilities Bitmask which contains information about Device’s available technologies for device discovery.\n"
    "\n"
    "  --discriminator <discriminator>\n"
    "       A 12-bit unsigned integer match the value which a device advertises during commissioning.\n"
    "\n"
    "  --passcode <passcode>\n"
    "       A 27-bit unsigned integer, which serves as proof of possession during commissioning. \n"
    "       If not provided to compute a verifier, the --spake2p-verifier-base64 must be provided. \n"
    "\n"
    "  --spake2p-verifier-base64 <PASE verifier as base64>\n"
    "       A raw concatenation of 'W0' and 'L' (67 bytes) as base64 to override the verifier\n"
    "       auto-computed from the passcode, if provided.\n"
    "\n"
    "  --spake2p-salt-base64 <PASE salt as base64>\n"
    "       16-32 bytes of salt to use for the PASE verifier, as base64. If omitted, will be generated\n"
    "       randomly. If a --spake2p-verifier-base64 is passed, it must match against the salt otherwise\n"
    "       failure will arise.\n"
    "\n"
    "  --spake2p-iterations <PASE PBKDF iterations>\n"
    "       Number of PBKDF iterations to use. If omitted, will be 1000. If a --spake2p-verifier-base64 is\n"
    "       passed, the iteration counts must match that used to generate the verifier otherwise failure will\n"
    "       arise.\n"
    "\n"
    "  --secured-device-port <port>\n"
    "       A 16-bit unsigned integer specifying the listen port to use for secure device messages (default is 5540).\n"
    "\n"
    "  --secured-commissioner-port <port>\n"
    "       A 16-bit unsigned integer specifying the listen port to use for secure commissioner messages (default is 5542). Only "
    "valid when app is both device and commissioner\n"
    "\n"
    "  --unsecured-commissioner-port <port>\n"
    "       A 16-bit unsigned integer specifying the port to use for unsecured commissioner messages (default is 5550).\n"
    "\n"
    "  --commissioner-fabric-id <fabricid>\n"
    "       The fabric ID to be used when this device is a commissioner (default in code is 1).\n"
    "\n"
    "  --command <command-name>\n"
    "       A name for a command to execute during startup.\n"
    "\n"
    "  --PICS <filepath>\n"
    "       A file containing PICS items.\n"
    "\n"
    "  --KVS <filepath>\n"
    "       A file to store Key Value Store items.\n"
    "\n"
    "  --interface-id <interface>\n"
    "       A interface id to advertise on.\n"
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    "\n"
    "  --trace_file <file>\n"
    "       Output trace data to the provided file.\n"
    "  --trace_log <1/0>\n"
    "       A value of 1 enables traces to go to the log, 0 disables this (default 0).\n"
    "  --trace_decode <1/0>\n"
    "       A value of 1 enables traces decoding, 0 disables this (default 0).\n"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    "  --cert_error_csr_incorrect_type\n"
    "       Configure the CSRResponse to be built with an invalid CSR type.\n"
    "  --cert_error_csr_existing_keypair\n"
    "       Configure the CSRResponse to be built with a CSR where the keypair already exists.\n"
    "  --cert_error_csr_nonce_incorrect_type\n"
    "       Configure the CSRResponse to be built with an invalid CSRNonce type.\n"
    "  --cert_error_csr_nonce_too_long\n"
    "       Configure the CSRResponse to be built with a CSRNonce that is longer than expected.\n"
    "  --cert_error_csr_nonce_invalid\n"
    "       Configure the CSRResponse to be built with a CSRNonce that does not match the CSRNonce from the CSRRequest.\n"
    "  --cert_error_nocsrelements_too_long\n"
    "       Configure the CSRResponse to contains an NOCSRElements larger than the allowed RESP_MAX.\n"
    "  --cert_error_attestation_signature_incorrect_type\n"
    "       Configure the CSRResponse to be build with an invalid AttestationSignature type.\n"
    "  --cert_error_attestation_signature_invalid\n"
    "       Configure the CSRResponse to be build with an AttestationSignature that does not match what is expected.\n"
    "  --enable-key <key>\n"
    "       A 16-byte, hex-encoded key, used to validate TestEventTrigger command of Generial Diagnostics cluster\n"
    "\n";

bool Base64ArgToVector(const char * arg, size_t maxSize, std::vector<uint8_t> & outVector)
{
    size_t maxBase64Size = BASE64_ENCODED_LEN(maxSize);
    outVector.resize(maxSize);

    size_t argLen = strlen(arg);
    if (argLen > maxBase64Size)
    {
        return false;
    }

    size_t decodedLen = chip::Base64Decode32(arg, argLen, reinterpret_cast<uint8_t *>(outVector.data()));
    if (decodedLen == 0)
    {
        return false;
    }

    outVector.resize(decodedLen);
    return true;
}

bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {

    case kDeviceOption_BleDevice:
        if (!ParseInt(aValue, LinuxDeviceOptions::GetInstance().mBleDevice))
        {
            PrintArgError("%s: invalid value specified for ble device number: %s\n", aProgram, aValue);
            retval = false;
        }
        break;

    case kDeviceOption_WiFi:
        LinuxDeviceOptions::GetInstance().mWiFi = true;
        break;

    case kDeviceOption_Thread:
        LinuxDeviceOptions::GetInstance().mThread = true;
        break;

    case kDeviceOption_Version:
        LinuxDeviceOptions::GetInstance().payload.version = static_cast<uint8_t>(atoi(aValue));
        break;

    case kDeviceOption_VendorID:
        LinuxDeviceOptions::GetInstance().payload.vendorID = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_ProductID:
        LinuxDeviceOptions::GetInstance().payload.productID = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_CustomFlow:
        LinuxDeviceOptions::GetInstance().payload.commissioningFlow = static_cast<CommissioningFlow>(atoi(aValue));
        break;

    case kDeviceOption_Capabilities:
        LinuxDeviceOptions::GetInstance().payload.rendezvousInformation.Emplace().SetRaw(static_cast<uint8_t>(atoi(aValue)));
        break;

    case kDeviceOption_Discriminator: {
        uint16_t value = static_cast<uint16_t>(atoi(aValue));
        if (value >= 4096)
        {
            PrintArgError("%s: invalid value specified for discriminator: %s\n", aProgram, aValue);
            retval = false;
        }
        else
        {
            LinuxDeviceOptions::GetInstance().discriminator.SetValue(value);
        }
        break;
    }

    case kDeviceOption_Passcode:
        LinuxDeviceOptions::GetInstance().payload.setUpPINCode = static_cast<uint32_t>(atoi(aValue));
        break;

    case kDeviceOption_Spake2pSaltBase64: {
        constexpr size_t kMaxSize = chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length;
        std::vector<uint8_t> saltVector;

        bool success = Base64ArgToVector(aValue, kMaxSize, saltVector);

        if (!success)
        {
            PrintArgError("%s: ERROR: Base64 format for argument %s was invalid\n", aProgram, aName);
            retval = false;
            break;
        }

        if ((saltVector.size() < chip::Crypto::kSpake2p_Min_PBKDF_Salt_Length) ||
            (saltVector.size() > chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length))
        {
            PrintArgError("%s: ERROR: argument %s not in range [%u, %u]\n", aProgram, aName,
                          chip::Crypto::kSpake2p_Min_PBKDF_Salt_Length, chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length);
            retval = false;
            break;
        }

        LinuxDeviceOptions::GetInstance().spake2pSalt.SetValue(std::move(saltVector));
        break;
    }

    case kDeviceOption_Spake2pVerifierBase64: {
        constexpr size_t kMaxSize = chip::Crypto::kSpake2p_VerifierSerialized_Length;
        std::vector<uint8_t> serializedVerifier;

        bool success = Base64ArgToVector(aValue, kMaxSize, serializedVerifier);

        if (!success)
        {
            PrintArgError("%s: ERROR: Base64 format for argument %s was invalid\n", aProgram, aName);
            retval = false;
            break;
        }

        if (serializedVerifier.size() != chip::Crypto::kSpake2p_VerifierSerialized_Length)
        {
            PrintArgError("%s: ERROR: argument %s should contain base64 for a %u bytes octet string \n", aProgram, aName,
                          chip::Crypto::kSpake2p_VerifierSerialized_Length);
            retval = false;
            break;
        }

        LinuxDeviceOptions::GetInstance().spake2pVerifier.SetValue(std::move(serializedVerifier));
        break;
    }

    case kDeviceOption_Spake2pIterations: {
        errno              = 0;
        uint32_t iterCount = static_cast<uint32_t>(strtoul(aValue, nullptr, 0));
        if (errno == ERANGE)
        {
            PrintArgError("%s: ERROR: argument %s was not parsable as an integer\n", aProgram, aName);
            retval = false;
            break;
        }
        if ((iterCount < chip::Crypto::kSpake2p_Min_PBKDF_Iterations) || (iterCount > chip::Crypto::kSpake2p_Max_PBKDF_Iterations))
        {
            PrintArgError("%s: ERROR: argument %s not in range [%u, %u]\n", aProgram, aName,
                          chip::Crypto::kSpake2p_Min_PBKDF_Iterations, chip::Crypto::kSpake2p_Max_PBKDF_Iterations);
            retval = false;
            break;
        }

        LinuxDeviceOptions::GetInstance().spake2pIterations = iterCount;
        break;
    }

    case kDeviceOption_SecuredDevicePort:
        LinuxDeviceOptions::GetInstance().securedDevicePort = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_SecuredCommissionerPort:
        LinuxDeviceOptions::GetInstance().securedCommissionerPort = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_UnsecuredCommissionerPort:
        LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_Command:
        LinuxDeviceOptions::GetInstance().command = aValue;
        break;

    case kDeviceOption_PICS:
        LinuxDeviceOptions::GetInstance().PICS = aValue;
        break;

    case kDeviceOption_KVS:
        LinuxDeviceOptions::GetInstance().KVS = aValue;
        break;

    case kDeviceOption_InterfaceId:
        LinuxDeviceOptions::GetInstance().interfaceId =
            Inet::InterfaceId(static_cast<chip::Inet::InterfaceId::PlatformType>(atoi(aValue)));
        break;

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    case kDeviceOption_TraceFile:
        LinuxDeviceOptions::GetInstance().traceStreamFilename.SetValue(std::string{ aValue });
        break;
    case kDeviceOption_TraceLog:
        if (atoi(aValue) != 0)
        {
            LinuxDeviceOptions::GetInstance().traceStreamToLogEnabled = true;
        }
        break;
    case kDeviceOption_TraceDecode:
        if (atoi(aValue) != 0)
        {
            LinuxDeviceOptions::GetInstance().traceStreamDecodeEnabled = true;
        }
        break;
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

    case kOptionCSRResponseCSRIncorrectType:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.csrIncorrectType = true;
        break;
    case kOptionCSRResponseCSRExistingKeyPair:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.csrExistingKeyPair = true;
        break;
    case kOptionCSRResponseCSRNonceIncorrectType:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.csrNonceIncorrectType = true;
        break;
    case kOptionCSRResponseCSRNonceTooLong:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.csrNonceTooLong = true;
        break;
    case kOptionCSRResponseCSRNonceInvalid:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.csrNonceInvalid = true;
        break;
    case kOptionCSRResponseNOCSRElementsTooLong:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.nocsrElementsTooLong = true;
        break;
    case kOptionCSRResponseAttestationSignatureIncorrectType:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.attestationSignatureIncorrectType = true;
        break;
    case kOptionCSRResponseAttestationSignatureInvalid:
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.attestationSignatureInvalid = true;
        break;
    case kDeviceOption_TestEventTriggerEnableKey: {
        constexpr size_t kEnableKeyLength = sizeof(LinuxDeviceOptions::GetInstance().testEventTriggerEnableKey);

        if (Encoding::HexToBytes(aValue, strlen(aValue), LinuxDeviceOptions::GetInstance().testEventTriggerEnableKey,
                                 kEnableKeyLength) != kEnableKeyLength)
        {

            PrintArgError("%s: ERROR: invalid value specified for %s\n", aProgram, aName);
            retval = false;
        }

        break;
    }
    case kCommissionerOption_FabricID: {
        char * eptr;
        LinuxDeviceOptions::GetInstance().commissionerFabricId = (chip::FabricId) strtoull(aValue, &eptr, 0);
        break;
    }

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionSet sDeviceOptions = { HandleOption, sDeviceOptionDefs, "GENERAL OPTIONS", sDeviceOptionHelp };

OptionSet * sLinuxDeviceOptionSets[] = { &sDeviceOptions, nullptr, nullptr, nullptr };
} // namespace

CHIP_ERROR ParseArguments(int argc, char * const argv[], OptionSet * customOptions)
{
    // Index 0 is for the general Linux options
    uint8_t optionSetIndex = 1;
    if (customOptions != nullptr)
    {
        // If there are custom options, include it during arg parsing
        sLinuxDeviceOptionSets[optionSetIndex++] = customOptions;
    }

    char usage[kAppUsageLength];
    snprintf(usage, kAppUsageLength, "Usage: %s [options]", argv[0]);

    HelpOptions helpOptions(argv[0], usage, "1.0");
    sLinuxDeviceOptionSets[optionSetIndex] = &helpOptions;

    if (!ParseArgs(argv[0], argc, argv, sLinuxDeviceOptionSets))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

LinuxDeviceOptions & LinuxDeviceOptions::GetInstance()
{
    if (gDeviceOptions.dacProvider == nullptr)
    {
        gDeviceOptions.dacProvider = chip::Credentials::Examples::GetExampleDACProvider();
    }
    return gDeviceOptions;
}

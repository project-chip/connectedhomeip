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
#include <string>

#include "Options.h"

#include <setup_payload/OnboardingCodesUtil.h>

#include <crypto/CHIPCryptoPAL.h>
#include <json/json.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

#include <app/tests/suites/credentials/TestHarnessDACProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#if CHIP_ATTESTATION_TRUSTY_OS
#include <platform/Linux/DeviceAttestationCredsTrusty.h>
#endif

#if ENABLE_TRACING
#include <TracingCommandLineArgument.h> // nogncheck
#endif

#if CHIP_WITH_NLFAULTINJECTION
#include <inet/InetFaultInjection.h>
#include <lib/support/CHIPFaultInjection.h>
#include <system/SystemFaultInjection.h>
#endif

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
#include <messaging/ReliableMessageProtocolConfig.h>
#endif

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Platform;

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
using namespace chip::Access;
#endif

namespace {
LinuxDeviceOptions gDeviceOptions;

// Follow the code style of command line arguments in case we need to add more options in the future.
enum
{
    kDeviceOption_BleDevice = 0x1000,
    kDeviceOption_WiFi,
    kDeviceOption_Thread,
    kDeviceOption_ThreadNodeId,
    kDeviceOption_Version,
    kDeviceOption_VendorID,
    kDeviceOption_ProductID,
    kDeviceOption_CustomFlow,
    kDeviceOption_Capabilities,
    kDeviceOption_Discriminator,
    kDeviceOption_Passcode,
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    kDeviceOption_SecuredDevicePort,
    kDeviceOption_UnsecuredCommissionerPort,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    kDeviceOption_SecuredCommissionerPort,
    kCommissionerOption_FabricID,
#endif
    kDeviceOption_Command,
    kDeviceOption_PICS,
    kDeviceOption_KVSFilePath,
    kDeviceOption_KVSDataDirectory,
    kDeviceOption_KVSFactoryDataDirectory,
    kDeviceOption_KVSConfigDataDirectory,
    kDeviceOption_KVSCountersDataDirectory,
    kDeviceOption_InterfaceId,
    kDeviceOption_AppPipe,
    kDeviceOption_AppPipeOut,
    kDeviceOption_Spake2pVerifierBase64,
    kDeviceOption_Spake2pSaltBase64,
    kDeviceOption_Spake2pIterations,
    kDeviceOption_TraceFile,
    kDeviceOption_TraceLog,
    kDeviceOption_TraceDecode,
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    kDeviceOption_CommissioningArlEntries,
    kDeviceOption_ArlEntries,
#endif
    kOptionCSRResponseCSRIncorrectType,
    kOptionCSRResponseCSRNonceIncorrectType,
    kOptionCSRResponseCSRNonceTooLong,
    kOptionCSRResponseCSRNonceInvalid,
    kOptionCSRResponseNOCSRElementsTooLong,
    kOptionCSRResponseAttestationSignatureIncorrectType,
    kOptionCSRResponseAttestationSignatureInvalid,
    kOptionCSRResponseCSRExistingKeyPair,
    kDeviceOption_TestEventTriggerEnableKey,
    kTraceTo,
    kOptionSimulateNoInternalTime,
#if defined(PW_RPC_ENABLED)
    kOptionRpcServerPort,
#endif
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    kDeviceOption_SubscriptionCapacity,
#endif
    kDeviceOption_WiFiSupports5g,
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    kDeviceOption_SubscriptionResumptionRetryIntervalSec,
    kDeviceOption_IdleRetransmitTimeout,
    kDeviceOption_ActiveRetransmitTimeout,
    kDeviceOption_ActiveThresholdTime,
#endif
#if CHIP_WITH_NLFAULTINJECTION
    kDeviceOption_FaultInjection,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    kDeviceOption_WiFi_PAF,
#endif
    kDeviceOption_DacProvider,
#if CHIP_ATTESTATION_TRUSTY_OS
    kDeviceOption_TrustyDacProvider,
#endif
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    kDeviceOption_TermsAndConditions_Version,
    kDeviceOption_TermsAndConditions_Required,
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    kDeviceOption_icdActiveModeDurationMs,
    kDeviceOption_icdIdleModeDuration,
    kDeviceOption_icdShortIdleModeDuration,
#endif
#if ENABLE_CAMERA_SERVER
    kDeviceOption_Camera_DeferredOffer,
    kDeviceOption_Camera_TestVideosrc,
    kDeviceOption_Camera_TestAudiosrc,
    kDeviceOption_Camera_AudioPlayback,
    kDeviceOption_Camera_VideoDevice,
#endif
    kDeviceOption_VendorName,
    kDeviceOption_ProductName,
    kDeviceOption_HardwareVersionString,
    kDeviceOption_SerialNumber,
};

constexpr unsigned kAppUsageLength = 64;

OptionDef sDeviceOptionDefs[] = {
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    { "ble-controller", kArgumentRequired, kDeviceOption_BleDevice },
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    { "wifi", kNoArgument, kDeviceOption_WiFi },
    { "wifi-supports-5g", kNoArgument, kDeviceOption_WiFiSupports5g },
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    { "wifipaf", kArgumentRequired, kDeviceOption_WiFi_PAF },
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
#if CHIP_ENABLE_OPENTHREAD
#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
    { "thread-node-id", kArgumentRequired, kDeviceOption_ThreadNodeId },
#else
    { "thread", kNoArgument, kDeviceOption_Thread },
#endif
#endif // CHIP_ENABLE_OPENTHREAD
    { "version", kArgumentRequired, kDeviceOption_Version },
    { "vendor-id", kArgumentRequired, kDeviceOption_VendorID },
    { "product-id", kArgumentRequired, kDeviceOption_ProductID },
    { "vendor-name", kArgumentRequired, kDeviceOption_VendorName },
    { "product-name", kArgumentRequired, kDeviceOption_ProductName },
    { "hardware-version-string", kArgumentRequired, kDeviceOption_HardwareVersionString },
    { "serial-number", kArgumentRequired, kDeviceOption_SerialNumber },
    { "custom-flow", kArgumentRequired, kDeviceOption_CustomFlow },
    { "capabilities", kArgumentRequired, kDeviceOption_Capabilities },
    { "discriminator", kArgumentRequired, kDeviceOption_Discriminator },
    { "passcode", kArgumentRequired, kDeviceOption_Passcode },
    { "spake2p-verifier-base64", kArgumentRequired, kDeviceOption_Spake2pVerifierBase64 },
    { "spake2p-salt-base64", kArgumentRequired, kDeviceOption_Spake2pSaltBase64 },
    { "spake2p-iterations", kArgumentRequired, kDeviceOption_Spake2pIterations },
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    { "secured-device-port", kArgumentRequired, kDeviceOption_SecuredDevicePort },
    { "unsecured-commissioner-port", kArgumentRequired, kDeviceOption_UnsecuredCommissionerPort },
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    { "secured-commissioner-port", kArgumentRequired, kDeviceOption_SecuredCommissionerPort },
    { "commissioner-fabric-id", kArgumentRequired, kCommissionerOption_FabricID },
#endif
    { "command", kArgumentRequired, kDeviceOption_Command },
    { "PICS", kArgumentRequired, kDeviceOption_PICS },
    { "KVS", kArgumentRequired, kDeviceOption_KVSFilePath },
    { "kvs-data-directory", kArgumentRequired, kDeviceOption_KVSDataDirectory },
    { "kvs-factory-directory", kArgumentRequired, kDeviceOption_KVSFactoryDataDirectory },
    { "kvs-config-directory", kArgumentRequired, kDeviceOption_KVSConfigDataDirectory },
    { "kvs-counters-directory", kArgumentRequired, kDeviceOption_KVSCountersDataDirectory },
    { "interface-id", kArgumentRequired, kDeviceOption_InterfaceId },
    { "app-pipe", kArgumentRequired, kDeviceOption_AppPipe },
    { "app-pipe-out", kArgumentRequired, kDeviceOption_AppPipeOut },
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    { "trace_file", kArgumentRequired, kDeviceOption_TraceFile },
    { "trace_log", kArgumentRequired, kDeviceOption_TraceLog },
    { "trace_decode", kArgumentRequired, kDeviceOption_TraceDecode },
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    { "commissioning-arl-entries", kArgumentRequired, kDeviceOption_CommissioningArlEntries },
    { "arl-entries", kArgumentRequired, kDeviceOption_ArlEntries },
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    { "cert_error_csr_incorrect_type", kNoArgument, kOptionCSRResponseCSRIncorrectType },
    { "cert_error_csr_existing_keypair", kNoArgument, kOptionCSRResponseCSRExistingKeyPair },
    { "cert_error_csr_nonce_incorrect_type", kNoArgument, kOptionCSRResponseCSRNonceIncorrectType },
    { "cert_error_csr_nonce_too_long", kNoArgument, kOptionCSRResponseCSRNonceTooLong },
    { "cert_error_csr_nonce_invalid", kNoArgument, kOptionCSRResponseCSRNonceInvalid },
    { "cert_error_nocsrelements_too_long", kNoArgument, kOptionCSRResponseNOCSRElementsTooLong },
    { "cert_error_attestation_signature_incorrect_type", kNoArgument, kOptionCSRResponseAttestationSignatureIncorrectType },
    { "cert_error_attestation_signature_invalid", kNoArgument, kOptionCSRResponseAttestationSignatureInvalid },
    { "enable-key", kArgumentRequired, kDeviceOption_TestEventTriggerEnableKey },
#if ENABLE_TRACING
    { "trace-to", kArgumentRequired, kTraceTo },
#endif
    { "simulate-no-internal-time", kNoArgument, kOptionSimulateNoInternalTime },
#if defined(PW_RPC_ENABLED)
    { "rpc-server-port", kArgumentRequired, kOptionRpcServerPort },
#endif
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    { "subscription-capacity", kArgumentRequired, kDeviceOption_SubscriptionCapacity },
    { "subscription-resumption-retry-interval", kArgumentRequired, kDeviceOption_SubscriptionResumptionRetryIntervalSec },
    { "idle-retransmit-timeout", kArgumentRequired, kDeviceOption_IdleRetransmitTimeout },
    { "active-retransmit-timeout", kArgumentRequired, kDeviceOption_ActiveRetransmitTimeout },
    { "active-threshold-time", kArgumentRequired, kDeviceOption_ActiveThresholdTime },
#endif
#if CHIP_WITH_NLFAULTINJECTION
    { "faults", kArgumentRequired, kDeviceOption_FaultInjection },
#endif
    { "dac_provider", kArgumentRequired, kDeviceOption_DacProvider },
#if CHIP_ATTESTATION_TRUSTY_OS
    { "dac_provider_trusty", kNoArgument, kDeviceOption_TrustyDacProvider },
#endif
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    { "tc-version", kArgumentRequired, kDeviceOption_TermsAndConditions_Version },
    { "tc-required", kArgumentRequired, kDeviceOption_TermsAndConditions_Required },
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    { "icdActiveModeDurationMs", kArgumentRequired, kDeviceOption_icdActiveModeDurationMs },
    { "icdIdleModeDuration", kArgumentRequired, kDeviceOption_icdIdleModeDuration },
    { "icdShortIdleModeDuration", kArgumentRequired, kDeviceOption_icdShortIdleModeDuration },
#endif
#if ENABLE_CAMERA_SERVER
    { "camera-deferred-offer", kNoArgument, kDeviceOption_Camera_DeferredOffer },
    { "camera-test-videosrc", kNoArgument, kDeviceOption_Camera_TestVideosrc },
    { "camera-test-audiosrc", kNoArgument, kDeviceOption_Camera_TestAudiosrc },
    { "camera-audio-playback", kNoArgument, kDeviceOption_Camera_AudioPlayback },
    { "camera-video-device", kArgumentRequired, kDeviceOption_Camera_VideoDevice },
#endif
    {}
};

const char * sDeviceOptionHelp =
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    "  --ble-controller <selector>\n"
    "       BLE controller selector, see example or platform docs for details\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    "\n"
    "  --wifi\n"
    "       Enable Wi-Fi management via wpa_supplicant.\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    "\n"
    "  --wifi-supports-5g\n"
    "       Indicate that local Wi-Fi hardware should report 5GHz support.\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    "\n"
    "  --wifipaf freq_list=<freq_1>,<freq_2>... \n"
    "       Enable Wi-Fi PAF via wpa_supplicant.\n"
    "       Give an empty string if not setting freq_list: \"\"\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAFs
#if CHIP_ENABLE_OPENTHREAD
#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
    "\n"
    "  --thread-node-id <node id>\n"
    "       Enable Thread Simulation with the specified node id.\n"
#else
    "  --thread\n"
    "       Enable Thread management via ot-agent.\n"
#endif
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
    "  --vendor-name <name>\n"
    "       The vendor name specified by the vendor.\n"
    "\n"
    "  --product-name <name>\n"
    "       The product name specified by vendor.\n"
    "\n"
    "  --hardware-version-string <string>\n"
    "       The HardwareVersionString used in the basic information cluster.\n"
    "\n"
    "  --serial-number <serial_number>\n"
    "       The serial number specified by vendor.\n"
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
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    "  --secured-device-port <port>\n"
    "       A 16-bit unsigned integer specifying the listen port to use for secure device messages (default is 5540).\n"
    "\n"
    "  --unsecured-commissioner-port <port>\n"
    "       A 16-bit unsigned integer specifying the port to use for unsecured commissioner messages (default is 5550).\n"
    "\n"
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    "  --secured-commissioner-port <port>\n"
    "       A 16-bit unsigned integer specifying the listen port to use for secure commissioner messages (default is 5552). Only "
    "valid when app is both device and commissioner\n"
    "\n"
    "  --commissioner-fabric-id <fabricid>\n"
    "       The fabric ID to be used when this device is a commissioner (default in code is 1).\n"
    "\n"
#endif
    "  --command <command-name>\n"
    "       A name for a command to execute during startup.\n"
    "\n"
    "  --PICS <filepath>\n"
    "       A file containing PICS items.\n"
    "\n"
    "  --KVS <filepath>\n"
    "       A file to store Key Value Store items.\n"
    "\n"
    "  --kvs-data-directory <filepath>\n"
    "       Base directory for Data KVS file.\n"
    "\n"
    "  --kvs-factory-directory <filepath>\n"
    "       Base directory for Factory KVS file.\n"
    "\n"
    "  --kvs-config-directory <filepath>\n"
    "       Base directory for Config KVS file.\n"
    "\n"
    "  --kvs-counters-directory <filepath>\n"
    "       Base directory for Counters KVS file.\n"
    "\n"
    "  --interface-id <interface>\n"
    "       A interface id to advertise on.\n"
    "\n"
    "  --app-pipe <filepath>\n"
    "       Custom path for the current application to receive out of band commands from the test.\n"
    "\n"
    "  --app-pipe-out <filepath>\n"
    "       Custom path for the current application to send out of band commands to the test.\n"
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    "\n"
    "  --trace_file <file>\n"
    "       Output trace data to the provided file.\n"
    "  --trace_log <1/0>\n"
    "       A value of 1 enables traces to go to the log, 0 disables this (default 0).\n"
    "  --trace_decode <1/0>\n"
    "       A value of 1 enables traces decoding, 0 disables this (default 0).\n"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    "  --commissioning-arl-entries <CommissioningARL JSON>\n"
    "       Enable ACL cluster access restrictions used during commissioning with the provided JSON. Example:\n"
    "       \"[{\\\"endpoint\\\": 1,\\\"cluster\\\": 1105,\\\"restrictions\\\": [{\\\"type\\\": 0,\\\"id\\\": 0}]}]\"\n"
    "  --arl-entries <ARL JSON>\n"
    "       Enable ACL cluster access restrictions applied to fabric index 1 with the provided JSON. Example:\n"
    "       \"[{\\\"endpoint\\\": 1,\\\"cluster\\\": 1105,\\\"restrictions\\\": [{\\\"type\\\": 0,\\\"id\\\": 0}]}]\"\n"
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
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
#if ENABLE_TRACING
    "  --trace-to <destination>\n"
    "       Trace destinations, comma separated (" SUPPORTED_COMMAND_LINE_TRACING_TARGETS ")\n"
#endif
    "  --simulate-no-internal-time\n"
    "       Time cluster does not use internal platform time\n"
#if defined(PW_RPC_ENABLED)
    "  --rpc-server-port\n"
    "       Start RPC server on specified port\n"
#endif
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    "  --subscription-capacity\n"
    "       Max number of subscriptions the device will allow\n"
    "  --subscription-resumption-retry-interval\n"
    "       subscription timeout resumption retry interval in seconds\n"
    "  --idle-retransmit-timeout <timeout>\n"
    "      Sets the MRP idle retry interval (in milliseconds).\n"
    "      This interval is used by the peer to calculate the retransmission timeout when the current device is considered idle.\n"
    "\n"
    "  --active-retransmit-timeout <timeout>\n"
    "      Sets the MRP active retry interval (in milliseconds).\n"
    "      This interval is used by the peer to calculate the retransmission timeout when the current device is considered "
    "active.\n"
    "\n"
    "  --active-threshold-time <time>\n"
    "      Sets the MRP active threshold (in milliseconds).\n"
    "      Specifies the time after which the device transitions from active to idle.\n"
    "\n"
#endif
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    "  --tc-version\n"
    "       Sets the minimum required version of the Terms and Conditions\n"
    "\n"
    "  --tc-required\n"
    "       Sets the required acknowledgements for the Terms and Conditions as a 16-bit enumeration.\n"
    "       Each bit represents an ordinal corresponding to a specific acknowledgment requirement.\n"
    "\n"
#endif
#if CHIP_WITH_NLFAULTINJECTION
    "  --faults <fault-string,...>\n"
    "       Inject specified fault(s) at runtime.\n"
#endif
    "  --dac_provider <filepath>\n"
    "       A json file with data used by the example dac provider to validate device attestation procedure.\n"
#if CHIP_ATTESTATION_TRUSTY_OS
    "  --dac_provider_trusty\n"
    "       Invoke Trusty OS to get device attestation from secure storage.\n"
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    "  --icdActiveModeDurationMs <icdActiveModeDurationMs>\n"
    "       Sets the ICD active mode duration (in milliseconds). (Default: 300) \n"
    "       This defines the how long the the server typically will stay in active mode after \n"
    "       initial transition out of idle mode.\n"
    "  --icdIdleModeDuration <icdIdleModeDuration>\n"
    "       Sets the ICD idle mode durations (in seconds). (Default: 300)\n"
    "       This defines the how long the ICD server can stay in idle mode.\n"
#endif
#if ENABLE_CAMERA_SERVER
    "\n"
    "  --camera-deferred-offer\n"
    "       Indicates the delayed processing hint of the WebRTC Provider.\n"
    "\n"
    "  --camera-video-device <path>\n"
    "       Path to a V4L2 video capture device (default: /dev/video0).\n"
    "\n"
    "  --camera-test-videosrc\n"
    "       Use gstreamer test video source for streaming. Overrides --camera-video-device.\n"
    "\n"
    "  --camera-test-audiosrc\n"
    "       Use gstreamer test audio source for streaming. Overrides --camera-video-device.\n"
    "\n"
    "  --camera-audio-playback\n"
    "       Enables audio playback gstreamer pipeline to play the audio received from remote peer.\n"
    "\n"
#endif
    "\n";

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
bool ParseAccessRestrictionEntriesFromJson(const char * jsonString, std::vector<AccessRestrictionProvider::Entry> & entries)
{
    Json::Value root;
    Json::Reader reader;
    VerifyOrReturnValue(reader.parse(jsonString, root), false);

    for (Json::Value::const_iterator eIt = root.begin(); eIt != root.end(); eIt++)
    {
        AccessRestrictionProvider::Entry entry;

        entry.endpointNumber = static_cast<EndpointId>((*eIt)["endpoint"].asUInt());
        entry.clusterId      = static_cast<ClusterId>((*eIt)["cluster"].asUInt());

        Json::Value restrictions = (*eIt)["restrictions"];
        for (Json::Value::const_iterator rIt = restrictions.begin(); rIt != restrictions.end(); rIt++)
        {
            AccessRestrictionProvider::Restriction restriction;
            restriction.restrictionType = static_cast<AccessRestrictionProvider::Type>((*rIt)["type"].asUInt());
            if ((*rIt).isMember("id"))
            {
                restriction.id.SetValue((*rIt)["id"].asUInt());
            }
            entry.restrictions.push_back(restriction);
        }

        entries.push_back(entry);
    }

    return true;
}
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

bool Base64ArgToVector(const char * arg, size_t maxSize, std::vector<uint8_t> & outVector)
{
    size_t maxBase64Size = BASE64_ENCODED_LEN(maxSize);
    outVector.resize(maxSize);

    size_t argLen = strlen(arg);
    VerifyOrReturnValue(argLen <= maxBase64Size, false);
    VerifyOrReturnValue(chip::CanCastTo<uint32_t>(argLen), false);

    size_t decodedLen = chip::Base64Decode32(arg, static_cast<uint32_t>(argLen), reinterpret_cast<uint8_t *>(outVector.data()));
    VerifyOrReturnValue(decodedLen != 0, false);

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

    case kDeviceOption_WiFiSupports5g:
        LinuxDeviceOptions::GetInstance().wifiSupports5g = true;
        break;

#if CHIP_ENABLE_OPENTHREAD
#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
    case kDeviceOption_ThreadNodeId:
        if (!ParseInt(aValue, LinuxDeviceOptions::GetInstance().mThreadNodeId))
        {
            PrintArgError("%s: invalid value specified for Thread node id: %s\n", aProgram, aValue);
            retval = false;
        }
        break;
#else
    case kDeviceOption_Thread:
        LinuxDeviceOptions::GetInstance().mThread = true;
        break;
#endif
#endif

    case kDeviceOption_Version:
        LinuxDeviceOptions::GetInstance().payload.version = static_cast<uint8_t>(strtoul(aValue, nullptr, 0));
        break;

    case kDeviceOption_VendorID:
        LinuxDeviceOptions::GetInstance().payload.vendorID = static_cast<uint16_t>(strtoul(aValue, nullptr, 0));
        break;

    case kDeviceOption_ProductID:
        LinuxDeviceOptions::GetInstance().payload.productID = static_cast<uint16_t>(strtoul(aValue, nullptr, 0));
        break;

    case kDeviceOption_VendorName:
        LinuxDeviceOptions::GetInstance().vendorName.SetValue(std::string{ aValue });
        break;

    case kDeviceOption_ProductName:
        LinuxDeviceOptions::GetInstance().productName.SetValue(std::string{ aValue });
        break;

    case kDeviceOption_HardwareVersionString:
        LinuxDeviceOptions::GetInstance().hardwareVersionString.SetValue(std::string{ aValue });
        break;

    case kDeviceOption_SerialNumber:
        LinuxDeviceOptions::GetInstance().serialNumber.SetValue(std::string{ aValue });
        break;

    case kDeviceOption_CustomFlow:
        LinuxDeviceOptions::GetInstance().payload.commissioningFlow = static_cast<CommissioningFlow>(strtoul(aValue, nullptr, 0));
        break;

    case kDeviceOption_Capabilities:
        LinuxDeviceOptions::GetInstance().payload.rendezvousInformation.Emplace().SetRaw(
            static_cast<uint8_t>(strtoul(aValue, nullptr, 0)));
        break;

    case kDeviceOption_Discriminator: {
        uint16_t value = static_cast<uint16_t>(strtoul(aValue, nullptr, 0));
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
        LinuxDeviceOptions::GetInstance().payload.setUpPINCode = static_cast<uint32_t>(strtoul(aValue, nullptr, 0));
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

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    case kDeviceOption_SecuredDevicePort:
        LinuxDeviceOptions::GetInstance().securedDevicePort = static_cast<uint16_t>(strtoul(aValue, nullptr, 0));
        break;

    case kDeviceOption_UnsecuredCommissionerPort:
        LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort = static_cast<uint16_t>(strtoul(aValue, nullptr, 0));
        break;

#endif

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    case kDeviceOption_SecuredCommissionerPort:
        LinuxDeviceOptions::GetInstance().securedCommissionerPort = static_cast<uint16_t>(strtoul(aValue, nullptr, 0));
        break;
    case kCommissionerOption_FabricID:
        LinuxDeviceOptions::GetInstance().commissionerFabricId = static_cast<chip::FabricId>(strtoull(aValue, nullptr, 0));
        break;
#endif

    case kDeviceOption_Command:
        LinuxDeviceOptions::GetInstance().command = aValue;
        break;

    case kDeviceOption_PICS:
        LinuxDeviceOptions::GetInstance().PICS = aValue;
        break;

    case kDeviceOption_KVSFilePath:
        LinuxDeviceOptions::GetInstance().KVS = aValue;
        break;

    case kDeviceOption_KVSDataDirectory:
        LinuxDeviceOptions::GetInstance().KVSDataDirectory.SetValue(std::string(aValue));
        break;

    case kDeviceOption_KVSFactoryDataDirectory:
        LinuxDeviceOptions::GetInstance().KVSFactoryDirectory.SetValue(std::string(aValue));
        break;

    case kDeviceOption_KVSConfigDataDirectory:
        LinuxDeviceOptions::GetInstance().KVSConfigDirectory.SetValue(std::string(aValue));
        break;

    case kDeviceOption_KVSCountersDataDirectory:
        LinuxDeviceOptions::GetInstance().KVSCountersDirectory.SetValue(std::string(aValue));
        break;

    case kDeviceOption_AppPipe:
        LinuxDeviceOptions::GetInstance().app_pipe = aValue;
        break;

    case kDeviceOption_AppPipeOut:
        LinuxDeviceOptions::GetInstance().app_pipe_out = aValue;
        break;

    case kDeviceOption_InterfaceId:
        LinuxDeviceOptions::GetInstance().interfaceId =
            Inet::InterfaceId(static_cast<chip::Inet::InterfaceId::PlatformType>(strtoul(aValue, nullptr, 0)));
        break;

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    case kDeviceOption_TraceFile:
        LinuxDeviceOptions::GetInstance().traceStreamFilename.SetValue(std::string{ aValue });
        break;
    case kDeviceOption_TraceLog:
        if (strtoul(aValue, nullptr, 0) != 0)
        {
            LinuxDeviceOptions::GetInstance().traceStreamToLogEnabled = true;
        }
        break;
    case kDeviceOption_TraceDecode:
        if (strtoul(aValue, nullptr, 0) != 0)
        {
            LinuxDeviceOptions::GetInstance().traceStreamDecodeEnabled = true;
        }
        break;
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    // TODO(#35189): change to use a path to JSON files instead
    case kDeviceOption_CommissioningArlEntries: {
        std::vector<AccessRestrictionProvider::Entry> entries;
        retval = ParseAccessRestrictionEntriesFromJson(aValue, entries);
        if (retval)
        {
            LinuxDeviceOptions::GetInstance().commissioningArlEntries.SetValue(std::move(entries));
        }
    }
    break;
    case kDeviceOption_ArlEntries: {
        std::vector<AccessRestrictionProvider::Entry> entries;
        retval = ParseAccessRestrictionEntriesFromJson(aValue, entries);
        if (retval)
        {
            LinuxDeviceOptions::GetInstance().arlEntries.SetValue(std::move(entries));
        }
    }
    break;
#endif // CHIP_CONFIG_USE_ACCESS_RESTRICTIONS

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
#if ENABLE_TRACING
    case kTraceTo:
        LinuxDeviceOptions::GetInstance().traceTo.push_back(aValue);
        break;
#endif
    case kOptionSimulateNoInternalTime:
        LinuxDeviceOptions::GetInstance().mSimulateNoInternalTime = true;
        break;
#if defined(PW_RPC_ENABLED)
    case kOptionRpcServerPort:
        LinuxDeviceOptions::GetInstance().rpcServerPort = static_cast<uint16_t>(strtoul(aValue, nullptr, 0));
        break;
#endif
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    case kDeviceOption_SubscriptionCapacity:
        LinuxDeviceOptions::GetInstance().subscriptionCapacity = static_cast<int32_t>(strtoul(aValue, nullptr, 0));
        break;
    case kDeviceOption_SubscriptionResumptionRetryIntervalSec:
        LinuxDeviceOptions::GetInstance().subscriptionResumptionRetryIntervalSec =
            static_cast<int32_t>(strtoul(aValue, nullptr, 0));
        break;
    case kDeviceOption_IdleRetransmitTimeout: {
        auto mrpConfig            = GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig());
        auto idleRetransTimeout   = System::Clock::Milliseconds32(static_cast<uint32_t>(strtoul(aValue, nullptr, 0)));
        auto activeRetransTimeout = mrpConfig.mActiveRetransTimeout;
        auto activeThresholdTime  = mrpConfig.mActiveThresholdTime;
        OverrideLocalMRPConfig(idleRetransTimeout, activeRetransTimeout, activeThresholdTime);
        break;
    }

    case kDeviceOption_ActiveRetransmitTimeout: {
        auto mrpConfig            = GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig());
        auto idleRetransTimeout   = mrpConfig.mIdleRetransTimeout;
        auto activeRetransTimeout = System::Clock::Milliseconds32(static_cast<uint32_t>(strtoul(aValue, nullptr, 0)));
        auto activeThresholdTime  = mrpConfig.mActiveThresholdTime;
        OverrideLocalMRPConfig(idleRetransTimeout, activeRetransTimeout, activeThresholdTime);
        break;
    }

    case kDeviceOption_ActiveThresholdTime: {
        auto mrpConfig            = GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig());
        auto idleRetransTimeout   = mrpConfig.mIdleRetransTimeout;
        auto activeRetransTimeout = mrpConfig.mActiveRetransTimeout;
        auto activeThresholdTime  = System::Clock::Milliseconds16(static_cast<uint16_t>(strtoul(aValue, nullptr, 0)));
        OverrideLocalMRPConfig(idleRetransTimeout, activeRetransTimeout, activeThresholdTime);
        break;
    }
#endif
#if CHIP_WITH_NLFAULTINJECTION
    case kDeviceOption_FaultInjection: {
        constexpr nl::FaultInjection::GetManagerFn faultManagerFns[] = { FaultInjection::GetManager,
                                                                         Inet::FaultInjection::GetManager,
                                                                         System::FaultInjection::GetManager };
        Platform::ScopedMemoryString mutableArg(aValue, strlen(aValue)); // ParseFaultInjectionStr may mutate
        if (!nl::FaultInjection::ParseFaultInjectionStr(mutableArg.Get(), faultManagerFns, MATTER_ARRAY_SIZE(faultManagerFns)))
        {
            PrintArgError("%s: Invalid fault injection specification\n", aProgram);
            retval = false;
        }
        break;
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    case kDeviceOption_WiFi_PAF: {
        LinuxDeviceOptions::GetInstance().mWiFiPAF        = true;
        LinuxDeviceOptions::GetInstance().mWiFiPAFExtCmds = aValue;
        break;
    }
#endif
    case kDeviceOption_DacProvider: {
        LinuxDeviceOptions::GetInstance().dacProviderFile.SetValue(aValue);
        static chip::Credentials::Examples::TestHarnessDACProvider testDacProvider;
        testDacProvider.Init(gDeviceOptions.dacProviderFile.Value().c_str());

        LinuxDeviceOptions::GetInstance().dacProvider = &testDacProvider;
        break;
    }
#if CHIP_ATTESTATION_TRUSTY_OS
    case kDeviceOption_TrustyDacProvider: {
        LinuxDeviceOptions::GetInstance().dacProvider = &chip::Credentials::Trusty::TrustyDACProvider::GetTrustyDACProvider();
        break;
    }
#endif
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    case kDeviceOption_TermsAndConditions_Version: {
        LinuxDeviceOptions::GetInstance().tcVersion.SetValue(static_cast<uint16_t>(strtoul(aValue, nullptr, 0)));
        break;
    }

    case kDeviceOption_TermsAndConditions_Required: {
        LinuxDeviceOptions::GetInstance().tcRequired.SetValue(static_cast<uint16_t>(strtoul(aValue, nullptr, 0)));
        break;
    }
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    case kDeviceOption_icdActiveModeDurationMs: {
        uint32_t value = static_cast<uint32_t>(strtoul(aValue, nullptr, 0));
        if (value < 1)
        {
            PrintArgError("%s: invalid value specified for icdActiveModeDurationMs: %s\n", aProgram, aValue);
            retval = false;
        }
        else
        {
            LinuxDeviceOptions::GetInstance().icdActiveModeDurationMs.SetValue(chip::System::Clock::Milliseconds32(value));
        }
        break;
    }
    case kDeviceOption_icdIdleModeDuration: {
        uint32_t value = static_cast<uint32_t>(strtoul(aValue, nullptr, 0));
        if ((value < 1) || (value > 86400))
        {
            PrintArgError("%s: invalid value specified for icdIdleModeDuration: %s\n", aProgram, aValue);
            retval = false;
        }
        else
        {
            // Covert from seconds to milli seconds
            LinuxDeviceOptions::GetInstance().icdIdleModeDurationMs.SetValue(chip::System::Clock::Milliseconds32(value * 1000));
        }
        break;
    }
    case kDeviceOption_icdShortIdleModeDuration: {
        uint32_t value = static_cast<uint32_t>(strtoul(aValue, nullptr, 0));
        if ((value < 1) || (value > 86400))
        {
            PrintArgError("%s: invalid value specified for icdShortIdleModeDuration: %s\n", aProgram, aValue);
            retval = false;
        }
        else
        {
            if (LinuxDeviceOptions::GetInstance().icdIdleModeDurationMs.HasValue() &&
                (value > std::chrono::duration_cast<System::Clock::Seconds32>(
                             LinuxDeviceOptions::GetInstance().icdIdleModeDurationMs.Value())
                             .count()))
            {
                PrintArgError("%s: icdShortIdleModeDuration value (%s) must be <= icdIdleModeDuration\n", aProgram, aValue);
                retval = false;
            }
            else
            {
                LinuxDeviceOptions::GetInstance().shortIdleModeDurationS = chip::System::Clock::Seconds32(value);
            }
        }
        break;
    }
#endif
#if ENABLE_CAMERA_SERVER
    case kDeviceOption_Camera_DeferredOffer: {
        LinuxDeviceOptions::GetInstance().cameraDeferredOffer = true;
        break;
    }
    case kDeviceOption_Camera_TestVideosrc: {
        LinuxDeviceOptions::GetInstance().cameraTestVideosrc = true;
        break;
    }
    case kDeviceOption_Camera_TestAudiosrc: {
        LinuxDeviceOptions::GetInstance().cameraTestAudiosrc = true;
        break;
    }
    case kDeviceOption_Camera_AudioPlayback: {
        LinuxDeviceOptions::GetInstance().cameraAudioPlayback = true;
        break;
    }
    case kDeviceOption_Camera_VideoDevice: {
        LinuxDeviceOptions::GetInstance().cameraVideoDevice.SetValue(aValue);
        break;
    }
#endif
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

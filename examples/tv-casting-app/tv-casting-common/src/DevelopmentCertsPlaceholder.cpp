/*
 *    Copyright (c) 2024 Project CHIP Authors
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
 * @file
 *
 * Provides placeholder DevelopmentCerts symbols for custom VID/PID configurations
 * where the standard ExampleDACs.cpp / ExamplePAI.cpp do not define certs.
 *
 * When using a non-development VID (e.g., 0x1123), ExampleDACs.cpp and
 * ExamplePAI.cpp conditional compilation guards exclude all cert definitions,
 * causing linker errors from code that references these symbols as fallback
 * defaults (e.g., AndroidAppServerWrapper, TestHarnessDACProvider,
 * ExampleDACProvider).
 *
 * These empty placeholders satisfy the linker. Real DAC credentials should be
 * provided by the platform-specific DAC provider (DACProviderStub.java on
 * Android, MCDeviceAttestationCredentialsProvider on Darwin).
 *
 * For development VIDs (0xFFF1/0xFFF2/0xFFF3), this file compiles to nothing
 * since ExampleDACs.cpp and ExamplePAI.cpp already provide the symbols.
 */

#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

#if !(CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID == 0xFFF1 || CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID == 0xFFF2 ||                            \
      CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID == 0xFFF3)

namespace chip {
namespace DevelopmentCerts {

ByteSpan kDacCert;
ByteSpan kDacPrivateKey;
ByteSpan kDacPublicKey;
ByteSpan kPaiCert;
ByteSpan kDeclaration;

} // namespace DevelopmentCerts
} // namespace chip

#endif

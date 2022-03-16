/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/server/Dnssd.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;

#define EXTENDED_DISCOVERY_TIMEOUT_SEC 20

void ApplicationInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(EXTENDED_DISCOVERY_TIMEOUT_SEC);
#endif

    if (!IsDeviceAttestationCredentialsProviderSet())
    {
        SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    }
}

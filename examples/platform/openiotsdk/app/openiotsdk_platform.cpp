/*
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *      This file provides the common Open IoT SDK platform functions.
 *      It can be used in Matter examples implementation.
 */

#include "openiotsdk_platform.h"

#include "cmsis_os2.h"
#include "iotsdk/ip_network_api.h"
#include "mbedtls/platform.h"

#ifdef CONFIG_CHIP_CRYPTO_PSA
#include "psa/crypto.h"
#endif

#include <DeviceInfoProviderImpl.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/logging/Constants.h>
#include <platform/openiotsdk/OpenIoTSDKArchUtils.h>

#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef USE_CHIP_DATA_MODEL
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>
#endif // USE_CHIP_DATA_MODEL

#ifdef CHIP_OPEN_IOT_SDK_OTA_ENABLE
#include "openiotsdk_dfu_manager.h"
#endif // CHIP_OPEN_IOT_SDK_OTA_ENABLE

#include "psa/fwu_config.h"
#include "psa/update.h"
#include "tfm_ns_interface.h"

using namespace ::chip;
using namespace ::chip::Platform;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging;

constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

#define NETWORK_UP_FLAG 0x00000001U
#define NETWORK_DOWN_FLAG 0x00000002U
#define ALL_EVENTS_FLAGS (NETWORK_UP_FLAG | NETWORK_DOWN_FLAG)

#define EVENT_TIMEOUT 5000

static osEventFlagsId_t event_flags_id;

static DeviceLayer::DeviceInfoProviderImpl gDeviceInfoProvider;

extern "C" {
// RTOS-specific initialization that is not declared in any header file
uint32_t tfm_ns_interface_init(void);
}

/** Wait for specific event and check error */
static int wait_for_event(uint32_t event)
{
    int res = EXIT_SUCCESS;
    int ret = osEventFlagsWait(event_flags_id, ALL_EVENTS_FLAGS, osFlagsWaitAny, ms2tick(EVENT_TIMEOUT));
    if (ret < 0)
    {
        ChipLogError(NotSpecified, "osEventFlagsWait failed %d", ret);
        return EXIT_FAILURE;
    }

    if (!(ret & event))
    {
        res = EXIT_FAILURE;
    }

    ret = osEventFlagsClear(event_flags_id, ALL_EVENTS_FLAGS);
    if (ret < 0)
    {
        ChipLogError(NotSpecified, "osEventFlagsClear failed %d", ret);
        return EXIT_FAILURE;
    }

    return res;
}

static void post_network_connect()
{
    // Iterate on the network interface to see if we already have beed assigned addresses.
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        char ifName[Inet::InterfaceId::kMaxIfNameLength];
        if (it.IsUp() && CHIP_NO_ERROR == it.GetInterfaceName(ifName, sizeof(ifName)))
        {
            chip::Inet::IPAddress addr;
            if ((it.GetAddress(addr) == CHIP_NO_ERROR))
            {
                char ipStrBuf[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
                addr.ToString(ipStrBuf);

                ChipLogProgress(DeviceLayer, "Got IP address on interface: %s IP: %s", ifName, ipStrBuf);
            }
        }
    }
}

/** This callback is called by the ip network task. It translates from a network event code
 * to platform event and sends it.
 *
 * @param event network up or down event.
 */
static void network_state_callback(network_state_callback_event_t event)
{
    uint32_t event_flag = (event == NETWORK_UP) ? NETWORK_UP_FLAG : NETWORK_DOWN_FLAG;
    ChipLogDetail(NotSpecified, "Network %s", (event == NETWORK_UP) ? "UP" : "DOWN");
    int res = osEventFlagsSet(event_flags_id, event_flag);
    if (res < 0)
    {
        ChipLogError(NotSpecified, "osEventFlagsSet failed %d", res);
    }
}

static int get_psa_images_details()
{
    psa_status_t status;
    psa_fwu_component_info_t image_info;

    status = psa_fwu_query(FWU_COMPONENT_ID_SECURE, &image_info);
    if (status != PSA_SUCCESS)
    {
        ChipLogError(NotSpecified, "Failed to query secure firmware information. Error %ld", status);
        return EXIT_FAILURE;
    }

    ChipLogProgress(NotSpecified, "Secure firmware version: %u.%u.%u-%lu\r\n", image_info.version.major, image_info.version.minor,
                    image_info.version.patch, image_info.version.build);

    status = psa_fwu_query(FWU_COMPONENT_ID_NONSECURE, &image_info);
    if (status != PSA_SUCCESS)
    {
        ChipLogError(NotSpecified, "Failed to query non-secure firmware information. Error %ld", status);
        return EXIT_FAILURE;
    }

    ChipLogProgress(NotSpecified, "Non-secure firmware version: %u.%u.%u-%lu\r\n", image_info.version.major,
                    image_info.version.minor, image_info.version.patch, image_info.version.build);

    return EXIT_SUCCESS;
}

int openiotsdk_platform_init(void)
{
    int ret;

#if defined(NDEBUG) && CHIP_CONFIG_TEST == 0
    SetLogFilter(LogCategory::kLogCategory_Progress);
#endif

    ret = mbedtls_platform_setup(NULL);
    if (ret)
    {
        ChipLogError(NotSpecified, "Mbed TLS platform initialization failed: %d", ret);
        return EXIT_FAILURE;
    }

#ifdef CONFIG_CHIP_CRYPTO_PSA
    ret = psa_crypto_init();
    if (ret)
    {
        ChipLogError(NotSpecified, "PSA crypto initialization failed: %d", ret);
        return EXIT_FAILURE;
    }
#endif

    ret = get_psa_images_details();
    if (ret != 0)
    {
        ChipLogError(NotSpecified, "Get PSA image details failed: %d", ret);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int openiotsdk_chip_init(void)
{
    CHIP_ERROR err;

    err = MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Memory initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Chip stack initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Chip stack start failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    DeviceLayer::SetDeviceInfoProvider(&gDeviceInfoProvider);

    return EXIT_SUCCESS;
}

int openiotsdk_network_init(bool wait)
{
    int ret;

    event_flags_id = osEventFlagsNew(NULL);
    if (event_flags_id == NULL)
    {
        ChipLogError(NotSpecified, "Create event flags failed");
        return EXIT_FAILURE;
    }

    ret = start_network_task(network_state_callback, NETWORK_THREAD_STACK_SIZE_DEFAULT);
    if (ret != osOK)
    {
        ChipLogError(NotSpecified, "start_network_task failed %d", ret);
        return EXIT_FAILURE;
    }

    if (wait)
    {
        if (wait_for_event(NETWORK_UP_FLAG) != EXIT_SUCCESS)
        {
            ChipLogError(NotSpecified, "Network initialization failed");
            return EXIT_FAILURE;
        }
        post_network_connect();
    }

    return EXIT_SUCCESS;
}

int openiotsdk_chip_run(void)
{
#ifdef USE_CHIP_DATA_MODEL
    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    CHIP_ERROR err = initParams.InitializeStaticResourcesBeforeServerInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Initialize static resources before server init failed: %s", err.AsString());
        return EXIT_FAILURE;
    }
    initParams.dataModelProvider             = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    err = Server::GetInstance().Init(initParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Initialize server failed: %s", err.AsString());
        return EXIT_FAILURE;
    }

    // Now that the server has started and we are done with our startup logging,
    // log our discovery/onboarding information again so it's not lost in the
    // noise.
    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork));

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
#endif // USE_CHIP_DATA_MODEL

    ChipLogProgress(NotSpecified, "Current software version: [%ld] %s", uint32_t(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION),
                    CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

#ifdef CHIP_OPEN_IOT_SDK_OTA_ENABLE
    err = GetDFUManager().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "DFU manager initialization failed: %s", err.AsString());
        return EXIT_FAILURE;
    }
#endif

    return EXIT_SUCCESS;
}

void openiotsdk_chip_shutdown()
{
#ifdef USE_CHIP_DATA_MODEL
    Server::GetInstance().Shutdown();
#endif // USE_CHIP_DATA_MODEL
}

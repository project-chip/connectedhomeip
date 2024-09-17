/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

/*! App to Host message queue for the Host Task */
messaging_t gApp2Host_TaskQueue;
/*! HCI to Host message queue for the Host Task */
messaging_t gHci2Host_TaskQueue;
/*! Event for the Host Task Queue */
OSA_EVENT_HANDLE_DEFINE(gHost_TaskEvent);

#ifdef EXTERNAL_BLEMANAGERIMPL_HEADER
#include EXTERNAL_BLEMANAGERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define BLEMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/BLEManagerImpl.h>
#include BLEMANAGERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

extern "C" bleResult_t Hci_Reset(void);

namespace chip {
namespace DeviceLayer {
namespace Internal {

BLEManagerImpl BLEManagerImpl::sInstance;

BLEManagerCommon * BLEManagerImpl::GetImplInstance()
{
    return &BLEManagerImpl::sInstance;
}

CHIP_ERROR BLEManagerImpl::InitHostController(BLECallbackDelegate::GapGenericCallback cb_fp)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(PLATFORM_InitTimerManager() >= 0, err = CHIP_ERROR_INCORRECT_STATE);

    PLATFORM_InitBle();

    (void) RNG_Init();

    /* Has to be called after RNG_Init(), once seed is generated. */
    (void) Controller_SetRandomSeed();

    /* Create BLE Host Task */
    VerifyOrExit(BLEManagerImpl::blekw_host_init() == CHIP_NO_ERROR, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(Hcit_Init(Ble_HciRecv) == gHciSuccess_c, err = CHIP_ERROR_INCORRECT_STATE);

    /* Set BD Address in Controller. Must be done after HCI init and before Host init. */
    Ble_SetBDAddr();

    /* BLE Host Stack Init */
    VerifyOrExit(Ble_HostInitialize(cb_fp, Hcit_SendPacket) == gBleSuccess_c, err = CHIP_ERROR_INCORRECT_STATE);

    /* configure tx power to use in NBU specific to BLE */
    Controller_SetTxPowerLevelDbm(mAdvertisingDefaultTxPower_c, gAdvTxChannel_c);
    Controller_SetTxPowerLevelDbm(mConnectionDefaultTxPower_c, gConnTxChannel_c);
    Controller_ConfigureInvalidPduHandling(gLlInvalidPduHandlingType_c);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::ResetController()
{
    bleResult_t res = Hci_Reset();
    if (res != gBleSuccess_c)
    {
        ChipLogProgress(DeviceLayer, "Failed to reset controller %d", res);
        return CHIP_ERROR_INTERNAL;
    }

    /* Wait for function to complete */
    PLATFORM_Delay(HCI_RESET_WAIT_TIME_US);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::Host_Task(osaTaskParam_t argument)
{
    Host_TaskHandler((void *) NULL);
}

CHIP_ERROR BLEManagerImpl::blekw_host_init(void)
{
    /* Initialization of task related */
    if (KOSA_StatusSuccess != OSA_EventCreate((osa_event_handle_t) gHost_TaskEvent, TRUE))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Initialization of task message queue */
    MSG_InitQueue(&gApp2Host_TaskQueue);
    MSG_InitQueue(&gHci2Host_TaskQueue);

    /* Task creation */
    if (pdPASS != xTaskCreate(Host_Task, "hostTask", HOST_TASK_STACK_SIZE, (void *) 0, HOST_TASK_PRIORITY, NULL))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */

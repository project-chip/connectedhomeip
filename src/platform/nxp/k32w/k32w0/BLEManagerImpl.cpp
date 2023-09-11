/*
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
#include "PWR_Configuration.h"
#include "PWR_Interface.h"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

/* controller task configuration */
#define CONTROLLER_TASK_PRIORITY (6U)
#define CONTROLLER_TASK_STACK_SIZE (gControllerTaskStackSize_c / sizeof(StackType_t))

extern "C" void (*pfBLE_SignalFromISR)(void);

extern osaEventId_t gHost_TaskEvent;
osaEventId_t mControllerTaskEvent;
extern msgQueue_t gApp2Host_TaskQueue;
extern msgQueue_t gHci2Host_TaskQueue;

#include <platform/nxp/k32w/k32w0/BLEManagerImpl.h>

extern "C" bool_t Ble_ConfigureHostStackConfig(void);

namespace chip {
namespace DeviceLayer {
namespace Internal {

BLEManagerImpl BLEManagerImpl::sInstance;

BLEManagerCommon * BLEManagerImpl::GetImplInstance()
{
    return &BLEManagerImpl::sInstance;
}
CHIP_ERROR BLEManagerImpl::InitHostController(ble_generic_cb_fp cb_fp)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    pfBLE_SignalFromISR = BLE_SignalFromISRCallback;

    /* Set the config structure to the host stack */
    VerifyOrExit(Ble_ConfigureHostStackConfig() == TRUE, err = CHIP_ERROR_INCORRECT_STATE);

    /* BLE Radio Init */
    VerifyOrExit(XCVR_Init(BLE_MODE, DR_2MBPS) == gXcvrSuccess_c, err = CHIP_ERROR_INCORRECT_STATE);

    /* Create BLE Controller Task */
    VerifyOrExit(blekw_controller_init() == CHIP_NO_ERROR, err = CHIP_ERROR_INCORRECT_STATE);

    /* Create BLE Host Task */
    VerifyOrExit(blekw_host_init() == CHIP_NO_ERROR, err = CHIP_ERROR_INCORRECT_STATE);

    /* BLE Host Stack Init */
    VerifyOrExit(Ble_HostInitialize(cb_fp, Hci_SendPacketToController) == gBleSuccess_c, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    return err;
}

/* Called by BLE when a connect is received */
void BLEManagerImpl::BLE_SignalFromISRCallback(void)
{
#if defined(chip_with_low_power)
    PWR_DisallowDeviceToSleep();
#endif
}

void BLEManagerImpl::Host_Task(osaTaskParam_t argument)
{
    Host_TaskHandler((void *) NULL);
}

CHIP_ERROR BLEManagerImpl::blekw_host_init(void)
{
    /* Initialization of task related */
    gHost_TaskEvent = OSA_EventCreate(TRUE);
    if (!gHost_TaskEvent)
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

CHIP_ERROR BLEManagerImpl::blekw_controller_init(void)
{
    mControllerTaskEvent = OSA_EventCreate(TRUE);

    if (!mControllerTaskEvent)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    Controller_TaskEventInit(mControllerTaskEvent, gUseRtos_c);

    /* Task creation */
    if (pdPASS !=
        xTaskCreate(Controller_TaskHandler, "controllerTask", CONTROLLER_TASK_STACK_SIZE, (void *) 0, CONTROLLER_TASK_PRIORITY,
                    NULL))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    /* BLE Controller Init */
    if (osaStatus_Success != Controller_Init(Ble_HciRecv))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */

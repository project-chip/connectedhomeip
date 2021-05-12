#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "BoltLockManager.h"

#include "freertos/FreeRTOS.h"
#include <ble/BLEEndPoint.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

class AppTask
{

public:
    int StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction);
    void PostEvent(const AppEvent * event);

    void ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction);

private:
    friend AppTask & GetAppTask(void);

    int Init();

    static void ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(BoltLockManager::Action_t aAction);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);
    static void LockActionEventHandler(AppEvent * aEvent);
    static void TimerEventHandler(TimerHandle_t xTimer);

    static void UpdateClusterState(void);

    void StartTimer(uint32_t aTimeoutMs);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_StartBleAdv    = 1,
        kFunction_FactoryReset   = 2,

        kFunction_Invalid
    } Function;

    Function_t mFunction;
    bool mFunctionTimerActive;
    bool mSyncClusterToButtonAction;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

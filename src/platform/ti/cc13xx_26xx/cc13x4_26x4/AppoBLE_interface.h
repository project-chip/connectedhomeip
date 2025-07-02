/*
 *
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

/*!*****************************************************************************
 *  @file       AppoBLE_interface.h
 *  @brief      <b>PRELIMINARY</b> Application Over BLE Interface
 *
 *  <b>WARNING</b> These APIs are <b>PRELIMINARY</b>, and subject to
 *  change in the next few months.
 *
 *  This file defines the Application Over BLE interface used
 *  to manage additional BLE advertisements and connections to
 *  work in tandem with the MatterOverBLE GATT service.
 *
 * At a high level, the AppoBLE interface bridges the gap between the BLE Manager, which interacts directly with the TI BLE Stack,
 * and the Application code. The Matter application may use the AppoBLE interface to issue requests to the BLE manager to execute
 * BLE stack or service specific code within the BLE manager task context and all BLE Stack API's must occur within the BLE manager
 * task context.
 *
 * The AppoBLE interface provides weakly defined Hook and Callback API's that are able to be overridden by the application.
 * The Hook based API's are points in the BLE manager, such as during initialization, which are made available to the application.
 * The Callback API's are issued as a side effect of sending AppoBLE Requests to the BLE manager and contain status information
 * regarding the API sent.
 *
 * When creating custom BLE services, user's must be sure to adapt the service for AppoBLE compatibility.
 * The following are required for Simple GATT based services:
 * Note; Currently the AppoBLE_interface only support 128-bit Service and Characteristic UUIDs.
 *
 * For Reference see AppoBLE.cpp within the application:
 * 1. The GATT service must be registered with the BLE manager through the provided AppoBLEHook_Init() API.
 * 2. The Generic serivce GET/SET API's must be expanded to include parsing for the custom service UUID
 * AppoBLE_GenericSetParameter(), AppoBLE_GenericGetParameter
 * 3. Logic may be added within the HandleAppoBLE* callback API's to act on service reads/writes.
 *
 * For Reference see lightService.c/.h within the application:
 * 1. The GATT service must implement a function which takes a 128-bit UUID and converts it to the service specific parameter id.
 * E.G gatt_LightService_GetParamId()
 * 2. The generic attribute write callback, AppoBLE_GenericcharValueChangeCB(), must be called within the services *_WriteAttrCB()
 * function with necessary parameters based on GenericServiceChange_t.
 * 3. The GATT service UUID's must be visible outside of the service definition and have been marked as extern within the service
 * header file.
 *  ============================================================================
 */

#ifndef APPOBLE_INTERFACE_H
#define APPOBLE_INTERFACE_H

#include <lib/support/DLLUtil.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <bcomdef.h>
#include <icall.h>
#include <icall_ble_api.h>

#include "hal_types.h"

#include "ti_ble_config.h"
#include "ti_drivers_config.h"
#include <AppoBLE_interface_c.h>

#ifdef __cplusplus
}
#endif

class DLL_EXPORT AppoBLE_interface
{
public:
    // App event passed from stack modules. This type is defined by the application
    // since it can queue events to itself however it wants.
    typedef struct
    {
        uint32_t event; // event type
        void * pData;   // pointer to message
    } GenericQueuedEvt_t;
#define ADV_INDEX_INVALID (0xFF)
    typedef struct
    {
        uint8_t len;                   // Data length
        uint8_t * pData;               // Pointer to message
        BLE_CONNECTION_OBJECT conId;   // Connection ID
        chip::Ble::ChipBleUUID svcId;  // Service ID
        chip::Ble::ChipBleUUID charId; // Characteristic ID
    } BLECharApi_msg;

    /*
     * ADV API to event map
     */
    typedef enum AppoBLE_api_type
    {
        AppoBLE_ADV_ADD_UPDATE = 0x00, // Advertisement addition/update
        AppoBLE_ADV_REMOVE,            // Advertisment set removal
        AppoBLE_ADV_CHANGE_INTERVAL,   // Advertisement interval change
    } AppoBLE_api_type_t;

    typedef struct
    {
        uint8_t advIndex;                // Advertising index
        uint8_t inputAdvIndex;           // Advertisement index of set to update
        GapAdv_params_t advParams;       // Advertisement parameters
        const uint8_t * advData;         // Advertisement data
        uint8_t advDataLen;              // Advertisement data length
        const uint8_t * scanRspData;     // Scan response data
        uint8_t scanRspDataLen;          // Scan response data length
        GapAdv_eventMaskFlags_t evtMask; // Advertisement event mask
    } AppoBLEAdvApi_msg;

#ifdef TI_APPOBLE_ENABLE

    /* Callback function prototype for ScheduleAppoBLEFxn() Calls */
    typedef void (*pfnCallInBleMgrCtx_t)(void *);

    typedef enum AppoBLEChar_api_type
    {
        AppoBLE_WRITE_REQ = 0x00, // AppoBLE write request
        AppoBLE_READ_REQ,         // AppoBLE Read request
        AppoBLE_IND,              // AppoBLE Indication request
    } AppoBLEChar_api_type_t;

    typedef struct
    {
        pfnCallInBleMgrCtx_t FxnPtr; // Generic function pointer
        void * arg;                  // Generic arg pointer
    } AppoBLEGenericApi_msg;

    /**
     * Register a custom User defined advertisement set with the BLE manager, returns ID to advertisement set.
     * If an advertisement set has already been created, stop the running advertisement set and update the payload
     *
     * @param advIndex Set to ADV_INDEX_INVALID on initial adv creation, valid index of adv set to update otherwise
     * @param advParams Advertisement parameters
     * @param advData Advertisement data
     * @param advDataLen Advertisement data length
     * @param scanRspData Scan response data
     * @param scanRspDataLen Scan response data length
     * @param evtMask Advertisement set event mask
     *
     * HandleAdvChange() called once advertisement set is created and ready to be started
     * @return Valid index > 0 if added successfully, otherwise 0
     *         Immediate return status indicates whether request was sent to BLE Manager successfully.
     */
    bStatus_t AddUpdateAppoBLEAdvSet(uint8_t advIndex, GapAdv_params_t advParams, uint8_t * advData, uint8_t advDataLen,
                                     uint8_t * scanRspData, uint8_t scanRspDataLen, GapAdv_eventMaskFlags_t evtMask);

    /**
     * Remove previously registered advertising set within the BLE manager and stop current advertisements for that set
     * Caller must wait for cbFxn to be executed to note when advertisement set has been removed
     * @param advHandle Advertisement handle of set to remove
     * HandleAdvChange() called once advertisement set is created and ready to be started
     * @return @ref SUCCESS if advertisement set removal request was successfully sent to BLE Manager, FAILURE otherwise
     */
    /* Advertising data must be static, not freed when removed. */
    bStatus_t RemoveAppoBLEAdvSet(uint8_t advHandle);

    /**
     * Update advertising interval for a specific input advertising set
     *
     * @param advHandle Advertisement handle of set to remove
     * @param intervalMax Maximum advertising interval requested for advertising set
     * @param intervalMin Minimum advertising interval requested for advertising set
     * HandleAdvChange() called once advertisement set is created and ready to be started
     * @return @ref SUCCESS if advertisement set removal request was successfully sent to BLE Manager, FAILURE otherwise
     */
    bStatus_t SetAppoBLEAdvInterval(uint8_t advHandle, uint32_t intervalMax, uint32_t intervalMin);

    /**
     * Start/Stop previously configured advertisement set.
     *
     * @param advHandle Advertisement handle of set to modify
     * @param enable true to enable, false to disable set
     *
     * @note Events registered for advertisement set will be issued in AppoBLEProcessAdvEvent
     *
     * @return @ref SUCCESS if advertisement set modification request was successfully sent to BLE Manager, FAILURE otherwise
     */
    uint8_t SetAppoBLEAdvertisingEnabled(uint8_t advHandle, uint8_t enable);
    /**
     * Query BLE manager for current state of input advertisement set handle
     *
     * @param advHandle Advertisement handle of set to modify
     *
     * @return @ref true if user based advertising set is currently enabled, false otherwise
     */
    uint8_t IsAppoBLEAdvertisingEnabled(uint8_t advHandle);
    /**
     * Query BLE manager for currently set Device name
     *
     * @param buf User supplied buffer to write device name into
     * @param bufSize Length of data written
     *
     */
    void GetAppoBLEDeviceName(char * buf, size_t bufSize);

    /**
     * Set the device name within the BLE stack
     * Note: This does not update any advertisement set data
     *
     * @param buf User supplied buffer to write device name into
     * @param bufSize Length of data written
     *
     */
    bStatus_t SetAppoBLEDeviceName(const char * deviceName);

    /**
     * Set Passcode for pairing when connecting to AppoBLE enabled device
     * Passcode stored in Class, default: B_APP_DEFAULT_PASSCODE
     * Automatically used via Pairing_passcodeHandler() callback within BLE Mgr
     *
     * @param passcode integer value containing the passcode.
     *
     */
    void SetAppoBLEPairingPassCode(uint32_t passcode);

    /**
     * Send Indication to specific service and characteristic and update GATT table
     * This should be used to send data via indication to the central
     * @param conId Connection ID on which to send the write request
     * @param svcId Service ID on which to send the write request
     * @param charId Characteristic ID on which to send the write request
     * @param srcBuf Application supplied buffer of data to send to specific characteristic.
     * @param srcBufLen Application supplied buffer length
     * HandleUserIndicationConfirmation() called once indication is confirmed by central
     *
     * @return @ref SUCCESS if write request was successfully sent to BLE Manager, FALSE otherwise
     */
    bStatus_t SendAppoBLEIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    uint8_t * srcBuf, uint16_t srcBufLen);

    /**
     * Send Write request to specific service and characteristic and update GATT table
     * This should be used to send data via notification to the central
     * @param conId Connection ID on which to send the write request
     * @param svcId Service ID on which to send the write request
     * @param charId Characteristic ID on which to send the write request
     * @param srcBuf Application supplied buffer of data to send to specific characteristic
     * HandleAppoBLEWriteConfirmation() Called once write is complete to service
     * HandleAppoBLEIndicationConfirmation() Called in lieu of HandleWriteConfirmation() if indication enabled on write
     *
     * @return @ref SUCCESS if write request was successfully sent to BLE Manager, FALSE otherwise
     */
    bStatus_t SendAppoBLEWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      uint8_t * srcBuf, uint16_t srcBufLen);

    /**
     * Send Read request to specific service and characteristic
     *
     * User MUST implement AppoBLEReadRequestHandler() to decode profile read API at run-time
     * @param conId Connection ID on which to send the read request
     * @param svcId Service ID on which to send the read request
     * @param charId Characteristic ID on which to send the read request
     * @param dstBuf Application supplied buffer of which specific characteristic data shall be written to
     * HandleAppoBLEReadConfirmation() issued once read request has completed
     *
     * @return @ref SUCCESS if write request was successfully sent to BLE Manager, FALSE otherwise
     */
    bStatus_t SendAppoBLEReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     uint8_t * dstBuf, uint16_t dstBufLen);

    /**
     * Schedule generic FxnPtr to execute within BLE manager context.
     * This is to be used for more complex generic BLE interactions that are not covered by simple characteristic reads/writes
     *
     * @param FxnPtr Function to be executed in BLE Manager context
     * @param arg Generic argument to be passed into function
     *
     * @return @ref SUCCESS if function request was successfully sent to BLE Manager, FALSE otherwise
     */
    bStatus_t ScheduleAppoBLEFxn(pfnCallInBleMgrCtx_t FxnPtr, void * arg);

    /**
     * Obtain running service mode for Matter
     *
     * @param connObj Connection ID on which to query the state of the MatteroBLE service
     *
     * @return Serivce mode state kCHIPoBLEServiceMode_Enabled or kCHIPoBLEServiceMode_Disabled
     */
    ConnectivityManager::CHIPoBLEServiceMode GetMatteroBLEServiceMode(BLE_CONNECTION_OBJECT conId);

    /**
     * Generic BLE Service Set Parameter API.
     * This API is used as a catch all for all registered BLE GATT services which have implemented the
     * AppoBLE_GenericcharValueChangeCB() callback handler.
     * Users must add Service specific logic to correctly call the lower level *_serviceSetParameter API.
     *
     * @param connObj Connection ID on which to send the write request
     * @param svcId Service ID on which to send the write request
     * @param charId Characteristic ID on which to send the write request
     * @param len Pointer to charactersitic data to be written
     * @param value Pointer to charactersitic data to be written
     *
     * @return bStatus_t Success if parameter was successfully written, Failure otherwise
     */
    uint8_t AppoBLE_GenericSetParameter(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                        uint8 len, void * value, uint8_t taskId);

    /**
     * Generic BLE Service Get Parameter API.
     * This API is used as a catch all for all registered BLE GATT services which have implemented the
     * AppoBLE_GenericcharValueChangeCB() callback handler.
     * Users must add Service specific logic to correctly call the lower level *_serviceGetParameter API.
     *
     * @param connObj Connection ID on which to send the write request
     * @param svcId Service ID on which to send the write request
     * @param charId Characteristic ID on which to send the write request
     * @param len Pointer to charactersitic data that was read
     * @param value Pointer to charactersitic data that was read
     *
     * @return bStatus_t Success if parameter was successfully retrieved, Failure otherwise
     */
    uint8_t AppoBLE_GenericGetParameter(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                        uint8 len, void * value);

    /**
     * Query BLE Manager active connection list and return the Nth entry.
     *
     * @param numConnSearch Number of connections to search for prior to returning (indexed from 1)
     *
     */
    BLE_CONNECTION_OBJECT AppoBLE_GetValidConnection(uint8_t numConnSearch);

    /**
     * Submit custom Service based events into the BLE Manager queue for processing in BLE Task context.
     * Queued events are based on the AppoBLE_events_t enum defined in ti_ble_gatt_helper.h.
     * Queued events are avaialable within AppoBLEHook_ProcessBleMgrEvt().
     * Custom Data may be issued via the second parameter and will automatically be freed on exit from
     * AppoBLEHook_ProcessBleMgrEvt().
     * @param serviceEvent  AppoBLE_events_t custom Service event.
     * @param pData Pointer to custom data relating to Service
     * @return SUCCESS if event was queued for processing in the BLE Manager, FAILURE otherwise.
     */
    bStatus_t EnqueueAppoBLEMsg(uint32_t serviceEvent, void * pData);

    /* Application callbacks */
    /**
     * Register a custom User defined advertisement set with the BLE manager, returns ID to advertisement set.
     * If an advertisement set has already been created, stop the running advertisement set and update the payload
     *
     * @param advHandle Advertisement handle which caused change
     * @param status API caller status
     * @param apitype Type of advertising update handler was called for
     *
     */
    __attribute__((weak)) void HandleAppoBLEAdvChange(uint8_t advHandle, uint8_t status, AppoBLE_api_type_t advApiType);

    /**
     * Called when an outstanding App based GATT write request receives a positive receipt confirmation.
     *
     * @param conId Connection ID on which to send the write request
     * @param svcId Service ID on which to send the write request
     * @param charId Characteristic ID on which to send the write request
     */
    __attribute__((weak)) void HandleAppoBLEWriteConfirmation(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                              const ChipBleUUID * charId);

    /**
     * Called when an outstanding App based GATT write request receives a positive receipt confirmation.
     * and ATT_HANDLE_VALUE_CFM is received from the central.
     *
     * @param conId Connection ID on which the indication completed
     */
    __attribute__((weak)) void HandleAppoBLEIndicationConfirmation(BLE_CONNECTION_OBJECT connObj);
    /**
     *  Called by BLE Manager when an internal read request has completed
     *
     * @param conId Connection ID on which the read request was sent
     * @param svcId Service ID on which the read request was sent
     * @param charId Characteristic ID on which the read request was sent
     * @param pBuf BLE Manager supplied buffer of data to read from specific characteristic, NULL on failure
     */
    __attribute__((weak)) void HandleAppoBLEReadConfirmation(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                             const ChipBleUUID * charId, const uint8_t * dstBuf, uint16_t dstBufLen,
                                                             uint8_t status);
    /**
     *  Called by BLE manager when an external GATT write to an AppoBLE characteristic is received.
     *
     * @param conId Connection ID on which to send the write request
     * @param svcId Service ID on which to send the write request
     * @param charId Characteristic ID on which to send the write request
     * @param pBuf BLE Manager supplied buffer of data received on characteristic
     *
     */
    __attribute__((weak)) void HandleAppoBLEWriteReceived(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                          const ChipBleUUID * charId, uint8_t const * dstBuf, uint16_t dstBufLen,
                                                          uint8_t status);

    /**
     * Callback function issued on write to a particular characteristic via SendAppoBLEWriteRequest()
     *
     * @param connObj Connection ID on which write request was issued
     * @param svcId Service ID on which write request was issued
     * @param charId Characteristic ID on which write request was issued
     * @param status status of write request on characteristic
     *
     */
    __attribute__((weak)) void HandleAppoBLEWriteConfirmation(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId,
                                                              const ChipBleUUID * charId, uint8_t status);

    /* Optional BLE Manager Hook API's, for advanced usage only */

    /**
     * Call User BLE defined initialization functions, such as profile initialization.
     */
    __attribute__((weak)) void AppoBLEHook_Init(void);

    /**
     * User BLE Hook into internal BLE Manager event processor
     * Not recommended for normal usage
     * @param pMsg Internal BLE Manager queued events.
     *
     */
    __attribute__((weak)) void AppoBLEHook_ProcessBleMgrEvt(const GenericQueuedEvt_t * pMsg);

    /**
     * Process additional GAP messages
     * This is to be used for more complex generic BLE interactions that are not covered by simple characteristic reads/writes
     * User MUST NOT free or modify the message structure
     *
     * @param pMsg GAP Event handler message structure
     *
     */
    __attribute__((weak)) void AppoBLEHook_ProcessGapMessage(const gapEventHdr_t * pMsg);

    /**
     * Process additional GAP messages
     * This is to be used for more complex generic BLE interactions that are not covered by simple characteristic reads/writes
     * User MUST NOT free or modify the message structure
     * @param pMsg GATT Event handler message structure
     *
     */
    __attribute__((weak)) void AppoBLEHook_ProcessGattMessage(const gattMsgEvent_t * pMsg);

    /**
     * Perform additional processing on incoming HCI events.
     * User MUST NOT free or modify the message structure
     *
     * @param pMsg ICALL header message.
     *
     */
    __attribute__((weak)) void AppoBLEHook_ProcessGapHCIEvt(const ICall_Hdr * pMsg);
#endif
};

#endif /* APPOBLE_INTERFACE_H */

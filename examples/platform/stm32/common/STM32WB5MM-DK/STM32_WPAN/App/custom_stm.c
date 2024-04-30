/**
 ******************************************************************************
 * @file    matter_stm.c
 * @author  MCD Application Team
 * @brief   matter Service using gatt(Custom STM)
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2018-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "app_matter.h"
#include "common_blesvc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    uint16_t PeerToPeerSvcHdle;               /**< Service handle */
    uint16_t P2PWriteClientToServerCharHdle;  /**< Characteristic handle */
    uint16_t P2PNotifyServerToClientCharHdle; /**< Characteristic handle */
} PeerToPeerContext_t;

/* Private defines -----------------------------------------------------------*/
#define UUID_128_SUPPORTED 1

#if (UUID_128_SUPPORTED == 1)
#define BM_UUID_LENGTH UUID_TYPE_128
#else
#define BM_UUID_LENGTH UUID_TYPE_16
#endif

#define BM_REQ_CHAR_SIZE (3)

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

MATTER_App_Notification_evt_t Notification;

/**
 * START of Section BLE_DRIVER_CONTEXT
 */
PLACE_IN_SECTION("BLE_DRIVER_CONTEXT") static PeerToPeerContext_t aPeerToPeerContext;

/**
 * END of Section BLE_DRIVER_CONTEXT
 */
/* Private function prototypes -----------------------------------------------*/
static SVCCTL_EvtAckStatus_t Matter_Event_Handler(void * Event);

/* Functions Definition ------------------------------------------------------*/
/* Private functions ----------------------------------------------------------*/

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5,   \
                      uuid_4, uuid_3, uuid_2, uuid_1, uuid_0)                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        uuid_struct[0]  = uuid_0;                                                                                                  \
        uuid_struct[1]  = uuid_1;                                                                                                  \
        uuid_struct[2]  = uuid_2;                                                                                                  \
        uuid_struct[3]  = uuid_3;                                                                                                  \
        uuid_struct[4]  = uuid_4;                                                                                                  \
        uuid_struct[5]  = uuid_5;                                                                                                  \
        uuid_struct[6]  = uuid_6;                                                                                                  \
        uuid_struct[7]  = uuid_7;                                                                                                  \
        uuid_struct[8]  = uuid_8;                                                                                                  \
        uuid_struct[9]  = uuid_9;                                                                                                  \
        uuid_struct[10] = uuid_10;                                                                                                 \
        uuid_struct[11] = uuid_11;                                                                                                 \
        uuid_struct[12] = uuid_12;                                                                                                 \
        uuid_struct[13] = uuid_13;                                                                                                 \
        uuid_struct[14] = uuid_14;                                                                                                 \
        uuid_struct[15] = uuid_15;                                                                                                 \
    } while (0)

/* Hardware Characteristics Service */

#define MATTER_SERVICE_UUID (0xFFF6)
#define COPY_CHAR_RX_UUID(uuid_struct)                                                                                             \
    COPY_UUID_128(uuid_struct, 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x11)
#define COPY_CHAR_TX_UUID(uuid_struct)                                                                                             \
    COPY_UUID_128(uuid_struct, 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x12)
#define COPY_CHAR_ADDCOMMISSIONING_UUID(uuid_struct)                                                                               \
    COPY_UUID_128(uuid_struct, 0x64, 0x63, 0x02, 0x38, 0x87, 0x72, 0x45, 0xF2, 0xB8, 0x7D, 0x74, 0x8A, 0x83, 0x21, 0x8F, 0x04)

/* Public functions ----------------------------------------------------------*/
/**
 * @brief  Service initialization
 * @param  None
 * @retval None
 */
void SVCCTL_InitCustomSvc(void)
{

    Char_UUID_t uuid16;

    /**
     *	Register the event handler to the BLE controller
     */
    SVCCTL_RegisterSvcHandler(Matter_Event_Handler);

    /**
     *  Peer To Peer Service
     *
     * Max_Attribute_Records = 2*no_of_char + 1
     * service_max_attribute_record = 1 for Peer To Peer service +
     *                                2 for P2P Write characteristic +
     *                                2 for P2P Notify characteristic +
     *                                1 for client char configuration descriptor +
     *
     */

    uint16_t uuid = MATTER_SERVICE_UUID;

    aci_gatt_add_service(UUID_TYPE_16, (Service_UUID_t *) &uuid, PRIMARY_SERVICE, 8, &(aPeerToPeerContext.PeerToPeerSvcHdle));

    /**
     *  Add RX Characteristic
     */
    COPY_CHAR_RX_UUID(uuid16.Char_UUID_128);
    aci_gatt_add_char(aPeerToPeerContext.PeerToPeerSvcHdle, UUID_TYPE_128, &uuid16, 247, CHAR_PROP_WRITE, ATTR_PERMISSION_NONE,
                      GATT_NOTIFY_ATTRIBUTE_WRITE | GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP, /* gattEvtMask */
                      10,                                                                         /* encryKeySize */
                      1,                                                                          /* isVariable */
                      &(aPeerToPeerContext.P2PWriteClientToServerCharHdle));

    /**
     *   Add notification Characteristic
     */

    COPY_CHAR_TX_UUID(uuid16.Char_UUID_128);
    aci_gatt_add_char(aPeerToPeerContext.PeerToPeerSvcHdle, UUID_TYPE_128, &uuid16, 247, CHAR_PROP_INDICATE, ATTR_PERMISSION_NONE,
                      GATT_NOTIFY_ATTRIBUTE_WRITE, /* gattEvtMask */
                      10,                          /* encryKeySize */
                      1,                           /* isVariable: 1 */
                      &(aPeerToPeerContext.P2PNotifyServerToClientCharHdle));

    return;
}

/**
 * @brief  Characteristic update
 * @param  UUID: UUID of the characteristic
 * @param  Service_Instance: Instance of the service to which the characteristic belongs
 *
 */
tBleStatus CUSTOM_STM_App_Update_Char(uint16_t UUID, uint8_t * pPayload, uint16_t Length)
{
    tBleStatus result = BLE_STATUS_INVALID_PARAMS;
    switch (UUID)
    {
    case P2P_NOTIFY_CHAR_UUID:

        result = aci_gatt_update_char_value(aPeerToPeerContext.PeerToPeerSvcHdle,
                                            aPeerToPeerContext.P2PNotifyServerToClientCharHdle, 0, /* charValOffset */
                                            Length,                                                /* charValueLen */
                                            (uint8_t *) pPayload);

        break;

    default:
        break;
    }

    return result;
}

/* Private functions ----------------------------------------------------------*/

/**
 * @brief  Event handler
 * @param  Event: Address of the buffer holding the Event
 * @retval Ack: Return whether the Event has been managed or not
 */
static SVCCTL_EvtAckStatus_t Matter_Event_Handler(void * Event)
{
    SVCCTL_EvtAckStatus_t return_value;
    hci_event_pckt * event_pckt;
    evt_blecore_aci * blecore_evt;
    aci_gatt_attribute_modified_event_rp0 * attribute_modified;

    return_value = SVCCTL_EvtNotAck;
    event_pckt   = (hci_event_pckt *) (((hci_uart_pckt *) Event)->data);

    switch (event_pckt->evt)
    {
    case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE: {
        blecore_evt = (evt_blecore_aci *) event_pckt->data;
        switch (blecore_evt->ecode)
        {
        case ACI_GATT_SERVER_CONFIRMATION_VSEVT_CODE: {
            attribute_modified            = (aci_gatt_attribute_modified_event_rp0 *) blecore_evt->data;
            Notification.P2P_Evt_Opcode   = MATTER_STM_ACK_INDICATE_EVT;
            Notification.ConnectionHandle = attribute_modified->Connection_Handle;
            APP_MATTER_Notification(&Notification);
        }
        case ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE: {
            attribute_modified = (aci_gatt_attribute_modified_event_rp0 *) blecore_evt->data;
            if (attribute_modified->Attr_Handle == (aPeerToPeerContext.P2PNotifyServerToClientCharHdle + 2))
            {
                /**
                 * Descriptor handle
                 */
                return_value = SVCCTL_EvtAckFlowEnable;
                APP_DBG_MSG("Subscribe for c2 notification\n");
                /**
                 * Indicate to application
                 */
                if (attribute_modified->Attr_Data[0] & COMSVC_Indication)
                {
                    Notification.P2P_Evt_Opcode   = MATTER_STM_INDICATE_ENABLED_EVT;
                    Notification.ConnectionHandle = attribute_modified->Connection_Handle;
                    APP_MATTER_Notification(&Notification);
                }
                else
                {
                    Notification.P2P_Evt_Opcode   = MATTER_STM_INDICATE_DISABLED_EVT;
                    Notification.ConnectionHandle = attribute_modified->Connection_Handle;
                    APP_MATTER_Notification(&Notification);
                }
            }

            else if (attribute_modified->Attr_Handle == (aPeerToPeerContext.P2PWriteClientToServerCharHdle + 1))
            {
                Notification.P2P_Evt_Opcode          = MATTER_STM_WRITE_EVT;
                Notification.DataTransfered.Length   = attribute_modified->Attr_Data_Length;
                Notification.DataTransfered.pPayload = attribute_modified->Attr_Data;
                Notification.ConnectionHandle        = attribute_modified->Connection_Handle;
                if (Notification.DataTransfered.Length == 0)
                {
                    /* Exit the function because of bad packet  */
                    break;
                }
                APP_MATTER_Notification(&Notification);
            }
        }
        break;
        case ACI_GATT_WRITE_PERMIT_REQ_VSEVT_CODE: {
            aci_gatt_write_permit_req_event_rp0 * write_perm_req;
            write_perm_req = (aci_gatt_write_permit_req_event_rp0 *) blecore_evt->data;
            aci_gatt_write_resp(write_perm_req->Connection_Handle, write_perm_req->Attribute_Handle,
                                0x00, /* write_status = 0 (no error))*/
                                0x00, /* err_code */
                                write_perm_req->Data_Length, (uint8_t *) &(write_perm_req->Data[0]));
        }
        break;
        default:
            break;
        }
    }
    break; /* HCI_HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE_SPECIFIC */

    default:
        break;
    }

    return (return_value);
} /* end SVCCTL_EvtAckStatus_t */

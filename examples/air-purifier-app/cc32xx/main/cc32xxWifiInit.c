/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

//****************************************************************************
//
//! \addtogroup
//! @{
//
//****************************************************************************

/* Standard Include */
#include <assert.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* TI-DRIVERS Header files */
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/net/wifi/slwificonn.h>

#include <ti/devices/cc32xx/inc/hw_types.h>

#include <ti/devices/cc32xx/driverlib/rom_map.h>

#include <ti/devices/cc32xx/driverlib/pin.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>

#include "ti_drivers_config.h"
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>
#include <ti/net/slnet.h>
#include <ti/net/slnetconn.h>
#include <ti/net/slnetif.h>

#include "utils_if.h"
#include "wifi_if.h"

#include <AppConfig.h>
int CHIP_IF_init();

/* Application Version and Naming*/
#define APPLICATION_NAME "CC32XX-MATTER:: E-LOCK"
#define APPLICATION_VERSION "01.00.00.00"

/* USER's defines */
#define SLNETCONN_TASK_STACK_SIZE (2048)
#define DISPLAY_TASK_STACK_SIZE (512)

/****************************************************************************
              LOCAL FUNCTION PROTOTYPES
 ****************************************************************************/
static void ButtonLeftEventHandler(Button_Handle handle, Button_EventMask events);

/****************************************************************************
              EXT. FUNCTION PROTOTYPES
 ****************************************************************************/
extern void LWIP_IF_start();

/****************************************************************************
              GLOBAL VARIABLES
 ****************************************************************************/
pthread_t gSlNetConnThread = (pthread_t) NULL;
Button_Handle gButtonLeftHandle, gButtonRightHandle;
LED_Handle gLedBlueHandle, gLedGreenHandle, gLedRedHandle;

/****************************************************************************
              STATIC VARIABLES
 ****************************************************************************/
static bool gIsConnected = 0;

//*****************************************************************************
//                 Local Functions
//*****************************************************************************

void SimpleLinkSockEventHandler(SlSockEvent_t * pSock)
{
    /* Unused in this application */
}

void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t * pHttpEvent, SlNetAppHttpServerResponse_t * pHttpResponse)
{
    /* Unused in this application */
}

void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t * pNetAppRequest, SlNetAppResponse_t * pNetAppResponse)
{
    /* Unused in this application */
}

void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t * buffer)
{
    /* Unused in this application */
}

//*****************************************************************************
//
//! \brief  Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void DisplayBanner()
{
    cc32xxLog("\n\n\n\r");
    cc32xxLog("\t\t *************************************************\n\r");
    cc32xxLog("\t\t            %s Application       \n\r", APPLICATION_NAME);
    cc32xxLog("\t\t            %s            \n\r", APPLICATION_VERSION);
    cc32xxLog("\t\t *************************************************\n\r");
    cc32xxLog("\n\n\n\r");
}

//*****************************************************************************
//
//! \brief  SlWifiConn Event Handler
//!
//*****************************************************************************
void SlNetConnEventHandler(uint32_t ifID, SlNetConnStatus_e netStatus, void * data)
{
    switch (netStatus)
    {
    case SLNETCONN_STATUS_CONNECTED_MAC:
        gIsConnected = 1;
        LED_setOn(gLedBlueHandle, LED_BRIGHTNESS_MAX);
        cc32xxLog("[SlNetConnEventHandler] I/F %d - CONNECTED (MAC LEVEL)!\n\r", ifID);
        break;
    case SLNETCONN_STATUS_CONNECTED_IP:
        gIsConnected = 1;
        cc32xxLog("[SlNetConnEventHandler] I/F %d - CONNECTED (IP LEVEL)!\n\r", ifID);
        break;
    case SLNETCONN_STATUS_CONNECTED_INTERNET:
        gIsConnected = 1;
        cc32xxLog("[SlNetConnEventHandler] I/F %d - CONNECTED (INTERNET LEVEL)!\n\r", ifID);
        break;
    case SLNETCONN_STATUS_WAITING_FOR_CONNECTION:
    case SLNETCONN_STATUS_DISCONNECTED:
        gIsConnected = 0;
        LED_setOff(gLedBlueHandle);
        cc32xxLog("[SlNetConnEventHandler] I/F %d - DISCONNECTED!\n\r", ifID);
        break;
    default:
        cc32xxLog("[SlNetConnEventHandler] I/F %d - UNKNOWN STATUS\n\r", ifID);
        break;
    }
}

//*****************************************************************************
//
//! \brief Launchpad switch used to enable one shot provisioning
//!
//*****************************************************************************
static void ButtonLeftEventHandler(Button_Handle handle, Button_EventMask events)
{
    // Enable Provisioning
    int retVal = SlWifiConn_enableProvisioning(WifiProvMode_ONE_SHOT, PROVISIONING_CMD, 0);
    assert(retVal == 0);
}

//*****************************************************************************
//
//! \brief  Main application thread
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
int WiFi_init()
{
    int retVal;
    Button_Params buttonParams;
    LED_Params ledParams;

    SPI_init();

#ifdef NWP_LOG
    // NWP log config
    //  If your application already has UART0 configured, no need for this line
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
    // Mux Pin 62 to mode 1 for outputting NWP logs
    MAP_PinTypeUART(PIN_62, PIN_MODE_1);
#endif

    LED_Params_init(&ledParams); // default PWM LED
    gLedBlueHandle = LED_open(CONFIG_LED_BLUE, &ledParams);
    LED_setOff(gLedBlueHandle);

    Button_Params_init(&buttonParams);
    buttonParams.buttonEventMask   = Button_EV_CLICKED | Button_EV_LONGCLICKED;
    buttonParams.longPressDuration = 1000U; // ms
    gButtonLeftHandle              = Button_open(CONFIG_BTN_LEFT, &buttonParams);
    Button_setCallback(gButtonLeftHandle, ButtonLeftEventHandler);

    /* Enable SlWifiConn */
    retVal = WIFI_IF_init();
    assert(retVal >= 0);

    /* Enable SlNet framework */
    retVal = ti_net_SlNet_initConfig();
    assert(retVal == 0);

    /* Enable SlNetConn */
    retVal = SlNetConn_init(0);
    assert(retVal == 0);
    gSlNetConnThread = OS_createTask(1, SLNETCONN_TASK_STACK_SIZE, SlNetConn_process, NULL, OS_TASK_FLAG_DETACHED);
    assert(gSlNetConnThread);

    return (retVal);
}

int WiFi_deinit()
{
    int retVal;
    cc32xxLog("[APP] Networking App Completed (entering low power mode)\r\n ");
    retVal = SlNetConn_stop(SlNetConnEventHandler);
    retVal = WIFI_IF_deinit();
    cc32xxLog("[APP] Exit (%d) \r\n", retVal);
    assert(retVal == 0);
    return (retVal);
}

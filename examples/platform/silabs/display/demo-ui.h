/***************************************************************************/
/**
 * @file
 * @brief User Interface for demo.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "glib.h"
#include "sl_status.h"
/**************************************************************************/
/**
 * DEMO UI uses the underlying DMD interface and the
 *GLIB and exposes several wrapper functions to
 *application. These functions are used to display
 * different bitmaps for the demo.
 *
 ******************************************************************************/

#define APP_NAME_MAX_LENGTH 20

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

typedef enum
{
    DEMO_UI_PROTOCOL1,
    DEMO_UI_PROTOCOL2
} demoUIProtocol;

typedef enum
{
    DEMO_UI_LIGHT_OFF,
    DEMO_UI_LIGHT_ON
} demoUILightState_t;

typedef enum
{
    DEMO_UI_DIRECTION_PROT1,
    DEMO_UI_DIRECTION_PROT2,
    DEMO_UI_DIRECTION_SWITCH,
    DEMO_UI_DIRECTION_INVALID
} demoUILightDirection_t;

typedef enum
{
    DEMO_UI_NO_NETWORK,
    DEMO_UI_SCANNING,
    DEMO_UI_JOINING,
    DEMO_UI_FORMING,
    DEMO_UI_NETWORK_UP,
    DEMO_UI_STATE_UNKNOWN
} demoUIZigBeeNetworkState_t;

/*******************************************************************************
 ******************************   PROTOTYPES   *********************************
 ******************************************************************************/

/**************************************************************************/
/**
 * @brief
 *   Initilize the GLIB and DMD interfaces.
 *
 * @param[in] void
 *
 * @return
 *      void
 *****************************************************************************/
void demoUIInit(GLIB_Context_t * context);

/**************************************************************************/
/**
 * @brief
 *   Update the display with Silicon Labs logo and
 *application name.
 *
 * @param[in] name name of the current application.
 *
 * @return
 *      void
 *****************************************************************************/
void demoUIDisplayHeader(char * name);

/**************************************************************************/
/**
 * @brief
 *   Update the display with App image. Bool state only
 *for now.
 *
 * @param[in] on status of App
 *
 * @return
 *      void
 *****************************************************************************/
void demoUIDisplayApp(bool on);

/**************************************************************************/
/**
 * @brief
 *   Update the display to show if the bluetooth is
 *connected to the mobile device.
 *
 * @param[in] bool, true if the Light is connected to
 *mobile device, false otherwise.
 *
 * @return
 *      void
 *****************************************************************************/
void demoUIDisplayProtocol(demoUIProtocol protocol, bool isConnected);

/**************************************************************************/
/**
 * @brief
 *   Clear the Lcd screen and display the main screen.
 *
 * @param[in] name - application name
 * @param[in] showPROT1 - show protocol 1 related icon.
 * @param[in] showPROT2 - show protocol 2 related icon.
 *
 * @return
 *      void
 *****************************************************************************/
void demoUIClearMainScreen(uint8_t * name);

/**************************************************************************/
/**
 * @brief
 *   Clear the Lcd screen and display the main screen.
 *
 * @return
 *      Returns SL_STATUS_OK if successful, error otherwise.
 *****************************************************************************/
sl_status_t updateDisplay(void);

#ifdef __cplusplus
}
#endif

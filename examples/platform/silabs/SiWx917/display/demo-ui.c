/**
 * @file
 * @brief User Interface core logic for demo.
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

#include "demo-ui.h"
#include "demo-ui-bitmaps.h"
#include "dmd/dmd.h"
#include "em_types.h"
#include "glib.h"
#include <stdio.h>
#include <string.h>

// Main Logo and App image
#define SILICONLABS_X_POSITION ((glibContext.pDisplayGeometry->xSize - SILICONLABS_BITMAP_WIDTH) / 2)
#define SILICONLABS_Y_POSITION 0
#define APP_BITMAP_WIDTH 64
#define APP_BITMAP_HEIGHT 64
#define APP_X_POSITION ((glibContext.pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2)
#define APP_Y_POSITION (glibContext.pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5)
#define PROT1_ID_X_POSITION 1
#define PROT2_ID_X_POSITION 79

// Matter Logo
#define PROT2_BITMAP_WIDTH MATTER_LOGO_WIDTH
#define PROT2_BITMAP_HEIGHT MATTER_LOGO_HEIGHT
#define PROT2_X_POSITION 104
#define PROT2_Y_POSITION (APP_Y_POSITION + (APP_Y_POSITION / 2))
#define PROT2_BITMAP (matterLogoBitmap)
#define PROT2_BITMAP_CONN (matterLogoBitmap)

// Networking Protocol Logo
#ifdef SL_WIFI
#define PROT1_BITMAP_WIDTH WIFI_BITMAP_WIDTH
#define PROT1_BITMAP_HEIGHT WIFI_BITMAP_HEIGHT
#define PROT1_X_POSITION 8
#define PROT1_Y_POSITION (APP_Y_POSITION + (APP_Y_POSITION / 2))
#define PROT1_BITMAP (networkBitMap)
#define PROT1_BITMAP_CONN (networkBitMap)
#else
#define PROT1_BITMAP_WIDTH THREAD_BITMAP_WIDTH
#define PROT1_BITMAP_HEIGHT THREAD_BITMAP_HEIGHT
#define PROT1_X_POSITION 8
#define PROT1_Y_POSITION (APP_Y_POSITION + (APP_Y_POSITION / 2))
#define PROT1_BITMAP (networkBitMap)
#define PROT1_BITMAP_CONN (networkBitMap)
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
static GLIB_Context_t glibContext; /* Global glib context */

static const uint8_t siliconlabsBitmap[] = { SILABS_BITMAP };
static const uint8_t matterLogoBitmap[]  = { MATTER_LOGO_BITMAP };

static const uint8_t OnStateBitMap[]  = { ON_DEMO_BITMAP };
static const uint8_t OffStateBitMap[] = { OFF_DEMO_BITMAP };

#ifdef SL_WIFI
static const uint8_t networkBitMap[] = { WIFI_BITMAP };
#else
static const uint8_t networkBitMap[] = { THREAD_BITMAP };
#endif

// Future usage
// static const uint8_t unconnectedBitMap[] = { QUESTION_MARK_BITMAP };

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void demoUIDisplayLogo(void)
{
    GLIB_drawBitmap(&glibContext, SILICONLABS_X_POSITION, SILICONLABS_Y_POSITION, SILICONLABS_BITMAP_WIDTH,
                    SILICONLABS_BITMAP_HEIGHT, siliconlabsBitmap);
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
void demoUIInit(GLIB_Context_t * context)
{
    memcpy(&glibContext, context, sizeof(GLIB_Context_t));
}

void demoUIDisplayHeader(char * name)
{
    demoUIDisplayLogo();
    if (APP_NAME_MAX_LENGTH >= strlen(name))
    {
        GLIB_drawStringOnLine(&glibContext, name, 5, GLIB_ALIGN_CENTER, 0, 0, true);
    }
    DMD_updateDisplay();
}

void demoUIDisplayApp(bool on)
{
    GLIB_drawBitmap(&glibContext, APP_X_POSITION, APP_Y_POSITION, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                    (on ? OnStateBitMap : OffStateBitMap));
    DMD_updateDisplay();
}

void demoUIDisplayProtocol(demoUIProtocol protocol, bool isConnected)
{
    GLIB_drawBitmap(&glibContext, (protocol == DEMO_UI_PROTOCOL1 ? PROT1_X_POSITION : PROT2_X_POSITION),
                    (protocol == DEMO_UI_PROTOCOL1 ? PROT1_Y_POSITION : PROT2_Y_POSITION),
                    (protocol == DEMO_UI_PROTOCOL1 ? PROT1_BITMAP_WIDTH : PROT2_BITMAP_WIDTH),
                    (protocol == DEMO_UI_PROTOCOL1 ? PROT1_BITMAP_HEIGHT : PROT2_BITMAP_HEIGHT),
                    (protocol == DEMO_UI_PROTOCOL1 ? (isConnected ? PROT1_BITMAP_CONN : PROT1_BITMAP)
                                                   : (isConnected ? PROT2_BITMAP_CONN : PROT2_BITMAP)));
    DMD_updateDisplay();
}

void demoUIClearMainScreen(uint8_t * name)
{
    GLIB_clear(&glibContext);
    demoUIDisplayHeader((char *) name);
    demoUIDisplayApp(false);
    demoUIDisplayProtocol(DEMO_UI_PROTOCOL1, false);
    demoUIDisplayProtocol(DEMO_UI_PROTOCOL2, false);
}

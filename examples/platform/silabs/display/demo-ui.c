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
#include "sl_component_catalog.h"
#include "sl_memlcd.h"
#if SL_WIFI && !SLI_SI91X_MCU_INTERFACE
#include "spi_multiplex.h"
#endif // SL_WIFI && !SLI_SI91X_MCU_INTERFACE
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
#define PROT2_X_POSITION 104
#define PROT2_Y_POSITION (APP_Y_POSITION + (APP_Y_POSITION / 2))

// Networking Protocol Logo
#ifdef SL_WIFI
#define PROT1_BITMAP_WIDTH WIFI_BITMAP_WIDTH
#define PROT1_BITMAP_HEIGHT WIFI_BITMAP_HEIGHT
#define PROT1_X_POSITION 8
#define PROT1_Y_POSITION (APP_Y_POSITION + (APP_Y_POSITION / 2))
#else
#define PROT1_BITMAP_WIDTH THREAD_BITMAP_WIDTH
#define PROT1_BITMAP_HEIGHT THREAD_BITMAP_HEIGHT
#define PROT1_X_POSITION 8
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#define ZIGBEE_POSITION_Y (APP_Y_POSITION + (APP_Y_POSITION / 2) + (ZIGBEE_BITMAP_HEIGHT / 2))
#define PROT1_Y_POSITION (APP_Y_POSITION + (APP_Y_POSITION / 2) - (ZIGBEE_BITMAP_HEIGHT / 2))
#else
#define PROT1_Y_POSITION (APP_Y_POSITION + (APP_Y_POSITION / 2))
#endif
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

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
static const uint8_t zigbeeBitMap[] = { ZIGBEE_BITMAP };
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

sl_status_t updateDisplay(void)
{
    sl_status_t status = SL_STATUS_OK;
#if SLI_SI91X_MCU_INTERFACE && SL_ICD_ENABLED && DISPLAY_ENABLED
    sl_memlcd_post_wakeup_init();
#endif // SLI_SI91X_MCU_INTERFACE && SL_ICD_ENABLED && DISPLAY_ENABLED
#if SL_LCDCTRL_MUX
    status = sl_wfx_host_pre_lcd_spi_transfer();
    if (status != SL_STATUS_OK)
        return status;
#endif // SL_LCDCTRL_MUX
    status = DMD_updateDisplay();
    if (status != DMD_OK)
        return SL_STATUS_FAIL;
#if SL_LCDCTRL_MUX
    status = sl_wfx_host_post_lcd_spi_transfer();
    if (status != SL_STATUS_OK)
        return status;
#endif // SL_LCDCTRL_MUX

    return SL_STATUS_OK;
}

void demoUIDisplayHeader(char * name)
{
    demoUIDisplayLogo();
    if (APP_NAME_MAX_LENGTH >= strlen(name))
    {
        GLIB_drawStringOnLine(&glibContext, name, 5, GLIB_ALIGN_CENTER, 0, 0, true);
    }
    updateDisplay();
}

void demoUIDisplayApp(bool on)
{
    GLIB_drawBitmap(&glibContext, APP_X_POSITION, APP_Y_POSITION, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                    (on ? OnStateBitMap : OffStateBitMap));
    updateDisplay();
}

void demoUIDisplayProtocols()
{
    GLIB_drawBitmap(&glibContext, PROT2_X_POSITION, PROT2_Y_POSITION, MATTER_LOGO_WIDTH, MATTER_LOGO_HEIGHT, matterLogoBitmap);
    GLIB_drawBitmap(&glibContext, PROT1_X_POSITION, PROT1_Y_POSITION, PROT1_BITMAP_WIDTH, PROT1_BITMAP_HEIGHT, networkBitMap);
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
    GLIB_drawBitmap(&glibContext, PROT1_X_POSITION, ZIGBEE_POSITION_Y, ZIGBEE_BITMAP_WIDTH, ZIGBEE_BITMAP_HEIGHT, zigbeeBitMap);
#endif
    updateDisplay();
}

void demoUIClearMainScreen(uint8_t * name)
{
    GLIB_clear(&glibContext);
    demoUIDisplayHeader((char *) name);
    demoUIDisplayApp(false);
    demoUIDisplayProtocols();
}

/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <stdio.h>
#include <string.h>

#include "AppConfig.h"
#include "SensorsUI.h"
#include "demo-ui.h"
#include "demo-ui-bitmaps.h"
#include "dmd.h"
#include "glib.h"
#include "lcd.h"

uint8_t mName[] = SENSOR_NAME;
SensorsUI SensorsUI::sSensorUIManager;

#define APP_BITMAP_WIDTH 64
#define APP_BITMAP_HEIGHT 64
#define APP_X_POSITION ((glibContext.pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2)
#define APP_Y_POSITION (glibContext.pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5)

static const uint8_t OnStateOccupancySensorBitMap[]  = { ON_DEMO_OCCUPANCY_SENSOR_BITMAP };
static const uint8_t OffStateOccupancySensorBitMap[] = { OFF_DEMO_OCCUPANCY_SENSOR_BITMAP };

static const uint8_t OnStateContactSensorBitMap[]  = { ON_DEMO_CONTACT_SENSOR_BITMAP };
static const uint8_t OffStateContactSensorBitMap[] = { OFF_DEMO_CONTACT_SENSOR_BITMAP };

static const uint8_t OnStateWaterLeakDetectorBitMap[]  = { ON_DEMO_WATER_LEAK_DECTECTOR_BITMAP };
static const uint8_t OffStateWaterLeakDetectorBitMap[] = { OFF_DEMO_WATER_LEAK_DECTECTOR_BITMAP };

GLIB_Context_t * glibContext;


void SensorsUI::DrawUI(GLIB_Context_t * incGlibContext)
{
    glibContext = incGlibContext;

    if (glibContext == nullptr)
    {
        SILABS_LOG("Context is null");
        return;
    }

    DrawNextScreen(glibContext);


#if SL_LCDCTRL_MUX
    sl_wfx_host_pre_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
    DMD_updateDisplay();
#if SL_LCDCTRL_MUX
    sl_wfx_host_post_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
}

void SensorsUI::UpdateDemoState(bool state)
{
    if (SensorMgr().getCurrentSensorMode() == CurrentSensorEnum::OccupancySensor)
    {
#ifdef DISPLAY_ENABLED
        if(state)
        {
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OffStateOccupancySensorBitMap));
        }
        else
        {
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OnStateOccupancySensorBitMap)); 
        }
#endif
    }
    else if(SensorMgr().getCurrentSensorMode() == CurrentSensorEnum::ContactSensor)
    {
#ifdef DISPLAY_ENABLED
        if(state)
        {
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OffStateContactSensorBitMap));
        }
        else
        {
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OnStateContactSensorBitMap)); 
        }
#endif
    }
    else if(SensorMgr().getCurrentSensorMode() == CurrentSensorEnum::WaterLeakDetector)
    {
#ifdef DISPLAY_ENABLED
        if(state)
        {
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OffStateContactSensorBitMap));
        }
        else
        {
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OffStateWaterLeakDetectorBitMap)); 
        }
#endif
    }

#if SL_LCDCTRL_MUX
    sl_wfx_host_pre_lcd_spi_transfer();
#endif // SL_LCDCTRL_MUX
    DMD_updateDisplay();
#if SL_LCDCTRL_MUX
    sl_wfx_host_post_lcd_spi_transfer();
#endif
}

void SensorsUI::DrawNextScreen(GLIB_Context_t * glibContext)
{

    if (SensorMgr().getCurrentSensorMode() == CurrentSensorEnum::OccupancySensor)
    {
#ifdef DISPLAY_ENABLED
        GLIB_clear(glibContext);
        demoUIDisplayHeader((char *) mName);
        demoUIDisplayProtocols();
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OnStateOccupancySensorBitMap));
        #if SL_LCDCTRL_MUX
            sl_wfx_host_pre_lcd_spi_transfer();
        #endif // SL_LCDCTRL_MUX
            DMD_updateDisplay();
        #if SL_LCDCTRL_MUX
            sl_wfx_host_post_lcd_spi_transfer();
        #endif
#endif
    }
    if (SensorMgr().getCurrentSensorMode() == CurrentSensorEnum::TemperatureSensor)
    {
#ifdef DISPLAY_ENABLED
        AppTask::GetAppTask().GetLCD().SetCustomUI(SilabsSensors::TemperatureUI);
        AppTask::GetAppTask().GetLCD().WriteDemoUI(false);

#endif
    }
    if (SensorMgr().getCurrentSensorMode() == CurrentSensorEnum::ContactSensor)
    {
#ifdef DISPLAY_ENABLED
        GLIB_clear(glibContext);
        demoUIDisplayHeader((char *) mName);
        demoUIDisplayProtocols();
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OnStateContactSensorBitMap));
        #if SL_LCDCTRL_MUX
            sl_wfx_host_pre_lcd_spi_transfer();
        #endif // SL_LCDCTRL_MUX
            DMD_updateDisplay();
        #if SL_LCDCTRL_MUX
            sl_wfx_host_post_lcd_spi_transfer();
        #endif
#endif
    }
    if (SensorMgr().getCurrentSensorMode() == CurrentSensorEnum::WaterLeakDetector)
    {
#ifdef DISPLAY_ENABLED
        GLIB_clear(glibContext);
        demoUIDisplayHeader((char *) mName);
        demoUIDisplayProtocols();
        GLIB_drawBitmap(glibContext, (glibContext->pDisplayGeometry->xSize - APP_BITMAP_WIDTH) / 2, glibContext->pDisplayGeometry->ySize - APP_BITMAP_HEIGHT - 5, APP_BITMAP_WIDTH, APP_BITMAP_HEIGHT,
                (OnStateWaterLeakDetectorBitMap));
        #if SL_LCDCTRL_MUX
            sl_wfx_host_pre_lcd_spi_transfer();
        #endif // SL_LCDCTRL_MUX
            DMD_updateDisplay();
        #if SL_LCDCTRL_MUX
            sl_wfx_host_post_lcd_spi_transfer();
        #endif
#endif
    }    

}

#pragma once

#include "SensorsCallbacks.h"
#include "AppTask.h"
#include "glib.h"

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#endif

class SensorsUI
{

public:

    static void DrawUI(GLIB_Context_t * glibContext);
    void UpdateDemoState(bool state);

private:
    static void DrawNextScreen(GLIB_Context_t * glibContext);
    friend SensorsUI & SensorUIMgr();

    static SensorsUI sSensorUIManager;
};

inline SensorsUI & SensorUIMgr()
{
    return SensorsUI::sSensorUIManager;
}

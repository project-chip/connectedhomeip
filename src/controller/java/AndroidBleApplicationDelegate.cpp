/* See Project CHIP LICENSE file for licensing information. */

#include <core/CHIPConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include "AndroidBleApplicationDelegate.h"

#include <stddef.h>

AndroidBleApplicationDelegate::AndroidBleApplicationDelegate() : NotifyChipConnectionClosedCb(NULL) {}

void AndroidBleApplicationDelegate::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj)
{
    if (NotifyChipConnectionClosedCb)
    {
        NotifyChipConnectionClosedCb(connObj);
    }
}

void AndroidBleApplicationDelegate::SetNotifyChipConnectionClosedCallback(NotifyChipConnectionClosedCallback cb)
{
    NotifyChipConnectionClosedCb = cb;
}

#endif /* CONFIG_NETWORK_LAYER_BLE */

/* See Project CHIP LICENSE file for licensing information. */

#include <ble/BleLayer.h>
#include <controller/java/AndroidBleConnectionDelegate.h>

void AndroidBleConnectionDelegate::NewConnection(chip::Ble::BleLayer * bleLayer, void * appState, const uint16_t connDiscriminator)
{
    if (newConnectionCb)
    {
        newConnectionCb(bleLayer->mAppState, connDiscriminator);
    }
}

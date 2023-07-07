/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <gio/gio.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#include "dbus/DBusLightApp.h"

namespace example {

class DBusInterface
{
public:
    DBusInterface(chip::EndpointId endpointId) : mEndpointId(endpointId) {}

    CHIP_ERROR Init();

    void SetOnOff(bool on);
    void SetLevel(uint8_t value);
    void SetHue(uint8_t value);
    void SetSaturation(uint8_t value);

private:
    static CHIP_ERROR InitOnGLibMatterContext(DBusInterface * self);
    static void OnBusAcquired(GDBusConnection *, const char *, DBusInterface * self);
    static void OnBusLost(GDBusConnection *, const char * name, DBusInterface * self);

    static gboolean OnOnOffChanged(LightAppOnOff *, GDBusMethodInvocation *, DBusInterface *);
    static gboolean OnCurrentLevelChanged(LightAppLevelControl *, GDBusMethodInvocation *, DBusInterface *);
    static gboolean OnCurrentHueChanged(LightAppColorControl *, GDBusMethodInvocation *, DBusInterface *);
    static gboolean OnCurrentSaturationChanged(LightAppColorControl *, GDBusMethodInvocation *, DBusInterface *);

    class InternalSetGuard
    {
    public:
        InternalSetGuard(DBusInterface * iface) : mIface(iface) { mIface->mInternalSet = true; }
        ~InternalSetGuard() { mIface->mInternalSet = false; }

    private:
        DBusInterface * mIface;
    };

    const chip::EndpointId mEndpointId;

    GDBusObjectManagerServer * mManager = nullptr;
    bool mNameAcquired                  = false;

    LightAppOnOff * mIfaceOnOff               = nullptr;
    LightAppLevelControl * mIfaceLevelControl = nullptr;
    LightAppColorControl * mIfaceColorControl = nullptr;
    bool mInternalSet                         = false;
};

} // namespace example

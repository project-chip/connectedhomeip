/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <glib.h>
#include <softap.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class SoftAPManager
{
public:
    static void Init(void);
    static void Deinit(void);

    static CHIP_ERROR IsEnabled(bool * isSoftAPEnabled);
    static CHIP_ERROR Enable(void);
    static CHIP_ERROR Disable(void);

    static CHIP_ERROR GetAPState(bool * apState);
    static CHIP_ERROR SetAPState(bool apState);

private:
    static void _EnabledCb(softap_error_e softAPErr, bool isRequested, void * userData);
    static void _DisabledCb(softap_error_e softAPErr, softap_disabled_cause_e disabledCode, void * userData);
    static void _SecurityTypeChangedCb(softap_security_type_e securityType, void * userData);
    static void _SsidVisibilityChangedCb(bool visibility, void * userData);
    static void _PassphraseChangedCb(void * userData);
    static void _ClientConnectionStateChangedCb(softap_client_h client, bool open, void * userData);

    static gboolean _SoftAPInitialize(gpointer userData);
    static gboolean _SoftAPEnable(GMainLoop * mainLoop, gpointer userData);
    static gboolean _SoftAPDisable(GMainLoop * mainLoop, gpointer userData);

    void _SoftAPCheckSupportedFeature(void);
    CHIP_ERROR _SoftAPConfigure(void);
    void _SoftAPDeinitialize(void);
    void _SoftAPSetCallbacks(void);
    void _SoftAPUnsetCallbacks(void);

    static SoftAPManager sInstance;

    softap_h mSoftAPHandle;
    GMainLoop * mEnableLoop;
    GMainLoop * mDisableLoop;
    bool mSupportedFeature;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

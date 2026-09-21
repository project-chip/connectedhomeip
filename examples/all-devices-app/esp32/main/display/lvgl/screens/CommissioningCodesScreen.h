/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lvgl.h>

/**
 * Fills @a parent with the commissioning codes: the QR code a commissioner
 * scans, plus the manual pairing code as a fallback.
 *
 * Both strings are copied into the widgets, so the caller may reuse its
 * buffers afterwards. The LVGL port lock must be held by the caller.
 */
void ShowCommissioningCodes(lv_obj_t * parent, const char * qrCodePayload, const char * manualPairingCode);

/**
 * Convenience entry point matching RenderScreenFn: queries setup codes and
 * calls ShowCommissioningCodes().
 */
void ShowCommissioningCodesScreen(lv_obj_t * parent);

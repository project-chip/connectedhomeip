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

#include <functional>
#include <lvgl.h>
#include <string>

using RenderScreenFn = std::function<void(lv_obj_t * parent)>;

/**
 * Push/pop screen navigation with a breadcrumb bar. Every entry point takes the
 * display lock's owner for granted: call from the LVGL task, or from another
 * thread with the display lock held.
 */
namespace NavigationStack {

void Init(lv_obj_t * rootScreen);

void Push(const std::string & title, RenderScreenFn renderFn);
void Pop();
void PopTo(size_t level);
void ResetToRoot();

/// Rebuilds the current screen in place, for content that changed underneath it.
void RefreshCurrent();
size_t GetDepth();

/// Connectivity shown at the right end of the bar.
void SetIndicators(bool wifiConnected, bool bleConnected, uint8_t fabricCount);

/// Forgets the widgets before the caller deletes them.
void Detach();

} // namespace NavigationStack

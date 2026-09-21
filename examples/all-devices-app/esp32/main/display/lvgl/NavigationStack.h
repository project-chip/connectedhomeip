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

namespace NavigationStack {

/**
 * Initializes the hierarchical navigation shell on @a rootScreen.
 * Must be called while holding the LVGL lock.
 */
void Init(lv_obj_t * rootScreen);

/**
 * Pushes a new screen to the navigation stack.
 * @param title Breadcrumb label displayed in the top bar.
 * @param renderFn Callback to populate the content container.
 */
void Push(const std::string & title, RenderScreenFn renderFn);

/**
 * Pops the current top screen from the stack and returns to its parent.
 */
void Pop();

/**
 * Pops all screens above @a level in the stack.
 */
void PopTo(size_t level);

/**
 * Pops back to the root screen (level 0).
 */
void ResetToRoot();

/**
 * Returns the current stack depth.
 */
size_t GetDepth();

/**
 * Forgets the stack contents and the widgets created by Init().
 *
 * Call this before deleting those widgets (for example with lv_obj_clean() on the root
 * screen), otherwise the shell keeps pointers to freed objects. Navigation calls render
 * nothing until Init() runs again.
 */
void Detach();

} // namespace NavigationStack

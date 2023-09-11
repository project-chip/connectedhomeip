/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

namespace example {
namespace Ui {

/**
 * Represents a generic UI window for a ImGUI application.
 *
 * Provides callbacks of loading state, updating state and rendering the
 * actual UI.
 *
 * UI rendering is expected to be done using imgui.
 */
class Window
{
public:
    virtual ~Window() = default;

    // State updates will run in the chip main loop

    virtual void LoadInitialState() {}
    virtual void UpdateState() {}

    // Render the UI
    // MUST use Imgui rendering, generally within a Begin/End block to
    // create a window.
    virtual void Render() = 0;
};

} // namespace Ui
} // namespace example

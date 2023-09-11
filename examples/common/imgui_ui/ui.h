/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <AppMain.h>
#include <imgui_ui/windows/window.h>

#include <semaphore.h>

#include <list>
#include <memory>

namespace example {
namespace Ui {

/**
 * Supports showing a UI using ImGUI
 *
 * The UI supports several windows, such as QR codes or device control.
 */
class ImguiUi : public AppMainLoopImplementation
{
public:
    ImguiUi() { sem_init(&mChipLoopWaitSemaphore, 0 /* shared */, 0); }
    virtual ~ImguiUi() { sem_destroy(&mChipLoopWaitSemaphore); }

    void AddWindow(std::unique_ptr<Window> window) { mWindows.push_back(std::move(window)); }

    void UpdateState(); // runs a state update from ember/app
    void Render();      // render windows to screen

    // AppMainLoopImplementation
    void RunMainLoop() override;
    void SignalSafeStopMainLoop() override;

private:
    // First initial state load
    void ChipLoopLoadInitialState();

    // Updates the window states. Run in the CHIP main loop (has access
    // to CHIP API calls)
    void ChipLoopStateUpdate();

    sem_t mChipLoopWaitSemaphore;
    std::list<std::unique_ptr<Window>> mWindows;

    static void ChipLoopUpdateCallback(intptr_t self);
};

} // namespace Ui
} // namespace example

/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "ui.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <lib/support/logging/CHIPLogging.h>

#include <atomic>
#include <thread>

namespace example {
namespace Ui {
namespace {

// Controls running the UI event loop
std::atomic<bool> gUiRunning{ false };

void UiInit(SDL_GLContext * gl_context, SDL_Window ** window)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        ChipLogError(AppServer, "SDL Init Error: %s\n", SDL_GetError());
        return;
    }

#if defined(__APPLE__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    *window     = SDL_CreateWindow("Light UI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    *gl_context = SDL_GL_CreateContext(*window);
    SDL_GL_MakeCurrent(*window, *gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    (void) io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(*window, *gl_context);
    ImGui_ImplOpenGL3_Init();
}

void UiShutdown(SDL_GLContext * gl_context, SDL_Window ** window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(*gl_context);
    SDL_DestroyWindow(*window);
    SDL_Quit();
}

void EventLoop(ImguiUi * ui)
{
    gUiRunning = true;
    SDL_GLContext gl_context;
    SDL_Window * window = nullptr;

    UiInit(&gl_context, &window);

    ImGuiIO & io = ImGui::GetIO();

    while (gUiRunning.load())
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if ((event.type == SDL_QUIT) ||
                (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID(window)))
            {
                gUiRunning = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ui->UpdateState();
        ui->Render();

        // rendering
        ImGui::Render();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    UiShutdown(&gl_context, &window);

    ChipLogProgress(AppServer, "UI thread Stopped...");
}

} // namespace

void ImguiUi::RunMainLoop()
{
    // Guaranteed to be on the main task (no chip event loop started yet)
    ChipLoopLoadInitialState();

    // Platform event loop will be on a separate thread,
    // while the event UI loop will be on the main thread.
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();

    // SignalSafeStopMainLoop will stop this loop below
    // or the loop exits by itself when processing a SDL
    // exit (generally by clicking the window close icon).
    EventLoop(this);

    // ensure shutdown events are generated (generally basic cluster
    // will send a shutdown event to subscribers).
    //
    // We attempt to wait for finish as the event will be sent sync.
    // Since the Main loop is stopped, there will be no MRP, however at least
    // one event is attempted to be sent.
    chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) {
            chip::DeviceLayer::PlatformMgr().HandleServerShuttingDown();
            sem_t * semaphore = reinterpret_cast<sem_t *>(arg);
            sem_post(semaphore); // notify complete
        },
        reinterpret_cast<intptr_t>(&mChipLoopWaitSemaphore));
    sem_wait(&mChipLoopWaitSemaphore);

    // Stop the chip main loop as well. This is expected to
    // wait for the task to finish.
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

void ImguiUi::SignalSafeStopMainLoop()
{
    gUiRunning = false;
}

void ImguiUi::ChipLoopStateUpdate()
{
    assertChipStackLockedByCurrentThread();
    for (auto it = mWindows.begin(); it != mWindows.end(); it++)
    {
        (*it)->UpdateState();
    }
}

void ImguiUi::ChipLoopLoadInitialState()
{
    assertChipStackLockedByCurrentThread();
    for (auto it = mWindows.begin(); it != mWindows.end(); it++)
    {
        (*it)->LoadInitialState();
    }
}

void ImguiUi::Render()
{
    for (auto it = mWindows.begin(); it != mWindows.end(); it++)
    {
        (*it)->Render();
    }
}

void ImguiUi::ChipLoopUpdateCallback(intptr_t self)
{
    ImguiUi * _this = reinterpret_cast<ImguiUi *>(self);
    _this->ChipLoopStateUpdate();
    sem_post(&_this->mChipLoopWaitSemaphore); // notify complete
}

void ImguiUi::UpdateState()
{
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(&ChipLoopUpdateCallback, reinterpret_cast<intptr_t>(this));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to schedule state update: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    // ensure update is done when exiting
    sem_wait(&mChipLoopWaitSemaphore);
}

} // namespace Ui
} // namespace example

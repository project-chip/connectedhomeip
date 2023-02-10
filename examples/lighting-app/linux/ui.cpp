/*
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

#include "ui.h"

#include <Options.h> // examples/platform/linux/Options.h
#include <app/server/OnboardingCodesUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/attribute-storage.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <qrcodegen.h>

#include <atomic>
#include <chrono>
#include <semaphore.h>
#include <thread>

namespace example {
namespace Ui {

namespace {

std::atomic<bool> gUiRunning{ false };

class DeviceState
{
public:
    DeviceState() { sem_init(&mChipLoopWaitSemaphore, 0 /* shared */, 0); }
    ~DeviceState() { sem_destroy(&mChipLoopWaitSemaphore); }

    // Initialize. MUST be called within the CHIP main loop as it
    // loads startup data.
    void Init();

    // Use ImgUI to show the current state
    void ShowUi();

    // Fetches the current state from Ember
    void UpdateState();

private:
    static constexpr int kQRCodeVersion   = qrcodegen_VERSION_MAX;
    static constexpr int kMaxQRBufferSize = qrcodegen_BUFFER_LEN_FOR_VERSION(kQRCodeVersion);

    sem_t mChipLoopWaitSemaphore;

    bool mHasQRCode                   = false;
    uint8_t mQRData[kMaxQRBufferSize] = { 0 };

    // light data:
    bool mOnOff = false;

    // Updates the data (run in the chip event loop)
    void ChipLoopUpdate();

    void InitQRCode();

    // Run in CHIPMainLoop to access ember in a single threaded
    // fashion
    static void ChipLoopUpdateCallback(intptr_t self);
};

DeviceState gDeviceState;

void DeviceState::Init()
{
    InitQRCode();
}

void DeviceState::InitQRCode()
{

    chip::PayloadContents payload = LinuxDeviceOptions::GetInstance().payload;
    if (!payload.isValidQRCodePayload())
    {
        return;
    }

    char payloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCode(payloadBuffer);

    CHIP_ERROR err = GetQRCode(qrCode, payload);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to load QR code: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    if (qrCode.size() > kMaxQRBufferSize)
    {
        ChipLogError(AppServer, "Insufficient qr code buffer size to encode");
        return;
    }

    uint8_t tempAndData[kMaxQRBufferSize];
    memcpy(tempAndData, qrCode.data(), qrCode.size());

    mHasQRCode = qrcodegen_encodeBinary(tempAndData, qrCode.size(), mQRData, qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN,
                                        qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

    if (!mHasQRCode)
    {
        ChipLogError(AppServer, "Failed to encode QR code");
        return;
    }
}

inline ImVec2 operator+(const ImVec2 & a, const ImVec2 & b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

void DeviceState::ShowUi()
{
    ImGui::Begin("Light app");
    ImGui::Text("Here is the current ember device state:");
    ImGui::Checkbox("Light is ON", &mOnOff);
    ImGui::End();

    if (mHasQRCode)
    {
        ImGui::Begin("QR Code.");

        ImDrawList * drawList = ImGui::GetWindowDrawList();

        constexpr int kBorderSize    = 35;
        constexpr int kMinWindowSize = 200;
        const int kQRCodeSize        = qrcodegen_getSize(mQRData);

        ImVec2 pos  = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();

        if (size.y < kMinWindowSize)
        {
            size = ImVec2(kMinWindowSize, kMinWindowSize);
            ImGui::SetWindowSize(size);
        }

        // Fill the entire window white, then figure out borders
        drawList->AddRectFilled(pos, pos + size, IM_COL32_WHITE);

        // add a border
        if (size.x >= 2 * kBorderSize && size.y >= 2 * kBorderSize)
        {
            size.x -= 2 * kBorderSize;
            size.y -= 2 * kBorderSize;
            pos.x += kBorderSize;
            pos.y += kBorderSize;
        }

        // create a square rectangle: keep only the smaller side and adjust the
        // other
        if (size.x > size.y)
        {
            pos.x += (size.x - size.y) / 2;
            size.x = size.y;
        }
        else if (size.y > size.x)
        {
            pos.y += (size.y - size.x) / 2;
            size.y = size.x;
        }

        const ImVec2 squareSize = ImVec2(size.x / static_cast<float>(kQRCodeSize), size.y / static_cast<float>(kQRCodeSize));

        for (int y = 0; y < kQRCodeSize; ++y)
        {
            for (int x = 0; x < kQRCodeSize; ++x)
            {
                if (qrcodegen_getModule(mQRData, x, y))
                {
                    ImVec2 placement =
                        ImVec2(pos.x + static_cast<float>(x) * squareSize.x, pos.y + static_cast<float>(y) * squareSize.y);
                    drawList->AddRectFilled(placement, placement + squareSize, IM_COL32_BLACK);
                }
            }
        }

        ImGui::End();
    }
}

void DeviceState::ChipLoopUpdate()
{
    // This will contain a dimmable light
    static constexpr chip::EndpointId kLightEndpointId = 1;

    // TODO:
    //    - consider error checking
    //    - add more attributes to the display (color? brightness?)
    {
        uint8_t value;
        emberAfReadServerAttribute(kLightEndpointId, chip::app::Clusters::OnOff::Id,
                                   chip::app::Clusters::OnOff::Attributes::OnOff::Id, &value, sizeof(value));
        mOnOff = (value != 0);
    }
}

void DeviceState::ChipLoopUpdateCallback(intptr_t self)
{
    DeviceState * _this = reinterpret_cast<DeviceState *>(self);
    _this->ChipLoopUpdate();
    sem_post(&_this->mChipLoopWaitSemaphore); // notify complete
}

void DeviceState::UpdateState()
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(&ChipLoopUpdateCallback, reinterpret_cast<intptr_t>(this));
    // ensure update is done when existing
    if (sem_trywait(&mChipLoopWaitSemaphore) != 0)
    {
        if (!gUiRunning.load())
        {
            // UI should stop, no need to wait, probably chip main loop is stopped
            return;
        }
        std::this_thread::yield();
    }
}

void UiInit(SDL_GLContext * gl_context, SDL_Window ** window)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        ChipLogError(AppServer, "SDL Init Error: %s\n", SDL_GetError());
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    *window     = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
                               window_flags);
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

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(*window, *gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
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

void UiLoop()
{
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
            if (event.type == SDL_QUIT)
            {
                chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
            {
                chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        gDeviceState.UpdateState();
        gDeviceState.ShowUi();

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

static std::thread gUiThread;

} // namespace

void Start()
{
    // Init inside the "main" thread, so that it can access globals
    // proparly (for QR code and such)
    gDeviceState.Init();

    gUiRunning = true;
    std::thread uiThread(&UiLoop);
    gUiThread.swap(uiThread);
}

void Stop()
{
    gUiRunning = false;
    gUiThread.join();
}

} // namespace Ui
} // namespace example

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
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>

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

using namespace chip::app::Clusters;
using chip::app::DataModel::Nullable;

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
    static constexpr int kMaxColors       = 6; // spec defined maximum

    sem_t mChipLoopWaitSemaphore;

    bool mHasQRCode                   = false;
    uint8_t mQRData[kMaxQRBufferSize] = { 0 };

    // OnOff
    bool mLightIsOn = false;

    // Level
    uint8_t mMinLevel = 0;
    uint8_t mMaxLevel = 0;
    Nullable<uint8_t> mCurrentLevel;
    uint16_t mLevelRemainingTime10sOfSec = 0;

    // Color control
    uint8_t mColorMode               = EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION;
    uint8_t mColorHue                = 0;
    uint8_t mColorSaturation         = 0;
    uint16_t mColorX                 = 0;
    uint16_t mColorY                 = 0;
    uint16_t mColorTemperatureMireds = 0;

    // Updates the data (run in the chip event loop)
    void ChipLoopUpdate();

    void InitQRCode();

    // displays a window with the QR Code.
    void DrawQRCode();

    // Run in CHIPMainLoop to access ember in a single threaded
    // fashion
    static void ChipLoopUpdateCallback(intptr_t self);
};

DeviceState gDeviceState;

/*
 * Converts HSV with assumption that V == 100% into a IMGui color vector
 */
ImVec4 HueSaturationToColor(float hueDegrees, float saturationPercent)
{
    saturationPercent /= 100.0f;

    float x = saturationPercent * static_cast<float>(1 - fabs(fmod(hueDegrees / 60, 2) - 1));

    if (hueDegrees < 60)
    {
        return ImVec4(saturationPercent, x, 0, 1.0f);
    }
    if (hueDegrees < 120)
    {
        return ImVec4(x, saturationPercent, 0, 1.0f);
    }
    if (hueDegrees < 180)
    {
        return ImVec4(0, saturationPercent, x, 1.0f);
    }
    if (hueDegrees < 240)
    {
        return ImVec4(0, x, saturationPercent, 1.0f);
    }
    if (hueDegrees < 300)
    {
        return ImVec4(x, 0, saturationPercent, 1.0f);
    }

    return ImVec4(saturationPercent, 0, x, 1.0f);
}

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
    ImGui::Begin("Light app state");

    ImGui::Text("On-Off:");
    ImGui::Indent();
    if (mLightIsOn)
    {
        ImGui::Text("Light is ON");
    }
    else
    {
        ImGui::Text("Light is OFF");
    }

    // bright yellow vs dark yellow on/off view
    ImGui::ColorButton("LightIsOn", mLightIsOn ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.3f, 0.3f, 0.0f, 1.0f),
                       0 /* ImGuiColorEditFlags_* */, ImVec2(80, 80));
    ImGui::Unindent();

    ImGui::Text("Level Control:");
    ImGui::Indent();
    ImGui::Text("Remaining Time (1/10s): %d", mLevelRemainingTime10sOfSec);
    ImGui::Text("MIN Level:              %d", mMinLevel);
    ImGui::Text("MAX Level:              %d", mMaxLevel);
    if (mCurrentLevel.IsNull())
    {
        ImGui::Text("Current Level: NULL");
    }
    else
    {
        int levelValue = mCurrentLevel.Value();
        ImGui::SliderInt("Current Level", &levelValue, mMinLevel, mMaxLevel);
    }
    ImGui::Unindent();

    ImGui::Text("Color Control:");
    ImGui::Indent();
    const char * mode = // based on ColorMode attribute: spec 3.2.7.9
        (mColorMode == EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION)
        ? "Hue/Saturation"
        : (mColorMode == EMBER_ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y)
            ? "X/Y"
            : (mColorMode == EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE) ? "Temperature/Mireds" : "UNKNOWN";

    ImGui::Text("Mode: %s", mode);

    if (mColorMode == EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION)
    {
        const float hueDegrees        = (mColorHue * 360.0f) / 254.0f;
        const float saturationPercent = 100.0f * (mColorSaturation / 254.0f);

        ImGui::Text("Current Hue:        %d (%f deg)", mColorHue, hueDegrees);
        ImGui::Text("Current Saturation: %d (%f %%)", mColorSaturation, saturationPercent);

        ImGui::ColorButton("LightColor", HueSaturationToColor(hueDegrees, saturationPercent), 0 /* ImGuiColorEditFlags_* */,
                           ImVec2(80, 80));
    }
    else if (mColorMode == EMBER_ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y)
    {
        ImGui::Text("Current X: %d", mColorX);
        ImGui::Text("Current Y: %d", mColorY);
    }
    else if (mColorMode == EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE)
    {
        ImGui::Text("Color Temperature Mireds: %d", mColorTemperatureMireds);
    }
    ImGui::Unindent();

    ImGui::End();

    DrawQRCode();
}

void DeviceState::DrawQRCode()
{
    if (!mHasQRCode)
    {
        return;
    }

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

void DeviceState::ChipLoopUpdate()
{
    // This will contain a dimmable light
    static constexpr chip::EndpointId kLightEndpointId = 1;

    // TODO:
    //    - consider error checking
    {
        OnOff::Attributes::OnOff::Get(kLightEndpointId, &mLightIsOn);

        // Level Control
        LevelControl::Attributes::CurrentLevel::Get(kLightEndpointId, mCurrentLevel);
        LevelControl::Attributes::MinLevel::Get(kLightEndpointId, &mMinLevel);
        LevelControl::Attributes::MaxLevel::Get(kLightEndpointId, &mMaxLevel);
        LevelControl::Attributes::RemainingTime::Get(kLightEndpointId, &mLevelRemainingTime10sOfSec);

        // Color control
        ColorControl::Attributes::ColorMode::Get(kLightEndpointId, &mColorMode);

        ColorControl::Attributes::CurrentHue::Get(kLightEndpointId, &mColorHue);
        ColorControl::Attributes::CurrentSaturation::Get(kLightEndpointId, &mColorSaturation);
        ColorControl::Attributes::CurrentX::Get(kLightEndpointId, &mColorX);
        ColorControl::Attributes::CurrentY::Get(kLightEndpointId, &mColorY);
        ColorControl::Attributes::ColorTemperatureMireds::Get(kLightEndpointId, &mColorTemperatureMireds);
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
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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

} // namespace

void Init()
{
    // Init inside the "main" thread, so that it can access globals
    // properly (for QR code and such)
    gDeviceState.Init();
}

void EventLoop()
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
                StopEventLoop();
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

void StopEventLoop()
{
    gUiRunning = false;
}

} // namespace Ui
} // namespace example

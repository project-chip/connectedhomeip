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

#include "qrcode.h"

#include <imgui.h>

#include <Options.h> // examples/platform/linux/Options.h
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

namespace example {
namespace Ui {
namespace Windows {
namespace {

inline ImVec2 operator+(const ImVec2 & a, const ImVec2 & b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

} // namespace

void QRCode::LoadInitialState()
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

void QRCode::Render()
{
    ImGui::Begin("QR Code.");
    if (!mHasQRCode)
    {
        ImGui::Text("MISSING/ERROR!");
        ImGui::End();
        return;
    }

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

} // namespace Windows
} // namespace Ui
} // namespace example

/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "window.h"

#include <qrcodegen.h>

#include <stdint.h>

namespace example {
namespace Ui {
namespace Windows {

class QRCode : public Window
{
public:
    void LoadInitialState() override;
    void Render() override;

private:
    static constexpr int kQRCodeVersion   = qrcodegen_VERSION_MAX;
    static constexpr int kMaxQRBufferSize = qrcodegen_BUFFER_LEN_FOR_VERSION(kQRCodeVersion);

    bool mHasQRCode                   = false;
    uint8_t mQRData[kMaxQRBufferSize] = { 0 };
};

} // namespace Windows
} // namespace Ui
} // namespace example

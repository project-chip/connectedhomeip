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

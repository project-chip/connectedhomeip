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

#include <stdint.h>

#include <controller/CHIPDeviceController.h>
#include <controller/python/matter/native/PyChipError.h>

extern "C" {

struct PychipImCaptureSnapshot
{
    uint32_t invokeResponseCount;
    uint32_t statusResponseCount;
    uint32_t writeResponseCount;
    uint32_t totalImResponseCount;
};

PyChipError pychip_im_capture_set_observer(chip::Controller::DeviceCommissioner * devCtrl);
PyChipError pychip_im_capture_reset(void);
PyChipError pychip_im_capture_get_snapshot(PychipImCaptureSnapshot * out);

} // extern "C"

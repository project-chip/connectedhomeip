/*
 *
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

#pragma once

#include <stddef.h>

#include <controller/python/chip/native/PyChipError.h>

extern "C" {
PyChipError pychip_ConvertX509CertToChipCert(const uint8_t * x509Cert, size_t x509CertLen, uint8_t * chipCert,
                                             size_t * chipCertLen);
PyChipError pychip_ConvertChipCertToX509Cert(const uint8_t * chipCert, size_t chipCertLen, uint8_t * x509Cert,
                                             size_t * x509CertLen);
}

/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#pragma once

#define _CHIP_JNI_ERROR(e) CHIP_APPLICATION_ERROR((e))

#define CHIP_JNI_ERROR_EXCEPTION_THROWN _CHIP_JNI_ERROR(0)
#define CHIP_JNI_ERROR_TYPE_NOT_FOUND _CHIP_JNI_ERROR(1)
#define CHIP_JNI_ERROR_METHOD_NOT_FOUND _CHIP_JNI_ERROR(2)
#define CHIP_JNI_ERROR_FIELD_NOT_FOUND _CHIP_JNI_ERROR(3)
#define CHIP_JNI_ERROR_NO_ENV _CHIP_JNI_ERROR(4)
#define CHIP_JNI_ERROR_NULL_OBJECT _CHIP_JNI_ERROR(5)
#define CHIP_JNI_ERROR_DEVICE_NOT_FOUND _CHIP_JNI_ERROR(6)
#define CHIP_JNI_ERROR_JAVA_ERROR _CHIP_JNI_ERROR(7)

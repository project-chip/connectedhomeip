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

#define CDC_JNI_ERROR_MIN 10000
#define CDC_JNI_ERROR_MAX 10999

#define _CDC_JNI_ERROR(e) (CDC_JNI_ERROR_MIN + (e))

#define CDC_JNI_ERROR_EXCEPTION_THROWN _CDC_JNI_ERROR(0)
#define CDC_JNI_ERROR_TYPE_NOT_FOUND _CDC_JNI_ERROR(1)
#define CDC_JNI_ERROR_METHOD_NOT_FOUND _CDC_JNI_ERROR(2)
#define CDC_JNI_ERROR_FIELD_NOT_FOUND _CDC_JNI_ERROR(3)

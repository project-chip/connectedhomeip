/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file declares test entry points for CHIP Internet (inet)
 *      layer library unit tests.
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int TestInetAddress(void);
int TestInetBuffer(void);
int TestInetErrorStr(void);
int TestInetTimer(void);
int TestInetEndPoint(void);
int TestInetLayerDNS(void);
#ifdef __cplusplus
}
#endif

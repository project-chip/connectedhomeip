/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#ifdef SIWX917_USE_COMISSIONABLE_DATA

uint32_t discriminatorValue = 3840;
uint64_t passcode           = 20202021;
uint32_t spake2Interation   = 1000;
char spake2Salt[]           = "U1BBS0UyUCBLZXkgU2FsdA==";
char spake2Verifier[]       = "uWFwqugDNGiEck/po7KHwwMwwqZgN10XuyBajPGuyzUEV/iree4lOrao5GuwnlQ65CJzbeUB49s31EH+NEkg0JVI5MGCQGMMT/"
                        "SRPFNRODm3wH/MBiehuFc6FJ/NH6Rmzw==";
char genSpake2Path[]   = "";
uint32_t productId     = 32773;
uint32_t vendorId      = 65521;
char productName[]     = "";
char vendorName[]      = "";
char hwVersionString[] = "";
uint32_t rotatingId;
uint32_t commissionableFlow = 1;
uint8_t rendezvousFlag      = 2;

#endif
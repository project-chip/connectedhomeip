/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file contains definitions of test certificates used by various unit tests.
 *
 */

#pragma once


#include <cstddef> // For size_t
#include <cstdint> // For uint8_t
#include <crypto/CHIPCryptoPAL.h>


// Valid NOC chain certificates for testing 
namespace chip {
namespace TestCerts {

// a root certificate generated with chip-tool
extern const size_t kTestRCAC_size;
extern const uint8_t kTestRCAC[];

// icac certificate genereated by chip-tool signed by kTestRCAC 
extern const size_t kTestICAC_size;
extern const uint8_t kTestICAC[];

// noc certificate genereated by chip-tool signed by kTestICAC 
extern const size_t kTestIcacNOC_size;
extern const uint8_t kTestIcacNOC[];

// noc certificate genereated by chip-tool signed by kTestRCAC

extern const size_t kTestNOIcacNOC_size;
extern const uint8_t kTestNOIcacNOC[];

extern const uint8_t kTestIPK[];

} // namespace TestCerts
} // namespace chip
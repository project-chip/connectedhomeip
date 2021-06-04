/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#define CHIP_CSG_TEST_HARNESS 1

#define CHARS_PER_BYTE (uint16_t)2
#include <map>
#include <string>
#include <stdlib.h>

const char * yaml_string_for_map(std::map<std::string, std::map<std::string, std::string>> *trace_map);
std::string stringForDataBuffer(const uint8_t *start, uint16_t data_length);

// Message Keys
extern std::string messageFromInitiator_key;
extern std::string messageFromResponder_key;

// Request parameter keys
extern std::string PBKDFParamRequest_key;

// Response parameter keys
extern std::string PBKDFParamResponse_key;
extern std::string PBKDFParamResponse_salt_length_key;
extern std::string PBKDFParamResponse_iter_count_key;

// PAKE Contribution (PAKE 1)
extern std::string PAKE_1_key;
extern std::string PAKE_1_Pa_key;
extern std::string PAKE_1_key_id_key;

// PAKE Contrib. Verification (PAKE 2)
extern std::string PAKE_2_key;
extern std::string PAKE_2_encryption_id_key;

// PAKE Verification (PAKE 3)
extern std::string PAKE_3_key;

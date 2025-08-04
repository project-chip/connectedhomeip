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

/**
 * @file - This file contains the Check-In message test vectors
 */

#pragma once

#include <protocols/secure_channel/CheckinMessage.h>
#include <stddef.h>
#include <stdint.h>

typedef struct CheckIn_Message_test_vector
{
    const uint8_t * key;
    const size_t key_len;
    const uint8_t * application_data;
    const size_t application_data_len;
    const chip::Protocols::SecureChannel::CounterType counter;
    const uint8_t * nonce;
    const size_t nonce_len;
    const uint8_t * ciphertext;
    const size_t ciphertext_len;
    const uint8_t * mic;
    const size_t mic_len;
    const uint8_t * payload;
    const size_t payload_len;
} CheckIn_Message_test_vector;

/**
 * vector 1
 */

const uint8_t kKey1[] = { 0xd9, 0x0e, 0x13, 0x18, 0x0d, 0x00, 0xba, 0xad, 0xd2, 0x0c, 0xf5, 0xed, 0x49, 0x13, 0xd3, 0xff };

const uint8_t kApplicationData1[] = {};

const uint8_t kNonce1[] = { 0x45, 0x80, 0xd2, 0xc6, 0xf1, 0x31, 0x0d, 0xc4, 0xeb, 0x64, 0xf1, 0xf8, 0xe8 };

const uint8_t kCiphertext1[] = { 0xbd, 0xc2, 0x1f, 0xb5 };

const uint8_t kMic1[] = { 0x19, 0x5d, 0x74, 0x7d, 0xd2, 0x87, 0x9b, 0x2b, 0x0d, 0x43, 0xce, 0x5b, 0x1c, 0x56, 0x50, 0x78 };

const uint8_t kPayload1[] = { 0x45, 0x80, 0xd2, 0xc6, 0xf1, 0x31, 0x0d, 0xc4, 0xeb, 0x64, 0xf1, 0xf8, 0xe8, 0xbd, 0xc2, 0x1f, 0xb5,
                              0x19, 0x5d, 0x74, 0x7d, 0xd2, 0x87, 0x9b, 0x2b, 0x0d, 0x43, 0xce, 0x5b, 0x1c, 0x56, 0x50, 0x78 };

const CheckIn_Message_test_vector vector1 = { .key                  = kKey1,
                                              .key_len              = sizeof(kKey1),
                                              .application_data     = kApplicationData1,
                                              .application_data_len = sizeof(kApplicationData1),
                                              .counter              = 12,
                                              .nonce                = kNonce1,
                                              .nonce_len            = sizeof(kNonce1),
                                              .ciphertext           = kCiphertext1,
                                              .ciphertext_len       = sizeof(kCiphertext1),
                                              .mic                  = kMic1,
                                              .mic_len              = sizeof(kMic1),
                                              .payload              = kPayload1,
                                              .payload_len          = sizeof(kPayload1) };

/**
 * vector 2
 */

const uint8_t kKey2[] = { 0x18, 0xfd, 0xbc, 0xea, 0xef, 0x01, 0x95, 0x5b, 0x0e, 0xc8, 0x75, 0xed, 0xa3, 0xae, 0x6e, 0xe8 };

const uint8_t kApplicationData2[] = { 0x54, 0x68, 0x69, 0x73 };

const uint8_t kNonce2[] = { 0x9b, 0x02, 0xed, 0x21, 0xee, 0x0c, 0x7b, 0x49, 0x19, 0x85, 0x50, 0x2e, 0x37 };

const uint8_t kCiphertext2[] = { 0x2d, 0xbd, 0x7b, 0x3f, 0x8b, 0x4f, 0x8e, 0x3c };

const uint8_t kMic2[] = { 0x5a, 0xd9, 0x94, 0x19, 0x38, 0x9f, 0x41, 0xa8, 0xd6, 0x09, 0x93, 0x8c, 0x67, 0xa8, 0x6d, 0x65 };

const uint8_t kPayload2[] = { 0x9b, 0x02, 0xed, 0x21, 0xee, 0x0c, 0x7b, 0x49, 0x19, 0x85, 0x50, 0x2e, 0x37,
                              0x2d, 0xbd, 0x7b, 0x3f, 0x8b, 0x4f, 0x8e, 0x3c, 0x5a, 0xd9, 0x94, 0x19, 0x38,
                              0x9f, 0x41, 0xa8, 0xd6, 0x09, 0x93, 0x8c, 0x67, 0xa8, 0x6d, 0x65 };

const CheckIn_Message_test_vector vector2 = { .key                  = kKey2,
                                              .key_len              = sizeof(kKey2),
                                              .application_data     = kApplicationData2,
                                              .application_data_len = sizeof(kApplicationData2),
                                              .counter              = 15,
                                              .nonce                = kNonce2,
                                              .nonce_len            = sizeof(kNonce2),
                                              .ciphertext           = kCiphertext2,
                                              .ciphertext_len       = sizeof(kCiphertext2),
                                              .mic                  = kMic2,
                                              .mic_len              = sizeof(kMic2),
                                              .payload              = kPayload2,
                                              .payload_len          = sizeof(kPayload2) };

/**
 * vector 3
 */

const uint8_t kKey3[] = { 0xd9, 0x0e, 0x13, 0x18, 0x0d, 0x00, 0xba, 0xad, 0xd2, 0x0c, 0xf5, 0xed, 0x49, 0x13, 0xd3, 0xff };

const uint8_t kApplicationData3[] = { 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61 };

const uint8_t kNonce3[] = { 0xaa, 0x84, 0xbc, 0x60, 0x88, 0x6a, 0x63, 0xa8, 0x47, 0x5d, 0x5d, 0xbe, 0xb5 };

const uint8_t kCiphertext3[] = { 0x6d, 0x63, 0x5f, 0xa9, 0x52, 0x85, 0xae, 0x33, 0x62, 0x66, 0x13, 0xc7, 0x63 };

const uint8_t kMic3[] = { 0x6c, 0xe3, 0xe3, 0xb2, 0xa8, 0xb1, 0x3a, 0x8c, 0x89, 0xbe, 0xf7, 0x68, 0x91, 0xe8, 0xe2, 0x96 };

const uint8_t kPayload3[] = { 0xaa, 0x84, 0xbc, 0x60, 0x88, 0x6a, 0x63, 0xa8, 0x47, 0x5d, 0x5d, 0xbe, 0xb5, 0x6d,
                              0x63, 0x5f, 0xa9, 0x52, 0x85, 0xae, 0x33, 0x62, 0x66, 0x13, 0xc7, 0x63, 0x6c, 0xe3,
                              0xe3, 0xb2, 0xa8, 0xb1, 0x3a, 0x8c, 0x89, 0xbe, 0xf7, 0x68, 0x91, 0xe8, 0xe2, 0x96 };

const CheckIn_Message_test_vector vector3 = { .key                  = kKey3,
                                              .key_len              = sizeof(kKey3),
                                              .application_data     = kApplicationData3,
                                              .application_data_len = sizeof(kApplicationData3),
                                              .counter              = 11,
                                              .nonce                = kNonce3,
                                              .nonce_len            = sizeof(kNonce3),
                                              .ciphertext           = kCiphertext3,
                                              .ciphertext_len       = sizeof(kCiphertext3),
                                              .mic                  = kMic3,
                                              .mic_len              = sizeof(kMic3),
                                              .payload              = kPayload3,
                                              .payload_len          = sizeof(kPayload3) };

/**
 * vector 4
 */

const uint8_t kKey4[] = { 0xca, 0x67, 0xd4, 0x1f, 0xf7, 0x11, 0x29, 0x10, 0xfd, 0xd1, 0x8a, 0x1b, 0xf9, 0x9e, 0xa9, 0x74 };

const uint8_t kApplicationData4[] = {
    0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x6c, 0x6f, 0x6e, 0x67, 0x65, 0x72
};

const uint8_t kNonce4[] = { 0x7a, 0x97, 0x72, 0x24, 0x3c, 0x97, 0xc8, 0x7d, 0x5f, 0x3a, 0x31, 0xc4, 0xe6 };

const uint8_t kCiphertext4[] = { 0xdb, 0xbc, 0x1a, 0xa5, 0x66, 0xc4, 0x43, 0xc2, 0x05, 0x86,
                                 0x06, 0x6b, 0x42, 0x7b, 0xfc, 0xaa, 0xad, 0x78, 0xda, 0x4a };

const uint8_t kMic4[] = { 0x10, 0x5a, 0x13, 0x42, 0xad, 0xbf, 0x3f, 0x47, 0x98, 0xcd, 0x81, 0xb9, 0xef, 0x97, 0xbb, 0xb7 };

const uint8_t kPayload4[] = { 0x7a, 0x97, 0x72, 0x24, 0x3c, 0x97, 0xc8, 0x7d, 0x5f, 0x3a, 0x31, 0xc4, 0xe6, 0xdb, 0xbc, 0x1a, 0xa5,
                              0x66, 0xc4, 0x43, 0xc2, 0x05, 0x86, 0x06, 0x6b, 0x42, 0x7b, 0xfc, 0xaa, 0xad, 0x78, 0xda, 0x4a, 0x10,
                              0x5a, 0x13, 0x42, 0xad, 0xbf, 0x3f, 0x47, 0x98, 0xcd, 0x81, 0xb9, 0xef, 0x97, 0xbb, 0xb7 };

const CheckIn_Message_test_vector vector4 = { .key                  = kKey4,
                                              .key_len              = sizeof(kKey4),
                                              .application_data     = kApplicationData4,
                                              .application_data_len = sizeof(kApplicationData4),
                                              .counter              = 11,
                                              .nonce                = kNonce4,
                                              .nonce_len            = sizeof(kNonce4),
                                              .ciphertext           = kCiphertext4,
                                              .ciphertext_len       = sizeof(kCiphertext4),
                                              .mic                  = kMic4,
                                              .mic_len              = sizeof(kMic4),
                                              .payload              = kPayload4,
                                              .payload_len          = sizeof(kPayload4) };

/**
 * vector 5
 */

const uint8_t kKey5[] = { 0xca, 0x67, 0xd4, 0x1f, 0xf7, 0x11, 0x29, 0x10, 0xfd, 0xd1, 0x8a, 0x1b, 0xf9, 0x9e, 0xa9, 0x74 };

const uint8_t kApplicationData5[] = { 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x6c, 0x6f,
                                      0x6e, 0x67, 0x65, 0x72, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67 };

const uint8_t kNonce5[] = { 0x06, 0x34, 0x67, 0x6e, 0xa6, 0xe0, 0x70, 0x7b, 0x7a, 0xd7, 0x81, 0x4f, 0xf8 };

const uint8_t kCiphertext5[] = { 0x2e, 0x5b, 0x18, 0xd1, 0x9a, 0x23, 0xb2, 0xe4, 0xfa, 0xdf, 0x82, 0x92, 0x53, 0x51,
                                 0x7f, 0xf3, 0xc9, 0x1d, 0x8d, 0x47, 0x84, 0x31, 0x5a, 0x1e, 0x32, 0x08, 0xb8 };

const uint8_t kMic5[] = { 0xec, 0xf6, 0x11, 0x8b, 0x02, 0x1a, 0x5a, 0x4c, 0xd4, 0xe9, 0xd4, 0x13, 0x8d, 0xff, 0x29, 0x71 };

const uint8_t kPayload5[] = { 0x06, 0x34, 0x67, 0x6e, 0xa6, 0xe0, 0x70, 0x7b, 0x7a, 0xd7, 0x81, 0x4f, 0xf8, 0x2e,
                              0x5b, 0x18, 0xd1, 0x9a, 0x23, 0xb2, 0xe4, 0xfa, 0xdf, 0x82, 0x92, 0x53, 0x51, 0x7f,
                              0xf3, 0xc9, 0x1d, 0x8d, 0x47, 0x84, 0x31, 0x5a, 0x1e, 0x32, 0x08, 0xb8, 0xec, 0xf6,
                              0x11, 0x8b, 0x02, 0x1a, 0x5a, 0x4c, 0xd4, 0xe9, 0xd4, 0x13, 0x8d, 0xff, 0x29, 0x71 };

const CheckIn_Message_test_vector vector5 = { .key                  = kKey5,
                                              .key_len              = sizeof(kKey5),
                                              .application_data     = kApplicationData5,
                                              .application_data_len = sizeof(kApplicationData5),
                                              .counter              = 12,
                                              .nonce                = kNonce5,
                                              .nonce_len            = sizeof(kNonce5),
                                              .ciphertext           = kCiphertext5,
                                              .ciphertext_len       = sizeof(kCiphertext5),
                                              .mic                  = kMic5,
                                              .mic_len              = sizeof(kMic5),
                                              .payload              = kPayload5,
                                              .payload_len          = sizeof(kPayload5) };

const CheckIn_Message_test_vector checkIn_message_test_vectors[]{ vector1, vector2, vector3, vector4, vector5 };

/**
 * Invalid Counter / Nonce Match vector
 */

const uint8_t kInvalidNonceKey[] = {
    0xca, 0x67, 0xd4, 0x1f, 0xf7, 0x11, 0x29, 0x10, 0xfd, 0xd1, 0x8a, 0x1b, 0xf9, 0x9e, 0xa9, 0x74
};

const uint8_t kInvalidNonceApplicationData[] = { 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20,
                                                 0x6c, 0x6f, 0x6e, 0x67, 0x65, 0x72, 0x20, 0x6c, 0x6f, 0x6e,
                                                 0x67, 0x65, 0x72, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67 };

const uint8_t kInvalidNonceNonce[] = { 0x06, 0x34, 0x67, 0x6e, 0xa6, 0xe0, 0x70, 0x7b, 0x7a, 0xd7, 0x81, 0x4f, 0xf8 };

const uint8_t kInvalidNonceCiphertext[] = { 0x29, 0x5b, 0x18, 0xd1, 0x9a, 0x23, 0xb2, 0xe4, 0xfa, 0xdf, 0x82, 0x92,
                                            0x53, 0x51, 0x7f, 0xf3, 0xc9, 0x1d, 0x8d, 0x47, 0x84, 0x2e, 0x41, 0x02,
                                            0x3c, 0x03, 0xad, 0x66, 0xac, 0x4d, 0xca, 0x72, 0x47, 0xe0 };

const uint8_t kInvalidNonceMic[] = {
    0xe4, 0xc6, 0x6b, 0xd9, 0xd3, 0x99, 0x13, 0xe2, 0x3d, 0x82, 0x32, 0xb9, 0x61, 0xfa, 0x92, 0x26
};

const uint8_t kInvalidNoncePayload[] = { 0x06, 0x34, 0x67, 0x6e, 0xa6, 0xe0, 0x70, 0x7b, 0x7a, 0xd7, 0x81, 0x4f, 0xf8,
                                         0x29, 0x5b, 0x18, 0xd1, 0x9a, 0x23, 0xb2, 0xe4, 0xfa, 0xdf, 0x82, 0x92, 0x53,
                                         0x51, 0x7f, 0xf3, 0xc9, 0x1d, 0x8d, 0x47, 0x84, 0x2e, 0x41, 0x02, 0x3c, 0x03,
                                         0xad, 0x66, 0xac, 0x4d, 0xca, 0x72, 0x47, 0xe0, 0xe4, 0xc6, 0x6b, 0xd9, 0xd3,
                                         0x99, 0x13, 0xe2, 0x3d, 0x82, 0x32, 0xb9, 0x61, 0xfa, 0x92, 0x26 };

const CheckIn_Message_test_vector invalidNonceVector = { .key                  = kInvalidNonceKey,
                                                         .key_len              = sizeof(kInvalidNonceKey),
                                                         .application_data     = kInvalidNonceApplicationData,
                                                         .application_data_len = sizeof(kInvalidNonceApplicationData),
                                                         .counter              = 12,
                                                         .nonce                = kInvalidNonceNonce,
                                                         .nonce_len            = sizeof(kInvalidNonceNonce),
                                                         .ciphertext           = kInvalidNonceCiphertext,
                                                         .ciphertext_len       = sizeof(kInvalidNonceCiphertext),
                                                         .mic                  = kInvalidNonceMic,
                                                         .mic_len              = sizeof(kInvalidNonceMic),
                                                         .payload              = kInvalidNoncePayload,
                                                         .payload_len          = sizeof(kInvalidNoncePayload) };

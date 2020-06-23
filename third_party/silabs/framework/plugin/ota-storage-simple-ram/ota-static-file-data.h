/**
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
 *
 *    Copyright (c) 2020 Silicon Labs
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
// Auto-generated C header created by ota-static-file.pl
// Input file: app/framework/plugin/ota-storage-simple-ram/ota-static-sample.ota

//  image-builder (C) 2010 by Ember Corporation.
//  Version: 1.0
//  ECC signature support present.
//
//
//  File: app/framework/plugin/ota-storage-simple-ram/ota-static-sample.ota
//  Magic Number:         0x0BEEF11E
//  Header Version:       0x0100
//  Header Length:        56 bytes
//  Field Control:        0x0000
//  Manufacturer ID:      0x1002
//  Image Type:           0x5678
//  Firmware Version:     0x00000005
//  Stack Version:        0x0002
//  Header String:        The latest and greatest upgrade.
//  Total Image Size:     182 bytes
//  Total Tags:           3
//    ID:                 0xF000   (Manufacturer Specific)
//    Length:             10 bytes
//
//    ID:                 0x0002   (ECDSA Signing Certificate)
//    Length:             48 bytes
//      Subject: (>)000D6F0000198B36
//      Issuer:  (>)5445535453454341 (Certicom TEST CA)
//
//    ID:                 0x0001   (ECDSA Signature)
//    Length:             50 bytes
//      Signer:  (>)000D6F0000198B36
//       Data:      01C2C31CB8C40064  EABB3189AD8969EA
//                  C25893183A023BD2  8D5FB2134D3E07B9
//                  2E06A258E478D20C  7ACC
//
//
//  Using Certicom TEST CA issued certificate.
//
//  Message Digest: E26A886E8030458DA084586F2DCB08A2
//
//  Signature is valid
//
//
//  Found 1 files.

#define STATIC_IMAGE_DATA {                         \
    0x1e, 0xf1, 0xee, 0x0b, 0x00, 0x01, 0x38, 0x00, \
    0x00, 0x00, 0x02, 0x10, 0x78, 0x56, 0x05, 0x00, \
    0x00, 0x00, 0x02, 0x00, 0x54, 0x68, 0x65, 0x20, \
    0x6c, 0x61, 0x74, 0x65, 0x73, 0x74, 0x20, 0x61, \
    0x6e, 0x64, 0x20, 0x67, 0x72, 0x65, 0x61, 0x74, \
    0x65, 0x73, 0x74, 0x20, 0x75, 0x70, 0x67, 0x72, \
    0x61, 0x64, 0x65, 0x2e, 0xb6, 0x00, 0x00, 0x00, \
    0x00, 0xf0, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x01, \
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, \
    0x02, 0x00, 0x30, 0x00, 0x00, 0x00, 0x03, 0x07, \
    0x79, 0x29, 0x47, 0xb3, 0x85, 0x0a, 0x95, 0x85, \
    0xbf, 0x8e, 0x25, 0xc1, 0x9d, 0x8e, 0x86, 0x78, \
    0x43, 0x4f, 0x58, 0x36, 0x00, 0x0d, 0x6f, 0x00, \
    0x00, 0x19, 0x8b, 0x36, 0x54, 0x45, 0x53, 0x54, \
    0x53, 0x45, 0x43, 0x41, 0x01, 0x09, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, \
    0x32, 0x00, 0x00, 0x00, 0x36, 0x8b, 0x19, 0x00, \
    0x00, 0x6f, 0x0d, 0x00, 0x01, 0xc2, 0xc3, 0x1c, \
    0xb8, 0xc4, 0x00, 0x64, 0xea, 0xbb, 0x31, 0x89, \
    0xad, 0x89, 0x69, 0xea, 0xc2, 0x58, 0x93, 0x18, \
    0x3a, 0x02, 0x3b, 0xd2, 0x8d, 0x5f, 0xb2, 0x13, \
    0x4d, 0x3e, 0x07, 0xb9, 0x2e, 0x06, 0xa2, 0x58, \
    0xe4, 0x78, 0xd2, 0x0c, 0x7a, 0xcc,             \
}

#define STATIC_IMAGE_DATA_SIZE 182L

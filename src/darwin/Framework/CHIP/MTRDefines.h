/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#define MTR_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
#define MTR_EXTERN extern "C" MTR_EXPORT
#else
#define MTR_EXTERN extern MTR_EXPORT
#endif

typedef NSData * MTRTLVBytes;
typedef NSData * MTRCSRDERBytes;
typedef NSData * MTRCertificateDERBytes;
typedef NSData * MTRCertificateTLVBytes;

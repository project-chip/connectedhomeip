/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#import <CHIP/CHIPDeviceController_Internal.h>
#import <CHIP/CHIPError_Internal.h>

CHIP_ERROR AddGroupTestData(CHIPDeviceController * controller)
{
    constexpr unsigned char epochKey1[]
        = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf };
    constexpr unsigned char epochKey2[]
        = { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf };
    constexpr unsigned char epochKey3[]
        = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf };
    NSDictionary * groupKeySet1 = @{
        @"groupId" : @0x101,
        @"groupName" : @"Group #1",
        @"endpointId" : @1,
        @"keySetId" : @0x01a1,
        @"keySetData" : @[
            @{@"epochStartTime" : @1110000,
                @"epochKey" : [NSData dataWithBytes:epochKey1 length:sizeof(epochKey1)]},
            @{@"epochStartTime" : @1110001,
                @"epochKey" : [NSData dataWithBytes:epochKey2 length:sizeof(epochKey2)]},
            @{@"epochStartTime" : @1110002,
                @"epochKey" : [NSData dataWithBytes:epochKey3 length:sizeof(epochKey3)]}
        ]
    };

    constexpr unsigned char epochKey4[]
        = { 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf };
    constexpr unsigned char epochKey5[]
        = { 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef };
    constexpr unsigned char epochKey6[]
        = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
    NSDictionary * groupKeySet2 = @{
        @"groupId" : @0x102,
        @"groupName" : @"Group #2",
        @"endpointId" : @0,
        @"keySetId" : @0x01a2,
        @"keySetData" : @[
            @{@"epochStartTime" : @2220000,
                @"epochKey" : [NSData dataWithBytes:epochKey4 length:sizeof(epochKey4)]},
            @{@"epochStartTime" : @2220001,
                @"epochKey" : [NSData dataWithBytes:epochKey5 length:sizeof(epochKey5)]},
            @{@"epochStartTime" : @2220002,
                @"epochKey" : [NSData dataWithBytes:epochKey6 length:sizeof(epochKey6)]}
        ]
    };

    NSError * error;
    if (![controller addGroupKeySet:groupKeySet1 error:&error]) {
        return [CHIPError errorToCHIPErrorCode:error];
    }

    if (![controller addGroupKeySet:groupKeySet2 error:&error]) {
        return [CHIPError errorToCHIPErrorCode:error];
    }

    return CHIP_NO_ERROR;
}

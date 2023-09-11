/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#import "UUIDHelper.h"

@implementation UUIDHelper

+ (CBUUID *)GetShortestServiceUUID:(const chip::Ble::ChipBleUUID *)svcId
{
    // shorten the 16-byte UUID reported by BLE Layer to shortest possible, 2 or 4 bytes
    // this is the BLE Service UUID Base. If a 16-byte service UUID partially matches with these 12 bytes,
    // it can be shortened to 2 or 4 bytes.
    static const uint8_t bleBaseUUID[12] = { 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
    if (0 == memcmp(svcId->bytes + 4, bleBaseUUID, sizeof(bleBaseUUID))) {
        // okay, let's try to shorten it
        if ((0 == svcId->bytes[0]) && (0 == svcId->bytes[1])) {
            // the highest 2 bytes are both 0, so we just need 2 bytes
            return [CBUUID UUIDWithData:[NSData dataWithBytes:(svcId->bytes + 2) length:2]];
        } // we need to use 4 bytes
        return [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:4]];
    }
    // it cannot be shortened as it doesn't match with the BLE Service UUID Base
    return [CBUUID UUIDWithData:[NSData dataWithBytes:svcId->bytes length:16]];
}
@end
